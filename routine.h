#ifndef BUTTONS_H
#define BUTTONS_H

#include <avr64dd32.h>   
#include "twi_init.h" 
//#include <twid_master.h>

// EEPROM variable:
extern eeprom unsigned char NewEEprom;
extern eeprom unsigned char SavedBacklight;
extern eeprom unsigned char SavedContrast; 
extern eeprom unsigned char SavedTimeOff;
extern eeprom unsigned char SavedMode;
extern eeprom unsigned char SavedTemp1_low;
extern eeprom unsigned char SavedTemp1_high;
extern eeprom unsigned char SavedTemp2_low;
extern eeprom unsigned char SavedTemp2_high;
extern eeprom unsigned int SavedYear;
extern eeprom unsigned char SavedMonth; 
extern eeprom unsigned char SavedDay;   
extern eeprom unsigned char eeprom_buffer[];  // EEPROM buffer  
extern eeprom unsigned char SavedOverride;   // 0=winter only, 1=seasonal, 2=+1w, 3=+1m
extern unsigned char current_buffer_index;  // Start with Buffer 0
extern unsigned char power_loss_flag;        // 0 = no power loss, 1 = power loss detected  
//extern eeprom unsigned char SavedHours[30];
//extern eeprom unsigned char SavedMinutes[30];    
extern unsigned char Temp1_low;    // winter band min  (x10, e.g. 185 = 18.5C)
extern unsigned char Temp1_high;   // winter band max
extern unsigned char Temp2_low;    // summer band min
extern unsigned char Temp2_high;   // summer band max  
extern eeprom unsigned char SavedDiffW;    // winter diff, x10 deg (5..50)
extern eeprom unsigned char SavedDiffS;   // summer diff, x10 deg (5..50)
extern unsigned char DiffW;
extern unsigned char DiffS;  

// ---- Differential settings ----
#define DIFF_MIN     5    // 5 = 0.5 C  (x10 internal, matches 0.5C sensor step)
#define DIFF_MAX     50   // 50 = 5.0 C
#define DIFF_STEP    5    // 5 = 0.5 C per key press
#define Default_DiffW  15   // 1.5 C heating
#define Default_DiffS  15   // 1.5 C cooling (tune per aircon type)

#define SavedEEprom 0xA5
#define    Default_Backlight    0x80 
#define    Default_Contrast     31  // from 32 to 63 
#define    Default_TimeOff      10
#define    Default_Mode         0
#define    Default_Temp1_low    30
#define    Default_Temp1_high   35
#define    Default_Temp2_low    30
#define    Default_Temp2_high   35
#define    Default_Year         2026
#define    Default_Month        1
#define    Default_Day          1

// --- Pin Bit-Mask Definitions ---
#define SW_DOWN_PIN   PIN3_bm   // PD3
#define SW_LEFT_PIN   PIN4_bm   // PD4
#define SW_RIGHT_PIN  PIN6_bm   // PD5
#define SW_OK_PIN     PIN5_bm   // PD6
#define SW_UP_PIN    PIN7_bm   // PD7  
#define SW_PORT_MASK (SW_DOWN_PIN | SW_LEFT_PIN | SW_RIGHT_PIN | SW_OK_PIN | SW_UP_PIN)

// --- Helper Macros to Read Button States ---
// Returns 0 if pressed (assuming active-low with internal pull-up)
#define READ_SW_DOWN()   (PORTD.IN & SW_DOWN_PIN)
#define READ_SW_LEFT()   (PORTD.IN & SW_LEFT_PIN)
#define READ_SW_RIGHT()  (PORTD.IN & SW_RIGHT_PIN)
#define READ_SW_OK()     (PORTD.IN & SW_OK_PIN)
#define READ_SW_UP()    (PORTD.IN & SW_UP_PIN) 

#define KEY_DOWN    0x01
#define KEY_LEFT    0x02
#define KEY_RIGHT   0x04
#define KEY_OK      0x08
#define KEY_UP      0x10  
#define KEY_NONE    0x00
 
unsigned char   ScanKeyBoard ( void);
//                                                                                  
// --- Pin Definitions ---
// Port F Outputs
#define LCD_RST_PIN       PIN2_bm
#define RELAY1_PIN        PIN3_bm
#define RELAY2_PIN        PIN4_bm

// --- Macro Controls for GPIO ---
#define LCD_RST_HIGH()    (PORTF.OUTSET = LCD_RST_PIN)
#define LCD_RST_LOW()     (PORTF.OUTCLR = LCD_RST_PIN)

#define RELAY1_ON()       (PORTF.OUTSET = RELAY1_PIN)
#define RELAY1_OFF()      (PORTF.OUTCLR = RELAY1_PIN)
#define RELAY1_TOGGLE()   (PORTF.OUTTGL = RELAY1_PIN)

#define RELAY2_ON()       (PORTF.OUTSET = RELAY2_PIN)
#define RELAY2_OFF()      (PORTF.OUTCLR = RELAY2_PIN)    
#define RELAY2_TOGGLE()   (PORTF.OUTTGL = RELAY2_PIN)    

// PWM
#define PWM_PERIOD_VAL    1599   
// Port C (PWM Outputs)
#define LCD_BK_PIN        PIN0_bm  // WO0
#define LED_ALM_PIN       PIN1_bm  // WO1
#define LED_SYS_PIN       PIN2_bm  // WO2      

#define NTC1_PORT       ADC_MUXPOS_AIN1_gc   
#define NTC2_PORT       ADC_MUXPOS_AIN2_gc  

// diverse timer:
#define MAX_MINUT_TO_SAVE   15
// save eeprom.
// --- Buffer Structure ---
// Define parameters for buffer size and EEPROM addresses
#define NUM_BUFFERS 32          // Number of buffers
#define BUFFER_SIZE 1           // Size of each buffer in bytes (hours + minutes)
#define EEPROM_BUFFER_SIZE (NUM_BUFFERS * BUFFER_SIZE)  // Total EEPROM size for buffers  
#define EMPTY_MARK  0xFF

// Function declarations
void save_time(unsigned char hours, unsigned char minutes);
unsigned char recover_last_time(unsigned char *hours, unsigned char *minutes);


//extern unsigned int FlagRTC;
extern unsigned char PWM_alarm;
extern unsigned char PWM_sys; 

// Global time tracking variables
extern volatile unsigned char seconds ;
extern volatile unsigned char minutes ;
extern volatile unsigned char hours   ;
extern volatile unsigned int  days    ; 
//extern volatile unsigned char  save_countdown;
//extern volatile unsigned char  SetFlagToSave;  
extern volatile unsigned char NewMinutes;
extern volatile unsigned char NewHours; 
extern volatile unsigned char NewDays;  
extern volatile unsigned char SaveCountdown;  
extern volatile unsigned char SetFlagToSave; 
extern volatile unsigned char TurnOffCountdown;  
extern volatile unsigned char SetFlagTurnOff;


// Calendar variables derived from the raw 'days' counter
extern volatile unsigned char day_of_month ; // 1 to 31
extern volatile unsigned char month        ; // 1 = Jan, 2 = Feb, ..., 12 = Dec
extern volatile unsigned char leap_year_cycle ; // 0, 1, 2 = Normal years (28 days). 3 = Leap year (29 days).
extern volatile unsigned int  current_year ; // Input any 4-digit calendar year
// Array containing the length of each month (Index 0 is unused to match Month 1-12 layout)
// Note: Month 2 (February) starts at 28 and is dynamically modified in code
extern unsigned char month_lengths[13];
unsigned char is_leap_year(unsigned int year);

 
void set_backlight_pwm(unsigned char duty);
void set_system_led_pwm(unsigned char duty);
void set_alarm_led_pwm(unsigned char duty);  
 
extern unsigned char BackLight;
extern unsigned char LCD_Contrast;
extern unsigned char Mode;
extern unsigned char TimeOff;       
extern unsigned char MainScreen;   // selected screen 0..2 — owned by main loop
extern unsigned char LastSeconds;   // snapshot for 1s redraw — owned by main loop       
extern unsigned int CurrentTemp1,CurrentTemp2;   
extern unsigned char LastScreen; // last drawn screen (0xFF = force first clear)   

#define STATE_HEATING  0     // default/fail-safe: relay off, NC closed
#define STATE_SAVING   1     // relay energised, heating blocked
#define STATE_COOLING  2     // summer/cooling counterpart

extern unsigned char State;   // optimiser writes this later; 0 = heating at boot
/**
 * @brief Initializes Port D pins for the switches with internal pull-ups.
 * Call this once during system startup.
 */
void Buttons_Init(void);                   

// LCD
// --- ST7032I I2C Definitions ---
#define ST7031_I2C_ADDR  0x7C>>1   // 0x3E shifted left 1 bit for Write (0x3E << 1 = 0x7C)
#define ST7031_COMMAND      0x00   // Control byte for instructions
#define ST7031_DATA         0x40   // Control byte for character data   
#define LCD_CDE     2   // delay in "ma" between commande

extern TWI_MASTER_INFO_t twi0_master;
// --- Function Prototypes ---
//void lcd_write_reg(unsigned char control_byte, unsigned char data_byte);  
void refresh_lcd_ints(int raw_temp, unsigned char hours, unsigned char minutes, unsigned char seconds);
void refresh_lcd_temps(int raw_temp1,int raw_temp2);
void lcd_print(char *str);   
void lcd_gotoxy(unsigned char x, unsigned char y);
void lcd_power_off(void);
void lcd_power_on(void); 
void ClearScreen (void);
void lcd_init(void);    
void lcd_cmd(unsigned char cmd);
void lcd_char(unsigned char data);

// NTC
int get_ntc_temperature(unsigned int adc_val);
extern flash unsigned int ntc_table[];
#define START_TEMP -200   // Table starts at -20.0 °C (multiplied by 10)
#define TEMP_STEP  5      // 0.5 °C steps (multiplied by 10)

// Read CPU core temperature
// Exact production memory addresses for the AVR64DD32 signature rows
#ifndef SIGROW_TEMPSENSE0_ADDR
#define SIGROW_TEMPSENSE0_ADDR (0x1120) // Slope / Gain correction factor (16-bit)
#define SIGROW_TEMPSENSE1_ADDR (0x1122) // Offset correction factor (16-bit)
#endif

//int GetCPUTemperature(void);
char ReadCPUtemperature (void);  
unsigned int ReadSystemVCC(void);  

// time saving control
void LoadFromEEprom (void);
void SetLCD_Contrast (unsigned char value); 
// Override; 0=winter only, 1=seasonal, 2=+1w, 3=+1m
extern unsigned char Override;
#endif // 