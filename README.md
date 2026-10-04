# History Log Integration — 2026

## What was added
- RAM ring buffer of the last 50 cycles (HIST_SIZE), newest at index 0
- HistPush():  called automatically on every State change (MeasureTick end)
- ViewLog():   setup menu entry 8 "Log view", UP/DOWN scroll, hold OK to exit
- Records: On-minutes, Off-minutes, State, day, hour:min, saving %

## Files changed
- LCD_meny.h : HIST block (unsigned int HistOn/Off/Save/Day), MAX_MENY_CHOICE 8, ViewLog proto
- Routine.c  : array definitions, HistCount=0, LastState=0xFF, MeasureTick hook:
               if (State != LastState) { HistPush(); LastState = State; }
- LCD_meny.c : HistPush() + ViewLog() at end of file; case 8 "Log view" in DisplayMeny()
- main.c     : case 8: ViewLog(); in setup switch
- routine.h  : extern CycleOnMin/CycleOffMin, extern LastState

## Design notes
- CycleOnMin/CycleOffMin are unsigned int: full range, NO 255 cap, NO casts
- HistSvPct = (CycleOffMin * 100UL) / (On+Off)  -> unsigned char %
- Push resets CycleOnMin/CycleOffMin for the next cycle

## Test plan (tomorrow)
1. Boot -> first record L00 appears automatically (LastState=0xFF trick)
2. OK -> menu -> "Log view" -> OK: browse with UP/DOWN, hold OK to exit
3. Trigger a state change -> verify new record L00, old ones shifted +1
