#include "LCD_meny.h"
#include "Routine.h"
#include <delay.h>  // Included here because we need delay_ms
#include <stdio.h>

// this file will have all routine for manke meny and LCD info
 
/*  Setup Clock
use key up/down to change value
righ / left to move 
ok to exit 
*/

// Global clock variables

// ---- LOG HISTORY (RAM, push buffer, newest at index 0) ----

unsigned int  HistOn[HIST_SIZE];
unsigned int  HistOff[HIST_SIZE];
unsigned int  HistSave[HIST_SIZE];
unsigned int  HistDay[HIST_SIZE];
unsigned char HistHour[HIST_SIZE];
unsigned char HistMin[HIST_SIZE];
unsigned char HistSvPct[HIST_SIZE];
unsigned char HistCount = 0;        // how many valid records (0..50)
unsigned int CycleOnMin;    // minutes heating this cycle
unsigned int CycleOffMin;   // minutes off/blocked this cycle

// Internal cursor position tracker (0..3)
static unsigned char edit_index = 0; 

// LCD X-coordinate mapping for "TIME: HH:MM    "
// Position 0 = Tens Hours (X=6), 1 = Ones Hours (X=7), 2 = Tens Mins (X=9), 3 = Ones Mins (X=10)
const unsigned char Time_cursor_x_map[4] = {6, 7, 9, 10};   


// -------------------------------------------------------------
// Helper: Draw time and place hardware LCD cursor
// -------------------------------------------------------------
static void refresh_clock_ui(void)
{
    char buf[17];
    unsigned char i;

    // %02d ensures 2 digits with leading zero (e.g. "09")
    // Trailing spaces erase any old characters on the line
    sprintf(buf, "TIME: %02d:%02d    ", hours, minutes);
    
    lcd_gotoxy(0, 0);
    for ( i = 0; buf[i] != '\0'; i++) {
        lcd_char(buf[i]);
    }

    // Place the hardware cursor under the selected digit
    lcd_gotoxy(Time_cursor_x_map[edit_index], 0);
}


// -------------------------------------------------------------
// Main Entry Point: Jump to this function when user selects "SET CLOCK"
// -------------------------------------------------------------
void run_clock_setting_menu(void)
{
    unsigned char key = KEY_NONE;
    unsigned char editing = 1;
    unsigned char ok_cnt = 0;      // OK hold counter
 
    // 1. Prepare display
    lcd_cmd(0x01);          // Clear LCD
    delay_ms(2);
    LoadCustomLCDChars ();    
    lcd_gotoxy(0, 1);
//    lcd_print("UP/DN:Chg OK:Save"); // User instruction on line 2   
    lcd_print(" ");
    lcd_char(0);   // Prints Up Arrow (?)
    lcd_print(" ");
    lcd_char(1);   // Prints Down Arrow (?)
    lcd_print(" ");
    lcd_char(2);   // Prints Right Arrow (?)
    lcd_print(" ");
    lcd_char(3);   // Prints Left Arrow (?)    
    lcd_print(" ");
    lcd_print("[OK]");
    edit_index = 0;         // Start at Tens Hours digit
    
    // 2. Turn ON ST7032i Underline + Blinking Cursor (Command 0x0F)
    lcd_cmd(0x0F); 

    // Initial draw
    refresh_clock_ui();

    // 3. Event Loop: Stay here until user presses OK
    while (editing)
    {
        key = ScanKeyBoard(); // Your function that reads hardware buttons
        if (key == KEY_OK)
        {
            ok_cnt++;                     // still held -> count up
            if (ok_cnt >= OK_EXIT_COUNT)  // held long enough -> exit
            {
                editing = 0;
            }
        }
        else
        {
            ok_cnt = 0;    // released (or another key) -> restart count
        }
        if (key != KEY_OK && key != KEY_NONE)
        {
            switch (key)
            {
                // --- MOVE CURSOR RIGHT ---
                case KEY_RIGHT:
                    edit_index++;
                    if (edit_index > 3) edit_index = 0;
                    break;

                // --- MOVE CURSOR LEFT ---
                case KEY_LEFT:
                    if (edit_index == 0) edit_index = 3;
                    else edit_index--;
                    break;

                // --- INCREMENT DIGIT ---
                case KEY_UP:
                    if (edit_index == 0) // Tens Hours
                    {
                        hours += 10;
                        if (hours > 23) hours -= 20;
                    }
                    else if (edit_index == 1) // Ones Hours
                    {
                        unsigned char ones = hours % 10;
                        if (ones == 9 || hours == 23) hours -= ones;
                        else hours++;
                    }
                    else if (edit_index == 2) // Tens Minutes
                    {
                        minutes += 10;
                        if (minutes >= 60) minutes -= 60;
                    }
                    else if (edit_index == 3) // Ones Minutes
                    {
                        unsigned char ones = minutes % 10;
                        if (ones == 9) minutes -= 9;
                        else minutes++;
                    }
                    break;

                // --- DECREMENT DIGIT ---
                case KEY_DOWN:
                    if (edit_index == 0) // Tens Hours
                    {
                        if (hours < 10) hours += 20;
                        else hours -= 10;
                    }
                    else if (edit_index == 1) // Ones Hours
                    {
                        unsigned char ones = hours % 10;
                        unsigned char tens = hours / 10;

                        if (tens == 2 && ones == 3) 
                            hours -= 3;       // 23 rolls over to 20
                        else if (tens < 2 && ones == 9) 
                            hours -= 9;   // 19 rolls over to 10, 09 to 00
                        else 
                            hours++;
                    }
                    else if (edit_index == 2) // Tens Minutes
                    {
                        if (minutes < 10) minutes += 50;
                        else minutes -= 10;
                    }
                    else if (edit_index == 3) // Ones Minutes
                    {
                        unsigned char ones = minutes % 10;
                        if (ones == 0) minutes += 9;
                        else minutes--;
                    }
                    break;

                // --- SAVE & EXIT ---
                case KEY_OK:
                    editing = 0; // Exit loop
                    break;
            }
            refresh_clock_ui();
        }
        // Update display and move cursor to new position
//        if (editing)
//        {
//            delay_ms(10);                  // 10ms loop tick -> OK_EXIT_COUNT x 10ms
//            idle_cnt++;
//            if (idle_cnt >= 3000)          // optional: 30 s timeout -> auto-exit
//            {
//                editing = 0;
//            }
//        }
    }

    // 4. Clean up before returning to main menu
    lcd_cmd(0x0C); // Turn OFF cursor (Normal display mode)  
    lcd_cmd(0x01); // Clear display and refresh main screen
    delay_ms(2);
}

// -------------------------------------------------------------
// Main Entry Point: Jump to this function when user selects "SET Date"
// -------------------------------------------------------------
void SetupDateSimple(void) 
{
    // Local workspace copies
    unsigned int  temp_year  = current_year;
    unsigned char temp_month = month;
    unsigned char temp_day   = day_of_month;
    unsigned char y2;                      // FIX: last 2 digits of year
    unsigned char edit_index = 0; // 0 = Year, 1 = Month, 2 = Day
    unsigned char editing    = 1;
    unsigned char key        = KEY_NONE; 
    unsigned char temp;
    unsigned char max_days;
    unsigned char lcd_line_buffer[17]; 
    unsigned char NewKey=99;
    
    LoadCustomLCDChars ();    
//    lcd_print("UP/DN:Chg OK:Save"); // User instruction on line 2   

    lcd_cmd(0x0E); // 0x0F = Display ON, Cursor ON, Blink ON      
    
    while (editing) 
    {        
        // 1. DYNAMICALLY FIND MAX DAYS FOR CURRENT SELECTION
        max_days = month_lengths[temp_month];
        if (temp_month == 2) 
        {   // Calculate leap year status directly from the selected temp_year
            if (((temp_year % 4 == 0) && (temp_year % 100 != 0)) || (temp_year % 400 == 0)) 
            {
                max_days = 29;
            }  
        }

        // 2. RE-RENDER DATA TO 16x2 DISPLAY
        // Format layout precisely: "   2026-12-03   " 
        //lcd_command(0x0F); // 0x0F = Display ON, Cursor ON, Blink ON  
        if ( NewKey !=0)
        {   NewKey=0;
            sprintf(lcd_line_buffer, "   %04u-%02u-%02u   ", temp_year, (unsigned int)temp_month, (unsigned int)temp_day);
            lcd_gotoxy(0, 0);
            lcd_print(lcd_line_buffer);
            lcd_gotoxy(0, 1);  
            lcd_print(" ");
            lcd_char(0);   // Prints Up Arrow (?)
            lcd_print(" ");
            lcd_char(1);   // Prints Down Arrow (?)
            lcd_print(" ");
            lcd_char(2);   // Prints Right Arrow (?)
            lcd_print(" ");
            lcd_char(3);   // Prints Left Arrow (?)    
            lcd_print(" ");
            lcd_print("[OK]");
            //lcd_cmd(0x0E); // 0x0F = Display ON, Cursor ON, Blink ON  
        }

        // 3. SET CURSOR POSITION UNDER THE SELECTED NUMBER VALUE
        if (edit_index == 0)      lcd_gotoxy(6, 0);  // Under Year (e.g. 202[6])
        else if (edit_index == 1) lcd_gotoxy(9, 0);  // Under Month (e.g. [1]2)
        else                      lcd_gotoxy(12, 0); // Under Day (e.g. [0]3)

        // 4. CAPTURE Keypad KEYSTROKE
        key = ScanKeyBoard();
        if (key == KEY_NONE) 
        {
            continue; 
        }
        else
        {   temp = key;
            while (temp)    // wait release
            {   temp= ScanKeyBoard();
                // use wdr if issue
            }
        }

        // 5. MATH LOGIC PROCESSING BLOCK
        switch (key) 
        {
            case KEY_RIGHT:
                NewKey++;
                edit_index++;
                if (edit_index > 2) edit_index = 0; // Rotates cursor back to Year selection
                break;
            case KEY_LEFT:
                NewKey++;
                if (edit_index == 0) edit_index = 2;
                else edit_index--;
                break;

            case KEY_UP:
                if (edit_index == 0)
                {
                    // FIX: edit only last 2 digits, base stays 2000
                    y2 = temp_year % 100;
                    y2++;
                    if (y2 > 99) y2 = 0;      // 2099 -> 2000
                    temp_year = 2000 + y2;
                }
                else if (edit_index == 1)
                {
                    temp_month++;
                    if (temp_month > 12) temp_month = 1;
                }
                else
                {
                    temp_day++;
                    if (temp_day > max_days) temp_day = 1;
                }
                NewKey++;
            break;

            case KEY_DOWN:
                if (edit_index == 0)
                {
                    // FIX: edit only last 2 digits
                    y2 = temp_year % 100;
                    if (y2 == 0) y2 = 99;
                    else y2--;
                    temp_year = 2000 + y2;
                }
                else if (edit_index == 1)
                {
                    if (temp_month <= 1) temp_month = 12;
                    else temp_month--;
                }
                else
                {
                    temp_day--;
                    if (temp_day < 1) temp_day = max_days;
                }
                NewKey++;
            break;

            case KEY_OK:
                #asm("cli")   // guard 16-bit writes if clock ISR runs
                current_year = temp_year;
                month        = temp_month;
                day_of_month = temp_day;
                //days = date_to_days(temp_year, temp_month, temp_day);  // resync day counter
                #asm("sei")
                SavedYear  = temp_year;
                SavedMonth = temp_month;
                SavedDay   = temp_day;
                editing = 0;
                lcd_cmd(0x0C);
                lcd_cmd(0x01);
                delay_ms(2);
            break;
        }

        // 6. SANITY VALUE COMPLIANCE CHECK
        // If user rolls from Jan 31 down to Feb, automatically push day down to safe limits
        if (temp_day > max_days)
        {
            temp_day = max_days;
            NewKey = 1;   // force redraw with the clamped value
        }
    }
}

// -------------------------------------------------------------
// Main Entry Point: Jump to this function when user selects "SET Backlight"
// use eeprom see routine for variable declaration,
// -------------------------------------------------------------
flash unsigned char bl_log_table[11] =
{
      0,   // level 0  = OFF
      8,   // level 1  - immediately visible
     15,   // level 2
     22,   // level 3
     30,   // level 4
     38,   // level 5
     46,   // level 6
     55,   // level 7
     65,   // level 8
     80,   // level 9
    100    // level 10 = full
};
void SetupBacklight (void)
{
     // Log curve: 16 steps, perceptually linear to the eye.
    // 0 = off, then roughly x1.35 per step up to 100.
 
    unsigned char lcd_line_buffer[17];
    unsigned char temp_idx;         // working index 0..15 (RAM)
    unsigned char editing = 1;
    unsigned char ok_cnt  = 0;
    unsigned char key     = KEY_NONE;
    unsigned char redraw  = 1;      // force first draw
    unsigned char i;

    // Entry: find nearest index for the current BackLight duty
    temp_idx = 10;
    if (BackLight < 100)
    {
        for (i = 10; i > 0; i--)
        {
            if (BackLight <= bl_log_table[i])
            {
                temp_idx = i;
            }
        }
    }
    if (BackLight == 0) temp_idx = 0;    // saved 0 -> level 0 (OFF)   
    set_backlight_pwm(BackLight);
    // Screen prepare
    lcd_cmd(0x01);                  // clear BOTH lines first
    delay_ms(2);
    
    LoadCustomLCDChars ();    

    while (editing)
    {
        // 1. DRAW SCREEN
        if (redraw)
        {
            redraw = 0;
//            lcd_cmd(0x01);                     // clear
//            delay_ms(2);
            lcd_gotoxy(0, 0);
            sprintf(lcd_line_buffer, " Light lvl %2u/10", (unsigned int)temp_idx);
            lcd_print(lcd_line_buffer);   
            lcd_gotoxy(0, 1);
            lcd_print(" ");
            lcd_char(0);   // Prints Up Arrow (?)
            lcd_print(" ");
            lcd_char(1);   // Prints Down Arrow (?)
            lcd_print(" ");
            lcd_char(2);   // Prints Right Arrow (?)
            lcd_print(" ");
            lcd_char(3);   // Prints Left Arrow (?)    
            lcd_print(" ");
            lcd_print("[OK]");
            
        }

        // 2. READ KEY
        key = ScanKeyBoard();
        if (key == KEY_NONE) continue;

        // 3. HANDLE KEY
        if (key == KEY_UP)
        {
            if (temp_idx < 10) temp_idx++;
            redraw = 1;
        }
        else if (key == KEY_DOWN)
        {
            if (temp_idx > 0) temp_idx--;
            redraw = 1;
        }
        else if (key == KEY_OK)
        {
            ok_cnt++;                            // hold OK to exit
            if (ok_cnt >= OK_EXIT_COUNT)
            {
                BackLight      = bl_log_table[temp_idx];   // save duty
                SavedBacklight = BackLight;               // save to EEPROM
                set_backlight_pwm(BackLight);
                editing = 0;
            }
        }

        if (key != KEY_OK) ok_cnt = 0;

        // 4. LIVE PREVIEW — PWM follows the log table while editing
        set_backlight_pwm(bl_log_table[temp_idx]);

//        // 5. WAIT KEY RELEASE
//        while (ScanKeyBoard() != KEY_NONE)
//        {
//            // use wdr if issue
//        }
    }

    // 6. CLEAN UP
    lcd_cmd(0x01);
    delay_ms(2);
}
// -------------------------------------------------------------
// Main Entry Point: Jump to this function when user selects "SET contrast"
// -------------------------------------------------------------
void SetupContrast(void)
{   unsigned char lcd_line_buffer[17];
    unsigned char temp_val;         // working copy (RAM)
    unsigned char editing  = 1;
    unsigned char ok_cnt   = 0;
    unsigned char key      = KEY_NONE;
    unsigned char redraw   = 1;     // force first draw

    // Entry: start from current value, clamped to allowed range
    temp_val = LCD_Contrast;
    if (temp_val > CONTRAST_SETUP_MAX) temp_val = CONTRAST_SETUP_MAX;

    // ===== Screen prepare =====
    LoadCustomLCDChars();           // reload arrows into CGRAM
    while (editing)
    {
        // 1. DRAW VALUE ON LINE 1
        if (redraw)
        {
            redraw = 0;
            lcd_gotoxy(0, 0);
            sprintf(lcd_line_buffer, " Contrast  %2u/63", (unsigned int)temp_val);
            lcd_print(lcd_line_buffer); 
            lcd_gotoxy(0, 1);
            lcd_print(" ");
            lcd_char(0);   // Prints Up Arrow (?)
            lcd_print(" ");
            lcd_char(1);   // Prints Down Arrow (?)
            lcd_print(" ");
            lcd_char(2);   // Prints Right Arrow (?)
            lcd_print(" ");
            lcd_char(3);   // Prints Left Arrow (?)    
            lcd_print(" ");
            lcd_print("[OK]");
        }

        // 2. READ KEY
        key = ScanKeyBoard();
        if (key == KEY_NONE) continue;

        // 3. OK = hold to save & exit (dedicated 10ms tick, time-based)
        if (key == KEY_OK)
        {
            ok_cnt = 0;
            while (ScanKeyBoard() == KEY_OK)   // still held?
            {
                delay_ms(10);
                ok_cnt++;
                if (ok_cnt >= OK_EXIT_COUNT)  // OK_EXIT_COUNT x 10ms
                {
                    LCD_Contrast  = temp_val;           // update RAM
                    SavedContrast = LCD_Contrast;       // save to EEPROM
                    SetLCD_Contrast(LCD_Contrast);      // apply
                    editing = 0;
                    break;
                }
            }
            continue;   // quick tap -> nothing, back to editing
        }

        // 4. UP/DOWN = change value
        if (key == KEY_UP)
        {
            if ((temp_val + CONTRAST_SETUP_STEP) <= CONTRAST_SETUP_MAX)
                temp_val = temp_val + CONTRAST_SETUP_STEP;
            redraw = 1;
        }
        else if (key == KEY_DOWN)
        {
            if (temp_val >= CONTRAST_SETUP_STEP)
                temp_val = temp_val - CONTRAST_SETUP_STEP;
            redraw = 1;
        }

        // 5. LIVE PREVIEW
        SetLCD_Contrast(temp_val);   // user sees the change instantly

        // 6. WAIT KEY RELEASE (UP/DOWN only — act once per press)
        while (ScanKeyBoard() != KEY_NONE)
        {
            // use wdr if issue
        }
    }

    // 7. CLEAN UP
    lcd_cmd(0x01);
    delay_ms(2);
}
// -------------------------------------------------------------
// Main Entry Point: Jump to this function when user selects "SET timer off"
// to turn off LCD  & backligh 
// -------------------------------------------------------------
void SetupTimeOff(void)
{   unsigned char lcd_line_buffer[17];
    unsigned char temp_val;         // working copy (RAM)
    unsigned char editing  = 1;
    unsigned char ok_cnt   = 0;
    unsigned char key      = KEY_NONE;
    unsigned char redraw   = 1;     // force first draw

    // Entry: current value, clamped to allowed range
    temp_val = TimeOff;
    if (temp_val > TIMEOFF_MAX) temp_val = TIMEOFF_MAX;

    // ===== Screen prepare =====
    lcd_cmd(0x01);                  // clear both lines
    delay_ms(2);
    LoadCustomLCDChars();           // reload arrows into CGRAM

    while (editing)
    {
        // 1. DRAW VALUE ON LINE 1
        if (redraw)
        {
            redraw = 0;
            lcd_gotoxy(0, 0);
            if (temp_val == 0)
            {
                lcd_print(" Off time  NEVER");   // 0 = display always on   
                lcd_gotoxy(0, 1);               // legend position
                lcd_print(" ");
                lcd_char(0);   // Up Arrow
                lcd_print(" ");
                lcd_char(1);   // Down Arrow
                lcd_print(" ");
                lcd_print("[OK]");
            }
            else
            {
                sprintf(lcd_line_buffer, " Off time %2u min", (unsigned int)temp_val);
                lcd_print(lcd_line_buffer);
                lcd_gotoxy(0, 1);               // legend position
                lcd_print(" ");
                lcd_char(0);   // Up Arrow
                lcd_print(" ");
                lcd_char(1);   // Down Arrow
                lcd_print(" ");
                lcd_print("[OK]");
            }
        }

        // 2. READ KEY
        key = ScanKeyBoard();
        if (key == KEY_NONE) continue;

        // 3. OK = hold to save & exit (dedicated 10ms tick, time-based)
        if (key == KEY_OK)
        {
            ok_cnt = 0;
            while (ScanKeyBoard() == KEY_OK)   // still held?
            {
                delay_ms(10);
                ok_cnt++;
                if (ok_cnt >= OK_EXIT_COUNT)   // OK_EXIT_COUNT x 10ms
                {
                    TimeOff      = temp_val;   // update RAM
                    SavedTimeOff = TimeOff;    // save to EEPROM
                    editing = 0;
                    break;
                }
            }
            continue;   // quick tap -> nothing, back to editing
        }

        // 4. UP/DOWN = change value in 1-minute steps
        if (key == KEY_UP)
        {
            if ((temp_val + TIMEOFF_STEP) <= TIMEOFF_MAX)
                temp_val = temp_val + TIMEOFF_STEP;
            redraw = 1;
        }
        else if (key == KEY_DOWN)
        {
            if (temp_val >= TIMEOFF_STEP)
                temp_val = temp_val - TIMEOFF_STEP;
            redraw = 1;
        }

        // 5. WAIT KEY RELEASE (UP/DOWN only — act once per press)
        while (ScanKeyBoard() != KEY_NONE)
        {
            // use wdr if issue
        }
    }

    // 6. CLEAN UP
    lcd_cmd(0x01);
    delay_ms(2);
}
// ---- Override options (file scope) ----
//flash unsigned char override_names[4][13] =
//{
//    "WINTER ONLY ",   // 0: ignore changeover, winter hw continuously
//    "SEASONAL    ",   // 1: automatic switch Apr 2 / Sep 1
//    "SEASONAL+1W ",   // 2: automatic, delayed by one week
//    "SEASONAL+1M "    // 3: automatic, delayed by one month
//};

void SetupOverride(void)
{   //unsigned char lcd_line_buffer[17];
    unsigned char temp_val;         // working copy (RAM)
    unsigned char editing  = 1;
    unsigned char ok_cnt   = 0;
    unsigned char key      = KEY_NONE;
    unsigned char redraw   = 1;     // force first draw

    // Entry: current value, clamped to valid range 0..3
    temp_val = Override;
    if (temp_val > 3) temp_val = 0;

    // ===== Screen prepare =====
    lcd_cmd(0x01);                  // clear both lines
    delay_ms(2);
    LoadCustomLCDChars();           // reload arrows into CGRAM
    lcd_gotoxy(0, 1);               // legend position
    lcd_print(" ");
    lcd_char(0);   // Up Arrow
    lcd_print(" ");
    lcd_char(1);   // Down Arrow
    lcd_print(" ");
    lcd_print("[OK]");

    while (editing)
    {
        // 1. DRAW VALUE ON LINE 1
        if (redraw)
        {
            redraw = 0;
            //strcpyf(lcd_line_buffer, override_names[temp_val]);  // flash -> RAM
            lcd_gotoxy(0, 0);
            lcd_print(">");
            switch (temp_val)
            {
                case 0: lcd_print("WINTER ONLY "); break;
                case 1: lcd_print("SEASONAL    "); break;
                case 2: lcd_print("SEASONAL+1W "); break;
                case 3: lcd_print("SEASONAL+1M "); break;
            }
        }

        // 2. READ KEY
        key = ScanKeyBoard();
        if (key == KEY_NONE) continue;

        // 3. OK = hold to save & exit (dedicated 10ms tick, time-based)
        if (key == KEY_OK)
        {
            ok_cnt = 0;
            while (ScanKeyBoard() == KEY_OK)   // still held?
            {
                delay_ms(10);
                ok_cnt++;
                if (ok_cnt >= OK_EXIT_COUNT)  // OK_EXIT_COUNT x 10ms
                {
                    Override       = temp_val;      // update RAM
                    SavedOverride  = temp_val;      // save to EEPROM
                    editing = 0;
                    break;
                }
            }
            continue;   // quick tap -> nothing, back to editing
        }

        // 4. UP/DOWN = change selection, wrap around
        if (key == KEY_UP)
        {
            temp_val++;
            if (temp_val > 3) temp_val = 0;
            redraw = 1;
        }
        else if (key == KEY_DOWN)
        {
            if (temp_val == 0) temp_val = 3;
            else temp_val--;
            redraw = 1;
        }

        // 5. WAIT KEY RELEASE (UP/DOWN only — act once per press)
        while (ScanKeyBoard() != KEY_NONE)
        {
            // use wdr if issue
        }
    }

    // 6. CLEAN UP
    lcd_cmd(0x01);
    delay_ms(2);
}

// -------------------------------------------------------------
// Main Entry Point: Jump to this function when user selects "SET Mode & temp"
// will choice sensor 1 or 2 and drive relay
// -------------------------------------------------------------
    void SetupMode(void)
    {
        unsigned char temp_val;         // working copy (RAM)
        unsigned char editing  = 1;
        unsigned char ok_cnt   = 0;
        unsigned char key      = KEY_NONE;
        unsigned char redraw   = 1;     // force first draw

        // Entry: current value, clamped to valid range 0..1
        temp_val = Mode;
        if (temp_val > 1) temp_val = MODE_COMBI;

        // ===== Screen prepare =====
        lcd_cmd(0x01);                  // clear both lines
        delay_ms(2);
        LoadCustomLCDChars();           // reload arrows into CGRAM
        lcd_gotoxy(0, 1);               // legend position
        lcd_print(" ");
        lcd_char(0);   // Up Arrow
        lcd_print(" ");
        lcd_char(1);   // Down Arrow
        lcd_print(" ");
        lcd_print("[OK]");

        while (editing)
        {
            // 1. DRAW VALUE ON LINE 1
            if (redraw)
            {
                redraw = 0;
                lcd_gotoxy(0, 0);
                lcd_print(">");
                if (temp_val == MODE_COMBI) lcd_print("COMBI/UFLOOR ");
                else                        lcd_print("2/3 PORT VLV ");
            }

            // 2. READ KEY
            key = ScanKeyBoard();
            if (key == KEY_NONE) continue;

            // 3. OK = hold to save & exit (dedicated 10ms tick, time-based)
            if (key == KEY_OK)
            {
                ok_cnt = 0;
                while (ScanKeyBoard() == KEY_OK)   // still held?
                {
                    delay_ms(10);
                    ok_cnt++;
                    if (ok_cnt >= OK_EXIT_COUNT)  // OK_EXIT_COUNT x 10ms
                    {
                        Mode       = temp_val;     // update RAM
                        SavedMode  = temp_val;     // save to EEPROM
                        editing = 0;
                        break;
                    }
                }
                continue;   // quick tap -> nothing, back to editing
            }

            // 4. UP/DOWN = toggle between the two modes
            if (key == KEY_UP)
            {
                if (temp_val == MODE_COMBI) temp_val = MODE_VALVE;
                else                        temp_val = MODE_COMBI;
                redraw = 1;
            }
            else if (key == KEY_DOWN)
            {
                if (temp_val == MODE_COMBI) temp_val = MODE_VALVE;
                else                        temp_val = MODE_COMBI;
                redraw = 1;
            }

            // 5. WAIT KEY RELEASE (UP/DOWN only — act once per press)
            while (ScanKeyBoard() != KEY_NONE)
            {
                // use wdr if issue
            }
        }

        // 6. CLEAN UP
        lcd_cmd(0x01);
        delay_ms(2);
    }
// -------------------------------------------------------------
// Main Entry Point: Jump to this function when user selects "SET xxxx"
// -------------------------------------------------------------
void SetupDiffWinter(void)
{   unsigned char lcd_line_buffer[17];
    unsigned char temp_val;         // working copy (RAM)
    unsigned char editing  = 1;
    unsigned char ok_cnt   = 0;
    unsigned char key      = KEY_NONE;
    unsigned char redraw   = 1;     // force first draw
    unsigned char whole;            // integer part of display
    unsigned char frac;             // decimal part of display

    // Entry: current value, clamped to allowed range
    temp_val = DiffW;
    if (temp_val < DIFF_MIN)  temp_val = DIFF_MIN;
    if (temp_val > DIFF_MAX)  temp_val = DIFF_MAX;

    // ===== Screen prepare =====
    lcd_cmd(0x01);                  // clear both lines
    delay_ms(2);
    LoadCustomLCDChars();           // reload arrows into CGRAM
    lcd_gotoxy(0, 1);               // legend position
    lcd_print(" ");
    lcd_char(0);   // Up Arrow
    lcd_print(" ");
    lcd_char(1);   // Down Arrow
    lcd_print(" ");
    lcd_print("[OK]");

    while (editing)
    {
        // 1. DRAW VALUE ON LINE 1: " Winter  1.5C"
        if (redraw)
        {
            redraw = 0;
            whole = temp_val / 10;
            frac  = temp_val % 10;
            lcd_gotoxy(0, 0);
            sprintf(lcd_line_buffer, " Winter  %u.%uC", (unsigned int)whole, (unsigned int)frac);
            lcd_print(lcd_line_buffer);
        }

        // 2. READ KEY
        key = ScanKeyBoard();
        if (key == KEY_NONE) continue;

        // 3. OK = hold to save & exit (dedicated 10ms tick, time-based)
        if (key == KEY_OK)
        {
            ok_cnt = 0;
            while (ScanKeyBoard() == KEY_OK)   // still held?
            {
                delay_ms(10);
                ok_cnt++;
                if (ok_cnt >= OK_EXIT_COUNT)   // OK_EXIT_COUNT x 10ms
                {
                    DiffW      = temp_val;     // update RAM
                    SavedDiffW = temp_val;     // save to EEPROM
                    editing = 0;
                    break;
                }
            }
            continue;   // quick tap -> nothing, back to editing
        }

        // 4. UP/DOWN = change value in 0.5C steps
        if (key == KEY_UP)
        {
            if ((temp_val + DIFF_STEP) <= DIFF_MAX)
                temp_val = temp_val + DIFF_STEP;
            redraw = 1;
        }
        else if (key == KEY_DOWN)
        {
            if (temp_val >= (DIFF_MIN + DIFF_STEP))
                temp_val = temp_val - DIFF_STEP;
            redraw = 1;
        }

        // 5. WAIT KEY RELEASE (UP/DOWN only — act once per press)
        while (ScanKeyBoard() != KEY_NONE)
        {
            // use wdr if issue
        }
    }

    // 6. CLEAN UP
    lcd_cmd(0x01);
    delay_ms(2);
}
void SetupDiffSummer(void)
{   unsigned char lcd_line_buffer[17];
    unsigned char temp_val;
    unsigned char editing  = 1;
    unsigned char ok_cnt   = 0;
    unsigned char key      = KEY_NONE;
    unsigned char redraw   = 1;
    unsigned char whole;
    unsigned char frac;

    temp_val = DiffS;
    if (temp_val < DIFF_MIN)  temp_val = DIFF_MIN;
    if (temp_val > DIFF_MAX)  temp_val = DIFF_MAX;

    // ===== Screen prepare =====
    lcd_cmd(0x01);
    delay_ms(2);
    LoadCustomLCDChars();
    lcd_gotoxy(0, 1);
    lcd_print(" ");
    lcd_char(0);   // Up Arrow
    lcd_print(" ");
    lcd_char(1);   // Down Arrow
    lcd_print(" ");
    lcd_print("[OK]");

    while (editing)
    {
        if (redraw)
        {
            redraw = 0;
            whole = temp_val / 10;
            frac  = temp_val % 10;
            lcd_gotoxy(0, 0);
            sprintf(lcd_line_buffer, " Summer  %u.%uC", (unsigned int)whole, (unsigned int)frac);
            lcd_print(lcd_line_buffer);
        }

        key = ScanKeyBoard();
        if (key == KEY_NONE) continue;

        if (key == KEY_OK)
        {
            ok_cnt = 0;
            while (ScanKeyBoard() == KEY_OK)
            {
                delay_ms(10);
                ok_cnt++;
                if (ok_cnt >= OK_EXIT_COUNT)
                {
                    DiffS      = temp_val;
                    SavedDiffS = temp_val;
                    editing = 0;
                    break;
                }
            }
            continue;
        }

        if (key == KEY_UP)
        {
            if ((temp_val + DIFF_STEP) <= DIFF_MAX)
                temp_val = temp_val + DIFF_STEP;
            redraw = 1;
        }
        else if (key == KEY_DOWN)
        {
            if (temp_val >= (DIFF_MIN + DIFF_STEP))
                temp_val = temp_val - DIFF_STEP;
            redraw = 1;
        }

        while (ScanKeyBoard() != KEY_NONE)
        {
            // use wdr if issue
        }
    }

    lcd_cmd(0x01);
    delay_ms(2);
}

// -------------------------------------------------------------
// Show CPU temp & V cpu, relay state
// -------------------------------------------------------------
char    DisplayMeny (void)
{
// Local workspace copies
    unsigned char ReturnMeny=0;
    unsigned char edit_index = 0; 
    unsigned char editing    = 1;
    unsigned char key        = KEY_NONE; 
    unsigned char temp;
    unsigned char lcd_line_buffer[17]; 
    unsigned char NewKey=99;
    ClearScreen ();
    LoadCustomLCDChars ();
    sprintf(lcd_line_buffer, "   Setup meny   ");
    lcd_gotoxy(0, 0);
    lcd_print(lcd_line_buffer);
    lcd_gotoxy(0, 1);
 // Print the custom symbols step-by-step 
    lcd_print(" ");
    lcd_char(0);   // Prints Up Arrow (?)
    lcd_print(" ");
    lcd_char(1);   // Prints Down Arrow (?)
    lcd_print(" ");
    lcd_char(2);   // Prints Right Arrow (?)
    lcd_print(" ");
    lcd_char(3);   // Prints Left Arrow (?)    
    lcd_print(" ");
    lcd_print("[OK]");
    
    lcd_cmd(0x0E); // 0x0F = Display ON, Cursor ON, Blink ON
    while (editing) 
    {        
        //lcd_command(0x0F); // 0x0F = Display ON, Cursor ON, Blink ON  
        if ( NewKey !=0)
        {   NewKey=0; 
            switch (edit_index)
                   {
                   case 1:  // Set Clock
                        sprintf(lcd_line_buffer, "   Setup dato   ");
                   break;
                        
                   case 2:
                        sprintf(lcd_line_buffer, " Setup contract ");     
                   break; 
                        
                   case 3:
                        sprintf(lcd_line_buffer, " Setup backlight");    
                   break; 
                   
                   case 4:
                        sprintf(lcd_line_buffer, "   Setup modus  ");
                   break;
                   
                   case 5:  // Set Diff temperature sommer   
                        sprintf(lcd_line_buffer, "Set Diff Sommer ");
                   break;  
                   
                   case 6:  // Set Diff temperature Winter  
                        sprintf(lcd_line_buffer, "Set Diff winter ");
                   break; 
                   
                   case 7:  // Set timer off 
                        sprintf(lcd_line_buffer, " Set Timer off  ");
                   break;
                   
                   default:
                        sprintf(lcd_line_buffer, "   Setup time   ");
                   break;
                   
                   }
            lcd_gotoxy(0, 0);
            lcd_print(lcd_line_buffer);
            //lcd_cmd(0x0E); // 0x0F = Display ON, Cursor ON, Blink ON  
        }

 
        // 4. CAPTURE Keypad KEYSTROKE
        key = ScanKeyBoard();
        if (key == KEY_NONE) 
        {
            continue; 
        }
        else
        {   temp = key;
            while (temp)    // wait release
            {   temp= ScanKeyBoard();
                // use wdr if issue
            }
        }

        // 5. MATH LOGIC PROCESSING BLOCK
        switch (key) 
        {
//            case KEY_RIGHT:
//                NewKey++; 
//                // valide meny, return 
//                ReturnMeny=edit_index;
//                editing=0;
//            break;

            case KEY_UP:
                if ( edit_index < MAX_MENY_CHOICE)
                    edit_index++;  
                else
                    edit_index=0;      
                NewKey++;
                break;

            case KEY_DOWN:
                if ( edit_index !=0)
                    edit_index--; 
                else
                    edit_index = MAX_MENY_CHOICE;
                NewKey++;
                break;

            case KEY_OK:
                editing = 0; // Triggers exit     
                ReturnMeny=edit_index;
                lcd_cmd(0x0C); // Turn OFF cursor (Normal display mode)  
                lcd_cmd(0x01); // Clear display and refresh main screen
                delay_ms(2);
            break;
        }
    }
    return ReturnMeny;   
}

// arrow
// Custom pixel arrays (5x8 matrix per character)
// 1 = Pixel On, 0 = Pixel Off
// const for show arrow 4, heat frozing,  
const unsigned char custom_arrows[6][8] = {
    {0x04, 0x0E, 0x15, 0x04, 0x04, 0x04, 0x04, 0x00}, // 0: Up Arrow (?)
    {0x04, 0x04, 0x04, 0x04, 0x15, 0x0E, 0x04, 0x00}, // 1: Down Arrow (?)
    {0x00, 0x04, 0x02, 0x1F, 0x02, 0x04, 0x00, 0x00}, // 2: Right Arrow (?)
    {0x00, 0x04, 0x08, 0x1F, 0x08, 0x04, 0x00, 0x00},  // 3: Left Arrow (?)  
    {0x04, 0x0A, 0x0A, 0x12, 0x15, 0x15, 0x0E, 0x00}, // 4: HEATING FLAME (CRISP DESIGN) 
     {0x04, 0x15, 0x0E, 0x1F, 0x0E, 0x15, 0x04, 0x00}  // 5: Symmetrical Cooling Icon
     //{0x0A, 0x15, 0x0E, 0x04, 0x0E, 0x15, 0x0A, 0x00}  // 5: Cooling Snowflake
};
const unsigned char custom_clock[4][8] = {
    {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00}, // 0: 
    {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00}, // 1: 
    {0x00, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00}, // 2: 
    {0x00, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x00}  // 3:      
};
//const unsigned char custom_block[8][8] = {
//    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F}, // 0: 
//    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00}, // 1: 
//    {0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00}, // 2: 
//    {0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00}, // 3:      
//    {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00}, // 4:      
//    {0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00}, // 5:      
//    {0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 6:      
//    {0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // 7:      
//};

const unsigned char custom_block[8][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F}, // 0: 
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F}, // 1: 
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F}, // 2: 
    {0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F}, // 3:      
    {0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}, // 4:      
    {0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}, // 5:      
    {0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}, // 6:      
    {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}  // 7:    
};

// make clock
/* 0
Row 0:  . . . . .  (0x00)  --> 
Row 1:  . . . . X  (0x01)  --> 
Row 2:  . . . X .  (0x02)  --> 
Row 3:  . . X . .  (0x04)  --> 
Row 4:  . X . . .  (0x08)  --> 
Row 5:  X . . . .  (0x10)  --> 
Row 6:  . . . . .  (0x00)  --> 
Row 7:  . . . . .  (0x00)  
{0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00}
*/
/* 1
Row 0:  . . . . .  (0x00)  --> 
Row 1:  . . . . .  (0x00)  --> 
Row 2:  . . . . .  (0x00)  --> 
Row 3:  X X X X X  (0x1F)  --> SOLID HORIZONTAL MIDDLE LINE!
Row 4:  . . . . .  (0x00)  --> 
Row 5:  . . . . .  (0x00)  --> 
Row 6:  . . . . .  (0x00)  --> 
Row 7:  . . . . .  (0x00)  
{0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00}
*/
/*  2
Row 0:  . . . . .  (0x00)  --> 
Row 1:  X . . . .  (0x10)  --> 
Row 2:  . X . . .  (0x08)  --> 
Row 3:  . . X . .  (0x04)  --> 
Row 4:  . . . X .  (0x02)  --> 
Row 5:  . . . . X  (0x01)  --> 
Row 6:  . . . . .  (0x00)  --> 
Row 7:  . . . . .  (0x00) 
{0x00, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00} 
*/
/*  3
Row 0:  . . . . .  (0x00)  --> 
Row 1:  . . X . .  (0x04)  --> 
Row 2:  . . X . .  (0x04)  --> 
Row 3:  . . X . .  (0x04)  --> 
Row 4:  . . X . .  (0x04)  --> 
Row 5:  . . X . .  (0x04)  --> 
Row 6:  . . . . .  (0x00)  --> 
Row 7:  . . . . .  (0x00)  
{0x00, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x00} 
*/

/*
Row 0:  . . X . .  (0x04)  --> Top center point
Row 1:  X . X . X  (0x15)  --> Upper prongs
Row 2:  . X X X .  (0x0E)  --> Center core taper
Row 3:  X X X X X  (0x1F)  --> SOLID HORIZONTAL MIDDLE LINE!
Row 4:  . X X X .  (0x0E)  --> Center core taper
Row 5:  X . X . X  (0x15)  --> Lower prongs
Row 6:  . . X . .  (0x04)  --> Bottom center point
Row 7:  . . . . .  (0x00)  
*/
/*
Row 0:  . . X . .  (0x04)  --> Top center point
Row 1:  X . X . X  (0x15)  --> Upper prongs
Row 2:  . X X X .  (0x0E)  --> Center core taper
Row 3:  X X X X X  (0x1F)  --> SOLID HORIZONTAL MIDDLE LINE!
Row 4:  . X X X .  (0x0E)  --> Center core taper
Row 5:  X . X . X  (0x15)  --> Lower prongs
Row 6:  . . X . .  (0x04)  --> Bottom center point
Row 7:  . . . . .  (0x00)  
*/

void LoadCustomLCDChars(void) {
    unsigned char i, row;
    
    // Command 0x40 sets the LCD pointer to the start of CGRAM (Custom Character 0)
    lcd_cmd(0x40); 
    
    // Write 3 characters (Up, Down, Right), each 8 rows tall
    for (i = 0; i < 6; i++) {
        for (row = 0; row < 8; row++) {
            lcd_char(custom_arrows[i][row]);
        }
    }
    
    // Reset LCD pointer back to normal screen text memory (DDRAM)
    lcd_cmd(0x80); 
}
void LoadCustomClockChars(void) 
{
    unsigned char i, row;
    
    // Command 0x40 sets the LCD pointer to the start of CGRAM (Custom Character 0)
    lcd_cmd(0x40); 
    for (i = 0; i < 4; i++) {
        for (row = 0; row < 8; row++) {
            lcd_char(custom_clock[i][row]);
        }
    }
    
    // Reset LCD pointer back to normal screen text memory (DDRAM)
    lcd_cmd(0x80); 
}
void LoadCustomBlock(void) 
{
    unsigned char i, row;
    
    // Command 0x40 sets the LCD pointer to the start of CGRAM (Custom Character 0)
    lcd_cmd(0x40); 
    for (i = 0; i < 8; i++) {
        for (row = 0; row < 8; row++) {
            lcd_char(custom_block[i][row]);
        }
    }
    
    // Reset LCD pointer back to normal screen text memory (DDRAM)
    lcd_cmd(0x80); 
}
// Option B: Passing them cleanly via an Octal String inside your menu line:
// \000 = Custom Character 0 (Up)
// \001 = Custom Character 1 (Down)
// \002 = Custom Character 2 (Right)
//lcd_gotoxy(0, 1);
//lcd_print(" \000  \001  \002  [OK]");
void PrintRotateClock ( unsigned char index, unsigned char pos, unsigned char line)
{   //unsigned char temp;
//    temp = index; 
    lcd_gotoxy(pos, line);
 // Print the custom symbols step-by-step 
    lcd_char(index);   //
//    if ( temp == 0)
//        lcd_char(0);   //
//    if ( temp == 1)
//        lcd_char(1);   //
//    if ( temp == 2)
//        lcd_char(2);   //
//    if ( temp == 3)
//        lcd_char(3);   //
    
}
void PrintBlock ( unsigned char index, unsigned char pos, unsigned char line)
{   //unsigned char temp;
//    temp = index; 
    lcd_gotoxy(pos, line);
 // Print the custom symbols step-by-step 
    lcd_char(index);   //
    
}

// ============================================================
// MAIN DISPLAY — stateless renderer
// ============================================================
void MainDisplay(unsigned char scr)
{
    unsigned char lcd_buffer[17];   // LOCAL line buffer
    unsigned char whole;            // temp integer part
    unsigned char frac;             // temp decimal part
    unsigned int  show_temp;        // temp of active sensor
    unsigned char active_s;         // active sensor number

    // ---- CLEAR only on screen CHANGE (kills the 1 Hz flicker) ----
    if (scr != LastScreen)
    {
        LastScreen = scr;
        lcd_cmd(0x01);
        delay_ms(2);
    }

    switch (scr)
    {
        case 0:   // ============ DATE + CLOCK ============
            lcd_gotoxy(0, 0);
            sprintf(lcd_buffer, "Date: %04u-%02u-%02u",
                    current_year,
                    (unsigned int)month,
                    (unsigned int)day_of_month);
            lcd_print(lcd_buffer);

            lcd_gotoxy(0, 1);
            sprintf(lcd_buffer, "Clock: %02u:%02u:%02u",
                    (unsigned int)hours,
                    (unsigned int)minutes,
                    (unsigned int)seconds);
            lcd_print(lcd_buffer);
            break;

        case 1:   // ============ TEMP + STATE ============
            // pick sensor of the ACTIVE side (simplified date pick,
            // full GetSeason() with Override replaces this later)
            if ((month >= 5) && (month <= 8))               // May..Aug = summer
            {
                show_temp = CurrentTemp2;
                active_s = 2;
            }
            else if ((month == 4) && (day_of_month >= 2))   // Apr 2..30 = summer
            {
                show_temp = CurrentTemp2;
                active_s = 2;
            }
            else                                            // winter
            {
                show_temp = CurrentTemp1;
                active_s = 1;
            }

            lcd_gotoxy(0, 0);
            whole = show_temp / 10;
            frac  = show_temp % 10;
            sprintf(lcd_buffer, "Temp S%u: %u.%u",
                    (unsigned int)active_s,
                    (unsigned int)whole,
                    (unsigned int)frac);
            lcd_print(lcd_buffer);
            lcd_print("\xDF");          // degree symbol (ST7032i CGROM 0xDF)
            lcd_print("C");

            lcd_gotoxy(0, 1);
            switch (State)
            {
                case STATE_SAVING:  lcd_print("State: Saving  "); break;
                case STATE_COOLING: lcd_print("State: Cooling "); break;
                default:            lcd_print("State: Heating "); break;
            }
            break;

        case 2:   // ============ MODE + OVERRIDE ============
            lcd_gotoxy(0, 0);
            if (Mode == MODE_COMBI)  lcd_print("Mode: COMBI    ");
            else                     lcd_print("Mode: 2/3 PORT ");

            lcd_gotoxy(0, 1);
            if (Mode == MODE_VALVE)
            {
                lcd_print("Auto by date   ");   // valve: always automatic
            }
            else
            {
                switch (Override)
                {
                    case 0:  lcd_print("Winter always  "); break;
                    case 1:  lcd_print("Auto seasonal  "); break;
                    case 2:  lcd_print("Auto +1 week   "); break;
                    default: lcd_print("Auto +1 month  "); break;
                }
            }
            break;
    }
}
void ViewLog(void)
{
    unsigned char rec;               // record being viewed
    unsigned char editing  = 1;
    unsigned char ok_cnt   = 0;
    unsigned char key      = KEY_NONE;
    unsigned char lcd_buffer[17];
    unsigned int  on_v, off_v, day_v;
    unsigned char hour_v, min_v, sv_v;

    rec = 0;                         // start at newest

    // ===== Screen prepare =====
    lcd_cmd(0x01);
    delay_ms(2);
    LoadCustomLCDChars();
    lcd_gotoxy(0, 1);
    lcd_print(" ");
    lcd_char(0);                     // Up Arrow
    lcd_print(" ");
    lcd_char(1);                     // Down Arrow
    lcd_print(" ");
    lcd_print("[OK]");

    while (editing)
    {
        // ---- DRAW RECORD ----
        if (rec < HistCount)         // valid record
        {
            on_v   = HistOn[rec];
            off_v  = HistOff[rec];
            day_v  = HistDay[rec] % 100;
            hour_v = HistHour[rec];
            min_v  = HistMin[rec];
            sv_v   = HistSvPct[rec];

            lcd_gotoxy(0, 0);
            sprintf(lcd_buffer, "L%02u On %u Off %u",
                    (unsigned int)rec, on_v, off_v);
            lcd_print(lcd_buffer);

            lcd_gotoxy(0, 1);
            sprintf(lcd_buffer, "%02u:%02u d%02u Sv %u%%",
                    (unsigned int)hour_v, (unsigned int)min_v,
                    (unsigned int)day_v, (unsigned int)sv_v);
            lcd_print(lcd_buffer);
            // note: line 2 overwrites the arrow legend; acceptable:
            // arrows were needed only to enter — or move legend draw
            // inside the redraw and accept the trade. See note below.
        }
        else                          // no record yet
        {
            lcd_gotoxy(0, 0);
            lcd_print("Log empty      ");
            lcd_gotoxy(0, 1);
            lcd_print("               ");
        }

        // ---- KEYS ----
        key = ScanKeyBoard();
        if (key == KEY_NONE) continue;

        if (key == KEY_OK)             // hold to exit
        {
            ok_cnt = 0;
            while (ScanKeyBoard() == KEY_OK)
            {
                delay_ms(10);
                ok_cnt++;
                if (ok_cnt >= OK_EXIT_COUNT)
                {
                    editing = 0;
                    break;
                }
            }
            continue;
        }

        if (key == KEY_UP)             // newer record
        {
            if (rec > 0) rec--;
        }
        else if (key == KEY_DOWN)     // older record
        {
            if ((rec + 1) < HistCount) rec++;
        }

        while (ScanKeyBoard() != KEY_NONE)
        {
            // use wdr if issue
        }
    }

    lcd_cmd(0x01);
    delay_ms(2);
}
void HistPush(void)
{   unsigned int  total;      // <-- must be here, before first use
    unsigned char i;

    // shift down: oldest (index 49) falls out
    for (i = HIST_SIZE - 1; i > 0; i--)
    {
        HistOn[i]    = HistOn[i - 1];
        HistOff[i]   = HistOff[i - 1];
        HistSave[i]  = HistSave[i - 1];
        HistDay[i]   = HistDay[i - 1];
        HistHour[i]  = HistHour[i - 1];
        HistMin[i]   = HistMin[i - 1];
        HistSvPct[i] = HistSvPct[i - 1];
    } 
    // store the just-finished cycle at index 0 (newest)
    HistOn[0]    = CycleOnMin;    // unsigned int -> unsigned int, no cast needed
    HistOff[0]   = CycleOffMin;   // full 0..65535 range preserved
    HistSave[0]  = State;
    HistDay[0]   = day_of_month;
    HistHour[0]  = hours;
    HistMin[0]   = minutes;

    total = CycleOnMin + CycleOffMin;
    if (total == 0) HistSvPct[0] = 0;
    else HistSvPct[0] = (unsigned char)((CycleOffMin * 100UL) / total);

    // cap the count at buffer size
    if (HistCount < HIST_SIZE) HistCount++;

    // reset cycle accumulators for the next cycle
    CycleOnMin  = 0;
    CycleOffMin = 0;
}