#ifndef LCD_MENY
#define LCD_MENY

#define MAX_MENY_CHOICE 7

#define OK_EXIT_COUNT  5   // ~30 x 10ms = 0.3 s hold (tune to taste)
#define CONTRAST_SETUP_MAX  45   // allowed user range: 0..45
#define CONTRAST_SETUP_STEP 1    // step per key press

#define MODE_COMBI  0
#define MODE_VALVE  1

// ---- Turn-off time settings ----
// TimeOff = minutes of inactivity before LCD backlight/display turns off.
// 0 = never turn off.
#define TIMEOFF_MAX   60   // allowed range: 0..60 minutes
#define TIMEOFF_STEP  1    // step per key press (minutes)


// Routine

//static void refresh_clock_ui(void); // not realy for display but used in Run_clock_setting
void run_clock_setting_menu(void);
//unsigned char get_max_days(unsigned int y, unsigned char m);
void SetupDateSimple(void);
char    DisplayMeny (void);
void LoadCustomLCDChars(void);
void LoadCustomClockChars(void);
void PrintRotateClock ( unsigned char index, unsigned char pos, unsigned char line);
void LoadCustomBlock(void) ;
void PrintBlock ( unsigned char index, unsigned char pos, unsigned char line);

void SetupBacklight (void);
void SetupContrast(void);
void SetupOverride(void);
void SetupMode(void);
void SetupDiffSummer(void);
void SetupDiffWinter(void);
void SetupTimeOff(void);
void MainDisplay(unsigned char scr);
#endif // BUTTONS_H