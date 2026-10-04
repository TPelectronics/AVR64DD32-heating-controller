#ifndef LCD_MENY
#define LCD_MENY

#define MAX_MENY_CHOICE 8

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

// ---- LOG HISTORY (RAM, push buffer, newest at index 0) ----
#define HIST_SIZE  50

extern unsigned int  HistOn[HIST_SIZE];
extern unsigned int  HistOff[HIST_SIZE];
extern unsigned int  HistSave[HIST_SIZE];
extern unsigned int  HistDay[HIST_SIZE];
extern unsigned char HistHour[HIST_SIZE];
extern unsigned char HistMin[HIST_SIZE];
extern unsigned char HistSvPct[HIST_SIZE];
extern unsigned char HistCount;        // how many valid records (0..50)
extern unsigned int CycleOnMin;    // minutes heating this cycle
extern unsigned int CycleOffMin;   // minutes off/blocked this cycle

// Routine

//static void refresh_clock_ui(void); // not realy for display but used in Run_clock_setting
void run_clock_setting_menu(void);
//unsigned char get_max_days(unsigned int y, unsigned char m);
void SetupDateSimple(void);
char DisplayMeny (void);
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
void HistPush(void);
void ViewLog(void);
#endif // BUTTONS_H