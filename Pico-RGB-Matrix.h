/* ============================================================================================================================================================= *\
   Pico-RGB-Matrix.h
   St-Louys Andre - August 2022
   astlouys@gmail.com
   Revision 02-MAR-2024
   Langage: C++ with arm-none-eabi

   Raspberry Pi Pico Firmware to drive the Waveshare Pico-RGB-Matrix.
   From an original software version 1.00 by Waveshare
   Released under 3-Clause BSD License.

   NOTE:
   THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
   WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
   TIME. AS A RESULT, WAVESHARE OR THE AUTHOR SHALL NOT BE HELD LIABLE FOR ANY
   DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
   FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
   CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
   ALSO, TAKE NOTE THAT THE AUTHOR IS NOT A WAVESHARE EMPLOYEE.
\* ============================================================================================================================================================= */

#ifndef __PICO_RGB_MATRIX_H
#define __PICO_RGB_MATRIX_H

#include "stdint.h"

/* --------------------------------------------------------------------------------------------------------------------------- *\
                                                      General definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */
typedef char          CHAR;
typedef unsigned char UCHAR;
typedef int8_t        INT8;
typedef int16_t       INT16;
typedef int32_t       INT32;
typedef int64_t       INT64;
typedef unsigned int  UINT;    // processor-optimized.
typedef uint8_t       UINT8;
typedef uint16_t      UINT16;
typedef uint32_t      UINT32;
typedef uint64_t      UINT64;

#define FLAG_OFF     0x00
#define FLAG_ON      0x01
#define FLAG_DAY     0x02
#define FLAG_AUTO    0x03
#define FLAG_NIGHT   0x04

#define FALSE        0
#define TRUE         1

#define TYPE_PICO                 0x01      // microcontroller is a Pico
#define TYPE_PICOW                0x02      // microcontroller is a Pico W

#define AIRCR_Register (*((volatile UINT32 *)(PPB_BASE + 0x0ED0C)))
/* --------------------------------------------------------------------------------------------------------------------------- *\
                                                   End of general definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */





/* --------------------------------------------------------------------------------------------------------------------------- *\
                                                      Debug flag definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */
/* Bit masks defining which logical sections of the code will be debugged. */
#define DEBUG_NONE        0x0000000000000000  // nothing to debug.

#define DEBUG_ALARM       0x0000000000000001  // debug alarm algorithm.
#define DEBUG_BLINK       0x0000000000000002  // debug blinking mechanism
#define DEBUG_BLUETOOTH   0x0000000000000004  // debug bluetooth communications.
#define DEBUG_BOX         0x0000000000000008  // debug exploding windows.
#define DEBUG_BRIGHTNESS  0x0000000000000010  // debug PWM brightness algorithm.
#define DEBUG_BUTTON      0x0000000000000020  // debug local button handling.
#define DEBUG_CORE        0x0000000000000040  // debug Pico's core 1 thread.
#define DEBUG_DS3231      0x0000000000000080  // debug DS3231 real-time IC.
#define DEBUG_EVENT       0x0000000000000100  // debug calendar event algorithm.
#define DEBUG_FLASH       0x0000000000000200  // debug flash memory configuration / operations.
#define DEBUG_FLOW        0x0000000000000400  // debug program sequence flow.
#define DEBUG_FUNCTION    0x0000000000000800  // debug remote control <Functions> operation.
#define DEBUG_IR          0x0000000000001000  // debug infrared remote control reception and decoding.
#define DEBUG_MATRIX      0x0000000000002000  // debug matrix display operations.
#define DEBUG_NTP         0x0000000000004000  // debug Network Time Protocol.
#define DEBUG_PWM         0x0000000000008000  // debug PWM behavior.
#define DEBUG_REMINDER    0x0000000000010000  // debug reminder mechanism.
#define DEBUG_SCROLL      0x0000000000020000  // debug scroll mechanism.
#define DEBUG_SOUND_QUEUE 0x0000000000040000  // debug sound queue engine.
#define DEBUG_STARTUP     0x0000000000080000  // debug startup sequence.
#define DEBUG_SUMMER_TIME 0x0000000000100000  // debug summer-time related logic.
#define DEBUG_TEST        0x0000000000200000  // debug test section.
#define DEBUG_WATCHDOG    0x0000000000400000  // debug watchdog behavior.
#define DEBUG_WIFI        0x0000000000800000  // debug WiFi communications.
#define DEBUG_WINDOW      0x0000000001000000  // debug window algorithm.
// #define DEBUG_ 0x0000000002000000  //
// #define DEBUG_ 0x0000000004000000  //
// #define DEBUG_ 0x0000000008000000  //
// #define DEBUG_ 0x0000000010000000  //
// #define DEBUG_ 0x0000000020000000  //
// #define DEBUG_ 0x0000000040000000  //
// #define DEBUG_ 0x0000000080000000  //
// #define DEBUG_ 0x0000000100000000  //
// #define DEBUG_ 0x0000000200000000  //
// #define DEBUG_ 0x0000000400000000  //
// #define DEBUG_ 0x0000000800000000  //
// #define DEBUG_ 0x0000001000000000  //
// #define DEBUG_ 0x0000002000000000  //
// #define DEBUG_ 0x0000004000000000  //
// #define DEBUG_ 0x0000008000000000  //
// #define DEBUG_ 0x0000010000000000  //
// #define DEBUG_ 0x0000020000000000  //
// #define DEBUG_ 0x0000040000000000  //
// #define DEBUG_ 0x0000080000000000  //
// #define DEBUG_ 0x0000100000000000  //
// #define DEBUG_ 0x0000200000000000  //
// #define DEBUG_ 0x0000400000000000  //
// #define DEBUG_ 0x0000800000000000  //
// #define DEBUG_ 0x0001000000000000  //
// #define DEBUG_ 0x0002000000000000  //
// #define DEBUG_ 0x0004000000000000  //
// #define DEBUG_ 0x0008000000000000  //
// #define DEBUG_ 0x0010000000000000  //
// #define DEBUG_ 0x0020000000000000  //
// #define DEBUG_ 0x0040000000000000  //
// #define DEBUG_ 0x0080000000000000  //
// #define DEBUG_ 0x0100000000000000  //
// #define DEBUG_ 0x0200000000000000  //
// #define DEBUG_ 0x0400000000000000  //
// #define DEBUG_ 0x0800000000000000  //
// #define DEBUG_ 0x1000000000000000  //
// #define DEBUG_ 0x2000000000000000  //
// #define DEBUG_ 0x4000000000000000  //
// #define DEBUG_ 0x8000000000000000  //
/* --------------------------------------------------------------------------------------------------------------------------- *\
                                                  End of debug flag definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */





/* --------------------------------------------------------------------------------------------------------------------------- *\
                                               Brightness control related definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */
/* Number of seconds over which to calculate ambient light average value. */
#define BRIGHTNESS_HYSTERESIS_SECONDS  120

/* PWM - "Pulse Wide Modulation" types. */
#define PWM_ID_LO_LIMIT       0x00
#define PWM_ID_BRIGHTNESS     0x00
#define PWM_ID_HI_LIMIT       0x01  // must be one-more than the last valid PWM ID (passive buzzer PWM will ba added later).

#define PWM_LO_LIMIT          1300  //  lowest possible value for PWM Level (highest display brightness).
#define PWM_HI_LIMIT          2000  // highest possible value for PWM Level  (lowest display brightness).

#define LIGHT_LO_LIMIT         300  // this value is considered to be the  lowest light reading value (although it could theorically go down to 0).
#define LIGHT_HI_LIMIT        3500  // this value is considered to be the highest light reading value (although it could theorically go up to 4096).
#define LIGHT_RANGE           3200  // prevent a recalculation every time.


/* Pulse Wide Modulation (PWM) structure definitions. */
struct pwm
{
  UINT8  OnOff;
  UINT8  Gpio;
  UINT8  Channel;
  UINT8  DutyCycle;
  UINT8  Slice;
  UINT16 Level;
  UINT16 Wrap;
  UINT32 Clock;
  UINT32 Frequency;
  float  ClockDivider;
};
/* --------------------------------------------------------------------------------------------------------------------------- *\
                                         End of brightness control related definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */





/* --------------------------------------------------------------------------------------------------------------------------- *\
                                                  Button specific definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */
#define BUTTON_TIME_OUT  30  // number of seconds to wait for button press (either local or remote).

#define BUTTON_LONG_PRESS_TIME  300000l  // 300000 usec (1/3rd of a second) or more is considered a "long" button press.

#define BUTTON_NONE        0x00
#define BUTTON_DOWN        0x01
#define BUTTON_SET         0x02
#define BUTTON_UP          0x03

#define BUTTON_DOWN_LONG   0x04
#define BUTTON_SET_LONG    0x05
#define BUTTON_UP_LONG     0x06

#define BUTTON_BUFFER_SIZE   10
/* --------------------------------------------------------------------------------------------------------------------------- *\
                                               End of button specific definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */





/* --------------------------------------------------------------------------------------------------------------------------- *\
                                                       Color definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */
#define BLACK      0x00
#define BLUE       0x01
#define GREEN      0x02
#define CYAN       0x03
#define RED        0x04
#define MAGENTA    0x05
#define YELLOW     0x06
#define WHITE      0x07
/* --------------------------------------------------------------------------------------------------------------------------- *\
                                                    End of color definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */





/* --------------------------------------------------------------------------------------------------------------------------- *\
                                              Date and time related definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */
#define H12  1  // time display mode is 12 hours.
#define H24  2  // time display mode is 24 hours.

#define SUN 0
#define MON 1
#define TUE 2
#define WED 3
#define THU 4
#define FRI 5
#define SAT 6

#define MORNING   0
#define AFTERNOON 1
#define EVENING   2
#define NIGHT     3

#define DELTA_TIME  60000000ll

#define MAX_ONE_SECOND_INTERVALS   48

#define WATCHDOG_SECONDS          320  // number of seconds before triggering a restart - software reset (better if it is a multiple of 32).


/* DST_COUNTRY valid choices (see details in User Guide). */
// #define DST_DEBUG                    /// this define to be used only for intensive DST debugging purposes.
#define DST_LO_LIMIT        0           // this specific define only to make the logic easier in the code.
#define DST_NONE            0           // there is no "Daylight Saving Time" in user's country.
#define DST_AUSTRALIA       1           // daylight saving time for most of Australia.
#define DST_AUSTRALIA_HOWE  2           // daylight saving time for Australia - Lord Howe Island.
#define DST_CHILE           3           // daylight saving time for Chile.
#define DST_CUBA            4           // daylight saving time for Cuba.
#define DST_EUROPE          5           // daylight saving time for European Union.
#define DST_ISRAEL          6           // daylight saving time for Israel.
#define DST_LEBANON         7           // daylight saving time for Lebanon.
#define DST_MOLDOVA         8           // daylight saving time for Moldova.
#define DST_NEW_ZEALAND     9           // daylight saving time for New Zealand.
#define DST_NORTH_AMERICA  10           // daylight saving time for most of Canada and United States.
#define DST_PALESTINE      11           // daylight saving time for Palestine.
#define DST_PARAGUAY       12           // daylight saving time for Paraguay.IR_DISPLAY_GENERIC
#define DST_HI_LIMIT       13           // to make the logic easier in the firmware.


/* Structure to contain time stamp under "human" format instead of "tm" standard. */
struct human_time
{
  UINT8 FlagDst;
  UINT8 Hour;
  UINT8 Minute;
  UINT8 Second;
  UINT8 DayOfWeek;
  UINT8 DayOfMonth;
  UINT8 Month;
  UINT16 Year;
  UINT16 DayOfYear;
};


struct dst_parameters
{
  UINT8  StartMonth;
  UINT8  StartDayOfWeek;
  int8_t StartDayOfMonthLow;
  int8_t StartDayOfMonthHigh;
  UINT8  StartHour;
  UINT16 StartDayOfYear;
  UINT8  EndMonth;
  UINT8  EndDayOfWeek;
  int8_t EndDayOfMonthLow;
  int8_t EndDayOfMonthHigh;
  UINT8  EndHour;
  UINT16 EndDayOfYear;
  UINT8  ShiftMinutes;
}DstParameters[25];
/* --------------------------------------------------------------------------------------------------------------------------- *\
                                           End of date and time related definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */





/* --------------------------------------------------------------------------------------------------------------------------- *\
                                                    DS3231 related definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */
#define SDA                          6
#define SCL                          7
#define I2C_PORT                  i2c1

#define DS3231_ADDRESS            0x68
#define WRITE                        0
#define READ                         1

#define DS3231_CTRL_OSCILLATOR    0x80
#define DS3231_CTRL_SQUAREWAVE_BB 0x40
#define DS3231_CTRL_TEMPCONV      0x20

#define DS3231_ADDR_TIME          0x00
#define DS3231_ADDR_CONTROL       0x0E
#define DS3231_ADDR_STATUS        0x0F
#define DS3231_ADDR_AGING         0x10
#define DS3231_ADDR_TEMP          0x11

#define DS3231_12HOUR_FLAG        0x40
#define DS3231_12HOUR_MASK        0x1F
#define DS3231_PM_FLAG            0x20
#define DS3231_MONTH_MASK         0x1F

#define DS3231_ADDR_TIME_SEC      0x00
#define DS3231_ADDR_TIME_MIN      0x01
#define DS3231_ADDR_TIME_HOUR     0x02
#define DS3231_ADDR_TIME_WDAY     0x03
#define DS3231_ADDR_TIME_MDAY     0x04
#define DS3231_ADDR_TIME_MON      0x05
#define DS3231_ADDR_TIME_YEAR     0x06
/* --------------------------------------------------------------------------------------------------------------------------- *\
                                                 End of DS3231 related definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */





/* --------------------------------------------------------------------------------------------------------------------------- *\
                                       Flash memory configuration data related definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */
/* Polynom used for CRC16 calculation. Different authorities use different polynoms:
   0x8005, 0x1021, 0x1DCF, 0x755B, 0x5935, 0x3D65, 0x8BB7, 0x0589, 0xC867, 0xA02B, 0x2F15, 0x6815, 0xC599, 0x202D, 0x0805, 0x1CF5 */
#define CRC16_POLYNOM           0x1021

/* Offset in Pico's 2 MB where to save RGB Matrix configuration 1 data. Starting at 2.00MB minus 4096 bytes (0x1000) - At the very end of flash. */
#define FLASH_CONFIG1_OFFSET  0x1FF000

/* Offset in Pico's 2 MB where to save RGB Matrix configuration 2 data. Starting at 2.00MB minus 2X 4096 bytes (0x2000).
   At the end of flash, just before configuration 1 data. */
#define FLASH_CONFIG2_OFFSET  0x1FE000

#define CELSIUS     1
#define FAHRENHEIT  2

/* -------------------- DST_COUNTRY valid choices (see details in User Guide). -------------------- */
// #define DST_DEBUG                    /// this define to be used only for intensive DST debugging purposes.
#define DST_LO_LIMIT        0           // this specific define only to make the logic easier in the code.
#define DST_NONE            0           // there is no "Daylight Saving Time" in user's country.
#define DST_AUSTRALIA       1           // daylight saving time for most of Australia.
#define DST_AUSTRALIA_HOWE  2           // daylight saving time for Australia - Lord Howe Island.
#define DST_CHILE           3           // daylight saving time for Chile.
#define DST_CUBA            4           // daylight saving time for Cuba.
#define DST_EUROPE          5           // daylight saving time for European Union.
#define DST_ISRAEL          6           // daylight saving time for Israel.
#define DST_LEBANON         7           // daylight saving time for Lebanon.
#define DST_MOLDOVA         8           // daylight saving time for Moldova.
#define DST_NEW_ZEALAND     9           // daylight saving time for New Zealand.
#define DST_NORTH_AMERICA  10           // daylight saving time for most of Canada and United States.
#define DST_PALESTINE      11           // daylight saving time for Palestine.
#define DST_PARAGUAY       12           // daylight saving time for Paraguay.IR_DISPLAY_GENERIC
#define DST_HI_LIMIT       13           // to make the logic easier in the firmware.


/* -------------------- Alarm related definitions. -------------------- */
#define ALARM_RING_FREQUENCY 10  // alarm ringer will be feed every "ALARM_RING_FREQUENCY" seconds.
#define MAX_ALARMS            9  // number of available alarms.

struct active_alarm
{
  INT16 CountDown;         // current remaining seconds of ringing for each alarm (when it has been triggered and not yet shut off by user).
  UINT32 PreviousTimer;    // timer value of this alarm's last ring (must not be in Alarm structure saved in flash).
};


struct alarm
{
  UINT8  FlagStatus;       // alarm status (On / Off).
  UINT8  Hour;             // target hour for alarm.
  UINT8  Minute;           // target minute for alarm.
  UINT8  DayMask;          // bitmask for target days-of-week for alarm.
  UCHAR  Message[40];      // alarm message (text to scroll on RGB matrix when alarm is triggered).
  UINT8  NumberOfScrolls;  // alarm message will scroll this number of times at each ringing.
  UINT16 NumberOfBeeps;    // number of beeps for each ring.
  UINT16 BeepMSec;         // number of msec for each beep.
  UINT16 RepeatPeriod;     // alarm beeps will repeat every so many seconds.
  UINT16 RingDuration;     // number of seconds for total beeps duration.
};


/* -------------------- Auto-scroll related definitions. -------------------- */
#define MAX_AUTO_SCROLLS   5  // maximum number of auto-scrolls available.
#define MAX_ITEMS         32  // maximum number of items that will be scrolled every time the period defeined is reached.

struct auto_scroll
{
  UINT16 Period;                     // period (or auto-repeat time) in minutes... for example, if "3", will scroll at xx:03 / xx:06 / xx:09, etc//.
  UINT16 FunctionId[MAX_ITEMS];      // function number to scroll (in the order they will be scrolled).
};



/* -------------------- Calendar events related definitions. -------------------- */
#define MAX_EVENTS        64  // number of calendar events to be set in a UINT64 bitmask.

struct event
{
  UINT8  Day;              // day of month.
  UINT8  Month;            // month.
  UINT16 Jingle;           // jingle id to play if not null.
  UCHAR  Message[41];  // text to scroll on display every half hour when reaching the target date.
};


/* -------------------- Type 1 Reminders related definitions. -------------------- */
#define MAX_REMINDERS1    40  // number of reminders of type 1.

/* NOTE: StartPeriod and EndPeriod represent the global overall period during which the other parameters will be checked.
         If the current time is outside of the period defined by StartPeriod and EndPeriod, a reminder1 will simply be ignored by the system.
         StartPeriod also corresponds to the time of the first alarm for this reminder.
         RingRepeatTimeSeconds represents the interval of time (in seconds) after which the alarm will sound again.
         RingDurationSeconds represents the period (in seconds) during which the alarm will sound at the intervals indicated above for this specific iteration.
         NextReminderDelaySeconds represents the next time this reminder will be triggered again. When EndPeriod is reached, NextReminderDelaySeconds is added
         to StartPeriod, EndPeriod and NextRingUnixTime and saved to flash. */
/* NOTE: Structure Reminder1 - based on struct reminder1 - will be kept and updated in flash memory. */
struct reminder1
{
  UINT64 StartPeriodUnixTime;
  UINT64 EndPeriodUnixTime;
  UINT64 RingRepeatTimeSeconds;
  UINT64 RingDurationSeconds;
  UINT64 NextReminderDelaySeconds;
  UCHAR  Message[51];
};

struct active_reminder1
{
  INT16 CountDown;         // current remaining seconds of ringing for each alarm (when it has been triggered and not yet shut off by user).
  UINT32 PreviousTimer;    // timer value of this alarm's last ring (must not be in Alarm structure saved in flash).
};



/* Structure containing the RGB Matrix configuration data being saved to flash memory.
   Those variables will be restored after a reboot and / or power failure. */
/* IMPORTANT: Version must always be the first element of the structure and
              CRC16   must always be the  last element of the structure. */
#define MAX_VERSION_DIGITS 8  // maximum number of digits in firmware version number.

struct flash_config1
{
  UCHAR  Version[8];               // firmware version number (format: "100.00a" - and including end-of-string).
  UINT8  FlagAutoBrightness;       // flag indicating we are in "Auto Brightness" mode.
  UINT16 BrightnessLoLimit;        // lowest  brightness setting when auto brightness is On (between 1 and 1000).
  UINT16 BrightnessHiLimit;        // highest brightness setting when auto brightness is On (between 1 and 1000).
  UINT16 BrightnessLevel;          // brightness intensity value when not in auto brightness mode (between 1 and 1000).
  UINT8  ChimeMode;                // chime mode (Off / On / Day).
  UINT8  ChimeTimeOn;              // hourly chime will begin at this hour.
  UINT8  ChimeTimeOff;             // hourly chime will be silent after this hour.
  UINT8  ChimeLightMode;           // half-hour light chime mode (Off / On / Day).
  UINT8  FlagButtonFeedback;       // flag for buttons audible feedback ("button-press" tone).
  UINT8  FlagIrFeedback;           // flag for remote control audible feedback ("remote button-press" tone).
  UINT8  FlagGoldenAge;            // help mode for old persons.
  UINT8  GoldenMorningStart;       // hour considered "morning start".
  UINT8  GoldenAfternoonStart;     // hour considered "afternoon start".
  UINT8  GoldenEveningStart;       // hour considered "evening start".
  UINT8  GoldenNightStart;         // hour considered "night start".
  UINT8  TimeDisplayMode;          // H24 or H12 hour format default value.
  UINT8  DSTCountry;               // specifies how to handle the daylight saving time (see User Guide).
  INT8   Timezone;                 // (in hours) value to add to UTC time (Universal Time Coordinate) to get the local time.
  UINT8  FlagSummerTime;           // flag indicating the current status (On or Off) of Daylight Saving Time / Summer Time (automatically managed by the system).
  UINT8  TemperatureUnit;          // CELSIUS or FAHRENHEIT default value.
  UINT8  WatchdogFlag;             // variable uses for watchdog mechanism.
  UINT8  WatchdogCounter;          // count the cumulative number of restart by watchdog.
  UINT8  Variable8FuturUse8;       // placeholder  8-bits variable reserved for future use.
  UINT8  Variable8FuturUse7;       // placeholder  8-bits variable reserved for future use.
  UINT8  Variable8FuturUse6;       // placeholder  8-bits variable reserved for future use.
  UINT8  Variable8FuturUse5;       // placeholder  8-bits variable reserved for future use.
  UINT8  Variable8FuturUse4;       // placeholder  8-bits variable reserved for future use.
  UINT8  Variable8FuturUse3;       // placeholder  8-bits variable reserved for future use.
  UINT8  Variable8FuturUse2;       // placeholder  8-bits variable reserved for future use.
  UINT8  Variable8FuturUse1;       // placeholder  8-bits variable reserved for future use.
  UINT16 Variable16FuturUse10;     // placeholder 16-bits variable reserved for future use.
  UINT16 Variable16FuturUse9;      // placeholder 16-bits variable reserved for future use.
  UINT16 Variable16FuturUse8;      // placeholder 16-bits variable reserved for future use.
  UINT16 Variable16FuturUse7;      // placeholder 16-bits variable reserved for future use.
  UINT16 Variable16FuturUse6;      // placeholder 16-bits variable reserved for future use.
  UINT16 Variable16FuturUse5;      // placeholder 16-bits variable reserved for future use.
  UINT16 Variable16FuturUse4;      // placeholder 16-bits variable reserved for future use.
  UINT16 Variable16FuturUse3;      // placeholder 16-bits variable reserved for future use.
  UINT16 Variable16FuturUse2;      // placeholder 16-bits variable reserved for future use.
  UINT16 Variable16FuturUse1;      // placeholder 16-bits variable reserved for future use.
  UINT32 Variable32FuturUse2;      // placeholder 32-bits variable reserved for future use.
  UINT32 Variable32FuturUse1;      // placeholder 32-bits variable reserved for future use.
  UCHAR  SSID[40];                 // SSID for Wi-Fi network. Note: SSID begins at position 5 of the variable string, so that a "footprint" can be confirmed prior to writing to flash.
  UCHAR  Password[72];             // password for Wi-Fi network. Note: password begins at position 5 of the variable string, for the same reason as SSID above.
  UINT8  FlagDisplayAlarms;        // flag indicating that we want to show alarms status on LED matrix.
  UINT8  FlagDisplayAlarmDays;     // flag indicating that we want to show days with an active alarms on LED matrix.
  struct alarm Alarm[MAX_ALARMS];  // Alarm 0 to 8 parameters (numbered 1 to 9 for clock users).
  struct auto_scroll AutoScroll[MAX_AUTO_SCROLLS];  // items to scroll automatically and periodically on the RGB-Matrix.
  UINT8  Reserved[145];            // reserve the rest of this flash sector space for future use.
  struct event Event[MAX_EVENTS];  // calendar events.
  UINT16 Crc16;                    // crc16 of all data above to validate configuration.
}FlashConfig1;


struct flash_config2
{
  UCHAR  Version[8];               // firmware version number (format: "100.00a" - and including end-of-string).
  struct reminder1 Reminder1[MAX_REMINDERS1];  // type 1 reminders.
  UINT8  Reserved[246];            // reserve the rest of this flash sector space for future use.
  UINT16 Crc16;                    // crc16 of all data above to validate configuration.
}FlashConfig2;
/* --------------------------------------------------------------------------------------------------------------------------- *\
                                      End of flash memory configuration related definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */



/* --------------------------------------------------------------------------------------------------------------------------- *\
                                                  Function related definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */
#define MAX_FUNCTIONS         200   // maximum number of different functions.
#define MAX_FUNCTION_DIGITS     3   // number of digits in function ID.

#define FUNCTION_SETUP_START    0   // function ID range for setup functions.
#define FUNCTION_SETUP_END    199

#define FUNCTION_INFO_START   200   // function ID range for info functions.
#define FUNCTION_INFO_END     399

#define FUNCTION_TOOLS_START  400   // function ID range for tools functions.
#define FUNCTION_TOOLS_END    499

struct function
{
  UINT16 Id;               // 3-digits function ID entered by user and converted to UINT16.
  UINT16 Number;           // sequential function number for microcontroller handling.
  CHAR   Name[41];         // function name to print or scroll LED matrix.
  void  (*Pointer)(void);  // pointer to function to execute when this function is called.
};
/* --------------------------------------------------------------------------------------------------------------------------- *\
                                               End of function related definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */



/* --------------------------------------------------------------------------------------------------------------------------- *\
                                                       GPIO definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */
#define R1 2  // RED   for top half of RGB matrix.
#define G1 3  // GREEN for top half of RGB matrix.
#define B1 4  // BLUE  for top half of RGB matrix.
#define R2 5  // RED   for bottom half of RGB matrix.
#define G2 8  // GREEN for bottom half of RGB matrix.
#define B2 9  // BLUE  for bottom half of RGB matrix.

#define A 10  // scan line select.
#define B 16  // scan line select.
#define C 18  // scan line select.
#define D 20  // scan line select.
#define E 22  // scan line select.

#define CLK 11  // RGB matrix scan clock

#define STB 12  // RGB matrix scan and color latch strobe.

#define  OE 13  // Output Enable (PWM brightness control).

#define BUTTON_UP_GPIO    21  // Up button (in the middle of RGB matrix), under the "Bootsel" and "Reset" buttons.
#define BUTTON_SET_GPIO   19  // Set / Function button (between the Up button and the Down button).
#define BUTTON_DOWN_GPIO  15  // Down button (at the bottom of then RGB matrix).

#define PICO_LED          25  // Pico on-board LED.
#define ADC_LIGHT_SENSOR  26  // ambient light detector (photo-resistor).
#define BUZZER            27  // RGB matrix integrated active buzzer.
#define IR_RX             28  // GPIO for infrared sensor.
#define ADC_VCC           29  // GPIO for Pico internal power supply.

#define UART_TX_PIN  0  // optional serial line to transmit data to   an external VT101-type monitor.
#define UART_RX_PIN  1  // optional serial line to  receive data from an external VT101-type monitor.
/* --------------------------------------------------------------------------------------------------------------------------- *\
                                                   End of GPIO definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */



/* --------------------------------------------------------------------------------------------------------------------------- *\
                                                      Language definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */
#define LANGUAGE_LO_LIMIT 0
#define ENGLISH           0
#define CZECH             1
#define FRENCH            2
#define GERMAN            3
#define ITALIAN           4
#define SPANISH           5
#define LANGUAGE_HI_LIMIT 5
/* --------------------------------------------------------------------------------------------------------------------------- *\
                                                  End of Language definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */



/* --------------------------------------------------------------------------------------------------------------------------- *\
                                           Queues (circular buffers) related definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */
#define MAX_QUEUE_TYPES       8  // maximum number of different queue types.

#define QUEUE_ACTIVE_BUZZER   0
#define QUEUE_IR_BUFFER       1
#define QUEUE_PASSIVE_BUZZER  2
#define QUEUE_HSCROLL_BUFFER  3
/* --------------------------------------------------------------------------------------------------------------------------- *\
                                      End of queues (circular buffers) related definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */



#ifdef  REMOTE_SUPPORT
/* --------------------------------------------------------------------------------------------------------------------------- *\
                                                 Remote control related definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */
/* List or commands available with remote control. */
#define MAX_IR_READINGS           500  // maximum number of "logic level changes" while receiving data from IR remote control.
#define IR_BUFFER_SIZE             10  // buffer size for commands received from remote control.
#define IR_INDICATOR_START_ROW     18  // infrared burst reception indicator on RGB matrix.
#define IR_INDICATOR_END_ROW       19  // infrared burst reception indicator on RGB matrix.
#define IR_INDICATOR_START_COLUMN  29  // infrared burst reception indicator on RGB matrix.
#define IR_INDICATOR_END_COLUMN    34  // infrared burst reception indicator on RGB matrix.

/* Remote control button decoding. */
#define IR_LO_LIMIT           BUTTON_NONE        // 0x00 - Remote control buffer status when no button has been pressed.
#define IR_DOWN               BUTTON_DOWN        // 0x02 - Remote control will replicate the <Down> local button press.
#define IR_SET                BUTTON_SET         // 0x01 - Remote control will replicate the <Function> local button press.
#define IR_UP                 BUTTON_UP          // 0x03 - Remote control will replicate the <Up>   local button press.
#define IR_DOWN_LONG          BUTTON_DOWN_LONG
#define IR_SET_LONG           BUTTON_SET_LONG
#define IR_UP_LONG            BUTTON_UP_LONG
#define IR_VOL_MINUS          0x07
#define IR_VOL_PLUS           0x08
#define IR_EQ                 0x09
#define IR_100                0x0A
#define IR_200                0x0B
#define IR_0                  0x0C
#define IR_1                  0x0D
#define IR_2                  0x0E
#define IR_3                  0x0F
#define IR_4                  0x10
#define IR_5                  0x11
#define IR_6                  0x12
#define IR_7                  0x13
#define IR_8                  0x14
#define IR_9                  0x15
#define IR_HI_LIMIT           0x16  // must be one more than last valid command.

/* --------------------------------------------------------------------------------------------------------------------------- *\
                                               End of remote control related definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */
#endif  // REMOTE_SUPPORT



/* --------------------------------------------------------------------------------------------------------------------------- *\
                                                  RGB matrix specific definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */
#define HALF_ROWS        (MAX_ROWS / 2)     // for color setting, RGB matrix is splitted in two sections (Top part: rows 0 to 15 and Bottom part: rows 16 to 31).
#define Matrix_COLS_BYTE (MAX_COLUMNS / 8)  // total number of "sectors" per row.
#define SECTORS_PER_ROW      8  // number of sectors (8 bits representing 8 pixels) per row.

#define MAX_COLUMNS         64  // total number of pixel columns on RGB matrix.
#define MAX_ROWS            32  // total number of pixel rows    on RGB matrix.

#define FRAMEBUFFER_SIZE   256  // bitmap corresponding to bits that are turned on on RGB matrix (color is managed independantly.)

#define BUTTON_BUFFER_SIZE  10
/* --------------------------------------------------------------------------------------------------------------------------- *\
                                               End of RGB matrix specific definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */



/* --------------------------------------------------------------------------------------------------------------------------- *\
                                           Scroll buffer queue related definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */
#define MAX_ACTIVE_SCROLL      10  // maximum number of simulataneous active scrolls.
#define MAX_MESSAGE_LENGTH   1024  // total length of all ASCII messages queued for scrolling.

struct active_scroll
{
  UINT8  Owner;                 // window ID of the owner of this active scroll.
  UINT8  StartRow;              // start row to be scrolled (0 - 31).
  UINT8  EndRow;                // end row to be scrolled (0 - 31).
  UINT8  FontType;              // font type to be scrolled.
  UINT8  ScrollTimes;           // number of times to scroll the text message.
  UINT8  ScrollSpeed;           // relative scroll speed to slide pixels left.
  INT16  PixelCountCurrent;     // number of pixels remaining to scroll on LED matrix.
  UINT16 PixelCountBuffer;      // number of pixels remaining to scroll in bitmap buffer.
  UINT16 AsciiBufferPointer;    // pointer to next character to be scrolled in ASCII buffer.
  UINT64 BitmapBuffer[MAX_ROWS];       // temporary bitmap buffer between "Message" text and actual bitmap FrameBuffer.
  UCHAR  Message[MAX_MESSAGE_LENGTH];  // message text to be scrolled.
};
/* --------------------------------------------------------------------------------------------------------------------------- *\
                                         End of scroll buffer queue related definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */



/* --------------------------------------------------------------------------------------------------------------------------- *\
                                              Active sound queue related definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */
#define SILENT  0

#define MAX_ACTIVE_SOUND_QUEUE  100       // maximum number of "sounds" in the active buzzer sound queue.

struct queue_active_sound_element
{
  UINT16 MSec;
  UINT16 RepeatCount;
};

struct queue_active_sound
{
  volatile UINT8 Head;
  volatile UINT8 Tail;
  struct queue_active_sound_element Element[MAX_ACTIVE_SOUND_QUEUE];
};
/* --------------------------------------------------------------------------------------------------------------------------- *\
                                          End of active sound queue related definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */



/* --------------------------------------------------------------------------------------------------------------------------- *\
                                    RGB matrix scanning and color latching related definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */
#define R1_HIGH   gpio_put(R1, 1)
#define R1_LOW    gpio_put(R1, 0)

#define G1_HIGH   gpio_put(G1, 1)
#define G1_LOW    gpio_put(G1, 0)

#define B1_HIGH   gpio_put(B1, 1)
#define B1_LOW    gpio_put(B1, 0)

#define R2_HIGH   gpio_put(R2, 1)
#define R2_LOW    gpio_put(R2, 0)

#define G2_HIGH   gpio_put(G2, 1)
#define G2_LOW    gpio_put(G2, 0)

#define B2_HIGH   gpio_put(B2, 1)
#define B2_LOW    gpio_put(B2, 0)

#define A_HIGH    gpio_put(A, 1)
#define A_LOW     gpio_put(A, 0)

#define B_HIGH    gpio_put(B, 1)
#define B_LOW     gpio_put(B, 0)

#define C_HIGH    gpio_put(C, 1)
#define C_LOW     gpio_put(C, 0)

#define D_HIGH    gpio_put(D, 1)
#define D_LOW     gpio_put(D, 0)

#define E_HIGH    gpio_put(E, 1)
#define E_LOW     gpio_put(E, 0)

#define CLK_HIGH  gpio_put(CLK, 1)
#define CLK_LOW   gpio_put(CLK, 0)

#define STB_HIGH  gpio_put(STB, 1)
#define STB_LOW   gpio_put(STB, 0)

#define OE_HIGH   gpio_put(OE, 1)
#define OE_LOW    gpio_put(OE, 0)
/* --------------------------------------------------------------------------------------------------------------------------- *\
                               End of RGB matrix scanning and color latching related definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */



/* --------------------------------------------------------------------------------------------------------------------------- *\
                                                Windows and Box related definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */
/* Window type. */
#define WIN_DATE         1
#define WIN_TIME         2
#define WIN_TEST         3
#define WIN_FUNCTION     4
#define WIN_DICE         5
#define WIN_MESSAGE      6
#define WIN_SETUP        7
#define WIN_HI_LIMIT     8  // must be one more than last window definition.

#define MAX_WINDOWS     20

/* Window status. */
#define WINDOW_UNUSED    0
#define WINDOW_ACTIVE    1
#define WINDOW_INACTIVE  2
#define WINDOW_COUNTDOWN 3

/* Action to take for border when box is done exploding. */
#define ACTION_ERASE 0
#define ACTION_DRAW  1


/* Window structure definition. */
struct window
{
  UINT8  StartRow;             // start row of this window.
  UINT8  StartColumn;          // start column of this window.
  UINT8  EndRow;               // end row of this window.
  UINT8  EndColumn;            // end column of this window.
  UINT8  BorderColor;          // specify the color of the animated exploding window.
  UINT8  InsideColor;          // specify the color to set for the inside of the window at the end of window explosion.
  UINT8  LastBoxState;         // defined what to do with last box after exploding the window (ACTION_DRAW or ACTION_ERASE).
  UINT8  WinStatus;            // current status of window (inactive, active, undefined, etc...).
  UINT8  FlagBlink;            // indicate that part of this window is blinking.
  UINT32 BlinkOnTimer;         // indicate timer last time blinking pixels were turned On.
  UINT16 CountDown;            // if not null, indicating a count-down time (in seconds) to clear before restoring back link window(s).
  UINT8  TopBackLink;          // back link for top window (not used if this window does not occupied top of matrix).
  UINT8  MidBackLink;          // back link for mid window (not used if this window does not occupied middle of matrix).
  UINT8  BotBackLink;          // back link for bot window (not used if this window does not occupied bottom of matrix).
  UINT8  FlagTopScroll;        // Flag indicating that the Top row is currently scrolling text..
  UINT8  FlagMidScroll;        // Flag indicating that the Mid row is currently scrolling text..
  UINT8  FlagBotScroll;        // Flag indicating that the Bot row is currently scrolling text..
  UCHAR  Name[22];             // window name may have 21 characters maximum.
};
/* --------------------------------------------------------------------------------------------------------------------------- *\
                                              End of Windows and Box related definitions.
\* --------------------------------------------------------------------------------------------------------------------------- */

#endif  // __PICO_RGB_MATRIX_H
