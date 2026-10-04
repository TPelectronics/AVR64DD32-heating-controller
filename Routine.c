

#include "Routine.h"
// ADC initialization functions
#include "adc_init.h"

#include <delay.h>  // Included here because we need delay_ms
#include <stdio.h>
 unsigned char PWM_alarm;
 unsigned char PWM_sys; 
// Global time tracking variables
volatile unsigned char seconds = 0;
volatile unsigned char minutes = 0;
volatile unsigned char hours   = 0;
volatile unsigned int  days    = 0; 
//volatile unsigned char  save_countdown=0;
//volatile unsigned char  SetFlagToSave=0 ;
volatile unsigned char  NewMinutes=0;
volatile unsigned char  NewHours=0;
volatile unsigned char  NewDays=0;
volatile unsigned char SaveCountdown;
volatile unsigned char SetFlagToSave=0;
volatile unsigned char TurnOffCountdown=0;  
volatile unsigned char SetFlagTurnOff=0;

// Calendar variables derived from the raw 'days' counter
volatile unsigned char day_of_month = 1; // 1 to 31
volatile unsigned char month        = 1; // 1 = Jan, 2 = Feb, ..., 12 = Dec
volatile unsigned char leap_year_cycle = 0; // 0, 1, 2 = Normal years (28 days). 3 = Leap year (29 days).
volatile unsigned int  current_year; // Input any 4-digit calendar year
// Array containing the length of each month (Index 0 is unused to match Month 1-12 layout)
// Note: Month 2 (February) starts at 28 and is dynamically modified in code
unsigned char month_lengths[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

unsigned char BackLight;
unsigned char LCD_Contrast;
unsigned char Mode;
unsigned char TimeOff;

unsigned char MainScreen   = 0;   // selected screen 0..2 — owned by main loop
unsigned char LastSeconds  = 0;   // snapshot for 1s redraw — owned by main loop

unsigned int CurrentTemp1,CurrentTemp2;  
// NTC
// 12-bit ADC lookup values for -20.0°C to +80.0°C (0.5°C steps)
// Circuit: VCC -> 4.7k Pull-up -> ADC Pin -> 10k NTC -> GND (B=3435)
flash unsigned int ntc_table[] = {
    3861, 3855, 3849, 3843, 3836, 3830, 3823, 3817, 3810, 3803, // -20.0C to -15.5C
    3796, 3788, 3781, 3774, 3766, 3758, 3750, 3742, 3734, 3726, // -15.0C to -10.5C
    3718, 3709, 3700, 3691, 3682, 3673, 3664, 3655, 3645, 3635, // -10.0C to -5.5C
    3625, 3615, 3605, 3595, 3585, 3574, 3563, 3552, 3541, 3530, // -5.0C to -0.5C
    3519, 3507, 3496, 3484, 3472, 3460, 3448, 3435, 3423, 3410, //  0.0C to  4.5C
    3398, 3385, 3372, 3358, 3345, 3332, 3318, 3304, 3290, 3276, //  5.0C to  9.5C
    3262, 3248, 3234, 3219, 3204, 3190, 3175, 3160, 3144, 3129, // 10.0C to 14.5C
    3114, 3098, 3083, 3067, 3051, 3035, 3019, 3003, 2987, 2971, // 15.0C to 19.5C
    2954, 2938, 2921, 2904, 2888, 2871, 2854, 2837, 2820, 2803, // 20.0C to 24.5C
    2786, 2768, 2751, 2734, 2716, 2699, 2681, 2664, 2646, 2629, // 25.0C to 29.5C
    2611, 2593, 2576, 2558, 2540, 2522, 2504, 2487, 2469, 2451, // 30.0C to 34.5C
    2433, 2415, 2397, 2379, 2362, 2344, 2326, 2308, 2290, 2273, // 35.0C to 39.5C
    2255, 2237, 2219, 2202, 2184, 2166, 2149, 2131, 2114, 2096, // 40.0C to 44.5C
    2079, 2062, 2044, 2027, 2010, 1993, 1976, 1959, 1942, 1925, // 45.0C to 49.5C
    1908, 1891, 1875, 1858, 1842, 1825, 1809, 1793, 1777, 1760, // 50.0C to 54.5C
    1744, 1728, 1713, 1697, 1681, 1666, 1650, 1635, 1620, 1604, // 55.0C to 59.5C
    1589, 1574, 1559, 1545, 1530, 1515, 1501, 1486, 1472, 1458, // 60.0C to 64.5C
    1444, 1430, 1416, 1402, 1388, 1375, 1361, 1348, 1335, 1322, // 65.0C to 69.5C
    1309, 1296, 1283, 1270, 1257, 1245, 1233, 1220, 1208, 1196, // 70.0C to 74.5C
    1184, 1172, 1160, 1149, 1137, 1126, 1114, 1103, 1092, 1081, // 75.0C to 79.5C
    1070                                                         // 80.0C
};

unsigned char Override;

unsigned char Temp1_low;    // winter band min  (x10, e.g. 185 = 18.5C)
unsigned char Temp1_high;   // winter band max
unsigned char Temp2_low;    // summer band min
unsigned char Temp2_high;   // summer band max
unsigned char LastState = 0xFF;

// EEPROM
    eeprom unsigned char NewEEprom;
    eeprom unsigned char SavedBacklight;
    eeprom unsigned char SavedContrast; 
    eeprom unsigned char SavedTimeOff; 
    eeprom unsigned char SavedMode;
    eeprom unsigned char SavedTemp1_low;
    eeprom unsigned char SavedTemp1_high;
    eeprom unsigned char SavedTemp2_low;
    eeprom unsigned char SavedTemp2_high;
    eeprom unsigned int SavedYear;
    eeprom unsigned char SavedMonth;
    eeprom unsigned char SavedDay;  
    eeprom unsigned char eeprom_buffer[EEPROM_BUFFER_SIZE];  // EEPROM buffer 
    eeprom unsigned char SavedOverride;   // 0=winter only, 1=seasonal, 2=+1w, 3=+1m     
    eeprom unsigned char SavedDiffW;    // winter diff, x10 deg (5..50)
    eeprom unsigned char SavedDiffS;   // summer diff, x10 deg (5..50)
    unsigned char DiffW;
    unsigned char DiffS;
    unsigned char current_buffer_index = 0;  // Start with Buffer 0
    unsigned char power_loss_flag = 0;        // 0 = no power loss, 1 = power loss detected    
    unsigned char State = STATE_HEATING;   // optimiser writes this later; 0 = heating at boot    
    unsigned char LastScreen   = 0xFF; // last drawn screen (0xFF = force first clear)

// read eeprom value and init all register 
// Save time to the next unused buffer and mark it as used
//void save_time(unsigned char hours, unsigned char minutes) 
//{
//    unsigned char next_buffer = current_buffer_index;
//    unsigned char next_next_buffer;
//    unsigned char tries;
//    tries = 0;
//
//    // Find the next unused buffer (hours == 0xFF)
//    while (eeprom_buffer[next_buffer * BUFFER_SIZE] != 0xFF) {
//        next_buffer = (next_buffer + 1) % NUM_BUFFERS;  
//        tries++;
//        if (tries >= NUM_BUFFERS)
//        {
//            eeprom_buffer[0] = 0xFF;      /* force-recycle slot 0 */
//            next_buffer = 0;
//            break;
//        }
//    }
//
//    // Write data to the unused buffer
//    eeprom_buffer[next_buffer * BUFFER_SIZE] = hours;
//    eeprom_buffer[next_buffer * BUFFER_SIZE + 1] = minutes;
//
//    // Mark the next buffer as unused (0xFF in hours)
//    next_next_buffer = (next_buffer + 1) % NUM_BUFFERS;
//    eeprom_buffer[next_next_buffer * BUFFER_SIZE] = 0xFF;
//
//    // Update current_buffer_index to the next buffer
//    current_buffer_index = next_next_buffer;
//}

void save_time(unsigned char hours, unsigned char minutes)
{
    unsigned char slot;
    unsigned char next_slot;
    unsigned char code;

    code = (hours << 3) | (minutes / MAX_MINUT_TO_SAVE);

    slot = 0;
    while (slot < NUM_BUFFERS)
    {
        if (eeprom_buffer[slot] == EMPTY_MARK)
        {
            break;
        }
        slot++;
    }

    if (slot >= NUM_BUFFERS)
    {
        slot = 0;
        eeprom_buffer[0] = EMPTY_MARK;
    }

    eeprom_buffer[slot] = code;

    next_slot = (slot + 1) % NUM_BUFFERS;
    eeprom_buffer[next_slot] = EMPTY_MARK;
}

 

// Recover the last saved time (for power loss)
unsigned char recover_last_time(unsigned char *hours, unsigned char *minutes)
{   unsigned char slot;
    unsigned char prev_slot;
    unsigned char code;

    slot = 0;
    while (slot < NUM_BUFFERS)
    {
        if (eeprom_buffer[slot] == EMPTY_MARK)
        {
            break;
        }
        slot++;
    }

    if (slot == 0)
    {
        prev_slot = NUM_BUFFERS - 1;
    }
    else
    {
        prev_slot = slot - 1;
    }

    code = eeprom_buffer[prev_slot];

    if (code == EMPTY_MARK)
    {
        return 0;
    }

    *hours   = code >> 3;
    *minutes = (code & 0x07) * MAX_MINUT_TO_SAVE;
    return 1;
}


//{    unsigned char last_buffer = current_buffer_index;
//    if (last_buffer == 0) {
//        last_buffer = NUM_BUFFERS - 1;
//    } else {
//        last_buffer--;
//    }
//    // Check if the last buffer is valid (hours != 0xFF)
//    if (eeprom_buffer[last_buffer * BUFFER_SIZE] != 0xFF) {
//        *hours = eeprom_buffer[last_buffer * BUFFER_SIZE];
//        *minutes = eeprom_buffer[last_buffer * BUFFER_SIZE + 1];
//        power_loss_flag = 1;  // Set power loss flag
//    } else {
//        // If no valid time found, return defaults
//        *hours = 0;
//        *minutes = 0;
//        power_loss_flag = 0;  // No power loss detected
//    }
//}

// Check if power loss was detected
//unsigned char is_power_loss_detected(void) {
//    return power_loss_flag;
//}
  
void LoadFromEEprom (void)
{   unsigned char temp;
    unsigned char i;
    unsigned char max_days;
    temp= NewEEprom;
    if ( temp != SavedEEprom )  // test if empty or corrupt eeprom
    {   NewEEprom = SavedEEprom;    // Skip nest time 
        SavedBacklight  =   Default_Backlight;
        SavedContrast   =   Default_Contrast;
        SavedTimeOff    =   Default_TimeOff;
        SavedMode       =   Default_Mode;
        SavedTemp1_low  =   Default_Temp1_low;
        SavedTemp1_high =   Default_Temp1_high;
        SavedTemp2_low  =   Default_Temp2_low;
        SavedTemp2_high =   Default_Temp2_high;
        SavedYear       =   Default_Year;
        SavedMonth      =   Default_Month;
        SavedDay        =   Default_Day;
        minutes = 0;
        hours = 0;
        seconds = 0;  
        // Clear ALL buffers to start fresh (hours = 0xFF)
        for (i = 0; i < NUM_BUFFERS; i++) {
            eeprom_buffer[i * BUFFER_SIZE] = 0xFF;  // hours = unused
        }
        current_buffer_index = 0;  // Start with Buffer 0
        power_loss_flag = 0;        // Reset power loss flag
    }
    // Get value from eeprom 
    // BACKLIGHT VALIDATION (Example: Must be between 0 and 100%)
    if (SavedBacklight <= 100) 
    {   BackLight = SavedBacklight;
    } 
    else
    {  BackLight = Default_Backlight; // Fallback if data is corrupted
    }
    set_backlight_pwm ( BackLight);
    // CONTRAST VALIDATION (Your ST7032i requires 0 to 63 max)
    if (SavedContrast <= 63) 
    {   LCD_Contrast = SavedContrast;
    } 
    else 
    {  LCD_Contrast = Default_Contrast;
    } 
    SetLCD_Contrast ( LCD_Contrast );

    // DATE BOUNDARY VALIDATION
    if (SavedMonth >= 1 && SavedMonth <= 12) 
    {   month = SavedMonth;
    } 
    else 
    {   month = Default_Month;
    }
     // 2. Determine the maximum allowed days for the current month
    max_days = month_lengths[month];
    // 3. Dynamic leap year check for February
    // If the year is evenly divisible by 4, it is a leap year (e.g., Year 24, 28, 32...)
    if (month == 2 && (SavedYear % 4 == 0)) {
        max_days = 29; 
    }

    // 4. Validate and load the Day using the dynamically calculated maximum
    if (SavedDay >= 1 && SavedDay <= max_days) 
    {   days = SavedDay;
    } 
    else 
    {   days = Default_Day; // Safe fallback if memory contains an impossible day
    }

    current_year = SavedYear; // Basic transfer
    TimeOff = SavedTimeOff;
    Mode = SavedMode;
    recover_last_time(&hours, &minutes);
    // control at values are in range of value
    // update all parameter
    
}

// set contrast
void    SetLCD_Contrast (unsigned char value)
{   unsigned char ContrastHigh;
    unsigned char ContrastLow;
    // 1. Force the 8-bit input down to a safe 6-bit limit (0 to 63 max)
    if (value > 63) {
        value = 63; 
    }
    ContrastHigh= (value >> 4) & 0x03; ;
    ContrastLow= value & 0x0F;  
    // Enter ST7032i Instruction Table 1 (Mandatory to change contrast)
    // 0x39 assumes an 8-bit bus mode with IS=1. Adjust if your initialization differs.  
    lcd_cmd(0x39); delay_ms(LCD_CDE);
    lcd_cmd(0x70 | ContrastLow); delay_ms(LCD_CDE);    // CONTRAST SET 0111CCCC ,C3,C2,C1,C0:1111
    lcd_cmd(0x54 | ContrastHigh); delay_ms(LCD_CDE);    // POWER/ICON/CONTRAST CTRL 010101 C5 C4
    // Return back to standard Instruction Table 0 (IS=0)
    // This ensures your normal printing/cursor commands work later
    lcd_cmd(0x38); 
    delay_ms(LCD_CDE);
}

int get_ntc_temperature(unsigned int adc_val) 
{
    int i; 
    // Check upper boundary limit (If reading is colder than -20°C)
    if (adc_val >= ntc_table[0]) {
        return START_TEMP;
    }   
    // Loop through the 200 comparison brackets 
    for (i = 0; i < 200; i++) {
        // Find when the raw 12-bit ADC drop crosses an index ceiling
        if (adc_val >= ntc_table[i+1]) {
            return START_TEMP + (i * TEMP_STEP);
        }
    }   
    // Check lower boundary limit (If reading is hotter than 80°C)
    return START_TEMP + (200 * TEMP_STEP);
}

unsigned char is_leap_year(unsigned int year)
{
    // 1. If not divisible by 4, it is a normal year
    if (year % 4 != 0) return 0; 
    
    // 2. If divisible by 100 but not 400, it is a normal year (e.g., 2100)
    if (year % 100 == 0 && year % 400 != 0) return 0;
    
    // 3. Otherwise, it is a leap year
    return 1; 
}

/**
 * @brief Initializes Port D pins for the switches
 */
void Buttons_Init(void) 
{
    PORTD.DIRCLR = SW_DOWN_PIN | SW_LEFT_PIN | SW_RIGHT_PIN | SW_OK_PIN | SW_UP_PIN;
    
    PORTD.PIN3CTRL = PORT_PULLUPEN_bm;
    PORTD.PIN4CTRL = PORT_PULLUPEN_bm;
    PORTD.PIN5CTRL = PORT_PULLUPEN_bm;
    PORTD.PIN6CTRL = PORT_PULLUPEN_bm;
    PORTD.PIN7CTRL = PORT_PULLUPEN_bm;
}

/**
 * @brief Safely reads a button with a 20ms debounce delay
 * @param button_pin The bitmask of the button (e.g., SW_OK_PIN)
 * @return 1 if successfully pressed and debounced, 0 if not pressed
 */
unsigned char Buttons_Read_Debounced(unsigned char button_pin) 
{
    // Check if button is pulled low (pressed)
    if ((PORTD.IN & button_pin) == 0) {
        delay_ms(20); // Wait out the mechanical switch bounce noise
        
        // Check again to ensure it's still pressed
        if ((PORTD.IN & button_pin) == 0) {
            return 1; // Valid press!
        }
    }
    return 0; // Not pressed
}
// Get Key
// read port and set bit of return char if active
/* this is define in file routine.h
#define FLAG_SW_DOWN    0x01
#define FLAG_SW_LEFT    0x02
#define FLAG_SW_RIGHT   0x04
#define FLAG_SW_OK      0x08
#define FLAG_SW_TOP     0x10
#define READ_SW_DOWN()   (PORTD.IN & SW_DOWN_PIN)
#define READ_SW_LEFT()   (PORTD.IN & SW_LEFT_PIN)
#define READ_SW_RIGHT()  (PORTD.IN & SW_RIGHT_PIN)
#define READ_SW_OK()     (PORTD.IN & SW_OK_PIN)
#define READ_SW_UP()    (PORTD.IN & SW_UP_PIN) 
*/
//unsigned char   ScanKeyBoard ( void)
//{   unsigned char ReturnData=KEY_NONE;
//    unsigned temp;
//    temp = READ_SW_DOWN ();
//    if ( temp == 0) // Key Activ
//    {   delay_ms(20); // Wait out the mechanical switch bounce noise  
//        temp = READ_SW_DOWN ();
//        if ( temp == 0) // Key Activ
//            ReturnData = KEY_DOWN;
//    }
//    temp = READ_SW_LEFT ();
//    if ( temp == 0) // Key Activ
//    {   delay_ms(20); // Wait out the mechanical switch bounce noise  
//        temp = READ_SW_LEFT ();
//        if ( temp == 0) // Key Activ
//            ReturnData = KEY_LEFT;
//    } 
//    temp = READ_SW_OK ();
//    if ( temp == 0) // Key Activ
//    {   delay_ms(20); // Wait out the mechanical switch bounce noise 
//        temp = READ_SW_OK ();
//        if ( temp == 0) // Key Activ
//            ReturnData = KEY_OK;
//    }
//    temp = READ_SW_RIGHT ();
//    if ( temp == 0) // Key Activ
//    {   delay_ms(20); // Wait out the mechanical switch bounce noise 
//        temp = READ_SW_RIGHT ();
//        if ( temp == 0) // Key Activ
//            ReturnData = KEY_RIGHT;
//    }   
//    temp = READ_SW_UP ();
//    if ( temp == 0) // Key Activ
//    {   delay_ms(20); // Wait out the mechanical switch bounce noise 
//        temp = READ_SW_UP ();
//        if ( temp == 0) // Key Activ
//            ReturnData = KEY_UP;
//    }
//    return  ReturnData; 
//    
//}

unsigned char ScanKeyBoard(void)
{
//    unsigned char pins;          // raw port snapshot
    unsigned char keys;         // active keys bitmask
    unsigned char temp;
    unsigned char ReturnData = KEY_NONE;

//    // 1. ONE port read instead of five
//    pins = PORTD.IN & SW_PORT_MASK;

    // 2. Build bitmask of active keys (pressed = pin reads 0)
    keys = KEY_NONE;
    if (READ_SW_DOWN()  == 0) keys |= KEY_DOWN;
    if (READ_SW_LEFT()  == 0) keys |= KEY_LEFT;
    if (READ_SW_RIGHT() == 0) keys |= KEY_RIGHT;
    if (READ_SW_OK()    == 0) keys |= KEY_OK;
    if (READ_SW_UP()    == 0) keys |= KEY_UP;

    if (keys == KEY_NONE)       // nothing pressed -> out immediately
    {
        return KEY_NONE;        // NO delay on the idle path!
    }

    // 3. ONE debounce for the whole port, then re-read
    delay_ms(20);
    temp = KEY_NONE;
    if (READ_SW_DOWN()  == 0) temp |= KEY_DOWN;
    if (READ_SW_LEFT()  == 0) temp |= KEY_LEFT;
    if (READ_SW_RIGHT() == 0) temp |= KEY_RIGHT;
    if (READ_SW_OK()    == 0) temp |= KEY_OK;
    if (READ_SW_UP()   == 0) temp |= KEY_UP;

    // 4. Keep only keys stable across the debounce
    keys &= temp;
    if (keys == KEY_NONE)
    {
        return KEY_NONE;        // was bounce, discard
    }

    // 5. Priority resolution (UP > OK > RIGHT > LEFT > DOWN)
    if      (keys & KEY_UP)    ReturnData = KEY_UP;
    else if (keys & KEY_OK)    ReturnData = KEY_OK;
    else if (keys & KEY_RIGHT) ReturnData = KEY_RIGHT;
    else if (keys & KEY_LEFT)  ReturnData = KEY_LEFT;
    else                        ReturnData = KEY_DOWN;

    return ReturnData;
}

void set_backlight_pwm(unsigned char duty)
{
    if (duty == 0)
    {
        TCA0.SINGLE.CMP0 = 0; // 100% OFF
    }
    else if (duty >= 100)
    {
        TCA0.SINGLE.CMP0 = PWM_PERIOD_VAL + 1; // 100% ON (forces pin to stay high)
    }
    else
    {
        // Calculate proportional duty step
        TCA0.SINGLE.CMP0 = (unsigned int)(((unsigned long)duty * PWM_PERIOD_VAL) / 100);
    }
}

void set_system_led_pwm(unsigned char duty)
{
    if (duty == 0)
    {
        TCA0.SINGLE.CMP2 = 0; // 100% OFF
    }
    else if (duty >= 100)
    {
        TCA0.SINGLE.CMP2 = PWM_PERIOD_VAL + 1; // 100% ON (forces pin to stay high)
    }
    else
    {
        // Calculate proportional duty step
        TCA0.SINGLE.CMP2 = (unsigned int)(((unsigned long)duty * PWM_PERIOD_VAL) / 100);
    }
}

void set_alarm_led_pwm(unsigned char duty)
{
    if (duty == 0)
    {
        TCA0.SINGLE.CMP1 = 0; // 100% OFF
    }
    else if (duty >= 100)
    {
        TCA0.SINGLE.CMP1 = PWM_PERIOD_VAL + 1; // 100% ON (forces pin to stay high)
    }
    else
    {
        // Calculate proportional duty step
        TCA0.SINGLE.CMP1 = (unsigned int)(((unsigned long)duty * PWM_PERIOD_VAL) / 100);
    }
}

// LCD
// Corrected I2C data transmission routine
void lcd_cmd(unsigned char cmd)
{
    unsigned char tx_buffer[2]; // Define an explicit 2-byte array buffer
    
    tx_buffer[0] = 0x00; // Control Byte: 0x00 means "Next byte is a Command"
    tx_buffer[1] = cmd;  // The actual layout instruction byte
    
    // Transmit 2 bytes over TWI0. We put 0, 0 for rx because we are not reading.
    twi_master_trans(&twi0_master, ST7031_I2C_ADDR, tx_buffer, 2, 0, 0);
    delay_us(30);     delay_us(30);        // Execution delay required by the ST display internal processor
}
void lcd_char(unsigned char data)
{
    unsigned char tx_buffer[2];
    
    tx_buffer[0] = 0x40; // Control Byte: 0x40 means "Next byte is visible text RAM data"
    tx_buffer[1] = data;  // The ASCII character byte
    
    twi_master_trans(&twi0_master, ST7031_I2C_ADDR, tx_buffer, 2, 0, 0);
    delay_us(30);
}

void lcd_init(void)
{
    /*
    #define ST7032_I2C_ADDRESS  0x7C   // 0x3E shifted left 1 bit for Write (0x3E << 1 = 0x7C)
    #define ST7032_COMMAND      0x00   // Control byte for instructions
    #define ST7032_DATA         0x40   // Control byte for character data
    */
    // Reset LCD
    delay_ms (10); // wait power stablized
    LCD_RST_LOW ();
    delay_ms (10); // wait power stablized
    LCD_RST_HIGH ();
    delay_ms (100); // wait power stablized
    // End of Reset
        
        lcd_cmd(0x39); delay_ms(5);    // //FUNCTION SET 001,DL,N,DH,IS2,IS1
        lcd_cmd(0x39); delay_ms(5);    // //FUNCTION SET 001,DL,N,DH,IS2,IS1

        //INTERNAL FREQ 0001B10Fx B:Bias ; Fx: Frame frequency
        lcd_cmd(0x1C); delay_ms(LCD_CDE);    //INTERNAL FREQ 0001BFx B:1 ; Fx:100=183hz
         // Contrast 2 byte
        lcd_cmd(0x7f); delay_ms(LCD_CDE);    // CONTRAST SET 0111CCCC ,C3,C2,C1,C0:1111
        lcd_cmd(0x55); delay_ms(LCD_CDE);    // POWER/ICON/CONTRAST CTRL 010101 C5 C4

        // Test:
        // lcd_cmd(0x7F) + lcd_cmd(0x57): to much contrast not readable
        // lcd_cmd(0x7F) + lcd_cmd(0x54): no text
        // lcd_cmd(0x7F) + lcd_cmd(0x55): ok
        // lcd_cmd(0x70) + lcd_cmd(0x55): No text
        // lcd_cmd(0x7F) + lcd_cmd(0x56): too much
        // lcd_cmd(0x78) + lcd_cmd(0x56): max readable 
        // lcd_cmd(0x74) + lcd_cmd(0x56): ok but see the 5x7 piksel
        // lcd_cmd(0x70) + lcd_cmd(0x56): good

        //	OPF1,OPF2:  connect to VSS,internal follower is turn on 
        lcd_cmd(0x6c); delay_ms(LCD_CDE);    // Default value
        lcd_cmd(0x38); delay_ms(LCD_CDE);     // Function Set: Switch back to normal instruction table mode
        // Display on/off 00001DCB
        lcd_cmd(0x0C); delay_ms(LCD_CDE);    // Display=1 Cursor=0 Blink=0
    //    lcd_cmd(0x0E); delay_ms(LCD_CDE);    // Display=1 Cursor=1 Blink=0
    //    lcd_cmd(0x0F); delay_ms(LCD_CDE);    // Display=1 Cursor=1 Blink=1
        // Clear Display & go home
        lcd_cmd(0x01); delay_ms(LCD_CDE);    // CLR Display
    //	lcd_cmd(0x02); delay_ms(LCD_CDE);    // Go home
}

void refresh_lcd_ints( int raw_temp, unsigned char hours, unsigned char minutes,unsigned char seconds) 
{
    char line1_buffer[17];
    char line2_buffer[17];
    unsigned char i;
    // Separate the raw integer (e.g., 235 becomes 23 and 5)
    int whole   = raw_temp / 10;
    int decimal = raw_temp % 10;  
    
    lcd_cmd(0x01); delay_ms(LCD_CDE);    // Go home
    // Handle negative temperatures correctly (e.g., -1.5°C)
    if (decimal < 0) {
        decimal = -decimal; 
    }
    // Format Line 1 using simple integers (%d)
    // \xDF is the degree symbol
    if (raw_temp < 0 && whole == 0) 
    {
        sprintf(line1_buffer, "Temp: -%d.%d\xDF""C", whole, decimal);
    } 
    else    // Normal printing handles positive numbers and numbers <= -1.0 automatically
    {     
        sprintf(line1_buffer, "Temp: %d.%d\xDF""C", whole, decimal);
    }    
    // Format Line 2
    sprintf(line2_buffer, "Time: %02d:%02d:%02d", hours, minutes, seconds);   
    // ---- Send to LCD ---- 
    // Line 1
    lcd_cmd(0x80);
    for ( i = 0; line1_buffer[i] != '\0'; i++) {
        lcd_char(line1_buffer[i]);
    }
    // line 2
    lcd_cmd(0xC0);
    for ( i = 0; line2_buffer[i] != '\0'; i++) {
        lcd_char(line2_buffer[i]);
    }
}

void refresh_lcd_temps(int raw_temp1,int raw_temp2) 
{
    char line1_buffer[17];
    char line2_buffer[17];
    unsigned char i;
    // Separate the raw integer (e.g., 235 becomes 23 and 5)
    int whole   = raw_temp1 / 10;
    int decimal = raw_temp1 % 10;  
    // Handle negative temperatures correctly (e.g., -1.5°C)
    if (decimal < 0) {
        decimal = -decimal; 
    }
    // Format Line 1 using simple integers (%d)
    // \xDF is the degree symbol
    if (raw_temp1 < 0 && whole == 0) {
        sprintf(line1_buffer, "Temp1: -%d.%d\xDF""C", whole, decimal);
    } else {
        // Normal printing handles positive numbers and numbers <= -1.0 automatically
        sprintf(line1_buffer, "Temp1: %d.%d\xDF""C", whole, decimal);
    }   
    // Format Line 2 
    whole   = raw_temp2 / 10;
    decimal = raw_temp2 % 10;    
    if (decimal < 0) {
        decimal = -decimal; 
    }
    // Format Line 1 using simple integers (%d)
    // \xDF is the degree symbol
    if (raw_temp2 < 0 && whole == 0) {
        sprintf(line2_buffer, "Temp2: -%d.%d\xDF""C", whole, decimal);
    } else {
        // Normal printing handles positive numbers and numbers <= -1.0 automatically
        sprintf(line2_buffer, "Temp2: %d.%d\xDF""C", whole, decimal);
    }   
    // ---- Send to LCD ----  
    // line 1
    lcd_cmd(0x80);
    for ( i = 0; line1_buffer[i] != '\0'; i++) {
        lcd_char(line1_buffer[i]);
    }
    // line 2
    lcd_cmd(0xC0);
    for ( i = 0; line2_buffer[i] != '\0'; i++) {
        lcd_char(line2_buffer[i]);
    }
}
void ClearScreen (void)
{   lcd_cmd(0x01); 
    delay_ms(LCD_CDE);    // Go home 
}
void lcd_print(char *str)
{
    while (*str)
    {
       lcd_char( *str++);
    }
}
void lcd_gotoxy(unsigned char x, unsigned char y)
{
    unsigned char address;
    
    if (y == 0)
    {
        address = 0x00 + x; // Line 1 address space
    }
    else
    {
        address = 0x40 + x; // Line 2 address space
    }
    
    // Command to set DDRAM address is (0x80 | address)
    lcd_cmd (0x80 | address);
}

// Clock Variables Save every 5min
void ReloadFromEEprom (void)
{
}

void lcd_power_off(void)
{
    set_backlight_pwm(0);       // Kill the LED backlight
//    lcd_write_reg(0x00, 0x08);  // ST7032I command: Entire display off
}

void lcd_power_on(void)
{
//    lcd_write_reg(0x00, 0x0C);  // ST7032I command: Display back on
//    set_backlight_pwm(backlight_saved); // Restore brightness
}
char ReadCPUtemperature (void)
{   
        uint32_t temp;
    unsigned char loop;
    unsigned int data; 
    
    // Read the factory 16-bit unsigned variables from the Signature Row
    uint16_t sigrow_offset = SIGROW.TEMPSENSE1; 
    uint16_t sigrow_slope  = SIGROW.TEMPSENSE0;  
    
    char CPU_temperature; 
    
    // Backup your existing NTC peripheral configurations safely
    unsigned char old_vref  = (unsigned char)VREF.ADC0REF;
    // 1. Select the internal 2.048V reference as required by the datasheet
    VREF.ADC0REF = (unsigned char)VREF_REFSEL_2V048_gc;  
    // Flush channel sample to clear the analog multiplexer tree stage
    adc0_read(ADC_MUXPOS_TEMPSENSE_gc);   
    
    // 5. Clean 8x sampling aggregation loop
    temp = 0;
    for (loop = 0; loop < 8; loop++)
    {   
        temp += adc0_read(ADC_MUXPOS_TEMPSENSE_gc);   
    }
    data = (unsigned int)(temp / 8);
    
    // 6. EXACT DATASHEET MATHEMATICAL ORDER OF OPERATIONS
    // Formula: T = (Offset - (ADC_Result * Slope)) / 4096
    
    temp = (uint32_t)data * (uint32_t)sigrow_slope; // Step A: Multiply reading by slope first!
    temp = (uint32_t)sigrow_offset - temp;          // Step B: Subtract that total from the offset
    
    temp += 2048;  // Step C: Rounding correction factor (SCALING_FACTOR / 2 where factor is 4096)
    temp >>= 12;   // Step D: Shift down by 12 bits (Equivalent to dividing by 4096)

    // Convert absolute Kelvin scale back to human-readable Celsius
    CPU_temperature = (char)(temp - 273);     
    
    // 7. RESTORE PERMANENT SYSTEM REGISTERS BACK FOR YOUR NTC SENSORS
    VREF.ADC0REF  = old_vref;    
    return CPU_temperature; 
}
unsigned int ReadSystemVCC(void)
{
    unsigned long total_adc;
    unsigned char loop;
    unsigned int  adc_average;
    unsigned int  vdd_mv;
    
    // 1. BACKUP YOUR CURRENT NTC ADC SETTINGS
    unsigned char old_vref  = (unsigned char)VREF.ADC0REF;
    unsigned char old_ctrlc = ADC0.CTRLC;
    unsigned char old_ctrlb = ADC0.CTRLB;
    unsigned char old_samp  = ADC0.SAMPCTRL;

    // 2. CONFIGURE FIXED 1.024V REFERENCE (As you requested!)
    VREF.ADC0REF = (unsigned char)VREF_REFSEL_1V024_gc;  
    
    ADC0.CTRLC   = ADC_PRESC_DIV16_gc;    // Slow stable ADC clock
    ADC0.CTRLB   = ADC_SAMPNUM_NONE_gc;   // No oversampling
    ADC0.SAMPCTRL = 0x1F;                 // Max sampling stabilization holding time
    delay_us(200);                        // Wait for reference to stabilize

    // 3. SAMPLE THE VDDDIV10 CHANNEL (0x44)
    adc0_read(ADC_MUXPOS_VDDDIV10_gc);    // Dummy read to clear mux channel tree
    
    total_adc = 0;
    for (loop = 0; loop < 8; loop++)
    {   
        total_adc += adc0_read(ADC_MUXPOS_VDDDIV10_gc); // 8x smoothing loop
    }
    adc_average = (unsigned int)(total_adc / 8);

    // 4. RESTORE SYSTEM BACK TO ORIGINAL NTC MODIFIERS IMMEDIATELY
    VREF.ADC0REF  = old_vref;
    ADC0.CTRLC    = old_ctrlc;
    ADC0.CTRLB    = old_ctrlb;
    ADC0.SAMPCTRL = old_samp;

    // 5. CONVERSION FORMULA (12-bit ADC, 1024mV Ref, Multiplied by 10)
    // Input Voltage = (ADC_Count * 1024mV) / 4096 steps = ADC_Count / 4
    // VDD = Input Voltage * 10 
    // Simplified: VDD_mV = (ADC_Count * 10) / 4 -> Which is: ADC_Count * 2.5
    // To do it in integers: (ADC_Count * 25) / 10
    vdd_mv = (unsigned int)(((unsigned long)adc_average * 25UL) / 10UL);

    return vdd_mv; 
}

/*
// ============================================================
// BASELINE MEASUREMENT — constants & globals
// ============================================================
#define TEMP_BUF_SIZE  4      // filter depth: 4 minutes

unsigned char ActiveSensor  = 1;     // 1 or 2: sensor currently used
unsigned int  TempBuf[TEMP_BUF_SIZE] = {0,0,0,0};  // rolling filter buffer
unsigned int  TempFiltered   = 0;     // filtered temp of active sensor
unsigned int  MaxTemp        = 0;     // cycle maximum (filtered)
unsigned int  MinTemp        = 0;     // cycle minimum (filtered)
unsigned char CycleActive    = 0;     // 1 = measurement in progress
unsigned int  CycleOnMin     = 0;     // minutes heating this cycle
unsigned int  CycleOffMin    = 0;     // minutes off/blocked this cycle
unsigned int  CycleMinutes   = 0;     // total minutes this cycle
unsigned int  BaseOnMin      = 0;     // learned baseline: on-time
unsigned int  BaseOffMin     = 0;     // learned baseline: off-time
unsigned int  RiseMin        = 0;     // minutes temp rising (slope tracker)
unsigned int  FallMin        = 0;     // minutes temp falling (slope tracker)
unsigned int  LastTemp       = 0;     // previous filtered temp

volatile unsigned char MinuteTick = 0; // set by ISR, consumed by main loop

// ============================================================
// ACTIVE SENSOR PICK — Mode + Override + date
// returns 1 = Sensor 1 (winter), 2 = Sensor 2 (summer)
// ============================================================
unsigned char GetActiveSensor(void)
{
    unsigned char mon;       // working month (may be shifted)
    unsigned char day;      // working day

    // 1. COMBI with Winter always: S1 permanently
    if (Mode == MODE_COMBI)
    {
        if (Override == 0)
        {
            return 1;
        }
    }

    // 2. Automatic by date
    mon = month;
    day = day_of_month;

    // 3. Shift the test date for +1W / +1M (Combi only)
    if (Mode == MODE_COMBI)
    {
        if (Override == 2)                      // +1 WEEK: test 7 days earlier
        {
            if (day > 7)
            {
                day = day - 7;
            }
            else                                // borrow from previous month
            {
                if (mon == 1) mon = 12;
                else          mon--;
                day = day + month_lengths[mon];
            }
        }
        else if (Override == 3)                 // +1 MONTH: test 1 month earlier
        {
            if (mon == 1) mon = 12;
            else          mon--;
        }
    }

    // 4. Date test: summer = Apr 2 .. Aug 31
    if ((mon >= 5) && (mon <= 8))               // May..Aug
    {
        return 2;
    }
    if ((mon == 4) && (day >= 2))               // Apr 2..30
    {
        return 2;
    }

    return 1;                                  // winter
}

// ============================================================
// MEASUREMENT TICK — call from main loop, once per MinuteTick
// tracks active sensor: filter, min/max, durations, slope
// ============================================================
void MeasureTick(void)
{
    unsigned char i;
    unsigned int  temp_sample;
    unsigned long sum;                          // filter sum, 4 x 65535 fits not -> long safe

    // ---- 1. pick active sensor & sample it ----
    ActiveSensor = GetActiveSensor();
    if (ActiveSensor == 2)  temp_sample = CurrentTemp2;
    else                    temp_sample = CurrentTemp1;

    // ---- 2. rolling filter (4-minute average) ----
    for (i = (TEMP_BUF_SIZE - 1); i > 0; i--)
    {
        TempBuf[i] = TempBuf[i - 1];
    }
    TempBuf[0] = temp_sample;

    sum = 0;
    for (i = 0; i < TEMP_BUF_SIZE; i++)
    {
        sum = sum + TempBuf[i];
    }
    TempFiltered = (unsigned int)(sum / TEMP_BUF_SIZE);

    // ---- 3. min/max with cycle-start reset ----
    if (CycleActive == 0)
    {
        CycleActive = 1;
        MaxTemp = TempFiltered;     // start both at current value
        MinTemp = TempFiltered;
    }
    else
    {
        if (TempFiltered > MaxTemp) MaxTemp = TempFiltered;
        if (TempFiltered < MinTemp) MinTemp = TempFiltered;
    }

    // ---- 4. duration counters (condition decides which) ----
    if (State == STATE_SAVING) CycleOffMin++;
    else                       CycleOnMin++;

    CycleMinutes++;

    // ---- 5. slope tracker (thermostat inference, counter style) ----
    if (TempFiltered > LastTemp)       RiseMin++;
    else if (TempFiltered < LastTemp)  FallMin++;
    // equal -> neither: natural plateau
    LastTemp = TempFiltered;
}

*/