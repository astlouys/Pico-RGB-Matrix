/* ============================================================================================================================================================= *\
   Pico-RGB-Matrix.c
   St-Louys Andre - August 2022
   astlouys@gmail.com
   Revision 25-APR-2024
   Langage: C with arm-none-eabi
   Version 2.01

   Raspberry Pi Pico Firmware to drive the Waveshare Pico-RGB-Matrix.
   From an original software version 1.00 by Waveshare
   Released under 3-Clause BSD License.

   NOTE:
   THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
   WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
   TIME. AS A RESULT, WAVESHARE OR THE AUTHOR SHALL NOT BE HELD LIABLE FOR ANY
   DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
   FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
   CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS. ALSO
   NOTE THAT THE AUTHOR IS NOT A WAVESHARE EMPLOYEE.



   REVISION HISTORY:
   =================
   31-JUL-2022 1.00 - Pull initial release from Waveshare.

   01-APR-2024 2.00 - Reformat code and general cleanup.
                    - Change many function names and variable names so that they are more representative of what they do.
                    - Port matrix framebuffer from 8-bits handling to 64-bits.
                    - Add 5x7  character set with variable-width.
                    - Add 8x10 character set with variable-width (not fully implemented yet).
                    - Handling of active buzzer transfered to a callback function for easier and transparent support.
                    - Add a circular buffer for easier active buzzer sound handling.
                    - Add support for 9 independent alarms, each one supporting any number of days-of-week.
                    - Add visual alarm indicators on LED matrix.
                    - Add visual target days-of-week alarm indicators on LED matrix.
                    - Invert the logic for the ambient light value so that more light means a higher number, which is more intuitive.
                    - Add ambient termperature reading.
                    - Add Pico's temperature reading.
                    - Add Auto-brightness control option.
                    - Add an hysteresis to Auto-Brightness to prevent fast brightness changes in case of a quick interference.
                    - Add Auto-detection of terminal emulator connection.
                    - Add Auto-detection of microcontroller type and remove support for the ESP32S2 microcontroller.
                    - Add Auto-scroll options, configurable by user.
                    - Begin implementation of Daylight Saving Time support (to be completed).
                    - Implement a brightness range control to set lowest and highest brightness settings for Auto-brightness.
                    - Add a timed window to trigger Pico's "Bootsel" mode while preventing overbright LEDs.
                    - Add support for up to 64 Calendar events.
                    - Begin implementation of a count-down timer (to be completed).
                    - Begin implementation of a count-up timer (to be completed).
                    - Implement a "debug engine" to toggle debugging sections of code at compile-time or at run-time.
                    - Add a mechanism to easily support device customization for different rooms, users, etc...
                    - Implement a "device integrity check" function to sequentially test all LEDs for all colors.
                    - Add an option to display "total device up time".
                    - Implement a "Golden age" option to help senior citizens.
                    - Implement "Hourly chime" option with time On and Time Off parameters to prevent sounds during the night.
                    - Implement a "Half-hour light chime" complying with "Hourly chime" time On and Time Off.
                    - Implement horizontal scrolling on LED matrix.
                    - Add an infrared burst pilot indicator on LED matrix.
                    - Add basic French language support to English language (and discarding Chinese language - sorry, I know nothing about it).
                    - Implement a mechanism for easy translation to any language using English-like ASCII characters.
                    - Change the layout of the RGB Matrix display to make it more elegant and allows more information to be displayed simultaneously.
                    - Add a "main system loop" status indicator.
                    - Support "Network Time Protocol" if a PicoW is used.
                    - Add a "night light" and "service light" utilities.
                    - Migrate some functions to Pico's core 1 to relief core 0 from some processing.
                    - Convert RGB Matrix from software-controlled brightness to PWM-controlled brightness to relief the microcontroller of this task.
                    - Add support of the included remote control unit.
                    - Add many "RGB_matrix_xxx()" high-level functions to make it easier to build more complex algorithm (RGB_matrix_set_pixel(), RGB_matrix_printf(), etc...)
                    - Plan for "silence period" to make the device silent for a specific period of time (to be completed).
                    - Implement a function to set the Pico in "Bootsel" mode with the remote control to overcome a glitch in RGB Matrix / PicoW incompatibility.
                    - Implement a watchdog to trigger a software reset if the "main system loop" crashes.
                    - Add a watchdog visual indicator on the LED matrix to see "how far we are" from a software reset if the watchdog becomes active.
                    - Add a compile-time option to completely turn Off device sound at compile time.
                    - Plan for temperature unit support (Fahrenheit or Celsius) - (to be completed).
                    - Add support for an external terminal emulator program with different menus and submenus.
                    - Add a test zone with many chunks of test code.
                    - Plan for hour display format (12-hours or 24-hours) - (to be completed).
                    - Implement different "time" functions to easily support NTP, DST, etc. ("human time" "Unix time", "tm time").
                    - Add a "Wi-Fi health indicator" (with the two Time "double-dots" indicator).
                    - Add many "win_xxx()" basic windowing functions to make it easier to build more complex windowing algorithm (win_open(), win_scroll(), win_blink(), etc...)
   25-APR-2024 2.01 - Adapt NTP algorithm so that connection will resume automatically after a connection lost.
                    - Apply a quick fix to comply with a critical timing in the new version of the RGB Matrix control board. Thanks to Waveshare for their support with this one !
                    - Upgrade the bitmap of the 8x10 digits "6" and "9".
\* ============================================================================================================================================================= */





/* ============================================================================================================================================================= *\
                                                              Pico-RGB-Matrix GPIO reference table
 * ============================================================================================================================================================= *
GPIO 00  (OUT)    UART TX pin.
GPIO 01  (IN)     UART RX pin.
GPIO 02  (OUT)    Red   top matrix half data byte.
GPIO 03  (OUT)    Green top matrix half data byte.
GPIO 04  (OUT)    Blue  top matrix half data byte.
GPIO 05  (OUT)    Red   bottom matrix half data byte.
GPIO 06   I2C     I2C SDA (data  line for DS3231).
GPIO 07   I2C     I2C SCL (clock line for DS3231).
GPIO 08  (OUT)    Green bottom matrix half data byte.
GPIO 09  (OUT)    Blue  bottom matrix half data byte.
GPIO 10  (OUT)    'A' scan bit line select.
GPIO 11  (OUT)    Clock.
GPIO 12  (OUT)    Latch (also called "Strobe").
GPIO 13  (OUT)    Output Enable (active Low).
GPIO 14  - - -    (not used).
GPIO 15  (IN)     Button "Up" - increase.
GPIO 16  (OUT)    'B' scan bit line select.
GPIO 17  - - -    (not used).
GPIO 18  (OUT)    'C' scan bit line select.
GPIO 19  (IN)     Button "Down"   - decrease.
GPIO 20  (OUT)    'D' scan bit line select.
GPIO 21  (IN)     Button "Set" - menu.
GPIO 22  (OUT)    'E' scan bit line select.
GPIO 23  - - -    (not used).
GPIO 24  - - -    (not used).
GPIO 25  (OUT)    Pico's on-board LED (different on PicoW).
GPIO 26  (IN)     ADC to read ambient light (photoresistor).
GPIO 27  (OUT)    Active buzzer control.
GPIO 28  (IN)     Infrared receive sensor.
GPIO 29  (IN)     ADC-Vref (Power supply reading).
GPIO 30  - - -    (not used).
\* ============================================================================================================================================================= */


/* Firmware version. */
#define FIRMWARE_VERSION "2.01"  ///

/* End users should uncomment the line below. If the line below is commented, a <DEVELOPER_VERSION> will be built and may perform
   unusual behavior for debugging and / or development purposes. In this case, you will be on your own to work through the code. */
#define RELEASE_VERSION  ///



/* $TITLE=System configuration or options. */
/* $PAGE */
/* ============================================================================================================================================================= *\
                         ===== SECTION BELOW CONTAINS COMPILE-TIME SYSTEM CONFIGURATION OR OPTIONS THAT CAN BE ADAPTED BY USERS =====
                                            SOME OF THESE ITEMS ARE ALSO ADJUSTABLE AT RUNTIME, OTHERS ARE NOT.
\* ============================================================================================================================================================= */
/* Device language selection. */
/* NOTES: Only ONE (1) language must be defined. */
/*        See User Guide for instructions on how to add a new language. */
#define LANGUAGE ENGLISH  ///
/// #define LANGUAGE FRENCH  ///


/* Select the customization file required to adapt the RGB Matrix flash configuration automatically. */
/* NOTES: See the User Guide to understand the right procedure for customization. */
/// #define CUSTOM_ANDRE
/// #define custom_andre_chambre
#define custom_generic
/// #define custom_roland



#define REMOTE_SUPPORT
#ifdef REMOTE_SUPPORT
#warning ===============> Built with remote control support.
#endif  // REMOTE_SUPPORT



#define NTP_SUPPORT
#ifdef NTP_SUPPORT
#warning ===============> Built with network time protocol support.
#endif  // NTP_SUPPORT



#define WATCHDOG_SUPPORT
#ifdef WATCHDOG_SUPPORT
#warning ===============> Built with watchdog support.
#endif  // WATCHDOG_SUPPORT



/* Conditional compile used to bypass some tests to allow for a quicker power-up sequence by-passing some device tests. */
/// #define QUICK_START  ///
#ifdef QUICK_START
#warning ===============> Built with QUICK_START option.
#endif  // QUICK_START



/* NOTE: Parameters below are default configuration parameters that will be used if RGB matrix does not contain a valid configuration
         and / or if configuration becomes corrupted.  When the configuration is changed while the RGB matrix is running, the new
         parameters are saved to flash and become active all the time (until configuration becomes corrupted again, in which case
         the default configuration will be restored). */

/* Hourly chime mode: FLAG_ON / FLAG_OFF / FLAG_DAY. */
#define CHIME_DEFAULT     FLAG_DAY  // determine if hourly chime is On, Off, or intermittent for a certain period of the day only (see next two lines).
#define CHIME_TIME_ON     9         // hourly chime (and half-hour light chime) will beep starting at this hour.
#define CHIME_TIME_OFF    21        // hourly chime, half-hour light chime, calendar events will stop beeping after this hour (after xxh59).
#define CHIME_HALF_HOUR   FLAG_DAY  // determine if half-hour light chime will beep or not.


/* Night light mode. */
#define NIGHT_LIGHT_DEFAULT   FLAG_AUTO   // night light mode (On / Off / Auto / Day).
#define NIGHT_LIGHT_TIME_ON   23          // default night light time On.
#define NIGHT_LIGHT_TIME_OFF  8           // default night light time Off.


/* Time display mode (12-hour format or 24-hour format). */
#define TIME_DISPLAY_DEFAULT  H24         // default time display mode (H12 or H24).


/* Determine how Daylight Saving Time ("DST" or summer time / winter time) is handled in the host country. */
/* See User Guide for list of available countries. */
#define DST_COUNTRY  DST_NORTH_AMERICA
#define TIMEZONE     -4


/* Default temperature unit to display. */
#define TEMPERATURE_DEFAULT  CELSIUS      // CELSIUS or FAHRENHEIT.
/* ============================================================================================================================================================= *\
                                                          ===== END OF SYSTEM CONFIGURATION OR OPTIONS =====
\* ============================================================================================================================================================= */





#ifdef RELEASE_VERSION
#warning ===============> Pico-RGB-Matrix built as RELEASE_VERSION.
#else  // ifdef RELEASE_VERSION (NOTE: Section below for developer's version only.)
/* ------------------------------------------------------------------------------------------------------------------------------------------------------------- *\
                                                              Section below for developers / tests only.
\* ------------------------------------------------------------------------------------------------------------------------------------------------------------- */
#warning ===============> Pico-RGB-Matrix built as DEVELOPER_VERSION.
#define DEVELOPER_VERSION

/// #define NO_SOUND
#ifdef NO_SOUND
#warning ===============> Built with NO_SOUND option TURNED ON !!!
#endif  // NO_SOUND


// #define PASSIVE_BUZZER_SUPPORT
#ifdef PASSIVE_BUZZER_SUPPORT
#warning ===============> Built with passive buzzer support.
#endif  // PASSIVE_BUZZER_SUPPORT
/* ------------------------------------------------------------------------------------------------------------------------------------------------------------- *\
                                                         Section above for developers / tests only.
\* ------------------------------------------------------------------------------------------------------------------------------------------------------------- */
#endif  // ifdef RELEASE_VERSION (NOTE: Section above for developer's version only.)





/* $TITLE=Definitions and include files. */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                               Definitions and include files
\* ============================================================================================================================================================= */
#include "font.h"
#include "hardware/adc.h"
#include "hardware/clocks.h"
#include "hardware/flash.h"
#include "hardware/i2c.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "hardware/sync.h"
#include "hardware/uart.h"
#include "hardware/watchdog.h"
#include "Pico-RGB-Matrix.h"
#include "pico/bootrom.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/unique_id.h"
#include "stdarg.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"

#ifdef NTP_SUPPORT
#include "PicoW-NTP-Client.h"
#endif  // NTP_SUPPORT



#if LANGUAGE == ENGLISH
#include "lang-english.h"
#endif

#if LANGUAGE == FRENCH
#include "lang-french.h"
#endif



/* $TITLE=Function prototypes. */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                                     Function prototypes.
\* ============================================================================================================================================================= */
/* Check if some alarms must be triggered. */
void alarm_check(void);

/* Feed alarm ringer for active ("triggered") alarms. */
void alarm_ring(void);

/* Scan active auto-scrolls and set a flag so that they are processed in the main endless system loop.  */
void auto_scroll_process(void);

/* Make a number of beeps through the buzzer (to be used until the 50msec callback is initialized and may take over). */
void beep_tone(UINT8 RepeatCount);

/* Callback in charge of matrix scan. */
bool callback_1msec_timer(struct repeating_timer *t);

/* Callback in charge of active buzzer sound queue and infrared remote control. */
bool callback_50msec_timer(struct repeating_timer *t);

/* One-second callback to update date and time on LED matrix. */
bool callback_1000msec_timer(struct repeating_timer *t);

/* Convert "HumanTime" to "tm_time".*/
void convert_human_to_tm(struct human_time *HumanTime, struct tm *TmTime);

/* Convert human time to Unix time. NOTE: Unix time is based on UTC time, not local time. */
UINT64 convert_human_to_unix(struct human_time *HumanTime, UINT8 FlagLocalTime);

/* Convert "TmTime" to Unix time. */
UINT64 convert_tm_to_unix(struct tm *TmTime, UINT8 FlagLocalTime);

/* Convert Unix time to human time and TmTime. */
void convert_unix_time(time_t UnixTime, struct tm *TmTime, struct human_time *HumanTime, UINT8 FlagLocalTime);

/* Thread to to be run on Pico's core 1. */
void core1_main(void);

/* Turn On a pixel for debugging purpose. */
void debug_pixel(UINT8 RowNumber, UINT8 ColumnNumber, UINT8 Color);

/* Display specified alarm number. */
void display_alarm(UINT8 AlarmNumber);

/* Display specified auto-scroll number. */
void display_auto_scroll(UINT8 AutoScrollNumber);

/* Display current Unix time. */
void display_current_unix_time(void);

/* Display specified calendar event number. */
void display_event(UINT8 EventNumber);

/* Display parameters of the specified function number. */
void display_function(UINT16 FunctionNumber);

/* Display parameters of all functions in the range, "range" corresponding to one of the function category ("setup", "info", "tools"). */
void display_function_id(UINT16 FunctionTypeStart, UINT16 FunctionTypeEnd);

/* Display function name on message window. */
void display_function_name(UINT16 FunctionId);

/* Display given text, followed by human time whose pointer is given as a parameter. */
void display_human_time(UCHAR *Text, struct human_time *HumanTime);

/* Display current content of specified matrix buffer. */
void display_matrix_buffer(UINT64 *BufferPointer);

/* Display one-second callback duration history. */
void display_one_second_callback(void);

/* Display the specified reminder number of type 1. */
void display_reminder1(UINT8 ReminderNumber);

/* Display scroll structures current parameters. */
void display_scroll(void);

/* Display given text followed by TmTime whose pointer is given as a parameter. */
void display_tm_time(UCHAR *Text, struct tm *TmTime);

/* Display total RGB Matrix Up time. */
void display_up_time(void);

/* Display current windows parameters. */
void display_windows(void);

/* Set the two "double-dots time separators" color. */
void double_dots_set_color(UINT8 Color);

/* Display all current variables read from real-time IC (DS3231).  */
void ds3231_display_values(void);

/* Read temperature from real-time IC (DS3231). */
void ds3231_get_temperature(float *DegreeC, float *DegreeF);

/* Read current human time from real-time clock. */
void ds3231_get_time(struct human_time *CurrentTime);

/* Initialize real-time clock IC (DS3231). */
void ds3231_init(void);

/* Set the day of month of the real-time clock IC (DS3231). */
void ds3231_set_dom(UINT8 DayOfMonth);

/* Set the day of week of the real-time clock IC (DS3231). */
void ds3231_set_dow(UINT8 DayOfWeek);

/* Set the hour of the real-time clock IC (DS3231). */
void ds3231_set_hour(UINT8 Hour);

/* Set the minute of the real-time clock IC (DS3231). */
void ds3231_set_minute(UINT8 Minute);

/* Set the month of the real-time clock IC (DS3231). */
void ds3231_set_month(UINT8 Month);

/* Set the second of the real-time clock IC (DS3231). */
void ds3231_set_second(UINT8 Second);

/* Set the time of the real-time clock IC (DS3231). */
void ds3231_set_time(struct human_time *CurrentTime);

/* Set the year of the real-time clock IC (DS3231). */
void ds3231_set_year(UINT16 YearValue);

/* Enter a human time and / or human date. */
void enter_human_time(struct human_time *HumanTime, UINT8 FlagDate, UINT8 FlagTime);

/* Check if some calendar events must be triggered. */
void event_check(void);

/* Compare crc16 between flash saved configuration and current active configuration. */
void flash_check_config(UINT8 ConfigNumber);

/* Display flash content through external monitor. */
void flash_display(UINT32 Offset, UINT32 Length);

/* Display RGB matrix configuration from flash memory. */
UINT8 flash_display_config(void);

/* Erase one sector of data in Pico flash memory. */
void flash_erase(UINT32 DataOffset);

/* Read RGB matrix configuration 1 from flash memory. */
UINT8 flash_read_config1(void);

/* Read RGB matrix configuration 2 from flash memory. */
UINT8 flash_read_config2(void);

/* Save current RGB matrix configuration 1 to flash memory. */
UINT8 flash_save_config1(void);

/* Save current RGB matrix configuration 2 to flash memory. */
UINT8 flash_save_config2(void);

/* Write data to Pico flash memory. */
UINT flash_write(UINT32 DataOffset, UINT8 *NewData, UINT16 NewDataSize);

/* Function to scroll alarms info on LED matrix. */
void function_alarms(void);

/* Function for alarms setup. */
void function_alarm_set(void);

/* Function to scroll auto-scrolls info on LED matrix. */
void function_auto_scroll(void);

/* Reset Pico in bootsel mode to upload a new Firmware. */
void function_bootsel(void);

/* Function to scroll brightness and ambient light info on LED matrix. */
void function_brightness(void);

/* Function to ajust brightness. */
void function_brightness_set(void);

/* Function to ajust hourly and half-hour chimes. */
void function_chime_set(void);

/* Function to launch countdown timer. */
void function_countdown_timer(void);

/* Function to launch countup timer. */
void function_countup_timer(void);

/* Function to ajust the date. */
void function_date_set(void);

/* Display function information about the specified function number on external terminal. */
void function_display(UINT16 FunctionNumber);

/* Function to scroll Daylight Saving Time info on LED matrix. */
void function_dst(void);

/* Function to ajust the daylight saving time. */
void function_dst_set(void);

/* Function for calendar events setup. */
void function_event_set(void);

/* Function to scroll calendar events of the day on LED matrix. */
void function_events_day(void);

/* Function to scroll calendar events of the month on LED matrix. */
void function_events_month(void);

/* Function to scroll calendar events of the week on LED matrix. */
void function_events_week(void);

/* Function to scroll calendar events of the year (all defined events) on LED matrix. */
void function_events_year(void);

/* Function to scroll Firmware Version on LED matrix. */
void function_firmware_version(void);

/* Function to scroll first free heap chunk memory pointer on LED matrix (to watch for memory leak). */
void function_free_heap(void);

/* Function for golden age mode setup. */
void function_golden_set(void);

/* Function to scroll system idle time monitor info on LED matrix. */
void function_idle_time(void);

/* Initialize functions. */
void function_init(void);

/* Function to sequentially check all LEDs of RGB Matrix. */
void function_integrity_check(void);

/* Function to scroll flashed network credentials on LED matrix. */
void function_network_credentials(void);

/* Function to scroll network info on LED matrix. */
void function_network_data(void);

/* Function for network setup. */
void function_network_set(void);

/* Function to scroll the type of microcontroller on LED matrix. */
void function_pico_type(void);

/* Function to simulate police lights. */
void function_police(void);

/* Find the function number corresponding to the specified function ID. */
UINT16 function_search(UINT16 FunctionId);

/* Function to scroll silence period info on LED matrix. */
void function_silence(void);

/* Function to scroll DS3231 & Pico internal temperature on LED matrix. */
void function_temperature(void);

/* Function to ajust the time. */
void function_time_set(void);

/* Function to scroll total RGB Matrix Up time on LED matrix. */
void function_up_time(void);

/* Get scroll number of active scroll on current active window. */
UINT8 get_scroll_number(void);

/* Return the day-of-week for the specified date. Sunday = 0   (...) Saturday = 6. */
UINT8 get_day_of_week(UINT8 DayOfMonth, UINT8 Month, UINT16 Year);

/* Determine the day-of-year of the date given in argument. */
UINT16 get_day_of_year(UINT8 DayOfMonth, UINT8 Month, UINT16 Year);

/* Return the function number and function name corresponding to the function ID given in argument. */
UINT16 get_function_number(UINT16 FunctionId, UCHAR *FunctionName);

/* Read ambient relative light value. */
UINT16 get_light_value(void);

/* Return the number of days of a specific month, given the specified year (to know if it is a leap year or not). */
UINT8 get_month_days(UINT8 MonthNumber, UINT16 TargetYear);

/* Read Pico's internal temperature from Pico's analog-to-digital gpio. */
void get_pico_temp(float *DegreeC, float *DegreeF);

/* Determine if the microcontroller is a Pico or a PicoW. */
UINT8 get_pico_type(void);

/* Retrieve Pico's Unique ID from its flash IC. It's better to call this function during initialization phase, before core 1 begins to run. */
void get_pico_unique_id(void);

/* Read a string from stdin. */
void input_string(UCHAR *String);

/* Interrupt handler for infrared data burst received from remote control through infrared sensor. */
/* NOTE: This function is the gpio callback definition function, so it must be initialized even if remote control is not to be used. */
gpio_irq_callback_t isr_signal_trap(UINT8 gpio, UINT32 Events);


#ifdef REMOTE_SUPPORT
/* --------------------------------------------------------------------------------------------------------------------------- *\
                                          Infrared remote control related function prototypes
\* --------------------------------------------------------------------------------------------------------------------------- */
/* Function to decode an infrared remote control button keystroke received using the "Car MP3" RGB Matrix remote control. */
UINT8 ir_decode_button(UINT8 *IrButton);

/* Display timings of the last received infrared data stream. This function must not be used inside a callback function (would crash the Firmware). */
void ir_display_timings(void);
/* --------------------------------------------------------------------------------------------------------------------------- *\
                                              End of IR sensor-related function prototypes
\* --------------------------------------------------------------------------------------------------------------------------- */
#endif  // REMOTE_SUPPORT

/* Set color for endless loop pilot LEDs. */
void pilot_set_color(UINT8 Color);

/* Toggle endless loop pilot LEDs On / Off. */
void pilot_toggle(void);

/* Function to process a button received from local RGB matrix buttons and / or from remote control button. */
void process_button(UINT8 CommandId);

/* Execute a function number. */
void process_function(UINT16 FunctionNumber);

/* Display PWM parameters for all PWM signals used in the RGB Matrix device. */
void pwm_display_parameters(void);

/* Initialize PWM GPIOs */
void pwm_initialize(void);

/* Turn On or Off the PWM signal specified in argument. */
void pwm_on_off(UINT8 PwmNumber, UINT8 FlagSwitch);

/* Set the duty cycle for the PWM controlling the brightness of the RGB Matrix LEDs. */
void pwm_set_duty_cycle(UINT8 DutyCycle);

/* Set the frequency for the specified PWM. */
void pwm_set_frequency(UINT8 PwmNumber, UINT32 Frequency);

/* Set the "Level" for the specified PWM. */
void pwm_set_level(UINT8 PwmNumber, UINT16 Level);

/* Add the given sound to the active sound queue. */
UINT16 queue_add_active(UINT16 MSeconds, UINT16 RepeatCount);

/* Return the number of free slots in active sound queue. */
UINT8 queue_free_active(void);

/* Remove next sound from the active sound queue. */
UINT8 queue_remove_active(UINT16 *MSeconds, UINT16 *RepeatCount);

/* Housekeeping for reminders of type 1. */
void reminder1_update(void);

/* Scan all reminders1 to find those that must be active. */
void reminder1_check(void);

/* Feed reminders1 ringers. */
void reminder1_ring(void);

/* Test remote control. */
void remote_control_test(void);

/* Blink all defined "blinking areas" in all active windows. */
void RGB_matrix_blink();

/* Draw or erase a box with specified borders. */
void RGB_matrix_box(UINT8 StartRow, UINT8 StartColumn, UINT8 EndRow, UINT8 EndColumn, UINT8 Color, UINT8 Action);

/* Check order of coordinates (start and end Row and Column and re-order if required. */
void RGB_matrix_check_coord(UINT8 *StartRow, UINT8 *StartColumn, UINT8 *EndRow, UINT8 *EndColumn);

/* Turn Off the pixels in the specified matrix area. */
void RGB_matrix_clear_pixel(UINT64 *BufferPointer, UINT8 StartRow, UINT8 StartColumn, UINT8 EndRow, UINT8 EndColumn);

/* Clear LED matrix display. */
void RGB_matrix_cls(UINT64 *BufferPointer);

/* Initialize RGB matrix gpio's. */
void RGB_matrix_device_init(void);

/* Display specified ASCII character using specified variable-width font type and beginning at the specified matrix position (upper left of character). */
UINT8 RGB_matrix_display(UINT64 *DisplayBuffer, UINT8 StartRow, UINT8 StartColumn, UINT8 Char, UINT8 FontType, UINT8 FlagMore);

/* Display date and time on LED matrix. */
void RGB_matrix_display_time(void);

/* LED matrix device integrity check. */
void RGB_matrix_integrity_check(UINT8 FlagTerminal);

/* Calculate the length of the string supplied when using the font type specified. */
UINT8 RGB_matrix_pixel_length(UINT8 FontType, UCHAR *Format, ...);

/* "Printf" specified string, using specified font type and beginning at the specified pixel row and specified pixel column (upper left of character). */
UINT8 RGB_matrix_printf(UINT64 *DisplayBuffer, UINT8 StartRow, UINT8 StartColumn, UINT8 FontType, UCHAR *Format, ...);

/* Scroll the rows specified in the scroll structure for the target window one pixel to the left while managing the scroll ASCII buffer. */
void RGB_matrix_scroll(UINT8 ScrollNumber);

/* Set matrix display color for the specified LED matrix area. */
void RGB_matrix_set_color(UINT8 StartRow, UINT8 StartColumn, UINT8 EndRow, UINT8 EndColumn, UINT8 Color);

/* Turn On the pixels in the specified LED matrix area of the specified LED matrix buffer. */
void RGB_matrix_set_pixel(UINT64 *BufferPointer, UINT8 StartRow, UINT8 StartColumn, UINT8 EndRow, UINT8 EndColumn);

/* Scan the LED matrix rows / columns. */
void RGB_matrix_update(UINT64 *FrameBuffer);

void RGB_matrix_write_data(UINT8 high_data, UINT8 low_data, UCHAR DisplayRGBCount);

/* Manage ambient light history and set automatic brightness if the configuration is set for auto-brightness. */
void set_auto_brightness(void);

/* Restart the RGB Matrix Firmware by software reset (watchdog). */
void software_reset(void);

/* Terminal submenu for alarm setup. */
void term_alarm_setup(void);

/* Terminal submenu for auto-scroll setup. */
void term_auto_scroll(void);

/* Terminal submenu for brightness setup. */
void term_brightness_setup(void);

/* Terminal submenu for calendar events setup. */
void term_calendar_events_setup(void);

/* Terminal submenu for date setup. */
void term_date_setup(void);

/* Terminal submenu for daylight saving time and time zone setup. */
void term_dst_setup(void);

/* Terminal submenu for "erase flash configuration". */
void term_erase_flash(void);

/* Terminal submenu for <info> functions. */
void term_info(void);

/* Display a section of Pico's memory. */
void term_memory_display(void);

/* Terminal menu when a CDC USB connection is detected during power-up or inside the main endless loop. */
void term_menu(void);

/* Terminal submenu for network credentials setup. */
void term_network_setup(void);

/* Terminal submenu for reminders of type 1 setup. */
void term_reminder1_setup(void);

/* Terminal submenu for <setup> selections. */
void term_setup(void);

/* Terminal submenu for temperature setup. */
void term_temperature_setup(void);

/* Terminal submenu for tests. */
void term_test(void);

/* Terminal submenu for time setup. */
void term_time_setup(void);

/* Terminal submenu for <tools> functions. */
void term_tools(void);

/* Test chunks of code. */
void test_zone(UINT TestNumber);

/* Send a string to terminal emulator. */
void uart_send(UINT LineNumber, const UCHAR *FunctionName, UCHAR *Format, ...);

/* Utility to convert a binary-coded-decimal value to a decimal value. */
UINT8 util_bcd2dec(UINT8 BCDValue);

/* Find the cyclic redundancy check of the specified data. */
UINT16 util_crc16(UINT8 *Data, UINT16 DataSize);

/* Utility to convert a decimal value to binary-coded-decimal value. */
UINT8 util_dec2bcd(UINT8 DecimalValue);

/* Display data whose pointer is sent in argument through external terminal. */
void util_display_data(UCHAR *Data, UINT32 Size);

/* Reverse the bit order of the UINT8 value given in argument. */
UINT8 util_reverse_8bits(UINT8 InputData);

/* Reverse the bit order of the UINT64 value given in argument. */
UINT64 util_reverse_64bits(UINT64 InputData);

/* Return the string representing the uint64_t value in binary. */
void util_uint64_to_binary_string(UINT64 Value, UINT8 StringLength, UCHAR *BinaryString);

/* Setup blink parameters for specific window area. Blinking itself is managed by the 1-second callback. */
void win_blink(UINT8 WindowNumber, UINT8 StartRow, UINT8 StartColumn, UINT8 EndRow, UINT8 EndColumn);

/* Turn Off blinking on the specific window area. */
void win_blink_off(UINT8 WindowNumber, UINT8 StartRow, UINT8 StartColumn, UINT8 EndRow, UINT8 EndColumn);

/* Close the specified window and restore the backlink. */
void win_close(UINT8 WindowNumber);

/* Clear the specified window. */
void win_cls(UINT8 WindowNumber);

/* Initialize window structures. */
void win_init();

/* Explode / open the specified window. */
void win_open(UINT8 WindowNumber, UINT8 FlagRestore);

/* Clear the specified part of the specified window. */
void win_part_cls(UINT8 WindowNumber, UINT8 StartRow, UINT8 EndRow);

/* Print data in the specified window. */
UINT8 win_printf(UINT8 WindowNumber, UINT8 StartRow, UINT8 StartColumn, UINT8 FontType, UCHAR *Format, ...);

/* Scroll the text in the specified window, on the specified line. Return ScrollNumber that has been assigned. */
UINT8 win_scroll(UINT8 WindowNumber, UINT8 StartRow, UINT8 EndRow, UINT16 ScrollTimes, UINT8 ScrollSpeed, UINT8 FontType, UCHAR *Format, ...);

/* If there is currently a scroll going on, cancel it and add a few character spaces to introduce new scrolling. */
void win_scroll_cancel(UINT8 WindowNumber, UINT8 StartRow, UINT8 EndRow);

/* Housekeeping when a scroll has been completed. */
void win_scroll_off(UINT8 ScrollNumber);

/* Set the colors of the specified window. */
void win_set_color(UINT8 WindowNumber, UINT8 InsideColor, UINT8 BoxColor);





/* $TITLE=Global variables declaration / definition. */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                             Global variables declaration / definition.
\* ============================================================================================================================================================= */
UCHAR DisplayRGB[HALF_ROWS][MAX_COLUMNS];
UCHAR PicoUniqueId[40];                       // Pico Unique ID read from flash IC.
UCHAR ScrollAsciiBuffer[3][1024];             // scroll ASCII buffer. 3 lines of 1024 characters each.

UINT8  AutoScrollBitMask;                     // BitMask representing auto-scrolls that must be scrolled by main system loop.
UINT8  ButtonBuffer[BUTTON_BUFFER_SIZE];      // buffer for buttons (local or remote) that have been pressed and not yet processed.
UINT8  FlagEndlessLoop = FLAG_OFF;            // flag indicating that we are in the context of the main system while loop.
UINT8  FlagFrameBufferBusy;                   // flag indicating that FrameBuffer is currently being updated.
UINT8 *FlashData;                             // pointer to an allocated RAM memory space used for flash operations.
UINT8 *Framebuffer;                           // original RGB Matrix 8-bits Framebuffer pointer (should be replaced with UINT64 *FrameBuffer).
UINT8  IrCounter = 0;                         // counter of remote control keystrokes received so far.
UINT8  OneSecondPointer;                      // pointer to the next slot in the circular buffer.
UINT8  PicoType;                              // contain type of microcontroller used (TYPE_PICO or TYPE_PICOW).
UINT8  RowScan = 0;                           // current matrix row being scanned.
UINT8  WinTop;                                // currently active window for top of matrix.
UINT8  WinMid;                                // currently active window for middle of matrix.
UINT8  WinBot;                                // currently active window for bottom of matrix.

UINT16 AlarmBitMask;                          // bitmask of currently triggered alarms (when not already shut off by user).
UINT16 AmbientLight[BRIGHTNESS_HYSTERESIS_SECONDS];  // ambient light readings for the last seconds.
UINT16 AutoScrollScheduleMask;                // bitmask of the auto-scrolls to be currently processed.
UINT16 AverageAmbientLight;                   // average ambient light value for the last "hysteresis" number of seconds.
UINT16 FunctionHiLimit;                       // one more than the last defined function.
UINT16 ServiceLightTimer;                     // count-down timer for service light.
UINT16 WatchdogCheck;                         // number being automatically incremented every second inside main system endless loop.
UINT16 WatchdogMiss;

INT64 Dum1Int64;
INT64 OneSecondInterval[MAX_ONE_SECOND_INTERVALS];

UINT64  DebugBitMask;                         // bitmask identifying logical sections of code to debug through external monitor.
UINT64  EventBitMask;                         // bitmask representing the calendar events that are triggered.
UINT64  Reminder1BitMask;                     // bitmask representing the reminders of type 1 that are currently active (their span period is not over).
UINT64  TermModeTimer = 0ll;                  // timer when last time we exited from terminal menu.
UINT64  BlinkBuffer[MAX_ROWS];                // temporary bitmask buffer of FrameBuffer LED positions being blinked.
UINT64  CheckBuffer[MAX_ROWS];                // bitmask of active LED blinking area.
UINT64  FrameBuffer[MAX_ROWS];                // RGB matrix LED display framebuffer.

absolute_time_t AbsoluteEntryTime;            // time stamp of an entry point (in a callback function).
absolute_time_t AbsoluteExitTime;             // time stamp of an exit point  (in a callback function).

#ifdef REMOTE_SUPPORT
/* Infrared-related global variables. */
volatile UINT8 IrBuffer[IR_BUFFER_SIZE];      // buffer for IR commands ("buttons") received from remote control.
UINT8  IrIndicator;                           // second count-down for infrared indicator on RGB matrix.
UINT16 IrStepCount;                           // number of "logic level changes" received from IR remote control in current data stream.
UINT64 IrInitialValue[MAX_IR_READINGS];       // initial timer value when receiving edge change from remote control.
UINT64 IrFinalValue[MAX_IR_READINGS];         // final timer value when receiving edge change from remote control.
UINT32 IrResultValue[MAX_IR_READINGS];        // duration of this logic level (Low or High) in the signal received from remote control.
UCHAR  IrLevel[MAX_IR_READINGS];              // logic levels of remote control signal: 'L' (low), 'H' (high), or 'X' (undefined).
UINT32 IrPulseDistance[MAX_IR_READINGS];      // variable to hold the pulse distance of every single 38kHz infrared burst.
UINT64 DataBuffer;                            // variable to hold the command received from remote control.
#endif  // REMOTE_SUPPORT


struct active_alarm ActiveAlarm[MAX_ALARMS];              // dynamic parameters for currently active alarms.
struct active_reminder1 ActiveReminder1[MAX_REMINDERS1];  // reminders of type 1 currently active.
struct active_scroll *ActiveScroll[MAX_ACTIVE_SCROLL];    // pointers to struct active_scroll to be malloc'ed
struct flash_config1 FlashConfig1;                        // RGB matrix main configuration data.
struct flash_config2 FlashConfig2;                        // reminders configuration saved to flash.
struct function Function[300];                            // functions to execute in response to IR.
struct human_time CurrentTime;                            // human time structure containing the time being displayed on RGB Matrix.
struct human_time StartTime;                              // time the RGB Matrix was last powered On.
struct pwm Pwm[2];                                        // PWM structures for matrix brightness and passive buzzer (not implemented yet).
struct queue_active_sound QueueActiveSound;               // circular buffer to hold active buzzer sounds to be processed.
struct window Window[MAX_WINDOWS];                        // windows definition and parameters.

struct repeating_timer Handle1MSecTimer;
struct repeating_timer Handle50MSecTimer;
struct repeating_timer Handle1000MSecTimer;

extern struct ntp_data NTPData;
/// critical_section_t ThreadLock;


/* Complete month names. */
UCHAR MonthName[13][13] =
{
  {" "}, {$JANUARY}, {$FEBRUARY}, {$MARCH}, {$APRIL}, {$MAY}, {$JUNE}, {$JULY}, {$AUGUST}, {$SEPTEMBER}, {$OCTOBER}, {$NOVEMBER}, {$DECEMBER}
};

/* Short - 3-letters - month names. */
UCHAR ShortMonth[13][4] =
{
  {" "}, {$JAN}, {$FEB}, {$MAR}, {$APR}, {$MAY}, {$JUN}, {$JUL}, {$AUG}, {$SEP}, {$OCT}, {$NOV}, {$DEC}
};

/* Complete day names. */
UCHAR DayName[7][13] =
{
  {$SUNDAY}, {$MONDAY}, {$TUESDAY}, {$WEDNESDAY}, {$THURSDAY}, {$FRIDAY}, {$SATURDAY}
};

/* Short - 3-letters - day names. */
UCHAR ShortDay[7][4] =
{
  {$SUN}, {$MON}, {$TUE}, {$WED}, {$THU}, {$FRI}, {$SAT}
};

/* Color names. */
UCHAR ColorName[8][10] =
{
  {}, {$BLUE}, {$GREEN}, {$CYAN}, {$RED}, {$MAGENTA}, {$YELLOW}, {$WHITE}
};

/* Period of the day. */
UCHAR DayPeriod[4][16] =
{
  {$MORNING}, {$AFTERNOON}, {$EVENING}, {$NIGHT}
};


#ifdef REMOTE_SUPPORT
/* Remote control buttons and local buttons name definition. */
UCHAR ButtonName[IR_HI_LIMIT][21] =
{
  {"None"}, {"Down"}, {"Set"}, {"Up"}, {"Long-Down"}, {"Long-Set"}, {"Long-Up"}, {"Vol-Minus"}, {"Vol-Plus"}, {"Eq"}, {"100+"}, {"200+"}, {"Digit-0"}, {"Digit-1"}, {"Digit-2"}, {"Digit-3"}, {"Digit-4"}, {"Digit-5"}, {"Digit-6"}, {"Digit-7"}, {"Digit-8"}, {"Digit-9"}
};
#endif






/* ============================================================================================================================================================= *\
                                                                      Main program entry point.
\* ============================================================================================================================================================= */
int main(void)
{
  UCHAR String[128];

  INT64 DeltaTime;

  UINT8 ColumnNumber;
  UINT8 DataInput;        // keyboard scan during main endless loop.
  UINT8 Dum1UInt8;
  UINT8 Loop1UInt8;
  UINT8 RowNumber;

  UINT16 Delay;
  UINT16 Dum1UInt16;
  UINT16 FunctionNumber;
  UINT16 PwmLevel;
  UINT16 Loop1UInt16;
  UINT16 Loop2UInt16;

  UINT64 CurrentTimer;
  UINT64 IrTimer;
  UINT64 LastTimer1Sec;
  UINT64 LastTimer2Sec;
  UINT64 LastTimer5Sec;
  UINT64 LastTimer10Sec;
  UINT64 LastTimer30Sec;
  UINT64 LastTimer1Min;
  UINT64 TempBuffer[MAX_ROWS];
  UINT64 WatchdogTimer;

  time_t UnixTime;

  Framebuffer = (UINT8 *)FrameBuffer;  // original 8bits framebuffer.

  struct human_time HumanTime;
  struct tm TempTime;



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                Set DebugBitMask for logical sections of code to be debugged through an external terminal emulator.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  DebugBitMask = DEBUG_NONE;            // initialize to zero on entry.

#ifdef DEVELOPER_VERSION
  /* NOTE: Terminal submenu "term_debug()" should be used to dynamically define debug sections for most cases (except maybe for DEBUG_STARTUP). */

  // DebugBitMask += DEBUG_ALARM;       // debug alarm algorithm.
  // DebugBitMask += DEBUG_BLINK;       // debug blinking mechanism.
  // DebugBitMask += DEBUG_BLUETOOTH;   // debug bluetooth communications.
  // DebugBitMask += DEBUG_BOX;         // debug exploding windows.
  // DebugBitMask += DEBUG_BRIGHTNESS;  // debug auto-brightness algorithm.
  // DebugBitMask += DEBUG_BUTTON;      // debug local button handling.
  // DebugBitMask += DEBUG_CORE;        // debug Pico's core 1 thread.
  // DebugBitMask += DEBUG_DS3231;      // debug DS3231 real-time IC.
  // DebugBitMask += DEBUG_EVENT;       // debug calendar event algorithm.
  // DebugBitMask += DEBUG_FLASH;       // debug flash memory operations.
  // DebugBitMask += DEBUG_FLOW;        // debug program sequence flow.
  // DebugBitMask += DEBUG_FUNCTION;    // debug IR <Functions> operation.
  // DebugBitMask += DEBUG_IR;          // debug infrared remote control.
  // DebugBitMask += DEBUG_MATRIX;      // debug matrix display operations.
  // DebugBitMask += DEBUG_NTP;         // debug Network Time Protocol.
  // DebugBitMask += DEBUG_PWM;         // debug PWM mechanism.
  // DebugBitMask += DEBUG_REMINDER;    // debug reminder1 algorithm.
  // DebugBitMask += DEBUG_SCROLL;      // debug scrolling mechanism.
  // DebugBitMask += DEBUG_SOUND_QUEUE; // debug queue engines.
  // DebugBitMask += DEBUG_STARTUP;     // debug startup sequence.
  // DebugBitMask += DEBUG_SUMMER_TIME; // debug summer-time related logic.
  // DebugBitMask += DEBUG_TEST;        // debug test section.
  // DebugBitMask += DEBUG_WATCHDOG;    // debug watchdog behavior.
  // DebugBitMask += DEBUG_WIFI;        // debug WiFi communications.
  // DebugBitMask += DEBUG_WINDOW;      // debug window algorithm.
#endif  // DEVELOPER_VERSION



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                 Handling of special characters for languages other than English.
  \* --------------------------------------------------------------------------------------------------------------------------- */
#if LANGUAGE == FRENCH
  MonthName[2][1]  = 0x0F;  // Fevrier
  MonthName[8][2]  = 0x1C;  // Aout
  MonthName[12][1] = 0x0F;  // Decembre

  DayPeriod[1][3] = 0x10;  // apres-midi
#endif



  /* --------------------------------------------------------------------------------------------------------------------------- *\
              Initialize GPIOs and clear matrix so that Pico can be switched in upload mode without overbright pixels.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  /* Initialize GPIOs. */
  stdio_init_all();
  RGB_matrix_device_init();  // NOTE: brightness is set to 0 % during power-up sequence.
#if 0
  /* This part to be uncommented if it is important to get the full log of startup sequence. */
  if (DebugBitMask & DEBUG_STARTUP)
  {
    beep_tone(2)
    sleep_ms(5000);  // slow down startup sequence if required for debugging purposes.
    printf("[%4u]   After stdio_init_all().\r", __LINE__);
    sleep_ms(1000);  // delay before next step below.
  }
#endif  // 0


  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                             Start 1 msec callback in charge of LED matrix scan.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Before launching 1-msec callback.\r", __LINE__);
    sleep_ms(1000);
  }

  add_repeating_timer_ms(-1, callback_1msec_timer, NULL, &Handle1MSecTimer);



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                        At this point of the power-up sequence, we can clear the LED matrix and turn Off PWM
                 so that user may press the bootsel button to upload a new Firmware while LED matrix remains all Off.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  /* Let some time for the developer to press the bootsel and reset buttons to switch the Pico into upload mode. Then, display a
     message to user that it is time to start the terminal emulator program if he wants, since generic GPIOs have now been initialized. */
  if (DebugBitMask & DEBUG_STARTUP)
  {
    /* If we are debugging the start-up sequence, make a beep and give some time for the user to start
       the terminal emulator program and adjust the serial port if required. */
    beep_tone(1);
    sleep_ms(2000);  // slow down startup sequence if required for debugging purposes.
    printf("[%4u]   Before message on LED matrix to start emulator program.\r", __LINE__);
    debug_pixel(31, 0, BLUE);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }
  sleep_ms(1000);
  pwm_set_duty_cycle(50);
  pwm_set_level(PWM_ID_BRIGHTNESS, 1300);
  RGB_matrix_set_color(0, 0, 31, 63, RED);
  sleep_ms(200);

  RGB_matrix_printf(FrameBuffer,  1, 99, FONT_5x7, "%s", $START);
  RGB_matrix_printf(FrameBuffer, 10, 99, FONT_5x7, "%s", $EMULATOR);
  RGB_matrix_printf(FrameBuffer, 19, 99, FONT_5x7, "%s", $NOW);
  sleep_ms(2000);

  if ((DebugBitMask & DEBUG_STARTUP) == 0)
  {
    RGB_matrix_cls(FrameBuffer);
    pwm_set_duty_cycle(0);
    pwm_set_level(PWM_ID_BRIGHTNESS, 2000);
  }
  sleep_ms(3000);  // let some time for user to start the terminal emulator program to log info.



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                            Reserve RAM space area for flash operations.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Before malloc() for flash operations.\r", __LINE__);
    debug_pixel(31, 1, BLUE);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }

  FlashData = (UINT8 *)malloc(FLASH_SECTOR_SIZE);



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                         Initialize sound queue for active buzzer on entry.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Before initializing sound queue.\r", __LINE__);
    debug_pixel(31, 2, BLUE);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }

  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_ACTIVE_SOUND_QUEUE; ++Loop1UInt16)
  {
    QueueActiveSound.Element[Loop1UInt16].MSec        = 0;
    QueueActiveSound.Element[Loop1UInt16].RepeatCount = 0;
  }
  QueueActiveSound.Head = 0;
  QueueActiveSound.Tail = 0;



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                  Initialize ambient light history with current ambient light value.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Before initializing ambient light history.\r", __LINE__);
    debug_pixel(31, 3, BLUE);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }

  Dum1UInt16 = get_light_value();
  for (Loop1UInt8 = 0; Loop1UInt8 < BRIGHTNESS_HYSTERESIS_SECONDS; ++Loop1UInt8)
    AmbientLight[Loop1UInt8] = Dum1UInt16;

  ServiceLightTimer = 0;  // service light will remain Off by default.



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                          Initialize one-second callback duration history.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Before initializing one-second callback duration history.\r", __LINE__);
    debug_pixel(31, 4, BLUE);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }

  OneSecondPointer = 0;  // point to first slot of the circular buffer.
  AbsoluteEntryTime = nil_time;
  AbsoluteExitTime  = nil_time;
  for (Loop1UInt8 = 0; Loop1UInt8 < MAX_ONE_SECOND_INTERVALS; ++Loop1UInt8)
    OneSecondInterval[Loop1UInt8] = 0ll;



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                         Initialize alarms.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Before initializing alarms.\r", __LINE__);
    debug_pixel(31, 5, BLUE);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }

  /* Parameters below are dynamic for active alarms. Consequently, they must not be in alarm structure saved in flash. */
  AlarmBitMask = 0;
  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_ALARMS; ++Loop1UInt16)
  {
    ActiveAlarm[Loop1UInt8].CountDown     = 0;      // assume no alarm is ringing on entry.
    ActiveAlarm[Loop1UInt8].PreviousTimer = 0x00l;  // timer value of this alarm's last ring.
  }



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                    Initialize calendar events.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Before initializing calendar events.\r", __LINE__);
    debug_pixel(31, 6, BLUE);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }

  EventBitMask = 0ll;



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                        Initialize functions.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Before initializing functions.\r", __LINE__);
    debug_pixel(31, 7, BLUE);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }

  function_init();



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                Initialize LED matrix "FrameBuffer", "BlinkBuffer" and "CheckBuffer".
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Before initializing display buffers (FrameBuffer, BlinkBuffer, CheckBuffer).\r", __LINE__);
    debug_pixel(31, 8, BLUE);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }

  for (Loop1UInt8 = 0; Loop1UInt8 < MAX_ROWS; ++Loop1UInt8)
  {
    FrameBuffer[Loop1UInt8] = 0ll;
    BlinkBuffer[Loop1UInt8] = 0ll;
    CheckBuffer[Loop1UInt8] = 0xFFFFFFFFFFFFFFFFll;
  }



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                   Initialize windows on entry.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Before initializing windows.\r", __LINE__);
    debug_pixel(31, 9, BLUE);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }

  win_init();



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                            Retrieve Pico's Unique ID from its flash memory.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Before retrieving Pico's Unique ID.\r", __LINE__);
    debug_pixel(31, 10, BLUE);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }

  get_pico_unique_id();



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                    Initialize critical section used to protect LED scanning and prevent glitches on LED matrix.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Before initializing critical_section.\r", __LINE__);
    debug_pixel(31, 11, BLUE);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }

  /// critical_section_init(&ThreadLock);



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                     Initialize DS3231 real-time IC.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Before initializing DS3231.\r", __LINE__);
    debug_pixel(31, 12, BLUE);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }

  ds3231_init();
  ds3231_get_time(&CurrentTime);
  ds3231_get_time(&StartTime);

  if (stdio_usb_connected())
  {
    /* Now that DS3231 has been initialized, read and display time read from it.
       NOTE: If wrong values are read back from DS3231, they could crash the Firmware trying to display an invalid day-of-week and / or month name. */
    printf("[%4u]======================================================================\r", __LINE__);
    printf("[%4u]   Current date and time retrieved from real-time clock IC (DS3231)\r",    __LINE__);
    printf("[%4u]======================================================================\r", __LINE__);
    printf("[%4u]      DayOfMonth:     %2u   (1 to 31)\r",                       __LINE__, CurrentTime.DayOfMonth);
    printf("[%4u]      Month:          %2u   (1 to 12)\r",                       __LINE__, CurrentTime.Month);
    printf("[%4u]      Year:         %4.4u\r",                                   __LINE__, CurrentTime.Year);
    printf("[%4u]      DayOfWeek:      %2u   (0 = Sunday (...) 6 = Saturday)\r", __LINE__, CurrentTime.DayOfWeek);
    printf("[%4u]      DayOfYear:     %3u   (1 to 366)\r",                       __LINE__, CurrentTime.DayOfYear);
    printf("[%4u]      FlagDst:      0x%2.2X\r",                                 __LINE__, CurrentTime.FlagDst);
    printf("[%4u]\r",                                                            __LINE__);
    printf("[%4u]      Hour:           %2.2u\r",                                 __LINE__, CurrentTime.Hour);
    printf("[%4u]      Minute:         %2.2u\r",                                 __LINE__, CurrentTime.Minute);
    printf("[%4u]      Second:         %2.2u\r",                                 __LINE__, CurrentTime.Second);
    printf("[%4u]======================================================================\r\r\r", __LINE__);
  }



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                  Initialize local buttons buffer.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  /* Initialize local buttons buffer. */
  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Before initializing buttons buffer.\r", __LINE__);
    debug_pixel(31, 13, BLUE);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }

  for (Loop1UInt8 = 0; Loop1UInt8 < BUTTON_BUFFER_SIZE; ++Loop1UInt8)
    ButtonBuffer[Loop1UInt8] = BUTTON_NONE;



#ifdef REMOTE_SUPPORT
  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                     Initialize infrared related data.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  IrStepCount = 0;

  /* Initialize IR buffer. */
  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Before initializing IR buffer.\r", __LINE__);
    debug_pixel(31, 14, BLUE);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }

  for (Loop1UInt8 = 0; Loop1UInt8 < IR_BUFFER_SIZE; ++Loop1UInt8)
    IrBuffer[Loop1UInt8] = 0x00;
#endif  // REMOTE_SUPPORT



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                     Blank LED matrix on entry.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  /* Now that the LED matrix refresh callback has been started, make sure LED matrix is blank... */
  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Before clearing LED matrix.\r", __LINE__);
    debug_pixel(31, 15, BLUE);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }

  RGB_matrix_cls(FrameBuffer);



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                   Start callback managing sound queue and infrared data stream.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  /* 50 msec callback to handle active buzzer sound queue and remote control infrared data stream reception. */
  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Before launching 50-msec remote control callback.\r", __LINE__);
    debug_pixel(31, 16, BLUE);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }

  add_repeating_timer_ms(-50, callback_50msec_timer, NULL, &Handle50MSecTimer);



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                         Start the callback in charge of time update on RGB matrix.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  /* One second callback for time update on LED matrix. */
  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Before launching time update callback.\r", __LINE__);
    debug_pixel(31, 17, BLUE);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }
  add_repeating_timer_ms(-1000, callback_1000msec_timer, NULL, &Handle1000MSecTimer);



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                        Display time and date for the first time on LED matrix.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  pwm_set_duty_cycle(50);
  win_open(WIN_DATE, FLAG_OFF);
  win_open(WIN_TIME, FLAG_OFF);
  RGB_matrix_display_time();
  pwm_set_level(PWM_ID_BRIGHTNESS, 1500);
  if (DebugBitMask & DEBUG_STARTUP) debug_pixel(31, 18, BLUE);
  sleep_ms(3000);


#if 0  // Remove this conditional compile once to erase flash configuration. Conditional compile must be put back one the configuration has been erase.
  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                         Retrieve RGB Matrix configuration from flash memory.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  /* Code section intentionally left in source code if we want to erase both flash configurations to force generating two new default ones. */
  /* NOTE: Terminal menu may also be used to erase both flash configurations. */

  printf("Erasing configuration sections 1 and 2 of Pico's flash memory to force generating a new configuration.\r");
  win_open(WIN_MESSAGE, FLAG_OFF);
  win_printf(WIN_MESSAGE, 1, 99, FONT_5x7, "ERASING");
  win_printf(WIN_MESSAGE, 9, 99, FONT_5x7, "FLASH");
  sleep_ms(4000);

  PwmLevel = Pwm[PWM_ID_BRIGHTNESS].Level;  // keep track of original PWM level.
  pwm_set_level(PWM_ID_BRIGHTNESS, 2000);   // blank LED matrix while interrupts are disabled.
  flash_erase(0x1FF000);   // erase configuration 1 (most configuration settings).
  flash_erase(0x1FE000);   // erase configuration 2 (calendar events and reminders1).

  win_close(WIN_MESSAGE);  // restore backlink window.
  pwm_set_level(PWM_ID_BRIGHTNESS, PwmLevel);  // restore brightness level when done
#endif  // Remove this conditional compile once to erase flash configuration. Conditional compile must be put back one the configuration has been erase.


  /* Check flash configurations 1 and 2. If they are corrupted, a new default configuration will be saved, preventing flash_read_config() to crash. */
  /// flash_check_config(1);
  /// flash_check_config(2);
  flash_read_config1();
  flash_read_config2();

  /*** Add support for automatic flash configuration update from version to version. ***/
  sprintf(FlashConfig1.Version, "%s", FIRMWARE_VERSION);
  sprintf(FlashConfig2.Version, "%s", FIRMWARE_VERSION);


  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                    Wait for CDC USB connection.
                                      System will give up and continue after a 15 seconds waiting.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  /* Wait for CDC USB connection or for user to press the "SET" button. */
  Delay = 0;
  pilot_set_color(BLUE);  // set a special color for the endless loop pilot.
  while ((stdio_usb_connected() == 0) && gpio_get(BUTTON_SET_GPIO))
  {
    /// if (FlashConfig1.WatchdogFlag == FLAG_OFF) beep_tone(1);  // do not beep if RGB Matrix has been restarterd by watchdog.
    pilot_toggle();  // Tell the user he can press the Set button to bypass CDC USB connection.
    ++Delay;  // one more 1-second cycle waiting for CDC USB connection.
    sleep_ms(1000);

    /* If we waited for more than this number of seconds for a CDC USB connection, get out of the loop and continue. */
    if (Delay > 10) break;
  }
  if (DebugBitMask & DEBUG_STARTUP) debug_pixel(31, 19, BLUE);



  if (FlashConfig1.WatchdogFlag == FLAG_OFF)
  {
    /// beep_tone(2);
    if (DebugBitMask & DEBUG_WATCHDOG) printf("FlashConfig1.WatchdogFlag found to be OFF\r");
  }
  else
  {
    if (DebugBitMask & DEBUG_WATCHDOG) printf("FlashConfig1.WatchdogFlag found to be ON\r");
    FlashConfig1.WatchdogFlag = FLAG_OFF;  // once restart is over, reset watchdog restart indicator.
  }
  if (DebugBitMask & DEBUG_STARTUP) debug_pixel(31, 20, BLUE);



  /* If a CDC USB connection has been detected, pass control to Terminal menu.*/
  if (stdio_usb_connected())
  {
    printf("[%4u]   CDC USB connection has been established...\r", __LINE__);
    /// beep_tone(5);
    term_menu();
  }
  if (DebugBitMask & DEBUG_STARTUP) debug_pixel(31, 21, BLUE);



  /* If a press on the "Set" button has been done, make sure it has been released before continuing. */
  if (gpio_get(BUTTON_SET_GPIO) == 0)
  {
    while (gpio_get(BUTTON_SET_GPIO) == 0)
    {
      uart_send(__LINE__, __func__, "<Set> button has been pressed... Waiting for <Set> button to be released.\r");
      sleep_ms(200);
    }
  }
  if (DebugBitMask & DEBUG_STARTUP) debug_pixel(31, 22, BLUE);



  /* If user requested any section to be debugged, send a time stamp to the log screen when we start. */
  if (DebugBitMask)
  {
    if (DebugBitMask & DEBUG_STARTUP) debug_pixel(31, 23, BLUE);

    uart_send(__LINE__, __func__, "\r\r\r\r----------------------- ======== %2.2u-%3s-%4.4u %2.2u:%2.2u:%2.2u PICO-RGB-MATRIX LOG INFO ======== -----------------------\r\r\r",
              CurrentTime.DayOfMonth, ShortMonth[CurrentTime.Month], CurrentTime.Year, CurrentTime.Hour, CurrentTime.Minute, CurrentTime.Second);

    uart_send(__LINE__, __func__, "Sections below will be logged:\r\r\r");

    /* Log all debug sections that are under analysis. */
    for (Loop1UInt16 = 0; Loop1UInt16 < 64; ++Loop1UInt16)
    {
      if (DebugBitMask & (0x01 << Loop1UInt16))
      {
        switch (0x01 << Loop1UInt16)
        {
          case DEBUG_ALARM:
            uart_send(__LINE__, __func__, "Debug alarm algorithm.\r");
          break;

          case DEBUG_BLINK:
            uart_send(__LINE__, __func__, "Debug blinking mechanism.\r");
          break;

          case DEBUG_BLUETOOTH:
            uart_send(__LINE__, __func__, "Debug Bluetooth communications.\r");
          break;

          case DEBUG_BOX:
            uart_send(__LINE__, __func__, "Debug exploding windows.\r");
          break;

          case DEBUG_BRIGHTNESS:
            uart_send(__LINE__, __func__, "Debug auto brightness algorithm.\r");
          break;

          case DEBUG_BUTTON:
            uart_send(__LINE__, __func__, "Debug local button handling.\r");
          break;

          case DEBUG_CORE:
            uart_send(__LINE__, __func__, "Debug Pico's core 1 thread.\r");
          break;

          case DEBUG_DS3231:
            uart_send(__LINE__, __func__, "Debug DS3231 real-time IC.\r");
          break;

          case DEBUG_EVENT:
            uart_send(__LINE__, __func__, "Debug calendar event algorithm.\r");
          break;

          case DEBUG_FLASH:
            uart_send(__LINE__, __func__, "Debug flash memory operations.\r");
          break;

          case DEBUG_FLOW:
            uart_send(__LINE__, __func__, "Debug program sequence logic flow.\r");
          break;

          case DEBUG_FUNCTION:
            uart_send(__LINE__, __func__, "Debug <Functions> operation.\r");
          break;

          case DEBUG_IR:
            uart_send(__LINE__, __func__, "Debug infrared remote control.\r");
          break;

          case DEBUG_MATRIX:
            uart_send(__LINE__, __func__, "Debug display matrix operations.\r");
          break;

          case DEBUG_NTP:
            uart_send(__LINE__, __func__, "Debug Network Time Protocol.\r");
          break;

          case DEBUG_PWM:
            uart_send(__LINE__, __func__, "Debug PWM behavior.\r");
          break;

          case DEBUG_REMINDER:
            uart_send(__LINE__, __func__, "Debug reminder algorithm.\r");
          break;

          case DEBUG_SCROLL:
            uart_send(__LINE__, __func__, "Debug scroll algorithm.\r");
          break;

          case DEBUG_SOUND_QUEUE:
            uart_send(__LINE__, __func__, "Debug queue engines.\r");
          break;

          case DEBUG_STARTUP:
            uart_send(__LINE__, __func__, "Debug startup sequence.\r");
          break;

          case DEBUG_SUMMER_TIME:
            uart_send(__LINE__, __func__, "Debug summer-time related logic.\r");
          break;

          case DEBUG_TEST:
            uart_send(__LINE__, __func__, "Debug test zone.\r");
          break;

          case DEBUG_WATCHDOG:
            uart_send(__LINE__, __func__, "Debug watchdog behavior.\r");
          break;

          case DEBUG_WIFI:
            uart_send(__LINE__, __func__, "Debug WiFi communications.\r");
          break;

          case DEBUG_WINDOW:
            uart_send(__LINE__, __func__, "Debug window algorithm.\r");
          break;

          default:
            uart_send(__LINE__, __func__, "Section #%u\r", Loop1UInt16);
          break;
        }
      }
    }
    uart_send(__LINE__, __func__, "\r---------------------------------------------------------------------------------------------------------------\r\r\r");
  }



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                    Initialize reminders of type 1.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Before initializing reminders.\r", __LINE__);
    debug_pixel(31, 24, BLUE);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }

  /* Now that DS3231 is initialized, get current time and manage reminders of type 1. */
  reminder1_update();



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                      Determine the type of microcontroller (Pico or PicoW).
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Before getting Pico type.\r", __LINE__);
    debug_pixel(31, 25, BLUE);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }

  PicoType = get_pico_type();



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                 Handling customization files.
    NOTE: The customization Firmware (built with "#define RELEASE_VERSION" commented out) must be uploaded to the RGB-Matrix
          and run for at least five minutes. Then, the final Firmware version (with the "#define RELEASE_VERSION" active) must
          be uploaded to the RGB-Matrix and run indefinitely.

          This process will have to be repeated again every time the RGB-Matrix is updated with a new Firmware version since
          it is most likely that the structure of the configuration data will have changed.
  \* --------------------------------------------------------------------------------------------------------------------------- */
#ifdef DEVELOPER_VERSION
#ifdef CUSTOM_ANDRE
#include "custom_andre.c"
#warning ===============> Firmware Version specifically customized for Andre
#endif  // CUSTOM_ANDRE

#ifdef custom_andre_chambre
#include "custom_andre_chambre.c"
#warning ===============> Firmware Version specifically customized for chambre Andre
#endif  // custom_andre_chambre

#ifdef custom_generic_user
#include "custom_generic_user.c"
#warning ===============> Firmware Version specifically customized for generic users
#endif  // custom_generic_user

#ifdef custom_roland
#include "custom_roland.c"
#warning ===============> Firmware Version specifically customized for Roland
#endif  // custom_roland
#endif  // DEVELOPER_VERSION

  if (DebugBitMask & DEBUG_STARTUP) debug_pixel(31, 26, BLUE);

#ifdef NTP_SUPPORT
  /* --------------------------------------------------------------------------------------------------------------------------- *\
                         If NTP_SUPPORT is defined, RGB Matrix' real-time clock IC will periodically
             be synchronized through network time protocol. User must have encoded credentials to Pico's flash before.
                                               (Refer to User Guide for details).
                                                   Initialize WiFi connection.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  /* Initialize NTP-related variables on entry. */
  NTPData.FlagNTPResync  = FLAG_ON;   // force NTP re-sync on power-up.
  NTPData.FlagNTPSuccess = FLAG_OFF;  // will be turned On after successful NTP answer.
  NTPData.FlagNTPHistory = FLAG_OFF;  // will be set according to the operations outcome.
  NTPData.NTPRefresh     = NTP_REFRESH;
  NTPData.NTPErrors      = 0l;        // reset number of NTP errors on entry.
  NTPData.NTPReadCycles  = 0l;
  NTPData.NTPPollCycles  = 0l;        // reset number of NTP poll cycles on entry.
  NTPData.UnixTime       = (time_t)0ll;
  NTPData.NTPUpdateTime  = nil_time;
  NTPData.NTPLag         = nil_time;

  /* Initialize the CYW43 driver and lwIP stack. */
  init_cyw43(CYW43_COUNTRY_WORLDWIDE);

  /* Initialize Wi-Fi connection. */
  if (ntp_init(FlashConfig1.SSID, FlashConfig1.Password) == false)
  {
    NTPData.NTPUpdateTime = make_timeout_time_ms(NTPData.NTPLagTime * 1000);
    uart_send(__LINE__, __func__, "ntp_init(): error while trying to establish Wi-Fi connection.\r");
    if (DebugBitMask & DEBUG_NTP)
    {
      uart_send(__LINE__, __func__, "=========================================================\r");
      uart_send(__LINE__, __func__, "            Variables after failed ntp_init()\r");
      display_ntp_info();
    }
  }
  else
  {
    if (DebugBitMask & DEBUG_NTP)
    {
      uart_send(__LINE__, __func__, "=========================================================\r");
      uart_send(__LINE__, __func__, "          Variables after successful ntp_init()\r");
      display_ntp_info();
    }
  }
#endif // NTP_SUPPORT


  if (DebugBitMask & DEBUG_STARTUP) debug_pixel(31, 27, BLUE);



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                              Display debug information if required.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (DebugBitMask & DEBUG_BRIGHTNESS) pwm_display_parameters();


  if (DebugBitMask & DEBUG_FLASH)
  {
    uart_send(__LINE__, __func__, "=================================================\r");
    uart_send(__LINE__, __func__, "     sizeof(FlashConfig1):     0x%4.4X (%4.4u)\r", sizeof(FlashConfig1),     sizeof(FlashConfig1));
    uart_send(__LINE__, __func__, "     sizeof(FlashConfig2):     0x%4.4X (%4.4u)\r", sizeof(FlashConfig2),     sizeof(FlashConfig2));
    uart_send(__LINE__, __func__, "     sizeof(struct alarm):     0x%4.4X (%4.4u)\r", sizeof(struct alarm),     sizeof(struct alarm));
    uart_send(__LINE__, __func__, "     sizeof(struct event):     0x%4.4X (%4.4u)\r", sizeof(struct event),     sizeof(struct event));
    uart_send(__LINE__, __func__, "     sizeof(struct reminder1): 0x%4.4X (%4.4u)\r", sizeof(struct reminder1), sizeof(struct reminder1));
    uart_send(__LINE__, __func__, "=================================================\r\r\r");
  }



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                    Seed random number generator.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Before random seed.\r", __LINE__);
    debug_pixel(31, 28, BLUE);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }

  srand(time_us_64());



  /* --------------------------------------------------------------------------------------------------------------------------- *\
              Start the thread to run on Pico's core 1 (second Pico's core) to read the infrared data stream received
             from remote control without interference from callback functions and other potential interrupts on core 0.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Before starting second core.\r", __LINE__);
    debug_pixel(31, 29, BLUE);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }

  multicore_launch_core1(core1_main);





  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                     Temporary code section.
                                    To display information before entry in the endless loop.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  /* Optionally display complete list of DS3231 current parameters. */
  if (DebugBitMask & DEBUG_DS3231) ds3231_display_values();


  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                  End of temporary code section.
  \* --------------------------------------------------------------------------------------------------------------------------- */



  /* Initialize all timestep timers on entry. */
  LastTimer1Sec  = time_us_64();
  LastTimer2Sec  = LastTimer1Sec;
  LastTimer5Sec  = LastTimer1Sec;
  LastTimer10Sec = LastTimer1Sec;
  LastTimer30Sec = LastTimer1Sec;
  LastTimer1Min  = LastTimer1Sec;
  IrTimer        = LastTimer1Sec;


  /* Scroll Firmware Version number when starting Pico-RGB-Matrix. */
  function_firmware_version();


#ifdef NO_SOUND
  win_scroll(WIN_DATE, 201, 201, 1, 1, FONT_5x7, "WARNING - This Firmware has been built WITH ALL SOUNDS DISABLED");
#endif  // NO_SOUND


  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Before entering endless loop.\r", __LINE__);
    debug_pixel(31, 30, BLUE);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                 Main system loop. Will loop forever.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (DebugBitMask & DEBUG_FLOW) printf("Entering main endless loop\r");
  FlagEndlessLoop = FLAG_ON;
  while (1)
  {
    CurrentTimer = time_us_64();



    /***
    // If a new CDC USB connection has been detected, pass control to Terminal menu.
    if (stdio_usb_connected())
    {
      printf("CDC USB connection has been established...\r");
      term_menu();
    }
    ***/



    /* If user pressed <Enter> on external terminal, branch to term_menu() function. */
    /* NOTE: Endless loop is suspended while user navigate the terminal menus / submenus. */
    DataInput = getchar_timeout_us(50000l);
    if (DataInput == 0x0D) term_menu();



    /* --------------------------------------------------------------------------------------------------------------------------- *\
                                 Check if something has been received from remote control buttons.
    \* --------------------------------------------------------------------------------------------------------------------------- */
#ifdef REMOTE_SUPPORT
    /* If infrared remote control support is enabled, check if data has been received by infrared sensor. */
    if (IrBuffer[0] != IR_LO_LIMIT)
    {
      if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "Button %s (%u) has been pressed on remote control...\r", ButtonName[IrBuffer[0]], IrBuffer[0]);
    }
#endif  // REMOTE_SUPPORT



    /* --------------------------------------------------------------------------------------------------------------------------- *\
                  Check if something has been received either from local buttons and / or from remote control buttons.
    \* --------------------------------------------------------------------------------------------------------------------------- */
    if (ButtonBuffer[0] != BUTTON_NONE)
    {
      if (DebugBitMask & DEBUG_BUTTON)
      {
        if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "Button %s (%u) has been pressed on remote control...\r", ButtonName[ButtonBuffer[0]], ButtonBuffer[0]);

        switch (ButtonBuffer[0])
        {
          case (BUTTON_SET):
            uart_send(__LINE__, __func__, "Button <Set> quick press...\r");
          break;

          case (BUTTON_SET_LONG):
            uart_send(__LINE__, __func__, "Button <Set> long press...\r");
          break;

          case (BUTTON_DOWN):
            uart_send(__LINE__, __func__, "Button <Down> quick press...\r");
          break;

          case (BUTTON_DOWN_LONG):
            uart_send(__LINE__, __func__, "Button <Down> long press...\r");
          break;

          case (BUTTON_UP):
            uart_send(__LINE__, __func__, "Button <Up> quick press...\r");
          break;

          case (BUTTON_UP_LONG):
            uart_send(__LINE__, __func__, "Button <Up> long press...\r");
          break;
        }
      }
    }

    /* Handle an eventual button press. */
    if ((IrBuffer[0] != BUTTON_NONE) && (ButtonBuffer[0] == BUTTON_NONE))
    {
      /* Local buttons have priority over remote control buttons. So, only if there is no local button press, process any eventual remote button press. */
      ButtonBuffer[0] = IrBuffer[0];
    }

    switch (ButtonBuffer[0])
    {
      case (BUTTON_SET):
      case (BUTTON_UP_LONG):
        process_button(ButtonBuffer[0]);
      break;

      default:
        /* For now, only <Set> button is valid for a first press. */
        IrBuffer[0]     = BUTTON_NONE;
        ButtonBuffer[0] = BUTTON_NONE;
      break;
    }



#ifdef WATCHDOG_SUPPORT
    /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                             Watchdog timer.
    \* --------------------------------------------------------------------------------------------------------------------------- */
    if ((CurrentTimer - WatchdogTimer) > 999900ll)
    {
      WatchdogTimer = CurrentTimer;

      /* NOTE: Watchdog timer has been set slightly faster than one second. This is to make sure that the WatchdogCheck variable
               will have been incremented when the callback tests its value and prevent a miss. In fact, one miss is not really
               important and everything would return to normal within the next few cycles. The idea of making it a little faster
               is to prevent a yellow LED that would turn On for a few seconds on the display if there is a miss. */
      ++WatchdogCheck;
    }
#endif  // WATCHDOG_SUPPORT


    /* --------------------------------------------------------------------------------------------------------------------------- *\
                    1-second timestep and schedule mark. Put here functions that we want to execute every second.
    \* --------------------------------------------------------------------------------------------------------------------------- */
    if ((CurrentTimer - LastTimer1Sec) > 1000000ll)
    {
      LastTimer1Sec = CurrentTimer;

      /// debug_pixel(31, CurrentTimer % 63, BLUE);  ///

      /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                  Manage endless loop pixel indicators pilot.
      \* --------------------------------------------------------------------------------------------------------------------------- */
      /* Blinking pixels to indicate that endless loop is still running. */
      pilot_set_color(CYAN);
      pilot_toggle();


#ifdef NTP_SUPPORT
      /* --------------------------------------------------------------------------------------------------------------------------- *\
                              Manage color of the two "double-dots time separator" to indicate network health.
      \* --------------------------------------------------------------------------------------------------------------------------- */
      /* Indicate Wi-Fi network health with double dots time separator color. */
      if (NTPData.FlagNTPHistory == FLAG_ON)  double_dots_set_color(GREEN);
      if (NTPData.FlagNTPHistory == FLAG_OFF) double_dots_set_color(RED);
#endif  // NTP_SUPPORT


      /* --------------------------------------------------------------------------------------------------------------------------- *\
                                        Keep track of time spent in the one-second callback routine.
      \* --------------------------------------------------------------------------------------------------------------------------- */
      if (is_nil_time(AbsoluteExitTime) == false)
      {
        Dum1Int64 = absolute_time_diff_us(AbsoluteEntryTime, AbsoluteExitTime);
        OneSecondInterval[OneSecondPointer] = Dum1Int64;
        ++OneSecondPointer;
        if (OneSecondPointer >= MAX_ONE_SECOND_INTERVALS) OneSecondPointer = 0;
        AbsoluteEntryTime = nil_time;
        AbsoluteExitTime  = nil_time;
      }



      /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                Check if it is time to execute an auto-scroll now.
      \* --------------------------------------------------------------------------------------------------------------------------- */
      for (Loop1UInt16 = 0; Loop1UInt16 < MAX_AUTO_SCROLLS; ++Loop1UInt16)
      {
        /// uart_send(__LINE__, __func__, "Checking auto-scroll number %u\r", Loop1UInt16);
        if (AutoScrollBitMask & (0x01 << Loop1UInt16))
        {
          /// uart_send(__LINE__, __func__, "Auto-scroll number %u flagged to be executed...\r", Loop1UInt16);
          /* It is time to execute this auto-scroll. */
          for (Loop2UInt16 = 0; Loop2UInt16 < MAX_ITEMS; ++Loop2UInt16)
          {
            /// uart_send(__LINE__, __func__, "Validating item %2u   FunctionId: %3u\r", Loop2UInt16, FlashConfig1.AutoScroll[Loop1UInt16].FunctionId[Loop2UInt16]);
            if (FlashConfig1.AutoScroll[Loop1UInt16].FunctionId[Loop2UInt16] != 0)
            {
              /* This function ID is a valid one in this auto-scroll, execute it. */
              FunctionNumber = get_function_number(FlashConfig1.AutoScroll[Loop1UInt16].FunctionId[Loop2UInt16], String);
              if (FunctionNumber != MAX_FUNCTIONS)
              {
                /// if (DebugBitMask & DEBUG_SCROLL)
                ///   uart_send(__LINE__, __func__, "Executing item  %2u   Function ID: %3u   Function number: %3u   FunctionName: %s   pointer: %p\r", Loop2UInt16, FlashConfig1.AutoScroll[Loop1UInt16].FunctionId[Loop2UInt16], FunctionNumber, Function[FunctionNumber].Name, Function[FunctionNumber].Pointer);
                Function[FunctionNumber].Pointer();
              }
            }
          }
        }
        /* Reset this function in the bitmask once this cycle is over. */
        AutoScrollBitMask &= ~(0x01 << Loop1UInt16);
      }
    }



    /* --------------------------------------------------------------------------------------------------------------------------- *\
                    2-seconds timestep and schedule mark. Put here functions that we want to execute every 2 seconds.
    \* --------------------------------------------------------------------------------------------------------------------------- */
    if ((CurrentTimer - LastTimer2Sec) > 2000000ll)
    {
      LastTimer2Sec = CurrentTimer;
    }



    /* --------------------------------------------------------------------------------------------------------------------------- *\
                    5-seconds timestep and schedule mark. Put here functions that we want to execute every 5 seconds.
    \* --------------------------------------------------------------------------------------------------------------------------- */
    if ((CurrentTimer - LastTimer5Sec) > 5000000ll)
    {
      LastTimer5Sec = CurrentTimer;


      /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                 Check if flash configuration has been changed.
      \* --------------------------------------------------------------------------------------------------------------------------- */
      /* Check if RGB Matrix flash configuration has been changed, more or less 30 seconds after exiting from terminal menu. */
      if ((TermModeTimer != 0ll) && ((CurrentTimer - TermModeTimer) > 30000000ll))
      {
        flash_check_config(1);  // configuration 1 (main configuration data).
        flash_check_config(2);  // configuration 2 (calendar events and reminders).
        TermModeTimer = 0ll;
      }
    }



    /* --------------------------------------------------------------------------------------------------------------------------- *\
                   10-seconds timestep and schedule mark. Put here functions that we want to execute every 10 seconds.
    \* --------------------------------------------------------------------------------------------------------------------------- */
    if ((CurrentTimer - LastTimer10Sec) > 10000000ll)
    {
      LastTimer10Sec = CurrentTimer;

#ifdef DEVELOPER_VERSION
      if (stdio_usb_connected())
      {
        /// uart_send(__LINE__, __func__, "10-seconds heartbeat... Absolute time reference: %6llu   WatchdogCheck: %4u\r", (time_us_64() / 1000000ll), WatchdogCheck);

        if (DebugBitMask & DEBUG_REMINDER)
        {
          util_uint64_to_binary_string(Reminder1BitMask, MAX_REMINDERS1, String);
          uart_send(__LINE__, __func__, "Reminder1BitMask:       0x%10.10llX   [%s]\r", Reminder1BitMask, String);
        }



        if (DebugBitMask & DEBUG_SCROLL)
        {
          Dum1UInt8 = get_scroll_number();
          // uart_send(__LINE__, __func__, "Received %u from get_scroll_number)\r", Dum1UInt8);
          if (Dum1UInt8 != MAX_ACTIVE_SCROLL)
          {
            uart_send(__LINE__, __func__, "Total length of scrolling message: %4u (active scroll number: %u     window: %s)\r", strlen(ActiveScroll[Dum1UInt8]->Message), Dum1UInt8, Window[ActiveScroll[Dum1UInt8]->Owner].Name);
            sleep_ms(20);  // prevent communication override.
            uart_send(__LINE__, __func__, "Current pointer in ASCII message:  %4u (remaining characters to be scrolled: %u)\r", ActiveScroll[Dum1UInt8]->AsciiBufferPointer, strlen(&ActiveScroll[Dum1UInt8]->Message[ActiveScroll[Dum1UInt8]->AsciiBufferPointer]));
            sleep_ms(20);  // prevent communication override.
            uart_send(__LINE__, __func__, "Text remaining to be scrolled:\r");
            sleep_ms(20);  // prevent communication override.
            printf("---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\r");
            sleep_ms(20);  // prevent communication override.
            // printf("%s\r", ActiveScroll[Dum1UInt8]->Message);  // display complete message.
            printf("%s\r", &ActiveScroll[Dum1UInt8]->Message[ActiveScroll[Dum1UInt8]->AsciiBufferPointer]);  // display what remains to be scrolled.
            sleep_ms(20);  // prevent communication override.
            printf("---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\r\r\r\r");
            sleep_ms(20);  // prevent communication override.
          }
        }



        /***
        if (DebugBitMask & DEBUG_NTP)
        {
          uart_send(__LINE__, __func__, "=========================================================\r");
          uart_send(__LINE__, __func__, "               NTP info in the endless loop\r");
          display_ntp_info();
        }
        ***/
      }
#endif  // DEVELOPER_VERSION
    }



    /* --------------------------------------------------------------------------------------------------------------------------- *\
                   30-seconds timestep and schedule mark. Put here functions that we want to execute every 30 seconds.
    \* --------------------------------------------------------------------------------------------------------------------------- */
    if ((CurrentTimer - LastTimer30Sec) > 30000000ll)
    {
      LastTimer30Sec = CurrentTimer;

      if (DebugBitMask & DEBUG_ALARM)
      {
        // Display alarms bitmask.
        util_uint64_to_binary_string((UINT64)AlarmBitMask, MAX_ALARMS, String);
        uart_send(__LINE__, __func__, "AlarmBitMask: [%s] (0x%4.4X)\r", String, AlarmBitMask);

        /* Display alarm current count-down value for every alarm. */
        for (Loop1UInt16 = 0; Loop1UInt16 < MAX_ALARMS; ++Loop1UInt16)
        {
          uart_send(__LINE__, __func__, "ActiveAlarm[%u].CountDown: %4u\r", Loop1UInt16, ActiveAlarm[Loop1UInt16].CountDown);
        }
      }


      if (DebugBitMask & DEBUG_EVENT)
      {
        /* Display triggered events bitmask. */
        util_uint64_to_binary_string((UINT64)EventBitMask, MAX_EVENTS, String);
        uart_send(__LINE__, __func__, "EventBitMask: [%s] (0x%16.16X)\r", String, EventBitMask);

        /* Display Event data for triggered events. */
        for (Loop1UInt16 = 0; Loop1UInt16 < MAX_EVENTS; ++Loop1UInt16)
        {
          if (Loop1UInt16 & (1 << Loop1UInt16))
          {
            uart_send(__LINE__, __func__, "Triggered event Number %u\r", Loop1UInt16);
            uart_send(__LINE__, __func__, "Event day:    %u\r",    FlashConfig1.Event[Loop1UInt16].Day);
            uart_send(__LINE__, __func__, "Event month:  %u\r",    FlashConfig1.Event[Loop1UInt16].Month);
            uart_send(__LINE__, __func__, "Event jingle: %u\r",    FlashConfig1.Event[Loop1UInt16].Jingle);
            uart_send(__LINE__, __func__, "Event message: <%s>\r", FlashConfig1.Event[Loop1UInt16].Message);
          }
        }
      }
    }



    /* --------------------------------------------------------------------------------------------------------------------------- *\
                     1-minute timestep and schedule mark. Put here functions that we want to execute every minute.
    \* --------------------------------------------------------------------------------------------------------------------------- */
    if ((CurrentTimer - LastTimer1Min) > 60000000ll)
    {
      LastTimer1Min = CurrentTimer;

      /* Reset Row 31 original border color in case we used DEBUG_STARTUP which uses the last row as a startup sequence progress indicator. */
      RGB_matrix_set_color(31, 0, 31, 63, Window[WIN_TIME].BorderColor);  /////

#ifdef DEVELOPER_VERSION
      if (stdio_usb_connected())
      {
        uart_send(__LINE__, __func__, "1-minute heartbeat... Absolute time reference: %6llu\r", time_us_64() / 1000000ll);


        if (DebugBitMask & DEBUG_REMINDER)
        {
          util_uint64_to_binary_string(Reminder1BitMask, MAX_REMINDERS1, String);
          uart_send(__LINE__, __func__, "Reminder1BitMask:                    0x%10.10llX   [%s]\r", Reminder1BitMask, String);
        }


#ifdef NTP_SUPPORT
        /***
        if (DebugBitMask & DEBUG_NTP)
        {
          uart_send(__LINE__, __func__, "=========================================================\r");
          uart_send(__LINE__, __func__, "               NTP info in the endless loop\r");
          display_ntp_info();
        }
        ***/
#endif  // NTP_SUPPORT
      }
#endif  // DEVELOPER_VERSION
    }



#ifdef NTP_SUPPORT
    /* Handle network time protocol (NTP) synchronization. */
    DeltaTime = (absolute_time_diff_us(get_absolute_time(), NTPData.NTPUpdateTime) / 1000000ll);
    if ((DeltaTime <= 0) || (is_nil_time(NTPData.NTPUpdateTime)))
    {
      if (DebugBitMask & DEBUG_NTP)
      {
        uart_send(__LINE__, __func__, "=========================================================\r");
        uart_send(__LINE__, __func__, "            Network Time Protocol cycle start\r");
        display_ntp_info();
      }


      if (NTPData.FlagNTPInit == FLAG_OFF)
      {
        if (DebugBitMask & DEBUG_NTP) uart_send(__LINE__, __func__, "Trying to initialize Wi-Fi connection...\r");
        if (ntp_init(FlashConfig1.SSID, FlashConfig1.Password) == false)
        {
          uart_send(__LINE__, __func__, "=========================================================\r");
          uart_send(__LINE__, __func__, "    ntp_init(): Failed to establish a Wi-Fi connection\r");
          NTPData.FlagNTPInit = FLAG_OFF;  // request a new ntp_init();
          display_ntp_info();
        }
      }
      else
      {
        if (DebugBitMask & DEBUG_NTP) uart_send(__LINE__, __func__, "Requesting RGB Matrix synchronization through NTP.\r\r");

        /* Initialize with invalid value on entry. */
        NTPData.FlagNTPSuccess = 0xFF;
        
        /* Retrieve UTC time from Network Time Protocol server. */
        ntp_get_time();

        /* Wait for NTP to return result. */
        for (Loop1UInt8 = 0; Loop1UInt8 < MAX_NTP_CHECKS; ++Loop1UInt8)
        {
          if (NTPData.FlagNTPSuccess == FLAG_POLL)
          {
            if (DebugBitMask & DEBUG_NTP)
            {
              uart_send(__LINE__, __func__, "\r\r\r\r");
              uart_send(__LINE__, __func__, "=========================================================\r");
              uart_send(__LINE__, __func__, "           Variables after successful NTP poll\r");
              display_ntp_info();
            }
            break;  // get out of "for" loop.
          }


          if (NTPData.FlagNTPSuccess == FLAG_ON)
          {
            NTPData.FlagNTPHistory = NTPData.FlagNTPSuccess;

            /* Convert UnixTime received from NTP server. */
            convert_unix_time(NTPData.UnixTime, &TempTime, &HumanTime, FLAG_ON);

            if (DebugBitMask & DEBUG_NTP)
            {
              uart_send(__LINE__, __func__, "\r\r\r\r");
              uart_send(__LINE__, __func__, "=========================================================\r");
              uart_send(__LINE__, __func__, "           Variables after successful NTP read\r");
              display_ntp_info();
              uart_send(__LINE__, __func__, "NTP synchronization succeeded (after %u retries)\r", Loop1UInt8);

              UnixTime = convert_human_to_unix(&CurrentTime, FLAG_ON);

              uart_send(__LINE__, __func__, "Current RGB-Matrix UnixTime:       %12llu\r", UnixTime);
              uart_send(__LINE__, __func__, "UnixTime returned from NTP:        %12llu\r", NTPData.UnixTime);
              uart_send(__LINE__, __func__, "Delta seconds between DS3231 and NTP server: %lld\r", NTPData.UnixTime - UnixTime);

              display_human_time("RGB Matrix time before resync:        ", &CurrentTime);
              display_human_time("HumanTime as decoded from NTP server: ", &HumanTime);
            }


            if (HumanTime.Second < 59)
            {
              /* Set time in the real-time clock IC while accounting for Internet latency. */
              sleep_ms(1000 - (NTPData.NTPLatency / 1000));
              ++HumanTime.Second;
            }
            ds3231_set_time(&HumanTime);

            NTPData.FlagNTPResync = FLAG_OFF;
            break;  // get out of "for" loop.
          }

          sleep_ms(500);
        }


        /* If current NTP update request failed. */
        if (Loop1UInt8 >= MAX_NTP_CHECKS)
        {
          NTPData.FlagNTPResync  = FLAG_OFF;  // NTP resync error... postpone re-sync.
          NTPData.FlagNTPHistory = NTPData.FlagNTPSuccess;
          ++NTPData.NTPErrors;
          if (DebugBitMask & DEBUG_NTP)
          {
            uart_send(__LINE__, __func__, "\r\r\r\r");
            uart_send(__LINE__, __func__, "=========================================================\r");
            uart_send(__LINE__, __func__, "           After failed NTP sync (%u retries)\r", Loop1UInt8);
            display_ntp_info();
          }

          /* If ntp_get_time() didn't work, try to re-initialize ntp in case the SSID and / or password has been updated lately,
             or in case network was down and is is back Up later. */
          NTPData.FlagNTPInit   = FLAG_OFF;  // request a new ntp_init();
          NTPData.NTPUpdateTime = delayed_by_ms(NTPData.NTPUpdateTime, ((UINT32)(NTP_REFRESH * 1000)));
        }
      }
    }
#endif  // NTP_SUPPORT

	  sleep_ms(1);  // slow down main system loop.
  }

	return 0;
}




/* $TITLE=alarm_check() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                                   Check if some alarms must be triggered.
\* ============================================================================================================================================================= */
void alarm_check(void)
{
#ifdef RELEASE_VERSION
  UINT8 FlagLocalDebug = FLAG_OFF;  // should be OFF all times
#else  // RELEASE_VERSION
  UINT8 FlagLocalDebug = FLAG_OFF;
#endif  // RELEASE_VERSION

  UCHAR String[31];

  UINT8 TotalActiveAlarms;

  UINT16 Loop1UInt16;


  // if (FlagLocalDebug) printf("Entering alarm_check()\r");

  /* Initialize total number of new active alarms. */
  TotalActiveAlarms = 0;


  /* Scan all alarms to find those matching current time and day-of-week. */
  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_ALARMS; ++Loop1UInt16)
  {
    if (FlashConfig1.Alarm[Loop1UInt16].FlagStatus != FLAG_ON)
    {
      // if (FlagLocalDebug) uart_send(__LINE__, __func__, "Alarm %u Off (%2.2u)\r", Loop1UInt16, FlashConfig1.Alarm[Loop1UInt16].FlagStatus);
      continue;
    }
    else
    {
      /* Display alarm status for debug purpose. */
      // if (FlagLocalDebug) uart_send(__LINE__, __func__, "Alarm %u ON  (%2.2u)\r", Loop1UInt16, FlashConfig1.Alarm[Loop1UInt16].FlagStatus);
    }


    /* Check if today is a target day for this alarm. */
    if ((FlashConfig1.Alarm[Loop1UInt16].DayMask & (1 << CurrentTime.DayOfWeek)) == 0)
    {
      // if (FlagLocalDebug) uart_send(__LINE__, __func__, "Alarm %u - DayMask does not match\r", Loop1UInt16);
      continue;
    }
    else
    {
      /* Display alarm status for debug purpose. */
      // if (FlagLocalDebug) uart_send(__LINE__, __func__, "Alarm %u - DayMask does match\r", Loop1UInt16);
    }


    if (FlashConfig1.Alarm[Loop1UInt16].Hour != CurrentTime.Hour)
    {
      // if (FlagLocalDebug) uart_send(__LINE__, __func__, "Alarm %u - Hour does not match (%u VS %u)\r", Loop1UInt16, FlashConfig1.Alarm[Loop1UInt16].Hour, CurrentTime.Hour);
      continue;
    }
    else
    {
      /* Display alarm status for debug purpose. */
      // if (FlagLocalDebug) uart_send(__LINE__, __func__, "Alarm %u - Hour does match (%u VS %u)\r", Loop1UInt16, FlashConfig1.Alarm[Loop1UInt16].Hour, CurrentTime.Hour);
    }


    if (FlashConfig1.Alarm[Loop1UInt16].Minute != CurrentTime.Minute)
    {
      // if (FlagLocalDebug) uart_send(__LINE__, __func__, "Alarm %u - Minute does not match (%u VS %u)\r", Loop1UInt16, FlashConfig1.Alarm[Loop1UInt16].Hour, CurrentTime.Hour);
      continue;
    }
    else
    {
      /* Display alarm status for debug purpose. */
      // if (FlagLocalDebug) uart_send(__LINE__, __func__, "Alarm %u - Minute does match (%u VS %u)\r", Loop1UInt16, FlashConfig1.Alarm[Loop1UInt16].Hour, CurrentTime.Hour);
    }

    AlarmBitMask |= ((UINT16)0x01 << Loop1UInt16);
    ActiveAlarm[Loop1UInt16].CountDown = FlashConfig1.Alarm[Loop1UInt16].RingDuration;  // global time alarm will be ringing.
    if (FlagLocalDebug) uart_send(__LINE__, __func__, "Al: %u   Mask: 0x%2.2X   CD: %u   (%s)\r", Loop1UInt16, AlarmBitMask, ActiveAlarm[Loop1UInt16].CountDown, FlashConfig1.Alarm[Loop1UInt16].Message);
  }

  // if (FlagLocalDebug) printf("Exiting alarm_check()\r");

  return;
}




/* $TITLE=alarm_ring() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                      Feed alarm ringer for currently active ("triggered") alarms.
\* ============================================================================================================================================================= */
void alarm_ring(void)
{
#ifdef RELEASE_VERSION
  UINT8 FlagLocalDebug = FLAG_OFF;  // should be OFF all times
#else  // RELEASE_VERSION
  UINT8 FlagLocalDebug = FLAG_OFF;
#endif  // RELEASE_VERSION

  UINT16 Loop1UInt16;

  UINT32 CurrentTimer;


  // if (FlagLocalDebug) printf("Entering alarm_ring()\r");

  /* When an alarm becomes active, check_alarm() will raise its bit in AlarmBitMask. */
  if (AlarmBitMask)
  {
    /* If some alarms are active, we'll need current timer to evaluate if it is time to feed ringer. */
    CurrentTimer = time_us_32();

    /* Scan alarm to find those requiring ringer repeat. */
    for (Loop1UInt16 = 0; Loop1UInt16 < MAX_ALARMS; ++Loop1UInt16)
    {
      /* Check if this alarm is active (global ring duration). */
      if (ActiveAlarm[Loop1UInt16].CountDown > 0)
      {
        /* Display details for first active alarm number for debug purpose. */
        if (FlagLocalDebug) uart_send(__LINE__, __func__, "0x%2.2X - %u - %3u - %3u - %9lu - %9lu - %4u\r", AlarmBitMask, Loop1UInt16, FlashConfig1.Alarm[Loop1UInt16].RepeatPeriod, ActiveAlarm[Loop1UInt16].CountDown, CurrentTimer, ActiveAlarm[Loop1UInt16].PreviousTimer, (UINT16)((CurrentTimer - ActiveAlarm[Loop1UInt16].PreviousTimer) / 1000000ll));

        if (((CurrentTimer - ActiveAlarm[Loop1UInt16].PreviousTimer) / 1000000ll) >= FlashConfig1.Alarm[Loop1UInt16].RepeatPeriod)
        {
          if (FlagLocalDebug) uart_send(__LINE__, __func__, "1) %4u - %3u", (UINT16)((CurrentTimer - ActiveAlarm[Loop1UInt16].PreviousTimer) / 1000000ll), FlashConfig1.Alarm[Loop1UInt16].RepeatPeriod);
          /* It is time to feed this ringer and repeat the scroll. */
          queue_add_active(FlashConfig1.Alarm[Loop1UInt16].BeepMSec, FlashConfig1.Alarm[Loop1UInt16].NumberOfBeeps);
          queue_add_active(2000, SILENT);
          win_scroll(WIN_DATE, 201, 201, 1, 1, FONT_5x7, "%s", FlashConfig1.Alarm[Loop1UInt16].Message);


          if (FlashConfig1.Alarm[Loop1UInt16].RepeatPeriod > ActiveAlarm[Loop1UInt16].CountDown)
          {
            /* Next ring would be beyond the global ringing period, reset it after current ring above. */
            ActiveAlarm[Loop1UInt16].CountDown     = 0;
            ActiveAlarm[Loop1UInt16].PreviousTimer = 0l;
            AlarmBitMask &= ~((UINT16)(0x01 << Loop1UInt16));  // turn Off this alarm number in the bitmask of active alarms.
          }
          else
          {
            ActiveAlarm[Loop1UInt16].CountDown -= FlashConfig1.Alarm[Loop1UInt16].RepeatPeriod;  // "FlashConfig1.Alarm[Loop1UInt16].RepeatPeriod" seconds have elapsed since last pass.
            if (ActiveAlarm[Loop1UInt16].CountDown > 0)
            {
              /* There are more rings to come. */
              ActiveAlarm[Loop1UInt16].PreviousTimer = CurrentTimer;
            }
            else
            {
              /* This alarm is over now. */
              ActiveAlarm[Loop1UInt16].PreviousTimer = 0l;
              AlarmBitMask &= ~((UINT16)(0x01 << Loop1UInt16));  // turn Off this alarm number in the bitmask of active alarms.
            }
          }
        }
      }
    }
  }

  // if (FlagLocalDebug) printf("Exiting alarm_ring()\r");

  return;
}





/* $TITLE=beep_tone() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                               Make a number of beeps through the buzzer.
                                   Function to be used until the passive buzzer circular buffer algorithm has been initialized.
\* ============================================================================================================================================================= */
void beep_tone(UINT8 RepeatCount)
{
	UINT8 Loop1UInt8;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering beep_tone()\r");

  for (Loop1UInt8 = 0; Loop1UInt8 < RepeatCount; ++Loop1UInt8)
  {
    gpio_put(BUZZER, 1);
    sleep_ms(50);
    gpio_put(BUZZER, 0);
    sleep_ms(50);
  }
  sleep_ms(1000);

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting beep_tone()\r");

  return;
}





/* $TITLE=callback_1msec_timer() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                                  Callback in charge of LED matrix scan.
\* ============================================================================================================================================================= */
bool callback_1msec_timer(struct repeating_timer *t)
{
  RGB_matrix_update(FrameBuffer);

  return true;
}





/* $TITLE=callback_50msec_timer() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                           Callback in charge of following activities:
                                                          - Remote control infrared reception.
                                                          - Text Scrolling.
                                                          - Active buzzer sound queue.
\* ============================================================================================================================================================= */
bool callback_50msec_timer(struct repeating_timer *t)
{
  UCHAR String[128];

  UINT8 IrButton;    // remote control button decoded from infrared data stream.
  UINT8 Loop1UInt8;
  UINT8 RowNumber;

  UINT64 Timer1;
  UINT64 Timer2;

  static UINT8 IrCycleCount;
  static UINT8 FlagActiveSound;
  static UINT8 FlagLocalDebug = FLAG_OFF;
  /// static UINT8  FlagPassiveSound;

  static UINT16 ActiveMSeconds;
  static UINT16 ActiveMSecCounter;
  static UINT16 ActiveRepeatCount;
  static UINT16 CurrentRepeat;
  /// static UINT16 Frequency;
  /// static UINT16 PassiveMSeconds;
  /// static UINT16 PassiveMSecCounter;


  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                 Manage infrared data stream reception.
  \* --------------------------------------------------------------------------------------------------------------------------- */
#ifdef REMOTE_SUPPORT
  if (IrStepCount)
  {
    /* If there is an infrared data stream now coming in, increment IrCycleCount, IR data stream will be decoded
       when IrCycleCount reaches the count of 3, to make sure data stream has been sent in its entirety. */
    ++IrCycleCount;

    // Debug line below should not be used inside callback context.
    // if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "\rIncrementing IrCycleCount to %u\r", IrCycleCount);

    /* If IrCycleCount reaches the value of 3, data stream began at least 100 msec ago, so it must be completed by now and ready to be processed. */
    if (IrCycleCount >= 3)
    {
      IrCycleCount = 0;  // reset IrCycleCount when infrared data stream has been entirely received to get ready for next IR data stream.

      /* Discard all spurious infrared bursts. */
      if (IrStepCount < 67)
      {
        if (DebugBitMask & DEBUG_IR) printf("\rIR Rejected %u\r\r", IrStepCount);
        IrStepCount = 0;  // received spurious IR burst, reset ir burst count.
      }
      else
      {
        /* If the number of IrStepCount seems valid, proceed with IR command decoding. */
        if (ir_decode_button(&IrButton) != IR_HI_LIMIT)
        {
          IrBuffer[0] = IrButton;  // put the button decoded in the IR buffer.
          ++IrCounter;
          if (DebugBitMask & DEBUG_IR)
          {
            printf("\r");
            uart_send(__LINE__, __func__, "Assign IrBuffer[0] = %u <%s>  (0x%2.2X)\r", IrButton, ButtonName[IrButton], IrButton);
          }
        }
      }
    }
  }
#endif  // REMOTE_CONTROL



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                 Manage active scrolling if there is one.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  for (Loop1UInt8 = 0; Loop1UInt8 < MAX_ACTIVE_SCROLL; ++Loop1UInt8)
  {
    if (ActiveScroll[Loop1UInt8] != 0x00l)
    {
      /* This ActiveScroll structure has been allocated, check who's the owner. */
      if (ActiveScroll[Loop1UInt8]->Owner == WinTop)
      {
        if (FlagLocalDebug) printf("<<< %u - %u >>>\r", Loop1UInt8, ActiveScroll[Loop1UInt8]->Owner);
        RGB_matrix_scroll(Loop1UInt8);
      }
      /***
      if (ActiveScroll[Loop1UInt8]->Owner == WinMid)
      {
        printf("++ 2 (%u - %u) ++  ", Loop1UInt8, ActiveScroll[Loop1UInt8]->Owner);
        // RGB_matrix_scroll(Loop1UInt8);
      }
      if (ActiveScroll[Loop1UInt8]->Owner == WinBot)
      {
        printf("++ 3 (%u - %u) ++  ", Loop1UInt8, ActiveScroll[Loop1UInt8]->Owner);
        // RGB_matrix_scroll(Loop1UInt8);
      }
      printf("\r");
      ***/
    }
  }



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                 Handling of active buzzer (the one integrated in the Pico-RGB-Matrix)
  \* --------------------------------------------------------------------------------------------------------------------------- */
  Timer1 = time_us_64();

  /* If there is a sound on-going on the active buzzer, check if it is completed. */
  if (FlagActiveSound == FLAG_ON)
  {
    if (DebugBitMask & DEBUG_SOUND_QUEUE)
    {
      if (ActiveRepeatCount == SILENT)
        sprintf(String, "- A-Silence     (%4u)\r", ActiveMSecCounter + 50);
      else
        sprintf(String, "- A-Sounding    (%4u)\r", ActiveMSecCounter + 50);

      uart_send(__LINE__, __func__, String);
    }

    ActiveMSecCounter += 50;  // 50 milliseconds more since last callback.

    if (ActiveMSecCounter >= ActiveMSeconds)
    {
      if (DebugBitMask & DEBUG_SOUND_QUEUE)
        uart_send(__LINE__, __func__, "- A-Shutoff\r");

      /* Current sound is over on active buzzer. Turn sound off to make a cut with next sound. */
      gpio_put(BUZZER, 0);

      /* One more repeat count has been done. */
      ++CurrentRepeat;

      /* Check if we reached the total repeat count required. */
      if (CurrentRepeat > ActiveRepeatCount) CurrentRepeat = 0;

      /* No active sound for now. */
      FlagActiveSound = FLAG_OFF;

      /* Reset sound elapsed time for now. */
      ActiveMSecCounter = 0;
    }
  }
  else
  {
    if (CurrentRepeat)
    {
      /* There are some more sound repeats to be done. */
      /* If RepeatCount is 0 ("SILENT"), we wanted to wait for specified duration without any sound. */
      if (ActiveRepeatCount != SILENT) gpio_put(BUZZER, 1);

      FlagActiveSound   = FLAG_ON;
      ActiveMSecCounter = 0;
    }
    else
    {
      /* Check if there are more sounds to play on active buzzer. Request duration and repeat count for next sound. */
      if (queue_remove_active(&ActiveMSeconds, &ActiveRepeatCount) == 0xFF)
      {
        /* Either there is no more sound, either there was an error while trying to unqueue next sound (corrupted sound queue).
        /* If sound_unqueue_active() is empty or unqueue failed, make sure audio is turned off. */
        gpio_put(BUZZER, 0);
      }
      else
      {
        if (DebugBitMask & DEBUG_SOUND_QUEUE)
          uart_send(__LINE__, __func__, "- A-Unqueued:            %5u   %5u\r", ActiveMSeconds, ActiveRepeatCount);

        /* If RepeatCount is 0 ("SILENT"), we wanted to wait for specified duration without any sound. */
        if (ActiveRepeatCount != SILENT) gpio_put(BUZZER, 1);
        FlagActiveSound   = FLAG_ON;
        CurrentRepeat     = 1;
        ActiveMSecCounter = 0;
      }
    }
  }


#if 0
#ifdef PASSIVE_BUZZER_SUPPORT
  /* --------------------------------------------------------------------------------------------------------------------------- *\
             Handling of passive buzzer (if user replaced the one installed in the Pico-RGB-Matrix - or add a new one).
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (FlagPassiveSound == FLAG_ON)
  {
    if (DebugBitMask & DEBUG_SOUND_QUEUE)
    {
      if (Frequency == SILENT)
        sprintf(String, "- P-Silence     (%4u)\r", PassiveMSecCounter + 50);
      else
        sprintf(String, "- P-Sounding    (%4u)\r", PassiveMSecCounter + 50);

      uart_send(__LINE__, __func__, String);
    }

    /* There is a sound on-going on the passive buzzer, check if it is completed. */
    PassiveMSecCounter += 50;  // 50 milliseconds more since last callback.

    if (PassiveMSecCounter >= PassiveMSeconds)
    {
      if (DebugBitMask & DEBUG_SOUND_QUEUE)
        uart_send(__LINE__, __func__, "- P-Shutoff\r");

      /* Current sound on passive buzzer is over. Turn sound Off to make a cut with next sound. */
      pwm_on_off(PWM_SOUND, FLAG_OFF);

      /* No active sound for now. */
      FlagPassiveSound = FLAG_OFF;

      /* Reset sound elapsed time for now. */
      PassiveMSecCounter = 0;
    }
  }
  else
  {
    /* If we asked passive sound queue to wait for active sound queue to complete and sound queue is still active, skip this callback cycle. */
    if (FlagPassiveSound == FLAG_WAIT)
    {
      /* Check if active sound queue is done for now. */
      if ((SoundActiveHead != SoundActiveTail) || (FlagActiveSound == FLAG_ON) || (CurrentRepeat != 0))
      {
        if (DebugBitMask & DEBUG_SOUND_QUEUE)
          uart_send(__LINE__, __func__, "- P-Waiting\r");

        /* Active sound queue not done yet. */
        return TRUE;
      }
      else
      {
        /* Active sound queue done for now, short pause before triggering passive buzzer. */
        FlagPassiveSound = FLAG_ON;
        PassiveMSeconds  = 400;  // 400 milliseconds pause between active buzzer and passive buzzer.

        return TRUE;
      }
    }


    /* Check if there are more sounds to play on passive buzzer. Request frequency and duration for next sound. */
    if (sound_unqueue_passive(&Frequency, &PassiveMSeconds) == 0xFF)
    {
      /* Either there is no more sound for passive buzzer, either there was an error while trying to unqueue next sound (corrupted sound queue). */
      /* If sound_unqueue_passive() failed, make sure audio PWM is turned off. */
      pwm_on_off(PWM_SOUND, FLAG_OFF);
      FlagPassiveSound = FLAG_OFF;
    }
    else
    {
      if (DebugBitMask & DEBUG_SOUND_QUEUE)
        uart_send(__LINE__, __func__, "- P-Unqueued:            %5u   %5u\r", Frequency, PassiveMSeconds);

      /* If frequency is 0 ("SILENT"), we wanted to wait for specified duration without any sound. */
      if (Frequency == SILENT)
      {
        if (PassiveMSeconds == WAIT_4_ACTIVE)
          FlagPassiveSound   = FLAG_WAIT;
        else
          FlagPassiveSound   = FLAG_ON;
      }
      else
      {
        /* Set new sound frequency... */
        pwm_set_frequency(Frequency);

        /* ...and turn On sound. */
        pwm_on_off(PWM_SOUND, FLAG_ON);
        FlagPassiveSound   = FLAG_ON;
      }
      PassiveMSecCounter = 0;
    }
  }
#endif  // PASSIVE_BUZZER_SUPPORT
#endif  // 0

  /***
  if (DebugBitMask & DEBUG_SOUND_QUEUE)
  {
    Timer2 = time_us_64();

    uart_send(__LINE__, __func__, "Sound callback: %5.2f msec\r", ((Timer2 - Timer1) / 1000.0));
  }
  ***/

  return true;
}





#if 0  // Used with core 1
/* $TITLE=callback_display_time() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                       One-second callback to update date and time on LED matrix.
\* ============================================================================================================================================================= */
bool callback_display_time(struct repeating_timer *t)
{
  UINT8 FlagLocalDebug;


  FlagLocalDebug = FLAG_OFF;

 /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                  Update date and time on RGB matrix.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (FlagLocalDebug) printf("%4u   3\r", __LINE__);
  /// if (FlagLocalDebug) printf("%4u   Before updating time\r", __LINE__);
  if (FlagFrameBufferBusy == FLAG_OFF) RGB_matrix_display_time();  /////
}
#endif  // 0





/* $TITLE=callback_1000msec_timer() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                       One-second callback to update date and time on LED matrix.
\* ============================================================================================================================================================= */
bool callback_1000msec_timer(struct repeating_timer *t)
{
  static UINT8 OneSecondCounter;
  static UINT8 PreviousDay;

  static UINT16 WatchdogPreviousCheck;

  UCHAR String[65];

  UINT8 ColumnNumber;
  UINT8 FlagLocalDebug = FLAG_OFF;

  UINT16 Loop1UInt16;

  UINT32 LocalCurrentTimer;


  /* --------------------------------------------------------------------------------------------------------------------------- *\
                             Mechanism to evaluate the actual time spent in the 1000MSec callback function.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (FlagLocalDebug) printf("%4u   1\r", __LINE__);
  ++OneSecondCounter;
  if (OneSecondCounter >= 7)
  {
    OneSecondCounter = 0;
    AbsoluteEntryTime = get_absolute_time();
  }



#ifdef WATCHDOG_SUPPORT
  /* --------------------------------------------------------------------------------------------------------------------------- *\
                              Watchdog mechanism to recover from a RGB Matrix endless loop thread crash.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  /* Check if endless loop (main thread) has crancked up watchdog check counter. */
  if (FlagLocalDebug) printf("%4u   2\r", __LINE__);
  if (WinTop == WIN_DATE)
  {
    if (WatchdogCheck != WatchdogPreviousCheck)
    {
      WatchdogPreviousCheck = WatchdogCheck;
      WatchdogMiss = 0;  // main thread is still alive, reset cumulative misses.
      RGB_matrix_set_color(18, 0, 18, 63, Window[WIN_TIME].BorderColor);  // reset box border original color.
    }
    else
    {
      /* Main thread seems not to be alive. It may be due to navigation through terminal menus, so let some time
         for the user to complete what he has to do before triggering a Firmware restart). */
      ++WatchdogMiss;
      if (WatchdogMiss >= WATCHDOG_SECONDS) software_reset();
      if (FlagLocalDebug) printf("\r%2u / %3u\r", WatchdogMiss, WATCHDOG_SECONDS);

      /* Display an incremental indicator on LED display to show "how soon" the restart will be triggered. */
      ColumnNumber = (UINT8)((WatchdogMiss / (WATCHDOG_SECONDS / (MAX_COLUMNS / 2))) - 0.5);

      if (ColumnNumber > 0)
      {
        /* LED on the left side of the line. */
        RGB_matrix_set_color(18, ColumnNumber, 18, ColumnNumber, YELLOW);

        /* LED on the right side of the line. */
        ColumnNumber = (63 - ColumnNumber);
        RGB_matrix_set_color(18, ColumnNumber, 18, ColumnNumber, YELLOW);
      }
    }
  }
#endif  // WATCHDOG_SUPPORT



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                  Update date and time on RGB matrix.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  /* NOTE: Make sure date and time updating is Off in core 1. */
  if (FlagLocalDebug) printf("%4u   3\r", __LINE__);
  /// if (FlagLocalDebug) printf("%4u   Before updating time\r", __LINE__);
  if (FlagFrameBufferBusy == FLAG_OFF) RGB_matrix_display_time();  /////



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                       Adjust automatic brightness.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (FlagLocalDebug) printf("%4u   4\r", __LINE__);
  /// if (FlagLocalDebug) printf("%4u   Before auto-brightness\r", __LINE__);

  /* If user turned On service light timer, let the brightness to highest value and decrement service light timer. */
  if (ServiceLightTimer == 0)
  {
    if (FlashConfig1.FlagAutoBrightness == FLAG_ON) set_auto_brightness();
  }
  else
  {
    /// printf("%4u   ServiceLightTimer value: %3u\r", __LINE__, ServiceLightTimer);
    --ServiceLightTimer;
  }



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                        Adjust infrared indicator.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (FlagLocalDebug) printf("%4u   5\r", __LINE__);
  if (IrIndicator > 0)
  {
    --IrIndicator;
    /* Turn Off infrared LED indicators on RGB matrix after a few seconds timeout (restore original window border color). */
    if (IrIndicator == 0)
    {
      /* Reset color to WIN_TIME border color. */
      RGB_matrix_set_color(IR_INDICATOR_START_ROW, IR_INDICATOR_START_COLUMN, IR_INDICATOR_END_ROW, IR_INDICATOR_END_COLUMN, Window[WinBot].BorderColor);

      /* Clear pixels of the extra line used for IR indicator. */
      RGB_matrix_clear_pixel(FrameBuffer, IR_INDICATOR_END_ROW, IR_INDICATOR_START_COLUMN, IR_INDICATOR_END_ROW, IR_INDICATOR_END_COLUMN);
    }
  }



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                        Manage hourly chime.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (FlagLocalDebug) printf("%4u   6\r", __LINE__);
  /// if (FlagLocalDebug) printf("%4u   Before hourly chime\r", __LINE__);
  if ((CurrentTime.Second == 0) && (CurrentTime.Minute == 0))
  {
    /* Hour has just changed... manage hourly chime as required. */
    if ((FlashConfig1.ChimeMode == FLAG_ON) ||
       ((FlashConfig1.ChimeMode == FLAG_DAY) && ((CurrentTime.Hour >= FlashConfig1.ChimeTimeOn) && (CurrentTime.Hour <= FlashConfig1.ChimeTimeOff))))
    {
      for (Loop1UInt16 = 0; Loop1UInt16 < 3; ++Loop1UInt16)
      {
        queue_add_active(50, 2);
        queue_add_active(50, SILENT);
      }
      queue_add_active(100, SILENT);
    }
  }



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                        Manage half-hour light chime.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (FlagLocalDebug) printf("%4u   7\r", __LINE__);
  /// if (FlagLocalDebug) printf("%4u   Before half-hour light chime\r", __LINE__);
  if ((CurrentTime.Second == 0) && (CurrentTime.Minute == 30))
  {
    /* We just reached half-hour... manage information as required. */
    if ((FlashConfig1.ChimeMode == FLAG_ON) ||
       ((FlashConfig1.ChimeMode == FLAG_DAY) && ((CurrentTime.Hour > FlashConfig1.ChimeTimeOn) && (CurrentTime.Hour < FlashConfig1.ChimeTimeOff))))
    {
      queue_add_active(50, 2);
      queue_add_active(100, SILENT);
    }
  }



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                         Manage reminders.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (FlagLocalDebug) printf("%4u   8\r", __LINE__);
  /// if (FlagLocalDebug) printf("%4u   Before reminders management\r", __LINE__);

  /* At minute change, check if new reminders must be triggered. */
  /* Note: <CurrentTime> has just been updated by RGB_matrix_display_time() above. */
  /// if (CurrentTime.Second == 0) reminder1_check();

  /* Check if it is time to feed rings for some active alarms (ringer feeding is not always on a minute boundary). */
  /// reminder1_ring();




  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                        Manage calendar events.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (FlagLocalDebug) printf("%4u   9\r", __LINE__);
  /// if (FlagLocalDebug) printf("%4u   Before calendar events\r", __LINE__);
  /* At date change, check if some calendar events must be triggered. */
  if (CurrentTime.DayOfMonth != PreviousDay)
  {
    /* Clear current event bitmask. */
    EventBitMask = 0ll;

    /* Check for eventual today's events. */
    event_check();

    /* We just checked calendar events for today's date. */
    PreviousDay = CurrentTime.DayOfMonth;
  }


  if (FlagLocalDebug) printf("%4u   10\r", __LINE__);
  /* Feed event ringer every half hour, at xx:05 and xx:35. */
  /// if (FlagLocalDebug)
  /// {
  ///   util_uint64_to_binary_string(EventBitMask, 64, String);
  ///   printf("%4u   Current EventBitMask: 0x%16.16llX <%s>\r", __LINE__, EventBitMask, String);
  /// }


  if (EventBitMask)
  {
    if (FlagLocalDebug) printf("%4u   11\r", __LINE__);

    /* There are some triggered events, feed ringer when required. */
    if ((CurrentTime.Second == 0) && ((CurrentTime.Minute == 5) || (CurrentTime.Minute == 35)))
    {
      if (FlagLocalDebug) printf("%4u   12\r", __LINE__);

      if ((CurrentTime.Hour >= FlashConfig1.ChimeTimeOn) && (CurrentTime.Hour <= FlashConfig1.ChimeTimeOff))
      {
        /* Feed one ringer for all events. */
        queue_add_active(250, 5);
        queue_add_active(400, SILENT);
        queue_add_active(250, 5);
        queue_add_active(5000, SILENT);  // make sure to isolate this sound train from any eventual others that could follow.
      }

      if (FlagLocalDebug) printf("%4u   13\r", __LINE__);

      for (Loop1UInt16 = 0; Loop1UInt16 < MAX_EVENTS; ++Loop1UInt16)
      {
        if (FlagLocalDebug) printf("%4u   14\r", __LINE__);

        if (EventBitMask & (1 << Loop1UInt16))
        {
          if (FlagLocalDebug) printf("%4u   15\r", __LINE__);

          /* Scroll message for this triggered event on RGB matrix. */
          win_scroll(WIN_DATE, 201, 201, 3, 1, FONT_5x7, "%s", FlashConfig1.Event[Loop1UInt16].Message);
        }
      }
    }
  }



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                            Manage alarms.
             Keep it at the bottom of the list so that this is what will remain on RGB display at the end of checkings.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (FlagLocalDebug) printf("%4u   16\r", __LINE__);
  /// if (FlagLocalDebug) printf("%4u   Before alarm management\r", __LINE__);

  /* At minute change, check if new alarms must be triggered. */
  /* Note: <CurrentTime> has just been updated by RGB_matrix_display_time() above. */
  if (CurrentTime.Second == 0) alarm_check();

  /* Check if it is time to feed rings for some active alarms (ringer feeding is not always on a minute boundary). */
  alarm_ring();



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                          Manage auto-scrolls.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (FlagLocalDebug) printf("%4u   17\r", __LINE__);
  if (CurrentTime.Second == 5)
  {
    /* Check if one of the auto-scrolls defined has reached its repeat period. */
    for (Loop1UInt16 = 0; Loop1UInt16 < MAX_AUTO_SCROLLS; ++Loop1UInt16)
    {
      /// printf("#%u   Pr: %u   CT.min: %u  CT.sec: %u\r", Loop1UInt16, FlashConfig1.AutoScroll[Loop1UInt16].Period, CurrentTime.Minute, CurrentTime.Second);
      /// printf("min pct period: %u\r", (CurrentTime.Minute % FlashConfig1.AutoScroll[Loop1UInt16].Period));
      if ((FlashConfig1.AutoScroll[Loop1UInt16].Period != 0) && ((CurrentTime.Minute % FlashConfig1.AutoScroll[Loop1UInt16].Period) == 0))
      {
        /* We reached the repeat period for this auto-scroll -> set the corresponding bit in the bitmask. It will be processed by the main system loop
           when available. This prevent the scrolling to interfere with user operation and it also prevent the scrolling buffer to fill-up if scrolling
           is not currently available. */
        /// printf("Adding %u\r", Loop1UInt16);
        AutoScrollBitMask |= (0x01 << Loop1UInt16);
      }
    }
  }



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                               Manage blinking area of active windows if there are some that are active.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (FlagLocalDebug) printf("%4u   18\r", __LINE__);
  /// if (FlagLocalDebug) printf("%4u   Before blinking\r", __LINE__);
  if ((Window[WinTop].FlagBlink) || (Window[WinMid].FlagBlink) || (Window[WinBot].FlagBlink)) RGB_matrix_blink();



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                               Mechanism to find the actual time spent in the 1-second callback routine.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (FlagLocalDebug) printf("%4u   19\r", __LINE__);
  if (OneSecondCounter == 0)
  {
    AbsoluteExitTime = get_absolute_time();
  }

  return true;
}





/* $PAGE */
/* $TITLE=convert_human_to_tm() */
/* ============================================================================================================================================================= *\
                                                                  Convert "HumanTime" to "tm_time".
\* ============================================================================================================================================================= */
/* Convert "HumanTime" to "tm_time".*/
void convert_human_to_tm(struct human_time *HumanTime, struct tm *TmTime)
{
  TmTime->tm_mday  = HumanTime->DayOfMonth;     // tm_mday: 1 to 31
  TmTime->tm_mon   = HumanTime->Month - 1;      // tm_mon:  months since January (0 to 11)
  TmTime->tm_year  = HumanTime->Year - 1900;    // tm_year: years since 1900
  TmTime->tm_wday  = HumanTime->DayOfWeek;      // tm_wday: Sunday = 0 (...)  Saturday = 6
  TmTime->tm_yday  = HumanTime->DayOfYear - 1;  // tm_yday: 0 to 365
  TmTime->tm_hour  = HumanTime->Hour;           // tm_hour: 0 to 23
  TmTime->tm_min   = HumanTime->Minute;         // tm_min:  0 to 59
  TmTime->tm_sec   = HumanTime->Second;         // tm_sec:  0 to 59
  TmTime->tm_isdst = 0;                         // tm_isdst: (if < 0 means not used)   (if > 0 means FLAG_ON)   (if = 0 means FLAG_OFF)

  return;
}





/* $PAGE */
/* $TITLE=convert_human_to_unix() */
/* ============================================================================================================================================================= *\
                                                                  Convert "HumanTime" to "Unix Time".
                                                         NOTE: Unix Time is based on UCT time, not local time.
\* ============================================================================================================================================================= */
UINT64 convert_human_to_unix(struct human_time *HumanTime, UINT8 FlagLocalTime)
{
  UINT64 UnixTime;

  struct tm TempTime;


  convert_human_to_tm(HumanTime, &TempTime);
  UnixTime = convert_tm_to_unix(&TempTime, FlagLocalTime);

  return UnixTime;
}





/* $PAGE */
/* $TITLE=convert_tm_to_unix() */
/* ============================================================================================================================================================= *\
                                                                     Convert "TmTime" to "Unix Time".
                                                         NOTE: Unix Time is based on UCT time, not local time.
\* ============================================================================================================================================================= */
UINT64 convert_tm_to_unix(struct tm *TmTime, UINT8 FlagLocalTime)
{
  time_t UnixTime;


  UnixTime = mktime(TmTime);
  if (FlagLocalTime) UnixTime -= FlashConfig1.Timezone * 60 * 60;

  return UnixTime;
}





/* $PAGE */
/* $TITLE=convert_unix_time() */
/* ============================================================================================================================================================= *\
                                                             Convert Unix time to tm time and human time.
\* ============================================================================================================================================================= */
void convert_unix_time(time_t UnixTime, struct tm *TmTime, struct human_time *HumanTime, UINT8 FlagLocalTime)
{
  struct tm TempTime;


  /* If caller asked to care about local time conversion, consider Timezone in the equation after converting it from hours to seconds. */
  if (FlagLocalTime == FLAG_ON) UnixTime += (FlashConfig1.Timezone * 60 * 60);

  /* Find tm_time */
  TempTime = *localtime(&UnixTime);


  /*** Transfer to calling structure. *** Must be optimized. ***/
  TmTime->tm_hour  = TempTime.tm_hour;
  TmTime->tm_min   = TempTime.tm_min;
  TmTime->tm_sec   = TempTime.tm_sec;
  TmTime->tm_mday  = TempTime.tm_mday;
  TmTime->tm_mon   = TempTime.tm_mon;
  TmTime->tm_year  = TempTime.tm_year;
  TmTime->tm_wday  = TempTime.tm_wday;
  TmTime->tm_yday  = TempTime.tm_yday;
  TmTime->tm_isdst = TempTime.tm_isdst;


  /* Find equivalent in human time. */
  HumanTime->Hour       = TmTime->tm_hour;
  HumanTime->Minute     = TmTime->tm_min;
  HumanTime->Second     = TmTime->tm_sec;
  HumanTime->DayOfMonth = TmTime->tm_mday;
  HumanTime->Month      = TmTime->tm_mon + 1;
  HumanTime->Year       = TmTime->tm_year + 1900;
  HumanTime->DayOfWeek  = TmTime->tm_wday;
  HumanTime->DayOfYear  = TmTime->tm_yday;
  HumanTime->FlagDst    = TmTime->tm_isdst;

#if 0
  if (DebugBitMask & DEBUG_NTP)
  {
    display_human_time("HumanTime after conversion from UnixTime: ", HumanTime);

    uart_send(__LINE__, __func__, "HumanTime->Hour        = %u\r", HumanTime->Hour);
    uart_send(__LINE__, __func__, "HumanTime->Minute      = %u\r", HumanTime->Minute);
    uart_send(__LINE__, __func__, "HumanTime->Second      = %u\r", HumanTime->Second);
    uart_send(__LINE__, __func__, "HumanTime->DayOfMonth  = %u\r", HumanTime->DayOfMonth);
    uart_send(__LINE__, __func__, "HumanTime->Month       = %u\r", HumanTime->Month);
    uart_send(__LINE__, __func__, "HumanTime->Year        = %u\r", HumanTime->Year);
    uart_send(__LINE__, __func__, "HumanTime->DayOfWeek   = %u\r", HumanTime->DayOfWeek);
    uart_send(__LINE__, __func__, "HumanTime->DayOfYear   = %u\r", HumanTime->DayOfYear);
    uart_send(__LINE__, __func__, "HumanTime->FlagDst     = %u\r\r\r", HumanTime->FlagDst);
  }
#endif  // 0

  return;
}





/* $PAGE */
/* $TITLE=core1_main() */
/* ============================================================================================================================================================= *\
                                                          Thread to be run on Pico's core 1 (second core).
                           Core 1 is in charge of receiving infrared data streams and monitoring hardware interrupts for local buttons.
\* ============================================================================================================================================================= */
void core1_main(void)
{
  if (DebugBitMask & DEBUG_CORE) printf("Entering core1_main()\r");

  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Before launching ISR for IR sensor.\r", __LINE__);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }

  /* Let the IR sensor be initialized even if we don't use the remote control since it is
     the base irq service routine eventual infrared glitches will simply be ignored. */
  gpio_set_irq_enabled_with_callback(IR_RX, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, (gpio_irq_callback_t)&isr_signal_trap);


  /* Since the gpio callback has been set above for infrared sensor, simply add other GPIO's to the same callback. */
  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Adding button <Set> to the ISR callback.\r", __LINE__);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }
  gpio_set_irq_enabled(BUTTON_SET_GPIO,  GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);


  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Adding button <Up> to the ISR callback.\r", __LINE__);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }
  gpio_set_irq_enabled(BUTTON_UP_GPIO,   GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);


  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Adding button <Down> to the ISR callback.\r", __LINE__);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }
  gpio_set_irq_enabled(BUTTON_DOWN_GPIO, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);


  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                          Start the 1 msec callback in charge of matrix scan.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  /// add_repeating_timer_ms(-1, callback_1msec_timer, NULL, &Handle1MSecTimer);



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                         Start the callback in charge of time update on RGB matrix.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  /* One second callback for time update on LED matrix. */
#if 0
  if (DebugBitMask & DEBUG_STARTUP)
  {
    printf("[%4u]   Before launching time update callback.\r", __LINE__);
    debug_pixel(31, 17, BLUE);
    sleep_ms(1000);  // slow down startup sequence if required for debugging purposes.
  }
  add_repeating_timer_ms(-1000, callback_display_time, NULL, &Handle1000MSecTimer);
#endif  // 0


  /* Keep the core 1 alive. */
  while(1) sleep_ms(1000);
}





/* $TITLE=debug_pixel() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                              Turn On a pixel for debugging purpose.
                                                     See an example of its use during the start-up sequence.
                         This could be used to test some behavior when there must not be a terminal emulator connected to the system.
\* ============================================================================================================================================================= */
void debug_pixel(UINT8 RowNumber, UINT8 ColumnNumber, UINT8 Color)
{
  RGB_matrix_set_color(RowNumber, ColumnNumber, RowNumber, ColumnNumber, Color);
  RGB_matrix_set_pixel(FrameBuffer, RowNumber, ColumnNumber, RowNumber, ColumnNumber);

  return;
}





/* $TITLE=display_alarm() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                                Display specified alarm number.
\* ============================================================================================================================================================= */
void display_alarm(UINT8 AlarmNumber)
{
  UCHAR DayMask[11];
  UCHAR String[61];

  UINT8 Loop1UInt8;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering display_alarm()\r");

  /* Prepare a string indicating if this alarm is On or Off. */
  if (FlashConfig1.Alarm[AlarmNumber].FlagStatus)
    sprintf(String, "***** ON *****");
  else
    String[0] = 0x00;

  printf("------------------------------ Alarm number %u ------------------------------\r", AlarmNumber + 1);
  printf("Alarm[%2.2u].Status:             %2.2u     (00 = Off   01 = On)     %s\r",        AlarmNumber + 1, FlashConfig1.Alarm[AlarmNumber].FlagStatus, String);
  printf("Alarm[%2.2u].Hour:               %2.2u\r",                                        AlarmNumber + 1, FlashConfig1.Alarm[AlarmNumber].Hour);
  printf("Alarm[%2.2u].Minute:             %2.2u\r",                                        AlarmNumber + 1, FlashConfig1.Alarm[AlarmNumber].Minute);


  /* Display taarget days-of-week. */
  util_uint64_to_binary_string(FlashConfig1.Alarm[AlarmNumber].DayMask, 8, DayMask);
  sprintf(String, "Alarm[%2.2u].DayMask:      %s        (0x%2.2X) ", AlarmNumber + 1, DayMask, FlashConfig1.Alarm[AlarmNumber].DayMask);

  /* Display short name of all days-of-week selected for this alarms (those selected in the bit mask). */
  for (Loop1UInt8 = 0; Loop1UInt8 < 7; ++Loop1UInt8)
  {
    if (FlashConfig1.Alarm[AlarmNumber].DayMask & (1 << Loop1UInt8))
      sprintf(&String[strlen(String)], "%s ", ShortDay[Loop1UInt8]);
  }
  printf("%s\r", String);


  printf("Alarm[%2.2u].NumberOfBeeps:     %3u\r",                           AlarmNumber + 1, FlashConfig1.Alarm[AlarmNumber].NumberOfBeeps);
  printf("Alarm[%2.2u].BeepMSec:          %3u msec\r",                      AlarmNumber + 1, FlashConfig1.Alarm[AlarmNumber].BeepMSec);
  printf("Alarm[%2.2u].RepeatPeriod:     %4u seconds\r",                    AlarmNumber + 1, FlashConfig1.Alarm[AlarmNumber].RepeatPeriod);
  printf("Alarm[%2.2u].RingDuration:     %4u seconds (global ring time)\r", AlarmNumber + 1, FlashConfig1.Alarm[AlarmNumber].RingDuration);


  /* Display message for this alarm. */
  printf("Alarm[%2.2u].Message:               <", AlarmNumber + 1);
  for (Loop1UInt8 = 0; Loop1UInt8 < sizeof(FlashConfig1.Alarm[AlarmNumber].Message); ++Loop1UInt8)
  {
    if  (FlashConfig1.Alarm[AlarmNumber].Message[Loop1UInt8] == 0x00) break;  // get out of <for> loop when reaching end-of-string.
    if ((FlashConfig1.Alarm[AlarmNumber].Message[Loop1UInt8] >= 0x20) && (FlashConfig1.Alarm[AlarmNumber].Message[Loop1UInt8] <= 0x7E))
      printf("%c", FlashConfig1.Alarm[AlarmNumber].Message[Loop1UInt8]);
    else
      printf("?");
  }
  printf(">\r");


  /* Display number of scrolls at each ring-time. */
  printf("Alarm[%2.2u].NumberOfScrolls:   %3u\r\r", AlarmNumber + 1, FlashConfig1.Alarm[AlarmNumber].NumberOfScrolls);


  if (DebugBitMask & DEBUG_FLOW) printf("Exiting display_alarm()\r");

  return;
}





/* $TITLE=display_auto_scroll() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                            Display auto-scrolls defined in the system.
\* ============================================================================================================================================================= */
void display_auto_scroll(UINT8 AutoScrollNumber)
{
  UCHAR String[65];

  UINT8 Loop1UInt8;

  UINT16 FunctionNumber;


  printf("      Auto-scroll number %u        Scroll period: %u minutes.\r\r", AutoScrollNumber + 1, FlashConfig1.AutoScroll[AutoScrollNumber].Period);
  printf("      Items being scrolled:\r\r");
  printf(" Item   Function    Function           Function\r");
  printf("Number   Number        ID                Name\r\r");
  printf("------  --------    --------   -------------------------\r\r");


  /* Display all items to be scrolled with this auto-scroll. */
  for (Loop1UInt8 = 0; Loop1UInt8 < MAX_ITEMS; ++Loop1UInt8)
  {
    /* If this item is a valid FunctionNumber, display the corresponding FunctionId. */
    if (FlashConfig1.AutoScroll[AutoScrollNumber].FunctionId[Loop1UInt8] == 0)
    {
      printf("  %2u       ---         %3u      -----------------------\r", Loop1UInt8 + 1, FlashConfig1.AutoScroll[AutoScrollNumber].FunctionId[Loop1UInt8]);
    }
    else
    {
      /* Display FunctionNumber. */
      FunctionNumber = get_function_number(FlashConfig1.AutoScroll[AutoScrollNumber].FunctionId[Loop1UInt8], String);

      if (FunctionNumber == MAX_FUNCTIONS)
      {
        printf("  %2u       ---         %3u      -----------------------\r", Loop1UInt8 + 1, FlashConfig1.AutoScroll[AutoScrollNumber].FunctionId[Loop1UInt8]);
      }
      else
      {
        printf("  %2u      (%3u)        %3u      %s\r", Loop1UInt8 + 1, FunctionNumber, FlashConfig1.AutoScroll[AutoScrollNumber].FunctionId[Loop1UInt8], String);
      }
    }
  }
  printf("------------------------------------------------------------------------\r\r");

  return;
}





/* $TITLE=display_event() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                                Display specified event number.
\* ============================================================================================================================================================= */
void display_event(UINT8 EventNumber)
{
  UINT8 Loop1UInt8;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering display_event()\r");

  printf("------------------------- Event number %u -------------------------\r", EventNumber + 1);
  printf("Event[%2.2u].Day:        %2.2u\r",      EventNumber + 1, FlashConfig1.Event[EventNumber].Day);
  printf("Event[%2.2u].Month:      %2.2u (%s)\r", EventNumber + 1, FlashConfig1.Event[EventNumber].Month, ShortMonth[FlashConfig1.Event[EventNumber].Month]);
  printf("Event[%2.2u].Jingle:     %2.2u\r",      EventNumber + 1, FlashConfig1.Event[EventNumber].Jingle);



  /* Display message for this event. */
  printf("Event[%2.2u].Message:    <", EventNumber + 1);
  for (Loop1UInt8 = 0; Loop1UInt8 < sizeof(FlashConfig1.Event[EventNumber].Message); ++Loop1UInt8)
  {
    if  (FlashConfig1.Event[EventNumber].Message[Loop1UInt8] == 0x00) break;  // get out of <for> loop when reaching end-of-string.
    if ((FlashConfig1.Event[EventNumber].Message[Loop1UInt8] >= 0x20) && (FlashConfig1.Event[EventNumber].Message[Loop1UInt8] <= 0x7E))
      printf("%c", FlashConfig1.Event[EventNumber].Message[Loop1UInt8]);
    else
      printf("?");
  }
  printf(">\r\r");

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting display_event()\r");

  return;
}





/* $TITLE=display_function() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                                  Display function parameters.
\* ============================================================================================================================================================= */
void display_function(UINT16 FunctionNumber)
{
  printf("------------------------- Function number %u -------------------------\r\r", FunctionNumber);
  printf("Function[%2.2u].Id:      %3.3u\r",  FunctionNumber, Function[FunctionNumber].Id);
  printf("Function[%2.2u].Name:    %s\r",     FunctionNumber, Function[FunctionNumber].Name);
  printf("Function[%2.2u].Pointer: %p\r\r\r", FunctionNumber, Function[FunctionNumber].Pointer);

  return;
}





/* $TITLE=display_function_id() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                     Display function parameters for the function category specified ("setup", "info", "tools").
\* ============================================================================================================================================================= */
void display_function_id(UINT16 FunctionTypeStart, UINT16 FunctionTypeEnd)
{
  UINT16 Loop1UInt16;
  UINT16 StartFunctionNumber;
  UINT16 EndFunctionNumber;


  StartFunctionNumber = 9999;  // assign invalid value on entry.
  /* Find the first and last FunctionNumber for those functions ID in the category specified. */
  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_FUNCTIONS; ++Loop1UInt16)
  {
    if ((Function[Loop1UInt16].Id > FunctionTypeStart) && (Function[Loop1UInt16].Id < FunctionTypeEnd))
    {
      if (StartFunctionNumber == 9999) StartFunctionNumber = Loop1UInt16;  // just found the first FunctionNumber for the specified category.
      EndFunctionNumber = Loop1UInt16;  // last FunctionNumber assigned will be the last FunctionNumber for the specified category.
    }
  }

  return;
}





/* $TITLE=display_function_name() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                             Display function name on message window.
\* ============================================================================================================================================================= */
void display_function_name(UINT16 FunctionId)
{
  UINT16 FunctionNumber;
  UINT16 Loop1UInt16;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering display_function_name()\r");

  /* Look for this function ID in the list of defined functions. */
  for (Loop1UInt16 = 0; Loop1UInt16 < FunctionHiLimit; ++Loop1UInt16)
  {
    if (Function[Loop1UInt16].Id == FunctionId)
    {
      /* If there is currently a scroll going on, cancel it and add a few character spaces to introduce new scrolling. */
      win_scroll_cancel(WIN_FUNCTION, 201, 201);

      /* Erase display area while keeping window border (if defined as "ACTION_DRAW") and then display current function name on LED matrix. */
      win_part_cls(WIN_FUNCTION, 201, 201);

      /* Scroll function name on first line of RGB Matrix. */
      win_scroll(WIN_FUNCTION, 201, 201, 1, 1, FONT_5x7, "%s", Function[Loop1UInt16].Name);  // function name too long, scroll it.

      FunctionNumber = Function[Loop1UInt16].Number;
      break;
    }
  }

  if (Loop1UInt16 == FunctionHiLimit)
  {
    /* Function ID has not been found. */
    win_part_cls(WIN_FUNCTION, 201, 201);
    win_printf(WIN_FUNCTION, 1, 99, FONT_5x7, "Not found");
  }

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting display_function_name()\r");

  return;
}





/* $TITLE=display_human_time() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                  Display "human time" whose pointer is given as a parameter.
\* ============================================================================================================================================================= */
void display_human_time(UCHAR *Text, struct human_time *HumanTime)
{
  UINT8 FlagValid = FLAG_ON;


  /* Make minimal validations to prevent a crash. */
  if (HumanTime->DayOfWeek > 6) FlagValid = FLAG_OFF;
  if ((HumanTime->Month < 1) || (HumanTime->Month > 12)) FlagValid = FLAG_OFF;

  if (FlagValid == FLAG_ON)
    uart_send(__LINE__, __func__, "%s %8s   %2.2u-%3s-%4u   %2.2u:%2.2u:%2.2u   (DoY: %3u   DST: 0x%2.2X)\r", Text, DayName[HumanTime->DayOfWeek], HumanTime->DayOfMonth, ShortMonth[HumanTime->Month], HumanTime->Year, HumanTime->Hour, HumanTime->Minute, HumanTime->Second, HumanTime->DayOfYear, HumanTime->FlagDst);
  else
    uart_send(__LINE__, __func__, "%s DoW:%u   %2.2u-%2.2u-%4u   %2.2u:%2.2u:%2.2u   (DoY: %3u   DST: %2.2X)\r", Text, HumanTime->DayOfWeek, HumanTime->DayOfMonth, HumanTime->Month, HumanTime->Year, HumanTime->Hour, HumanTime->Minute, HumanTime->Second, HumanTime->DayOfYear, HumanTime->FlagDst);

  return;
}





/* $TITLE=display_matrix_buffer() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                       Display current content of specified matrix buffer.
\* ============================================================================================================================================================= */
void display_matrix_buffer(UINT64 *BufferPointer)
{
  UCHAR String[MAX_COLUMNS + 1];

  UINT8 RowNumber;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering display_matrix_buffer()\r");

  for (RowNumber = 0; RowNumber < MAX_ROWS; ++RowNumber)
  {
    util_uint64_to_binary_string(BufferPointer[RowNumber], MAX_COLUMNS, String);
    uart_send(__LINE__, __func__, "Row[%2u]: 0x%16.16llX   -   %s\r", RowNumber, BufferPointer[RowNumber], String);
    sleep_ms(20);  // prevent communication override.
  }

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting display_matrix_buffer()\r");

  return;
}





/* $TITLE=display_one_second_callback() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                                Display one-second callback history.
\* ============================================================================================================================================================= */
void display_one_second_callback(void)
{
  UCHAR String[31];

  UINT8 Loop1UInt8;


  printf("One-second callback duration history (values given are microseconds):\r\r");
  for (Loop1UInt8 = 0; Loop1UInt8 < (MAX_ONE_SECOND_INTERVALS / 4); ++Loop1UInt8)
  {
    printf("%3u) %5lld        %3u) %5lld        %3u) %5lld        %3u) %5lld\r", Loop1UInt8,                                        OneSecondInterval[Loop1UInt8],
                                                                                 Loop1UInt8 +  (MAX_ONE_SECOND_INTERVALS / 4),      OneSecondInterval[Loop1UInt8 +  (MAX_ONE_SECOND_INTERVALS / 4)],
                                                                                 Loop1UInt8 + ((MAX_ONE_SECOND_INTERVALS / 4) * 2), OneSecondInterval[Loop1UInt8 + ((MAX_ONE_SECOND_INTERVALS / 4) * 2)],
                                                                                 Loop1UInt8 + ((MAX_ONE_SECOND_INTERVALS / 4) * 3), OneSecondInterval[Loop1UInt8 + ((MAX_ONE_SECOND_INTERVALS / 4) * 3)]);
  }
  printf("\r\r");
  printf("Press <Enter> to continue: ");
  input_string(String);

  return;
}





/* $TITLE=display_reminder1() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                          Display the specified reminder of type 1.
\* ============================================================================================================================================================= */
void display_reminder1(UINT8 ReminderNumber)
{
  struct human_time HumanTime;
  struct tm TmTime;

  UINT16 Loop1UInt16;


  printf("------------------------------------- Reminder number %u -------------------------------------\r\r", ReminderNumber + 1);

  if (FlashConfig2.Reminder1[ReminderNumber].StartPeriodUnixTime == 0)
  {
    /* If this reminder is not defined, display Unix time. */
    printf("Reminder number %u is currently undefined\r\r", ReminderNumber + 1);
    printf("Start period Unix time:   %12llu\r", FlashConfig2.Reminder1[ReminderNumber].StartPeriodUnixTime);
    printf("End   period Unix time:   %12llu\r", FlashConfig2.Reminder1[ReminderNumber].EndPeriodUnixTime);
  }
  else
  {
    /* If thie reminder is defined, display date and time details. */
    convert_unix_time(FlashConfig2.Reminder1[ReminderNumber].StartPeriodUnixTime, &TmTime, &HumanTime, FLAG_ON);
    display_human_time("Start period: ", &HumanTime);
    convert_unix_time(FlashConfig2.Reminder1[ReminderNumber].EndPeriodUnixTime, &TmTime, &HumanTime, FLAG_ON);
    display_human_time("End   period: ", &HumanTime);
  }

  /* Display other reminder parameters. */
  printf("Ring repeat time seconds:     %8llu\r", FlashConfig2.Reminder1[ReminderNumber].RingRepeatTimeSeconds);
  printf("Ring duration seconds:        %8llu\r", FlashConfig2.Reminder1[ReminderNumber].RingDurationSeconds);
  printf("Next reminder delay seconds:  %8llu\r", FlashConfig2.Reminder1[ReminderNumber].NextReminderDelaySeconds);

  /* Display message to be scrolled, making sure characters are displayable. */
  printf("Message to scroll on LED display: <");
  for (Loop1UInt16 = 0; Loop1UInt16 < sizeof(FlashConfig2.Reminder1[ReminderNumber].Message); ++Loop1UInt16)
  {
    if  (FlashConfig2.Reminder1[ReminderNumber].Message[Loop1UInt16] == 0x00) break;  // get out of <for> loop when reaching end-of-string.
    if ((FlashConfig2.Reminder1[ReminderNumber].Message[Loop1UInt16] >= 0x20) && (FlashConfig2.Reminder1[ReminderNumber].Message[Loop1UInt16] <= 0x7E))
      printf("%c", FlashConfig2.Reminder1[ReminderNumber].Message[Loop1UInt16]);
    else
      printf("?");
  }
  printf(">\r\r");

  return;
}





/* $TITLE=display_scroll() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                              Display active scroll structures.
\* ============================================================================================================================================================= */
void display_scroll(void)
{
  UCHAR String[65];

  UINT8 *Dum1Ptr;
  UINT8 Loop1UInt8;
  UINT8 Loop2UInt8;


  /* Display the size of memory chunk that will be malloc'ed every time an active scroll is required. */
  uart_send(__LINE__, __func__, "sizeof(struct active_scroll): %u (0x%2.2X)\r", sizeof(struct active_scroll), sizeof(struct active_scroll));

  /* Find first free memory chunk in the heap. */
  Dum1Ptr = malloc(sizeof(struct active_scroll));
  free(Dum1Ptr);
  uart_send(__LINE__, __func__, "First free memory chunk in the heap: 0x%p\r\r\r", Dum1Ptr);


  /* Display info about each active scroll structure. */
  for (Loop1UInt8 = 0; Loop1UInt8 < MAX_ACTIVE_SCROLL; ++Loop1UInt8)
  {
    uart_send(__LINE__, __func__, " ------------------------------ Scroll number %u ------------------------------\r\r", Loop1UInt8);
    if (ActiveScroll[Loop1UInt8] == 0x00l)
    {
      /* If this ActiveScroll structure is not assigned. */
      uart_send(__LINE__, __func__, " [0x%p] ActiveScroll[%u] - Not assigned\r\r", ActiveScroll[Loop1UInt8], Loop1UInt8);
    }
    else
    {
      /* If this ActiveScroll structure is currently assigned. */
      uart_send(__LINE__, __func__, " [0x%p] ActiveScroll[%u]\r", ActiveScroll[Loop1UInt8], Loop1UInt8);
      uart_send(__LINE__, __func__, " [0x%p] Owner:              %3u    %s\r", &ActiveScroll[Loop1UInt8]->Owner,              ActiveScroll[Loop1UInt8]->Owner, Window[ActiveScroll[Loop1UInt8]->Owner].Name);
      uart_send(__LINE__, __func__, " [0x%p] StartRow:           %3u\r",       &ActiveScroll[Loop1UInt8]->StartRow,           ActiveScroll[Loop1UInt8]->StartRow);
      uart_send(__LINE__, __func__, " [0x%p] EndRow:             %3u\r",       &ActiveScroll[Loop1UInt8]->EndRow,             ActiveScroll[Loop1UInt8]->EndRow);
      uart_send(__LINE__, __func__, " [0x%p] ScrollTimes:        %3u\r",       &ActiveScroll[Loop1UInt8]->ScrollTimes,        ActiveScroll[Loop1UInt8]->ScrollTimes);
      uart_send(__LINE__, __func__, " [0x%p] ScrollSpeed:        %3u\r",       &ActiveScroll[Loop1UInt8]->ScrollSpeed,        ActiveScroll[Loop1UInt8]->ScrollSpeed);
      uart_send(__LINE__, __func__, " [0x%p] PixelCountCurrent:  %3u\r",       &ActiveScroll[Loop1UInt8]->PixelCountCurrent,  ActiveScroll[Loop1UInt8]->PixelCountCurrent);
      uart_send(__LINE__, __func__, " [0x%p] PixelCountBuffer:   %3u\r",       &ActiveScroll[Loop1UInt8]->PixelCountBuffer,   ActiveScroll[Loop1UInt8]->PixelCountBuffer);
      uart_send(__LINE__, __func__, " [0x%p] AsciiBufferPointer: %3u\r",       &ActiveScroll[Loop1UInt8]->AsciiBufferPointer, ActiveScroll[Loop1UInt8]->AsciiBufferPointer);


      for (Loop2UInt8 = 0; Loop2UInt8 < MAX_ROWS; ++Loop2UInt8)
      {
        util_uint64_to_binary_string((UINT64)ActiveScroll[Loop1UInt8]->BitmapBuffer[Loop2UInt8], 64, String);
        uart_send(__LINE__, __func__, " [0x%p]BitmapBuffer[%2u]:   0x%8.8llX  %s\r", &ActiveScroll[Loop1UInt8]->BitmapBuffer[Loop2UInt8], Loop2UInt8, ActiveScroll[Loop1UInt8]->BitmapBuffer[Loop2UInt8], String);
        sleep_ms(50);  // to prevent communication overrun.
      }
      uart_send(__LINE__, __func__, " [0x%p] to [0x%p] Complete text being scrolled:\r", &ActiveScroll[Loop1UInt8]->Message[0], &ActiveScroll[Loop1UInt8]->Message[sizeof(ActiveScroll[Loop1UInt8]->Message)]);
      printf("---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\r");
      printf("%s\r", &ActiveScroll[Loop1UInt8]->Message);
      // printf("%s\r", &ActiveScroll[Loop1UInt8]->Message[ActiveScroll[Loop1UInt8]->AsciiBufferPointer]);  // part of text remaining to be scrolled.
      printf("---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\r\r\r\r");
    }
    printf("\r\r");
  }

  printf("\r\r");

  return;
}





/* $TITLE=display_tm_time() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                  Display "human time" whose pointer is given as a parameter.
\* ============================================================================================================================================================= */
void display_tm_time(UCHAR *Text, struct tm *TmTime)
{
  UINT8 FlagValid = FLAG_ON;


  if (DebugBitMask & DEBUG_NTP) uart_send(__LINE__, __func__, "Inside display_tm_time() (TmTime address: %p)  FlagValid stack variable: %p  Text ptr: %p\r", TmTime, &FlagValid, Text);

  /* Display TmTime exact values. */
  uart_send(__LINE__, __func__, "TmTime exact values:\r");
  uart_send(__LINE__, __func__, "TmTime->tm_hour  = %u\r", TmTime->tm_hour);
  uart_send(__LINE__, __func__, "TmTime->tm_min   = %u\r", TmTime->tm_min);
  uart_send(__LINE__, __func__, "TmTime->tm_sec   = %u\r", TmTime->tm_sec);
  uart_send(__LINE__, __func__, "TmTime->tm_mday  = %u\r", TmTime->tm_mday);
  uart_send(__LINE__, __func__, "TmTime->tm_mon   = %u\r", TmTime->tm_mon);
  uart_send(__LINE__, __func__, "TmTime->tm_year  = %u\r", TmTime->tm_year);
  uart_send(__LINE__, __func__, "TmTime->tm_wday  = %u\r", TmTime->tm_wday);
  uart_send(__LINE__, __func__, "TmTime->tm_yday  = %u\r", TmTime->tm_yday);
  uart_send(__LINE__, __func__, "TmTime->tm_isdst = %u\r\r\r", TmTime->tm_isdst);


  /* Make minimal validations to prevent a crash. */
  if (TmTime->tm_wday > 6) FlagValid = FLAG_OFF;
  if ((TmTime->tm_mon < 1) || (TmTime->tm_mon > 12)) FlagValid = FLAG_OFF;

  if (FlagValid == FLAG_ON)
  {
    printf("TmTime after interpretation:              %9s   %2.2u-%3s-%4u   %2.2u:%2.2u:%2.2u   (DoY: %3u   DST: %3d)\r", DayName[TmTime->tm_wday], TmTime->tm_mday, ShortMonth[TmTime->tm_mon + 1], TmTime->tm_year + 1900, TmTime->tm_hour, TmTime->tm_min, TmTime->tm_sec, TmTime->tm_yday + 1, TmTime->tm_isdst);
  }

  return;
}






/* $TITLE=display_current_unix_time() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                                      Display current Unix time.
\* ============================================================================================================================================================= */
void display_current_unix_time(void)
{
  UINT64 Dum1UInt64;

  struct human_time TimeNow;
  struct tm TempTime;

  if (DebugBitMask & DEBUG_NTP) uart_send(__LINE__, __func__, "display_current_unix_time():\r");

  TimeNow.Hour       = CurrentTime.Hour;
  TimeNow.Minute     = CurrentTime.Minute;
  TimeNow.Second     = CurrentTime.Second;
  TimeNow.DayOfMonth = CurrentTime.DayOfMonth;
  TimeNow.Month      = CurrentTime.Month;
  TimeNow.Year       = CurrentTime.Year;
  TimeNow.DayOfWeek  = CurrentTime.DayOfWeek;
  TimeNow.DayOfYear  = get_day_of_year(TimeNow.DayOfMonth, TimeNow.Month, TimeNow.Year);
  TimeNow.FlagDst    = 0;
  display_human_time("HumanTime:", &TimeNow);

  convert_human_to_tm(&TimeNow, &TempTime);
  display_tm_time("TmTime:     ", &TempTime);

  Dum1UInt64 = convert_tm_to_unix(&TempTime, FLAG_ON);
  printf("Unix time: %llu\r", Dum1UInt64);

  return;
}





/* $TITLE=display_up_time() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                               Display total RGB Matrix Up time.
\* ============================================================================================================================================================= */
void display_up_time(void)
{
  UINT8 Hours;
  UINT8 Minutes;
  UINT8 Seconds;

  UINT16 Days;

  UINT64 Dum1UInt64;


  /* Initializations. */
  Days    = 0;
  Hours   = 0;
  Minutes = 0;
  Seconds = 0;


  /* Display date and time of last power-on. */
  printf("RGB Matrix has been powered On:  %2.2u-%s-%4.4u at %2.2u:%2.2u:%2.2u\r",
          StartTime.DayOfMonth, ShortMonth[StartTime.Month], StartTime.Year, StartTime.Hour, StartTime.Minute, StartTime.Second);


  /* Get total Up time in seconds. */
  Dum1UInt64 = time_us_64();
  Dum1UInt64 /= 1000000ll;


  printf("Total number of up time seconds: %llu\r", Dum1UInt64);

  /* Get number of seconds. */
  Seconds = Dum1UInt64 % 60ll;  // remaining seconds
  /// printf("Seconds: %2u\r", Seconds);


  /* Get number of minutes. */
  Dum1UInt64 /= 60ll;  // total minutes.
  /// printf("Remaining minutes: %2llu\r", Dum1UInt64);
  if (Dum1UInt64)
  {
    Minutes = Dum1UInt64 % 60ll;  // remaining minutes
    /// printf("Minutes: %2u\r", Minutes);
  }


  /* Get number of hours. */
  Dum1UInt64 /= 60ll;  // total hours.
  /// printf("Remaining hours:   %2llu\r", Dum1UInt64);
  if (Dum1UInt64)
  {
    Hours = Dum1UInt64 % 24ll;  // remaining hours.
    /// printf("Hours:   %2u\r", Hours);
  }

  /* Get number of days. */
  Dum1UInt64 /= 24ll;  // total days.
  /// printf("Remaining days:    %2llu\r", Dum1UInt64);
  if (Dum1UInt64)
  {
    Days = Dum1UInt64;
    /// printf("Days:   %4u\r\r\r", Days);
  }


  printf("Total Up time:                 ");
  if (Days == 1) printf("  %u day",  Days);
  if (Days  > 1) printf("  %u days", Days);

  if ((Hours == 0) && (Days != 0)) printf("  %u hour", Hours);
  if (Hours == 1) printf("  %u hour",  Hours);
  if (Hours  > 1) printf("  %u hours", Hours);

  if ((Minutes == 0) && ((Hours != 0) || (Days != 0))) printf("  %u minute", Minutes);
  if (Minutes == 1) printf("  %u minute",  Minutes);
  if (Minutes  > 1) printf("  %u minutes", Minutes);

  if ((Seconds == 0) && ((Minutes != 0) || (Hours != 0) || (Days != 0)))
    printf("  %u second\r",  Seconds);
  else
  {
    if (Seconds < 2) printf("  %u second\r",  Seconds);
    if (Seconds > 1) printf("  %u seconds\r", Seconds);
  }

  return;
}





/* $TITLE=display_windows() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                               Display current windows parameters.
\* ============================================================================================================================================================= */
void display_windows(void)
{
  UCHAR String[65];

  UINT8 Loop1UInt8;
  UINT8 Loop2UInt8;


  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                          Display window names.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  uart_send(__LINE__, __func__, " Window names:\r");
  uart_send(__LINE__, __func__, " -------------\r");
  for (Loop1UInt8 = 0; Loop1UInt8 < MAX_WINDOWS; ++Loop1UInt8)
  {
    if ((Loop1UInt8 % 5) == 0)
    {
      uart_send(__LINE__, __func__, "%2u) <%s>   ", Loop1UInt8, Window[Loop1UInt8].Name);
    }
    else
    {
      uart_send(__LINE__, __func__, "- %2u) <%s>     ", Loop1UInt8, Window[Loop1UInt8].Name);
    }

    if (((Loop1UInt8 + 1) % 5) == 0)
      printf("\r");
    else
    {
      for (Loop2UInt8 = strlen(Window[Loop1UInt8].Name); Loop2UInt8 < 15; ++Loop2UInt8)
        printf(" ");
    }
  }
  printf("\r\r");


  uart_send(__LINE__, __func__, "Current active windows:       WinTop: %s (%u)       WinMid: %s (%u)       WinBot: %s (%u)\r\r", Window[WinTop].Name, WinTop, Window[WinMid].Name, WinMid, Window[WinBot].Name, WinBot);

  uart_send(__LINE__, __func__, "Window status definitions:    WINDOW_UNUSED: %u   WINDOW_ACTIVE: %u   WINDOW_INACTIVE: %u   WINDOW_COUNTDOWN: %u\r\r", WINDOW_UNUSED, WINDOW_ACTIVE, WINDOW_INACTIVE, WINDOW_COUNTDOWN);

  if (FlagEndlessLoop)
    uart_send(__LINE__, __func__, "Main endless system loop is now active.\r\r");
  else
    uart_send(__LINE__, __func__, "Main endless system loop is not active yet.\r\r");



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                    Display parameters for each window.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  for (Loop1UInt8 = 0; Loop1UInt8 < WIN_HI_LIMIT; ++Loop1UInt8)
  {
    uart_send(__LINE__, __func__, "Window:  =========================================================================== %s (%u) =================\r",     Window[Loop1UInt8].Name, Loop1UInt8);
    uart_send(__LINE__, __func__, "StartRow:        %3u           StartColumn: %3u", Window[Loop1UInt8].StartRow,  Window[Loop1UInt8].StartColumn);

    /* Indicate if this window is currently active. */
    if ((WinTop == Loop1UInt8) || (WinMid == Loop1UInt8) || (WinBot == Loop1UInt8))
      printf("                ***** ACTIVE *****\r");
    else
      printf("\r");

    uart_send(__LINE__, __func__, "EndRow:          %3u           EndColumn:   %3u\r", Window[Loop1UInt8].EndRow,    Window[Loop1UInt8].EndColumn);
    uart_send(__LINE__, __func__, "BorderColor:     %-8s      InsideColor: %-8s         LastBoxState: %u\r", ColorName[Window[Loop1UInt8].BorderColor], ColorName[Window[Loop1UInt8].InsideColor], Window[Loop1UInt8].LastBoxState);
    uart_send(__LINE__, __func__, "WinStatus:       %3u   (%u = WINDOW_UNUSED   %u = WINDOW_ACTIVE   %u = WINDOW_INACTIVE   %u = WINDOW_COUNTDOWN)\r",
              Window[Loop1UInt8].WinStatus, WINDOW_UNUSED, WINDOW_ACTIVE, WINDOW_INACTIVE, WINDOW_COUNTDOWN);
    uart_send(__LINE__, __func__, "CountDown:       %3u\r",            Window[Loop1UInt8].CountDown);

    if (Window[Loop1UInt8].TopBackLink < MAX_WINDOWS)
      uart_send(__LINE__, __func__, "TopBackLink:     %s  (%3u)\r",    Window[Window[Loop1UInt8].TopBackLink].Name, Window[Loop1UInt8].TopBackLink);
    else
      uart_send(__LINE__, __func__, "TopBackLink:     %3u\r",          Window[Loop1UInt8].TopBackLink);

    if (Window[Loop1UInt8].MidBackLink < MAX_WINDOWS)
      uart_send(__LINE__, __func__, "MidBackLink:     %s  (%3u)\r",    Window[Window[Loop1UInt8].MidBackLink].Name, Window[Loop1UInt8].MidBackLink);
    else
      uart_send(__LINE__, __func__, "MidBackLink:     %3u\r",          Window[Loop1UInt8].MidBackLink);

    if (Window[Loop1UInt8].BotBackLink < MAX_WINDOWS)
      uart_send(__LINE__, __func__, "BotBackLink:     %s  (%3u)\r\r",  Window[Window[Loop1UInt8].BotBackLink].Name, Window[Loop1UInt8].BotBackLink);
    else
      uart_send(__LINE__, __func__, "BotBackLink:     %3u\r",          Window[Loop1UInt8].BotBackLink);

    uart_send(__LINE__, __func__, "FlagTopScroll:  0x%2.2X\r",   Window[Loop1UInt8].FlagTopScroll);
    uart_send(__LINE__, __func__, "FlagMidScroll:  0x%2.2X\r",   Window[Loop1UInt8].FlagMidScroll);
    uart_send(__LINE__, __func__, "FlagBotScroll:  0x%2.2X\r\r", Window[Loop1UInt8].FlagBotScroll);
  }

  printf("\r\r");

  return;
}





/* $PAGE */
/* $TITLE=double_dot_set_color() */
/* ============================================================================================================================================================= *\
                                                            Set color for two "double-dots time separators".
\* ============================================================================================================================================================= */
void double_dots_set_color(UINT8 Color)
{
  /* Top double-dots. */
  RGB_matrix_set_color(22, 21, 23, 22, Color);
  RGB_matrix_set_color(22, 40, 23, 41, Color);

  /* Bottom double-dots. */
  RGB_matrix_set_color(26, 21, 27, 22, Color);
  RGB_matrix_set_color(26, 40, 27, 41, Color);

  return;
}





/* $TITLE=ds3231_display_values() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                    Display all current variables read from real-time IC (DS3231).
\* ============================================================================================================================================================= */
void ds3231_display_values()
{
  UCHAR String[31];

  UINT8 Data[0x12];
	UINT8 Loop1UInt8;
  UINT8 Value = 0x00;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering ds3231_display_values()\r");

  i2c_write_blocking(I2C_PORT, DS3231_ADDRESS, &Value,  1, true);
  i2c_read_blocking(I2C_PORT,  DS3231_ADDRESS, Data, 0x12, false);


  uart_send(__LINE__, __func__, "====================================================================================================================================\r");
  uart_send(__LINE__, __func__, "                                        List of values read from real-time IC (DS3231)\r");
  uart_send(__LINE__, __func__, "====================================================================================================================================\r");

  util_uint64_to_binary_string((UINT64)Data[0x00], 8, String);
  uart_send(__LINE__, __func__, "Parameter <0x00>: %2.2X   %8s   Seconds          (0 to 59) bits 0 to 3 = seconds      bits 4 to 7  = 10 seconds\r", Data[0x00], String);

  util_uint64_to_binary_string((UINT64)Data[0x01], 8, String);
  uart_send(__LINE__, __func__, "Parameter <0x01>: %2.2X   %8s   Minutes          (0 to 59) bits 0 to 3 = minutes      bits 4 to 7  = 10 minutes\r", Data[0x01], String);

  util_uint64_to_binary_string((UINT64)Data[0x02], 8, String);
  uart_send(__LINE__, __func__, "Parameter <0x02>: %2.2X   %8s   Hours            (0 to 23) bits 0 to 3 = hours        bits 4 and 5 = 10 hours           bit 6: 0 = 24 / 1 = 12 hours format\r", Data[0x2], String);

  util_uint64_to_binary_string((UINT64)Data[0x03], 8, String);
  uart_send(__LINE__, __func__, "Parameter <0x03>: %2.2X   %8s   Day-of-week      (1 to 7)  bits 0 to 2 = day-of-week  0 has been arbitrarily assigned to Sunday\r", Data[0x03], String);

  util_uint64_to_binary_string((UINT64)Data[0x04], 8, String);
  uart_send(__LINE__, __func__, "Parameter <0x04>: %2.2X   %8s   Date             (1 to 31) bits 0 to 3 = date         bits 4 to 5 = 10 dates\r", Data[0x04], String);

  util_uint64_to_binary_string((UINT64)Data[0x05], 8, String);
  uart_send(__LINE__, __func__, "Parameter <0x05>: %2.2X   %8s   Month            (1 to 12) bits 0 to 3 = month        bit 5 = 10 month                  bit 7 = century\r", Data[0x05], String);

  util_uint64_to_binary_string((UINT64)Data[0x06], 8, String);
  uart_send(__LINE__, __func__, "Parameter <0x06>: %2.2X   %8s   Year             (0 to 99) bits 0 to 3 = year         bits 4 to 7 = 10 year\r", Data[0x06], String);

  util_uint64_to_binary_string((UINT64)Data[0x07], 8, String);
  uart_send(__LINE__, __func__, "Parameter <0x07>: %2.2X   %8s   Alarm1 seconds   (0 to 59) bits 0 to 3 = seconds      bits 4 to 6 = 10 alarm1 seconds   bit 7 = A1M1\r", Data[0x07], String);

  util_uint64_to_binary_string((UINT64)Data[0x08], 8, String);
  uart_send(__LINE__, __func__, "Parameter <0x08>: %2.2X   %8s   Alarm1 minutes   (0 to 59) bits 0 to 3 = minutes      bits 4 to 6 = 10 alarm1 minutes   bit 7 = A1M2\r", Data[0x08], String);

  util_uint64_to_binary_string((UINT64)Data[0x09], 8, String);
  uart_send(__LINE__, __func__, "Parameter <0x09>: %2.2X   %8s   Alarm1 hour      (0 to 23) bits 0 to 3 = hours        bits 4 to 5 = 10 alarm1 hour      bit 6 = 12 or 24 format   bit 7 = A1M3\r", Data[0x09], String);

  util_uint64_to_binary_string((UINT64)Data[0x0A], 8, String);
  uart_send(__LINE__, __func__, "Parameter <0x0A>: %2.2X   %8s   Alarm1 day       (1 to 7)  bits 0 to 3 = day          bits 6 = alarm1 dow or date       bit 7 = A1M4\r", Data[0x0A], String);

  util_uint64_to_binary_string((UINT64)Data[0x0B], 8, String);
  uart_send(__LINE__, __func__, "Parameter <0x0B>: %2.2X   %8s   Alarm2 minutes   (0 to 59) bits 0 to 3 = minute       bits 4 to 6 = 10 alarm2 minutes   bit 7 = A2M2\r", Data[0x0B], String);

  util_uint64_to_binary_string((UINT64)Data[0x0C], 8, String);
  uart_send(__LINE__, __func__, "Parameter <0x0C>: %2.2X   %8s   Alarm2 hour      (0 to 23) bits 0 to 3 = hour         bits 4 to 5 = 10 alarm2 hour      bit 6 = 12 or 24 format   bit 7 = A2M3\r", Data[0x0C], String);

  util_uint64_to_binary_string((UINT64)Data[0x0D], 8, String);
  uart_send(__LINE__, __func__, "Parameter <0x0D>: %2.2X   %8s   Alarm2 day       (1 to 7)  bits 0 to 3 = day          bits 6 = alarm2 dow or date       bit 7 = A2M4\r", Data[0x0D], String);

  util_uint64_to_binary_string((UINT64)Data[0x0E], 8, String);
  uart_send(__LINE__, __func__, "Parameter <0x0E>: %2.2X   %8s   Control\r", Data[0x0E], String);

  util_uint64_to_binary_string((UINT64)Data[0x0F], 8, String);
  uart_send(__LINE__, __func__, "Parameter <0x0F>: %2.2X   %8s   Control / Status\r", Data[0x0F], String);

  util_uint64_to_binary_string((UINT64)Data[0x10], 8, String);
  uart_send(__LINE__, __func__, "Parameter <0x10>: %2.2X   %8s   Aging offset\r", Data[0x10], String);

  util_uint64_to_binary_string((UINT64)Data[0x11], 8, String);
  uart_send(__LINE__, __func__, "Parameter <0x11>: %2.2X   %8s   MSB of temperature\r", Data[0x11], String);

  util_uint64_to_binary_string((UINT64)Data[0x12], 8, String);
  uart_send(__LINE__, __func__, "Parameter <0x12>: %2.2X   %8s   LSB of temperature\r", Data[0x12], String);

  uart_send(__LINE__, __func__, "====================================================================================================================================\r\r\r");

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting ds3231_display_values()\r");

  return;
}





/* $TITLE=ds3231_get_temperature() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                             Read temperature from real-time IC (DS3231).
\* ============================================================================================================================================================= */
void ds3231_get_temperature(float *DegreeC, float *DegreeF)
{
	UCHAR Command[2];
  UCHAR GetMSB = 0x11;  // request to get temperature MSB.
  UCHAR GetLSB = 0x12;  // request to get temperature LSB.
  UCHAR TempMSB;        // temperature  Most Significant Byte (first 7 bits for data and 8th bit for sign).
  UCHAR TempLSB;        // temperature Least Significant Byte (bits 6 and 7 are used only).

  UINT16 TemperatureRead;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering ds3231_get_temperature()\r");

	Command[0] = 0x0E;
  Command[1] = 0x20;


  i2c_write_blocking(I2C_PORT, DS3231_ADDRESS, Command,  2, FALSE);
  i2c_write_blocking(I2C_PORT, DS3231_ADDRESS, &GetMSB,  1, TRUE);
  i2c_read_blocking (I2C_PORT, DS3231_ADDRESS, &TempMSB, 1, FALSE);
  i2c_write_blocking(I2C_PORT, DS3231_ADDRESS, &GetLSB,  1, TRUE);
  i2c_read_blocking (I2C_PORT, DS3231_ADDRESS, &TempLSB, 1, FALSE);

  TemperatureRead = (UINT16)((TempMSB << 2) | (TempLSB >> 6));  // add MSB and LSB in a single 16-bits variable.

  /* Take care of sign bit on 16-bits variable. */
  if (TempMSB & 0x80) TemperatureRead |= 0xFC00;

  TemperatureRead *= 25;

  *DegreeC = ((TemperatureRead / 100.0) - 2.5);  // real temperature seems always to be more or less 2.5 degrees C lower than what is returned from DS3231.
  *DegreeF = ((*DegreeC * 9 / 5) + 32);

  // uart_send(__LINE__, __func__, "Temperature read from DS3231 - DegreeC: %2.2f   DegreeF: %2.2f\r", *DegreeC, *DegreeF);

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting ds3231_get_temperature()\r");

  return;
}





/* $TITLE=ds3231_get_time() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                                Read time from real-time clock IC (DS3231).
\* ============================================================================================================================================================= */
void ds3231_get_time(struct human_time *CurrentTime)
{
  UINT8 Data[7];
  UINT8 FlagLocalDebug = FLAG_OFF;
	UINT8 Value = 0x00;


  if (FlagLocalDebug) printf("%u   Entering ds3231_get_time()\r", __LINE__);

  /***
  // Force a specific date and time.
  CurrentTime->DayOfWeek  = 3;
  CurrentTime->DayOfMonth = 26;
  CurrentTime->Month      = 3;
  CurrentTime->Year       = 2024;
  CurrentTime->DayOfYear  = 86;
  CurrentTime->FlagDst    = FLAG_OFF;

  CurrentTime->Hour   = 12;
  CurrentTime->Minute = 5;
  CurrentTime->Second = 0;

  return;
  ***/

  i2c_write_blocking(I2C_PORT, DS3231_ADDRESS, &Value, 1, true);
  i2c_read_blocking(I2C_PORT,  DS3231_ADDRESS, Data,   7, false);


  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                       Read seconds (HumanTime.Second: 0-59 and DS3231: 0-59).
  \* --------------------------------------------------------------------------------------------------------------------------- */
  CurrentTime->Second = util_bcd2dec(Data[0]);  // reading seconds.


  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                       Read minutes (HumanTime.Minute: 0-59 and DS3231: 0-59).
  \* --------------------------------------------------------------------------------------------------------------------------- */
  CurrentTime->Minute = util_bcd2dec(Data[1]);  // reading minutes.


  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                         Read hours (HumanTime.Hour: 0-23 and DS3231: 0-23).
  \* --------------------------------------------------------------------------------------------------------------------------- */
  /* Read current hour while converting to 24-hours format (0-23 instead of 1-12). */
  /// if (Data[2] & DS3231_12HOUR_FLAG)
  /// {
  ///  /* If hour is saved in 12-hours format. */
  ///   CurrentTime->Hour = util_bcd2dec(Data[2] & DS3231_12HOUR_MASK) - 1;  // convert from 1-12 to 0-11...
  ///
  ///   if (Data[2] & DS3231_PM_FLAG) CurrentTime->Hour += 12;  // ...then, add if needed 12 to get 0-23
  /// }
  /// else
  /// {
    /* If hour is saved in 24-hours format. */
    CurrentTime->Hour = util_bcd2dec(Data[2]);
  /// }


  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                      Read day-of-week (HumanTime.DayOfWeek: 0-6 and DS3231: 1-7).
  \* --------------------------------------------------------------------------------------------------------------------------- */
  CurrentTime->DayOfWeek = (util_bcd2dec(Data[3]) - 1);  // reading day-of-week
  /// if (CurrentTime->DayOfWeek == 7) CurrentTime->DayOfWeek = 0;  // Sunday is 0 instead of 7.


  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                    Read day-of-month (HumanTime.DayOfMonth: 1-31 and DS3231: 0-31).
  \* --------------------------------------------------------------------------------------------------------------------------- */
  CurrentTime->DayOfMonth = util_bcd2dec(Data[4]);  // reading day-of-month.


  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                           Read month (HumanTime.Month: 1-12 and DS3231: 1-12).
  \* --------------------------------------------------------------------------------------------------------------------------- */
  CurrentTime->Month = util_bcd2dec(Data[5] & DS3231_MONTH_MASK);  // reading month.


  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                  Read year (HumanTime.Year: 2000 and up and DS3231: year since 2000).
  \* --------------------------------------------------------------------------------------------------------------------------- */
  CurrentTime->Year = util_bcd2dec(Data[6]) + 2000;  // reading year.


  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                      Compute day-of-year.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  CurrentTime->DayOfYear = get_day_of_year(CurrentTime->DayOfMonth, CurrentTime->Month, CurrentTime->Year);  // give an error during power-up



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                    Compute daylight saving time.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  CurrentTime->FlagDst = FLAG_OFF;

  if (FlagLocalDebug) printf("%u   Exiting DS3231_get_time()\r", __LINE__);

  return;
}





/* $TITLE=ds3231_init() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                              Initialize real-time clock (DS3231).
\* ============================================================================================================================================================= */
void ds3231_init(void)
{
	UINT8 FlagLocalDebug = FLAG_OFF;
  UINT8 Value[2];


  if (FlagLocalDebug) printf("%u   Entering ds3231_init()\r", __LINE__);

  i2c_init(I2C_PORT, 400000);
	gpio_set_function(SDA, GPIO_FUNC_I2C);
	gpio_set_function(SCL, GPIO_FUNC_I2C);
	gpio_pull_up(SDA);
	gpio_pull_up(SCL);

  if (FlagLocalDebug) printf("%u   ds3231__init() - tag 1\r", __LINE__);

  Value[0] = DS3231_ADDR_CONTROL;
  Value[1] = DS3231_CTRL_TEMPCONV;
  i2c_write_blocking(I2C_PORT, DS3231_ADDRESS, Value, 2, false);

	if (FlagLocalDebug) printf("%u   ds3231__init() - tag 2\r", __LINE__);

  Value[0] = DS3231_ADDR_STATUS;
  Value[1] = DS3231_ADDR_TIME;
  i2c_write_blocking(I2C_PORT, DS3231_ADDRESS, Value, 2, false);

  if (FlagLocalDebug) printf("%u   Exiting ds3231_init()\r", __LINE__);

  return;
}





/* $TITLE=ds3231_set_dom() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                       Set the day of month of the real-time clock IC (DS3231).
\* ============================================================================================================================================================= */
void ds3231_set_dom(UINT8 DayOfMonthValue)
{
	UINT8 Data[2];


  if (DebugBitMask & DEBUG_FLOW) printf("Entering ds3231_set_dom()\r");

	Data[0] = DS3231_ADDR_TIME_MDAY;
	Data[1] = util_dec2bcd(DayOfMonthValue);

	i2c_write_blocking(I2C_PORT, DS3231_ADDRESS, Data, 2, false);

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting ds3231_set_dom()\r");

  return;
}





/* $TITLE=ds3231_set_dow() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                        Set the day of week of the real-time clock IC (DS3231).
\* ============================================================================================================================================================= */
void ds3231_set_dow(UINT8 DayOfWeekValue)
{
	UINT8 Data[2];


  if (DebugBitMask & DEBUG_FLOW) printf("Entering ds3231_set_dow()\r");

	Data[0] = DS3231_ADDR_TIME_WDAY;
	Data[1] = util_dec2bcd(DayOfWeekValue);

	i2c_write_blocking(I2C_PORT, DS3231_ADDRESS, Data, 2, false);

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting ds3231_set_dow()\r");

  return;
}





/* $TITLE=ds3231_set_hour() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                          Set the hour of the real-time clock IC (DS3231).
\* ============================================================================================================================================================= */
void ds3231_set_hour(UINT8 Hour)
{
	UINT8 Data[2];


  if (DebugBitMask & DEBUG_FLOW) printf("Entering ds3231_set_hour()\r");

	Data[0] = DS3231_ADDR_TIME_HOUR;
	Data[1] = util_dec2bcd(Hour);

	i2c_write_blocking(I2C_PORT, DS3231_ADDRESS, Data, 2, false);

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting ds3231_set_hour()\r");

  return;
}




/* $TITLE=ds3231_set_minute() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                        Set the minutes of the real-time clock IC (DS3231).
\* ============================================================================================================================================================= */
void ds3231_set_minute(UINT8 Minutes)
{
	UINT8 Data[2];


  if (DebugBitMask & DEBUG_FLOW) printf("Entering ds3231_set_minute()\r");

	Data[0] = DS3231_ADDR_TIME_MIN;
	Data[1] = util_dec2bcd(Minutes);

	i2c_write_blocking(I2C_PORT, DS3231_ADDRESS, Data, 2, false);

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting ds3231_set_minute()\r");

  return;
}





/* $TITLE=ds3231_set_month() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                         Set the month of the real-time clock IC (DS3231).
\* ============================================================================================================================================================= */
void ds3231_set_month(UINT8 MonthValue)
{
	UINT8 Data[2];


  if (DebugBitMask & DEBUG_FLOW) printf("Entering ds3231_set_month()\r");

	Data[0] = DS3231_ADDR_TIME_MON;
	Data[1] = util_dec2bcd(MonthValue);

	i2c_write_blocking(I2C_PORT, DS3231_ADDRESS, Data, 2, false);

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting ds3231_set_month()\r");

  return;
}





/* $TITLE=ds3231_set_second() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                         Set the seconds of the real-time clock IC (DS3231).
\* ============================================================================================================================================================= */
void ds3231_set_second(UINT8 SecondValue)
{
	UINT8 Data[2];


  if (DebugBitMask & DEBUG_FLOW) printf("Entering ds3231_set_second()\r");

	Data[0] = DS3231_ADDR_TIME_SEC;
	Data[1] = util_dec2bcd(SecondValue);

	i2c_write_blocking(I2C_PORT, DS3231_ADDRESS, Data, 2, false);

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting ds3231_set_second()\r");

  return;
}





/* $TITLE=ds3231_set_time() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                          Set the time of the real-time clock IC (DS3231).
\* ============================================================================================================================================================= */
void ds3231_set_time(struct human_time *CurrentTime)
{
	UINT8 Data[8];

	Data[0] = 0x00;

  if (DebugBitMask & DEBUG_DS3231) printf("Entering ds3231_set_time()\r");

  if (DebugBitMask & DEBUG_NTP) display_human_time("Inside ds3231_set_time():             ", CurrentTime);

  /* Set the time data. */
  Data[1] = util_dec2bcd(CurrentTime->Second);
  Data[2] = util_dec2bcd(CurrentTime->Minute);
  Data[3] = util_dec2bcd(CurrentTime->Hour);

	/* Set the date data. */
  Data[4] = util_dec2bcd(CurrentTime->DayOfWeek + 1);
  Data[5] = util_dec2bcd(CurrentTime->DayOfMonth);
  Data[6] = util_dec2bcd(CurrentTime->Month);
  Data[7] = util_dec2bcd(CurrentTime->Year - 2000);

	i2c_write_blocking(I2C_PORT, DS3231_ADDRESS, Data, 8, false);

  if (DebugBitMask & DEBUG_DS3231) printf("Exiting ds3231_set_time()\r");

  return;
}





/* $TITLE=ds3231_set_year() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                          Set the year of the real-time clock IC (DS3231).
\* ============================================================================================================================================================= */
void ds3231_set_year(UINT16 YearValue)
{
	UINT8 Data[2];

  if (DebugBitMask & DEBUG_FLOW) printf("Entering ds3231_set_year()\r");

	Data[0] = DS3231_ADDR_TIME_YEAR;
	Data[1] = util_dec2bcd(YearValue - 2000);

	i2c_write_blocking(I2C_PORT, DS3231_ADDRESS, Data, 2, false);

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting ds3231_set_year()\r");

  return;
}





/* $TITLE=enter_human_time() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                              Enter a human time and / or human date.
\* ============================================================================================================================================================= */
void enter_human_time(struct human_time *HumanTime, UINT8 FlagDate, UINT8 FlagTime)
{
  UCHAR String[31];

  UINT8 FlagFirst;


  FlagFirst = FLAG_ON;

  while (1)
  {
    if (FlagFirst == FLAG_ON)
      FlagFirst = FLAG_OFF;
    else
    {
      display_human_time("Please double-check the information entered and press <ESC> if you're satisfied with it:\r", HumanTime);
      /// printf("Please double-check the information entered and press <ESC> if you're satisfied with it:\r");
      printf("---------------------------------------------------------------------------------\r\r");
    }

    if (FlagDate)
    {
      do
      {
        /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                                  Enter day-of-month.
        \* --------------------------------------------------------------------------------------------------------------------------- */
        while (1)
        {
          printf("Current value for day-of-month is: %u\r", HumanTime->DayOfMonth);
          printf("Enter day-of-month, <Enter> to keep current value or <ESC> to exit: ");
          input_string(String);
          if (String[0] == 0x1B) return;
          if (String[0] == 0x0D) break;
          HumanTime->DayOfMonth = atoi(String);
          while ((HumanTime->DayOfMonth < 1) || (HumanTime->DayOfMonth > 31))
          {
            printf("Invalid day-of-month, please re-enter (1 to 31): ");
            printf("or <ESC> to exit: ");
            input_string(String);
            if (String[0] == 0x1B) return;
            HumanTime->DayOfMonth = atoi(String);
          }
        }
        printf("\r\r");



        /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                                   Enter month.
        \* --------------------------------------------------------------------------------------------------------------------------- */
        while (1)
        {
          printf("Current value for month is: %u\r", HumanTime->Month);
          printf("Enter month, <Enter> to keep current value or <ESC> to exit: ");
          input_string(String);
          if (String[0] == 0x1B) return;
          if (String[0] == 0x0D) break;
          HumanTime->Month = atoi(String);
          while ((HumanTime->Month < 1) || (HumanTime->Month > 12))
          {
            printf("Invalid month, please re-enter (1 to 12)\r");
            printf("or <ESC> to exit: ");
            input_string(String);
            if (String[0] == 0x1B) return;
            HumanTime->Month = atoi(String);
          }
        }
        printf("\r\r");



        /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                                   Enter year.
        \* --------------------------------------------------------------------------------------------------------------------------- */
        while (1)
        {
          printf("Current value for year is: %u\r", HumanTime->Year);
          printf("Enter year, <Enter> to keep current value or <ESC> to exit: ");
          input_string(String);
          if (String[0] == 0x1B) return;
          if (String[0] == 0x0D) break;
          HumanTime->Year = atoi(String);
          while ((HumanTime->Year < CurrentTime.Year) || (HumanTime->Year > 2100))
          {
            printf("Invalid year, please re-enter (%4.4u to 2100)\r", CurrentTime.Year);
            printf("or <ESC> to exit: ");
            input_string(String);
            if (String[0] == 0x1B) return;
            HumanTime->Year = atoi(String);
          }
        }
        printf("\r\r");

        /* Validate DayOfMonth, given the month and year (leap year or not). */
        if (HumanTime->DayOfMonth > get_month_days(HumanTime->Month, HumanTime->Year))
        {
          printf("Invalid day-of-month entered for month of %s (must be between 1 and %u)\r", MonthName[HumanTime->Month], get_month_days(HumanTime->Month, HumanTime->Year));
          printf("Please check and enter valid values...\r\r");
        }
      } while (HumanTime->DayOfMonth > get_month_days(HumanTime->Month, HumanTime->Year));
    }





    if (FlagTime)
    {
      /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                                Enter hour.
      \* --------------------------------------------------------------------------------------------------------------------------- */
      while (1)
      {
        printf("Current value for hour is: %u\r", HumanTime->Hour);
        printf("Enter hour, <Enter> to keep current value or <ESC> to exit: ");
        input_string(String);
        if (String[0] == 0x1B) return;
        if (String[0] == 0x0D) break;
        HumanTime->Hour = atoi(String);
        while (HumanTime->Hour > 23)
        {
          printf("Invalid hour, please re-enter (0 to 23): ");
          input_string(String);
          if (String[0] == 0x1B) return;
          HumanTime->Hour = atoi(String);
        }
      }
      printf("\r\r");



      /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                              Enter minute.
      \* --------------------------------------------------------------------------------------------------------------------------- */
      while (1)
      {
        printf("Current value for minute is: %u\r", HumanTime->Minute);
        printf("Enter minute, <Enter> to keep current value or <ESC> to exit: ");
        input_string(String);
        if (String[0] == 0x1B) return;
        if (String[0] == 0x0D) break;
        HumanTime->Minute = atoi(String);
        while (HumanTime->Minute > 59)
        {
          printf("Invalid minute, please re-enter (0 to 59): ");
          input_string(String);
          if (String[0] == 0x1B) return;
          HumanTime->Minute = atoi(String);
        }
      }
      printf("\r\r");



      /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                                Enter second.
      \* --------------------------------------------------------------------------------------------------------------------------- */
      while (1)
      {
          printf("Current value for second is: %u\r", HumanTime->Second);
        printf("Enter second, <Enter> to keep current value or <ESC> to exit: ");
        input_string(String);
        if (String[0] == 0x1B) return;
        if (String[0] == 0x0D) break;
        HumanTime->Second = atoi(String);
        while (HumanTime->Second > 59)
        {
          printf("Invalid second, please re-enter (0 to 59): ");
          input_string(String);
          if (String[0] == 0x1B) return;
          HumanTime->Second = atoi(String);
        }
      }
      printf("\r\r");

      /* For sake of completeness, fill-up the missing items. */
      HumanTime->DayOfWeek = get_day_of_week(HumanTime->DayOfMonth, HumanTime->Month, HumanTime->Year);  // Sunday = 0   (...) Saturday = 6.
      if (DebugBitMask & DEBUG_NTP) uart_send(__LINE__, __func__, "DayOfWeek: %u\r", HumanTime->DayOfWeek);

      HumanTime->DayOfYear = get_day_of_year(HumanTime->DayOfMonth, HumanTime->Month, HumanTime->Year);
      if (DebugBitMask & DEBUG_NTP) uart_send(__LINE__, __func__, "DayOfYear: %u\r", HumanTime->DayOfYear);

      HumanTime->FlagDst = FLAG_OFF;  // to be implemented.
      if (DebugBitMask & DEBUG_NTP) uart_send(__LINE__, __func__, "Flag DST: %u\r", HumanTime->FlagDst);

      if (DebugBitMask & DEBUG_NTP) display_human_time("Display human time entered:", HumanTime);
    }
  }

  return;
}





/* $TITLE=event_check() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                            Check if some calendar events must be triggered.
\* ============================================================================================================================================================= */
void event_check(void)
{
  UCHAR String[31];

  UINT8 FlagLocalDebug;
  UINT8 TotalEvents;

  UINT16 Loop1UInt16;


  FlagLocalDebug = FLAG_OFF;
  TotalEvents = 0;

  if (FlagLocalDebug) printf("Entering event_check()\r");

  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_EVENTS; ++Loop1UInt16)
  {
    if (FlashConfig1.Event[Loop1UInt16].Day == CurrentTime.DayOfMonth)
    {
      if (FlagLocalDebug)
      {
        uart_send(__LINE__, __func__, "Event %u - Day does match\r", Loop1UInt16);
      }
    }
    else
    {
      if (FlagLocalDebug)
      {
        uart_send(__LINE__, __func__, "Event %u - Day does not match\r", Loop1UInt16);
      }
      continue;
    }



    if (FlashConfig1.Event[Loop1UInt16].Month == CurrentTime.Month)
    {
      if (FlagLocalDebug)
      {
        uart_send(__LINE__, __func__, "Event %u - Month does match (%u VS %u)\r", Loop1UInt16, FlashConfig1.Event[Loop1UInt16].Month, CurrentTime.Month);
      }
    }
    else
    {
      if (FlagLocalDebug)
      {
        uart_send(__LINE__, __func__, "Event %u - Month does not match (%u VS %u)\r", Loop1UInt16, FlashConfig1.Event[Loop1UInt16].Month, CurrentTime.Month);
      }
      continue;
    }


    EventBitMask |= (1 << Loop1UInt16);
    if (FlagLocalDebug)
      uart_send(__LINE__, __func__, "Setting ringer for event number %u -> %s (0x%8.8llX - %llu)\r", Loop1UInt16, FlashConfig1.Event[Loop1UInt16].Message, EventBitMask, EventBitMask);
  }

  if (FlagLocalDebug) printf("Exiting event_check()\r");

  return;
}





/* $PAGE */
/* $TITLE=flash_check_config() */
/* ============================================================================================================================================================= *\
                                       Compare crc16 between flash saved configuration and current active configuration.
\* ============================================================================================================================================================= */
void flash_check_config(UINT8 ConfigNumber)
{
  UINT16 Crc16;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering flash_check_config()\r");

  switch (ConfigNumber)
  {
    case (1):
    {
      /* Calculate CRC16 for current active RGB matrix configuration 1. */
      Crc16 = util_crc16((UINT8 *)&FlashConfig1, ((UINT32)&FlashConfig1.Crc16 - (UINT32)&FlashConfig1.Version));


      /* We know that FlashConfig1.Crc16 corresponds to the crc16 value that has been retrieved the last time the configuration 1 was restored from
         (or saved to) flash. So, if the current active configuration's CRC16 is still the same than the last one retrieved from (or saved to) flash,
         it means that configuration 1 has not changed from what it is in flash). */
      if (FlashConfig1.Crc16 == Crc16)
      {
        if (DebugBitMask & DEBUG_FLASH)
          uart_send(__LINE__, __func__, "CRC16 computed for current active clock configuration 1: 0x%4.4X - same than last one read from flash.\r", Crc16, FlashConfig1.Crc16);

        return;
      }
      else
      {
        if (DebugBitMask & DEBUG_FLASH)
        {
          uart_send(__LINE__, __func__, "Current active configuration is different than the one in flash (0x%4.4X VS 0x%4.4X).\r", Crc16, FlashConfig1.Crc16);
          uart_send(__LINE__, __func__, "Update flash configuration.\r\r\r");
        }

        /* Update flash configuration so that it matches the current active configuration. */
        flash_save_config1();
      }
    }
    break;

    case (2):
      /* Calculate CRC16 for current active RGB matrix configuration 2. */
      Crc16 = util_crc16((UINT8 *)&FlashConfig2, ((UINT32)&FlashConfig2.Crc16 - (UINT32)&FlashConfig2.Version));


      /* We know that FlashConfig2.Crc16 corresponds to the crc16 value that has been retrieved the last time the configuration 2 was restored from
         (or saved to) flash. So, if the current active configuration's CRC16 is still the same than the last one retrieved from (or saved to) flash,
         it means that configuration 2 has not changed from what it is in flash). */
      if (FlashConfig2.Crc16 == Crc16)
      {
        if (DebugBitMask & DEBUG_FLASH)
          uart_send(__LINE__, __func__, "CRC16 computed for current active clock configuration 2: 0x%4.4X - same than last one read from flash.\r", Crc16, FlashConfig2.Crc16);

        return;
      }
      else
      {
        if (DebugBitMask & DEBUG_FLASH)
        {
          uart_send(__LINE__, __func__, "Current active configuration 2 is different than the one in flash (0x%4.4X VS 0x%4.4X).\r", Crc16, FlashConfig2.Crc16);
          uart_send(__LINE__, __func__, "Update flash configuration.\r\r\r");
        }

        /* Update flash configuration so that it matches the current active configuration. */
        flash_save_config2();
      }
    break;
  }

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting flash_check_config()\r");

  return;
}





/* $PAGE */
/* $TITLE=flash_display() */
/* ============================================================================================================================================================= *\
                                                     Display flash content through external monitor.
\* ============================================================================================================================================================= */
void flash_display(UINT32 Offset, UINT32 Length)
{
  UCHAR String[256];

  UINT8 *FlashBaseAddress;

  UINT32 Loop1UInt32;
  UINT32 Loop2UInt32;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering flash_display()\r");

  /* Compute target flash memory address.
     NOTE: XIP_BASE ("eXecute-In-Place") is the base address of the flash memory in the Pico's address space (memory map). */
  FlashBaseAddress = (UINT8 *)(XIP_BASE);

  if (DebugBitMask & DEBUG_FLOW)
    uart_send(__LINE__, __func__, "Entering flash_display()\r");

  uart_send(__LINE__, __func__, "XIP_BASE: 0x%p   Offset: 0x%6.6X   Length: 0x%X (%u)\r", XIP_BASE, Offset, Length, Length);


  for (Loop1UInt32 = Offset; Loop1UInt32 < (Offset + Length); Loop1UInt32 += 16)
  {
    sprintf(String, "[%p] ", XIP_BASE + Loop1UInt32);

    for (Loop2UInt32 = 0; Loop2UInt32 < 16; ++Loop2UInt32)
    {
      sprintf(&String[strlen(String)], "%2.2X ", FlashBaseAddress[Loop1UInt32 + Loop2UInt32]);
    }
    uart_send(__LINE__, __func__, String);


    /* Add separator. */
    sprintf(String, "| ");


    for (Loop2UInt32 = 0; Loop2UInt32 < 16; ++Loop2UInt32)
    {
      if ((FlashBaseAddress[Loop1UInt32 + Loop2UInt32] >= 0x20) && (FlashBaseAddress[Loop1UInt32 + Loop2UInt32] <= 0x7E)  && (FlashBaseAddress[Loop1UInt32 + Loop2UInt32] != 0x25))
      {
        sprintf(&String[Loop2UInt32 + 2], "%c", FlashBaseAddress[Loop1UInt32 + Loop2UInt32]);
      }
      else
      {
        sprintf(&String[Loop2UInt32 + 2], ".");
      }
    }
    uart_send(__LINE__, __func__, String);
    uart_send(__LINE__, __func__, "\r");
  }

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting flash_display()\r");

  return;
}





/* $PAGE */
/* $TITLE=flash_display_config1() */
/* ============================================================================================================================================================= *\
                                           Display RGB Matrix current active configuration 1 to an external monitor.
                                        The parameters displayed are those that are being saved to Pico's flash memory.
                         NOTE: There are 4 consecutive <%%%%> in some strings since the string needs to go throught two decoding processes:
                               One while calling uart_send(), and another one in uart_send() at "printf()" time.
\* ============================================================================================================================================================= */
UINT8 flash_display_config1(void)
{
  UCHAR DayMask[16];
  UCHAR String[256];

  UINT8 Dum1UInt8;

  UINT16 Loop1UInt16;
  UINT16 Loop2UInt16;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering flash_display_config1()\r");

  uart_send(__LINE__, __func__, "============================================================================================================\r");
  uart_send(__LINE__, __func__, "                   Entering flash_display_config1()   sizeof(FlashConfig1): 0x%4.4X (%u)\r", sizeof(FlashConfig1), sizeof(FlashConfig1));
  uart_send(__LINE__, __func__, "             sizeof(struct alarm): %u   sizeof(struct auto_scroll): %u   sizeof(struct event): %u\r", sizeof(struct alarm), sizeof(struct auto_scroll), sizeof(struct event));
  uart_send(__LINE__, __func__, "============================================================================================================\r");

  /* Display Firmware version number. */
  uart_send(__LINE__, __func__, "[%X] Firmware version:              ", &FlashConfig1.Version);
  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_VERSION_DIGITS; ++Loop1UInt16)
  {
    if  (FlashConfig1.Version[Loop1UInt16] == 0x00) break;  // get out of <for> loop when reaching end-of-string.
    if ((FlashConfig1.Version[Loop1UInt16] >= 0x20) && (FlashConfig1.Version[Loop1UInt16] <= 0x7E))
      printf("%c", FlashConfig1.Version[Loop1UInt16]);
    else
      printf("?");
  }
  printf("\r");

  uart_send(__LINE__, __func__, "[%X] FlagAutoBrightness:              %2.2u     (00 = Off   01 = On)\r",             &FlashConfig1.FlagAutoBrightness,     FlashConfig1.FlagAutoBrightness);
  uart_send(__LINE__, __func__, "[%X] Auto brightness low  limit:    %4u\r",                                          &FlashConfig1.BrightnessLoLimit,      FlashConfig1.BrightnessLoLimit);
  uart_send(__LINE__, __func__, "[%X] Auto brightness high limit:    %4u\r",                                          &FlashConfig1.BrightnessHiLimit,      FlashConfig1.BrightnessHiLimit);
  uart_send(__LINE__, __func__, "[%X] Steady brightness intensity:   %4u\r",                                          &FlashConfig1.BrightnessLevel,        FlashConfig1.BrightnessLevel);
  uart_send(__LINE__, __func__, "[%X] ChimeMode:                       %2.2u     (00 = Off   01 = On   02 = Day)\r",  &FlashConfig1.ChimeMode,              FlashConfig1.ChimeMode);
  uart_send(__LINE__, __func__, "[%X] ChimeTimeOn:                     %2u\r",                                        &FlashConfig1.ChimeTimeOn,            FlashConfig1.ChimeTimeOn);
  uart_send(__LINE__, __func__, "[%X] ChimeTimeOff:                    %2u\r",                                        &FlashConfig1.ChimeTimeOff,           FlashConfig1.ChimeTimeOff);
  uart_send(__LINE__, __func__, "[%X] ChimeLightMode:                  %2.2u     (00 = Off   01 = On   02 = Day)\r",  &FlashConfig1.ChimeLightMode,         FlashConfig1.ChimeLightMode);
  uart_send(__LINE__, __func__, "[%X] FlagButtonFeedback:              %2.2u     (00 = Off   01 = On)\r",             &FlashConfig1.FlagButtonFeedback,     FlashConfig1.FlagButtonFeedback);
  uart_send(__LINE__, __func__, "[%X] FlagIrFeedback:                  %2.2u     (00 = Off   01 = On)\r",             &FlashConfig1.FlagIrFeedback,         FlashConfig1.FlagIrFeedback);
  uart_send(__LINE__, __func__, "[%X] FlagGoldenAge:                   %2.2u     (00 = Off   01 = On)\r",             &FlashConfig1.FlagGoldenAge,          FlashConfig1.FlagGoldenAge);
  uart_send(__LINE__, __func__, "[%X] Hour considered morning start:   %2u\r",                                        &FlashConfig1.GoldenMorningStart,     FlashConfig1.GoldenMorningStart);
  uart_send(__LINE__, __func__, "[%X] Hour considered afternoon start: %2u\r",                                        &FlashConfig1.GoldenAfternoonStart,   FlashConfig1.GoldenAfternoonStart);
  uart_send(__LINE__, __func__, "[%X] Hour considered evening start:   %2u\r",                                        &FlashConfig1.GoldenEveningStart,     FlashConfig1.GoldenEveningStart);
  uart_send(__LINE__, __func__, "[%X] Hour considered night start:     %2u\r",                                        &FlashConfig1.GoldenNightStart,       FlashConfig1.GoldenNightStart);
  uart_send(__LINE__, __func__, "[%X] TimeDisplayMode:                 %2.2u     (01 = 12-Hours    02 = 24-Hours)\r", &FlashConfig1.TimeDisplayMode,        FlashConfig1.TimeDisplayMode);
  uart_send(__LINE__, __func__, "[%X] DSTCountry:                      %2.2u     (00 = No DST support   Refer to user guide for all others)\r",             &FlashConfig1.DSTCountry, FlashConfig1.DSTCountry);
  uart_send(__LINE__, __func__, "[%X] Timezone:                       %3d     (differential with UTC time)\r",                                              &FlashConfig1.Timezone,              FlashConfig1.Timezone);
  uart_send(__LINE__, __func__, "[%X] Flag Summer Time status:         %2.2u     (00 = Inactive   01 = Active)\r",     &FlashConfig1.FlagSummerTime,        FlashConfig1.FlagSummerTime);
  uart_send(__LINE__, __func__, "[%X] TemperatureUnit:                 %2.2u     (01 = Celsius    02 = Fahrenheit)\r", &FlashConfig1.TemperatureUnit,       FlashConfig1.TemperatureUnit);
  uart_send(__LINE__, __func__, "[%X] WatchdogFlag:                    %2.2u     (00 = Off   01 = On)\r",              &FlashConfig1.WatchdogFlag,          FlashConfig1.WatchdogFlag);
  uart_send(__LINE__, __func__, "[%X] WatchdogCounter:                 %2.2u\r",                                       &FlashConfig1.WatchdogCounter,       FlashConfig1.WatchdogCounter);
  uart_send(__LINE__, __func__, "[%X] Variable8FuturUse8:              %2.2u\r",                                       &FlashConfig1.Variable8FuturUse8,    FlashConfig1.Variable8FuturUse8);
  uart_send(__LINE__, __func__, "[%X] Variable8FuturUse7:              %2.2u\r",                                       &FlashConfig1.Variable8FuturUse7,    FlashConfig1.Variable8FuturUse7);
  uart_send(__LINE__, __func__, "[%X] Variable8FuturUse6:              %2.2u\r",                                       &FlashConfig1.Variable8FuturUse6,    FlashConfig1.Variable8FuturUse6);
  uart_send(__LINE__, __func__, "[%X] Variable8FuturUse5:              %2.2u\r",                                       &FlashConfig1.Variable8FuturUse5,    FlashConfig1.Variable8FuturUse5);
  uart_send(__LINE__, __func__, "[%X] Variable8FuturUse4:              %2.2u\r",                                       &FlashConfig1.Variable8FuturUse4,    FlashConfig1.Variable8FuturUse4);
  uart_send(__LINE__, __func__, "[%X] Variable8FuturUse3:              %2.2u\r",                                       &FlashConfig1.Variable8FuturUse3,    FlashConfig1.Variable8FuturUse3);
  uart_send(__LINE__, __func__, "[%X] Variable8FuturUse2:              %2.2u\r",                                       &FlashConfig1.Variable8FuturUse2,    FlashConfig1.Variable8FuturUse2);
  uart_send(__LINE__, __func__, "[%X] Variable8FuturUse1:              %2.2u\r",                                       &FlashConfig1.Variable8FuturUse1,    FlashConfig1.Variable8FuturUse1);
  uart_send(__LINE__, __func__, "[%X] Variable16FuturUse10:            %2.2u\r",                                       &FlashConfig1.Variable16FuturUse10,  FlashConfig1.Variable16FuturUse10);
  uart_send(__LINE__, __func__, "[%X] Variable16FuturUse9:             %2.2u\r",                                       &FlashConfig1.Variable16FuturUse9,   FlashConfig1.Variable16FuturUse9);
  uart_send(__LINE__, __func__, "[%X] Variable16FuturUse8:             %2.2u\r",                                       &FlashConfig1.Variable16FuturUse8,   FlashConfig1.Variable16FuturUse8);
  uart_send(__LINE__, __func__, "[%X] Variable16FuturUse7:             %2.2u\r",                                       &FlashConfig1.Variable16FuturUse7,   FlashConfig1.Variable16FuturUse7);
  uart_send(__LINE__, __func__, "[%X] Variable16FuturUse6:             %2.2u\r",                                       &FlashConfig1.Variable16FuturUse6,   FlashConfig1.Variable16FuturUse6);
  uart_send(__LINE__, __func__, "[%X] Variable16FuturUse5:             %2.2u\r",                                       &FlashConfig1.Variable16FuturUse5,   FlashConfig1.Variable16FuturUse5);
  uart_send(__LINE__, __func__, "[%X] Variable16FuturUse4:             %2.2u\r",                                       &FlashConfig1.Variable16FuturUse4,   FlashConfig1.Variable16FuturUse4);
  uart_send(__LINE__, __func__, "[%X] Variable16FuturUse3:             %2.2u\r",                                       &FlashConfig1.Variable16FuturUse3,   FlashConfig1.Variable16FuturUse3);
  uart_send(__LINE__, __func__, "[%X] Variable16FuturUse2:             %2.2u\r",                                       &FlashConfig1.Variable16FuturUse2,   FlashConfig1.Variable16FuturUse2);
  uart_send(__LINE__, __func__, "[%X] Variable16FuturUse1:             %2.2u\r",                                       &FlashConfig1.Variable16FuturUse1,   FlashConfig1.Variable16FuturUse1);
  uart_send(__LINE__, __func__, "[%X] Variable32FuturUse2:             %2.2lu\r",                                      &FlashConfig1.Variable32FuturUse2,   FlashConfig1.Variable32FuturUse2);
  uart_send(__LINE__, __func__, "[%X] Variable32FuturUse1:             %2.2lu\r",                                      &FlashConfig1.Variable32FuturUse1,   FlashConfig1.Variable32FuturUse1);
  printf("\r");
  sleep_ms(30);  // prevent communication override.


  /* In case SSID is not initialized, display it character by character. */
  uart_send(__LINE__, __func__, "Note: SSID and Password begin at 5th character position, superimposed on top of two different footprints.\r");
  uart_send(__LINE__, __func__, "      If a non-displayable character is found in either string, it is replaced with a <?> on the screen.\r");
  uart_send(__LINE__, __func__, "      End-of-string characters are also replaced by an <?> on the screen.\r");
  sprintf(String, "SSID:     [");
  for (Loop1UInt16 = 0; Loop1UInt16 < sizeof(FlashConfig1.SSID); ++Loop1UInt16)
  {
    if ((FlashConfig1.SSID[Loop1UInt16] >= 0x20) && (FlashConfig1.SSID[Loop1UInt16] <= 0x7E))
      sprintf(&String[strlen(String)], "%c", FlashConfig1.SSID[Loop1UInt16]);
    else
    {
      if (FlashConfig1.SSID[Loop1UInt16] == 0x00) break;  // get out of for loop when encountering an end-of-string.
      strcat(&String[strlen(String)], "?");
    }
  }
  strcat(String, "]\r");
  uart_send(__LINE__, __func__, String);



  /* In case password is not initialized, display it character by character. */
  sprintf(String, "Password: [");
  for (Loop1UInt16 = 0; Loop1UInt16 < sizeof(FlashConfig1.Password); ++Loop1UInt16)
  {
    if ((FlashConfig1.Password[Loop1UInt16] >= 0x20) && (FlashConfig1.Password[Loop1UInt16] <= 0x7E))
      sprintf(&String[strlen(String)], "%c", FlashConfig1.Password[Loop1UInt16]);
    else
    {
      if (FlashConfig1.Password[Loop1UInt16] == 0x00) break;  // get out of for loop when encountering an end-of-string.
      strcat(&String[strlen(String)], "?");
    }
  }
  strcat(String, "]\r\r");
  uart_send(__LINE__, __func__, String);



  /* Display all 7 days-of-week with its corresponding bitmask. */
  printf("\r");
  uart_send(__LINE__, __func__, "Bit mask used for alarm DayOfWeek selection:\r");
  for (Loop1UInt16 = 0; Loop1UInt16 < 7; ++Loop1UInt16)
  {
    /* Get binary string representing day mask for this particular day-of-week. */
    util_uint64_to_binary_string(1 << Loop1UInt16, 8, String);

    sprintf(&String[strlen(String)], "   %s\r", DayName[Loop1UInt16]);
    uart_send(__LINE__, __func__, String);
  }
  printf("r\r");



  /* Flag indicating that we want to show alarms status on LED matrix. */
  uart_send(__LINE__, __func__, "[%X] FlagDisplayAlarms:               %2.2u     (00 = Off   01 = On)\r",  &FlashConfig1.FlagDisplayAlarms,    FlashConfig1.FlagDisplayAlarms);
  /* Flag indicating that we want to show target days for active alarms on LED matrix. */
  uart_send(__LINE__, __func__, "[%X] FlagDisplayAlarmDays:            %2.2u     (00 = Off   01 = On)\r\r", &FlashConfig1.FlagDisplayAlarmDays, FlashConfig1.FlagDisplayAlarmDays);



  /* Scan and display all alarms (0 to 8) -> (correspond to alarm number 1 to 9 for RGB Matrix user). */
  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_ALARMS; ++Loop1UInt16)
  {
    if (FlashConfig1.Alarm[Loop1UInt16].FlagStatus)
      sprintf(String, "***** ON *****");
    else
      String[0] = 0x00;
    uart_send(__LINE__, __func__, " --------------------------------- Alarm number %u ----------------------------------\r", Loop1UInt16);
    uart_send(__LINE__, __func__, "[%X] Alarm[%2.2u].Status:             %2.2u     (00 = Off   01 = On)  %s\r", &FlashConfig1.Alarm[Loop1UInt16].FlagStatus, Loop1UInt16, FlashConfig1.Alarm[Loop1UInt16].FlagStatus, String);
    uart_send(__LINE__, __func__, "[%X] Alarm[%2.2u].Hour:               %2.2u\r", &FlashConfig1.Alarm[Loop1UInt16].Hour,          Loop1UInt16, FlashConfig1.Alarm[Loop1UInt16].Hour);
    uart_send(__LINE__, __func__, "[%X] Alarm[%2.2u].Minute:             %2.2u\r", &FlashConfig1.Alarm[Loop1UInt16].Minute,        Loop1UInt16, FlashConfig1.Alarm[Loop1UInt16].Minute);
    uart_send(__LINE__, __func__, "[%X] Alarm[%2.2u].NumberOfBeeps:    %4u\r",     &FlashConfig1.Alarm[Loop1UInt16].NumberOfBeeps, Loop1UInt16, FlashConfig1.Alarm[Loop1UInt16].NumberOfBeeps);
    uart_send(__LINE__, __func__, "[%X] Alarm[%2.2u].BeepMSec:         %4u\r",     &FlashConfig1.Alarm[Loop1UInt16].BeepMSec,      Loop1UInt16, FlashConfig1.Alarm[Loop1UInt16].BeepMSec);
    uart_send(__LINE__, __func__, "[%X] Alarm[%2.2u].RepeatPeriod:     %4u  seconds\r", &FlashConfig1.Alarm[Loop1UInt16].RepeatPeriod,  Loop1UInt16, FlashConfig1.Alarm[Loop1UInt16].RepeatPeriod);
    uart_send(__LINE__, __func__, "[%X] Alarm[%2.2u].RingDuration:     %4u  seconds (global time)\r", &FlashConfig1.Alarm[Loop1UInt16].RingDuration,  Loop1UInt16, FlashConfig1.Alarm[Loop1UInt16].RingDuration);


    /* Display days-of-week. */
    util_uint64_to_binary_string(FlashConfig1.Alarm[Loop1UInt16].DayMask, 8, DayMask);
    sprintf(String, "[%8.8X] Alarm[%2.2u].DayMask:      %s  (0x%2.2X) ", &FlashConfig1.Alarm[Loop1UInt16].DayMask, Loop1UInt16, DayMask, FlashConfig1.Alarm[Loop1UInt16].DayMask);

    /* Display short name of all days-of-week selected for this alarms (those selected in the bit mask). */
    for (Loop2UInt16 = 0; Loop2UInt16 < 7; ++Loop2UInt16)
    {
      if (FlashConfig1.Alarm[Loop1UInt16].DayMask & (1 << Loop2UInt16))
        sprintf(&String[strlen(String)], "%s ", ShortDay[Loop2UInt16]);
    }
    strcat(String, "\r");
    uart_send(__LINE__, __func__, String);


    /* Display message for this alarm. */
    uart_send(__LINE__, __func__, "[%X] Alarm[%2.2u].Message:                <", &FlashConfig1.Alarm[Loop1UInt16].Message, Loop1UInt16, FlashConfig1.Alarm[Loop1UInt16].Message);
    for (Loop2UInt16 = 0; Loop2UInt16 < sizeof(FlashConfig1.Alarm[Loop1UInt16].Message); ++Loop2UInt16)
    {
      if  (FlashConfig1.Alarm[Loop1UInt16].Message[Loop2UInt16] == 0x00) break;  // get out of <for> loop when reaching end-of-string.
      if ((FlashConfig1.Alarm[Loop1UInt16].Message[Loop2UInt16] >= 0x20) && (FlashConfig1.Alarm[Loop1UInt16].Message[Loop2UInt16] <= 0x7E))
        printf("%c", FlashConfig1.Alarm[Loop1UInt16].Message[Loop2UInt16]);
      else
        printf("?");
    }
    printf(">\r");

    /* Number of times the message will scroll at each "ring". */
    uart_send(__LINE__, __func__, "[%X] Alarm[%2.2u].NumberOfScrolls:  %4u\r\r", &FlashConfig1.Alarm[Loop1UInt16].NumberOfScrolls, Loop1UInt16, FlashConfig1.Alarm[Loop1UInt16].NumberOfScrolls);
  }
  printf("\r");
  sleep_ms(30);  // prevent communication override.



  /* Display auto scroll data. */
  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_AUTO_SCROLLS; ++Loop1UInt16)
  {
    printf("------------------------- Auto-Scroll number %u -------------------------\r\r", Loop1UInt16);
    display_auto_scroll(Loop1UInt16);
  }
  printf("\r\r");



  /* Display Reserved data. */
  uart_send(__LINE__, __func__, "[%X] Reserved - size: 0x%2.2X (%3u):\r", &FlashConfig1.Reserved, sizeof(FlashConfig1.Reserved), sizeof(FlashConfig1.Reserved));
  uart_send(__LINE__, __func__, "[%8.8X] ", &FlashConfig1.Reserved);
  for (Loop1UInt16 = 0; Loop1UInt16 < sizeof(FlashConfig1.Reserved); ++Loop1UInt16)
  {
    if (((((UINT32)(&FlashConfig1.Reserved[Loop1UInt16])) % 16) == 0) && (Loop1UInt16 != 0))
    {
      printf("\r");
      uart_send(__LINE__, __func__, "[%8.8X] ", &FlashConfig1.Reserved[Loop1UInt16]);
    }
    uart_send(__LINE__, __func__, "- 0x%2.2X ", FlashConfig1.Reserved[Loop1UInt16]);
  }
  uart_send(__LINE__, __func__, "\r");
  printf("\r");
  sleep_ms(30);  // prevent communication override.



  /* Display calendar events. */
  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_EVENTS; ++Loop1UInt16)
  {
    uart_send(__LINE__, __func__, " ------------------------- Event number %u -------------------------\r\r", Loop1UInt16);
    uart_send(__LINE__, __func__, "[%X] Day:                           %2u\r",           &FlashConfig1.Event[Loop1UInt16].Day,         FlashConfig1.Event[Loop1UInt16].Day);         // day of month.
    uart_send(__LINE__, __func__, "[%X] Month:                         %2u\r",           &FlashConfig1.Event[Loop1UInt16].Month,       FlashConfig1.Event[Loop1UInt16].Month);       // month.
    uart_send(__LINE__, __func__, "[%X] Jingle ID:                     %2u\r",           &FlashConfig1.Event[Loop1UInt16].Jingle,      FlashConfig1.Event[Loop1UInt16].Jingle);      // jingle ID.

    uart_send(__LINE__, __func__, "[%X] String to scroll: <", &FlashConfig1.Event[Loop1UInt16].Message);
    for (Loop2UInt16 = 0; Loop2UInt16 < sizeof(FlashConfig1.Event[Loop1UInt16].Message); ++Loop2UInt16)
    {
      if  (FlashConfig1.Event[Loop1UInt16].Message[Loop2UInt16] == 0x00) break;  // get out of <for> loop when reaching end-of-string.
      if ((FlashConfig1.Event[Loop1UInt16].Message[Loop2UInt16] >= 0x20) && (FlashConfig1.Event[Loop1UInt16].Message[Loop2UInt16] <= 0x7E))
        printf("%c", FlashConfig1.Event[Loop1UInt16].Message[Loop2UInt16]);
      else
        printf("?");
    }
    printf(">\r\r");
    sleep_ms(3);  // prevent communication override.
  }
  printf("\r\r");
  sleep_ms(30);  // prevent communication override.



  uart_send(__LINE__, __func__, "[%X] CRC16: 0x%4.4X\r\r\r", &FlashConfig1.Crc16, FlashConfig1.Crc16);
  uart_send(__LINE__, __func__, "Size of data for CRC16:  %9u -  %9u = 0x%4.4X    (%lu)\r", &FlashConfig1.Crc16, &FlashConfig1.Version, (UINT32)&FlashConfig1.Crc16 - (UINT32)&FlashConfig1.Version, (UINT32)&FlashConfig1.Crc16 - (UINT32)&FlashConfig1.Version);
  uart_send(__LINE__, __func__, "                in hex: 0x%8.8X - 0x%8.8X = 0x%4.4X\r",    &FlashConfig1.Crc16, &FlashConfig1.Version, (UINT32)&FlashConfig1.Crc16 - (UINT32)&FlashConfig1.Version);
  uart_send(__LINE__, __func__, "========================================================================================================================\r\r\r\r\r\r");

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting flash_display_config1()\r");

  return 0;
}





/* $PAGE */
/* $TITLE=flash_display_config2() */
/* ============================================================================================================================================================= *\
                                           Display RGB Matrix current active configuration 2 to an external monitor.
                                        The parameters displayed are those that are being saved to Pico's flash memory.
\* ============================================================================================================================================================= */
UINT8 flash_display_config2(void)
{
  UCHAR DayMask[16];
  UCHAR String[256];

  UINT8 Dum1UInt8;

  UINT16 Loop1UInt16;
  UINT16 Loop2UInt16;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering flash_display_config2()\r");

  uart_send(__LINE__, __func__, "============================================================================================================\r");
  uart_send(__LINE__, __func__, "                   Entering flash_display_config2()   sizeof(FlashConfig2): 0x%4.4X (%u)\r", sizeof(FlashConfig2), sizeof(FlashConfig2));
  uart_send(__LINE__, __func__, "                   sizeof(struct reminder1): %u\r", sizeof(struct reminder1));
  uart_send(__LINE__, __func__, "============================================================================================================\r");

  /* Display Firmware version number. */
  uart_send(__LINE__, __func__, "[%X] Firmware version:            ", &FlashConfig2.Version);
  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_VERSION_DIGITS; ++Loop1UInt16)
  {
    if  (FlashConfig2.Version[Loop1UInt16] == 0x00) break;  // get out of <for> loop when reaching end-of-string.
    if ((FlashConfig2.Version[Loop1UInt16] >= 0x20) && (FlashConfig2.Version[Loop1UInt16] <= 0x7E))
      printf("%c", FlashConfig2.Version[Loop1UInt16]);
    else
      printf("?");
  }
  printf("\r\r");



  /* Display reminders1. */
  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_REMINDERS1; ++Loop1UInt16)
  {
    display_reminder1(Loop1UInt16);
    sleep_ms(10);  // prevent communication override.
  }
  printf("\f\f");


  /* Display Reserved data. */
  uart_send(__LINE__, __func__, "[%X] Reserved - size: 0x%2.2X (%3u):\r", &FlashConfig2.Reserved[0], sizeof(FlashConfig2.Reserved), sizeof(FlashConfig2.Reserved));
  uart_send(__LINE__, __func__, "[%8.8X] ", &FlashConfig2.Reserved[Loop1UInt16]);
  for (Loop1UInt16 = 0; Loop1UInt16 < sizeof(FlashConfig2.Reserved); ++Loop1UInt16)
  {
    if (((((UINT32)(&FlashConfig2.Reserved[Loop1UInt16])) % 16) == 0) && (Loop1UInt16 != 0))
    {
      printf("\r");
      uart_send(__LINE__, __func__, "[%8.8X] ", &FlashConfig2.Reserved[Loop1UInt16]);
    }
    uart_send(__LINE__, __func__, "- 0x%2.2X ", FlashConfig2.Reserved[Loop1UInt16]);
  }
  uart_send(__LINE__, __func__, "\r");
  printf("\r");



  uart_send(__LINE__, __func__, "[%X] CRC16:                  0x%4.4X\r\r\r", &FlashConfig2.Crc16, FlashConfig2.Crc16);
  uart_send(__LINE__, __func__, "Size of data for CRC16:  %9u -  %9u = 0x%4.4X    (%lu)\r", &FlashConfig2.Crc16, &FlashConfig2.Version, (UINT32)&FlashConfig2.Crc16 - (UINT32)&FlashConfig2.Version, (UINT32)&FlashConfig2.Crc16 - (UINT32)&FlashConfig2.Version);
  uart_send(__LINE__, __func__, "                in hex: 0x%8.8X - 0x%8.8X = 0x%4.4X\r",    &FlashConfig2.Crc16, &FlashConfig2.Version, (UINT32)&FlashConfig2.Crc16 - (UINT32)&FlashConfig2.Version);
  uart_send(__LINE__, __func__, "=========================================================================================================\r\r\r");

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting flash_display_config2()\r");

  return 0;
}





/* $PAGE */
/* $TITLE=flash_erase() */
/* ============================================================================================================================================================= *\
                                                        Erase configuration data data in Pico's flash memory.
                                 The way the electronics is done, one sector of the flash (4096 bytes) must be erased at a time.
                         This function has been kept simple and one sector (4096 bytes) will be erased, beginning at the specified offset,
                                                         which must be aligned on a sector boundary (4096).
\* ============================================================================================================================================================= */
void flash_erase(UINT32 DataOffset)
{
  UCHAR String[256];

  UINT8 OriginalClockMode;

  UINT16 Loop1UInt16;

  UINT32 InterruptMask;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering flash_erase()\r");

  /* Erase an area of the Pico's flash memory. */
  /* NOTE: Caller is responsible for blanking LED display while interrupts are disabled. */
  /* Keep track of interrupt mask on entry. */
  InterruptMask = save_and_disable_interrupts();

  /* Erase flash area to reprogram. */
  flash_range_erase(DataOffset, FLASH_SECTOR_SIZE);

  /* Restore original interrupt mask when done. */
  restore_interrupts(InterruptMask);

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting flash_erase()\r");

  return;
}





/* $PAGE */
/* $TITLE=flash_read_config1() */
/* ============================================================================================================================================================= *\
                                                   Read RGB Matrix configuration 1 data from flash memory.
\* ============================================================================================================================================================= */
UINT8 flash_read_config1(void)
{
  UINT8 *FlashBaseAddress;

  UINT16 Dum1UInt16;
  UINT16 Loop1UInt16;
  UINT16 Loop2UInt16;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering flash_read_config1()\r");

  if (DebugBitMask & DEBUG_FLASH)
  {
    uart_send(__LINE__, __func__, " =======================================================================================================================\r");
    uart_send(__LINE__, __func__, "     Entering flash_read_config1()\r");
    uart_send(__LINE__, __func__, "     Read current configuration 1 from Pico's flash\r");
    uart_send(__LINE__, __func__, "     XIP_BASE: 0x%X     FLASH_CONFIG1_OFFSET: 0x%X\r", XIP_BASE, FLASH_CONFIG1_OFFSET);
    uart_send(__LINE__, __func__, "     sizeof(FlashConfig1): 0x%4.4X (%4u)\r", sizeof(FlashConfig1), sizeof(FlashConfig1));
    uart_send(__LINE__, __func__, "     sizeof(struct alarm): 0x%4.4X (%4u)   sizeof(struct event): 0x%4.4X (%4u)\r", sizeof(struct alarm), sizeof(struct alarm), sizeof(struct event), sizeof(struct event));
    uart_send(__LINE__, __func__, " =======================================================================================================================\r");
  }

  /* Read RGB Matrix configuration data from Pico's flash memory. */
  FlashBaseAddress = (UINT8 *)(XIP_BASE);
  for (Loop1UInt16 = 0; Loop1UInt16 < sizeof(FlashConfig1); ++Loop1UInt16)
    ((UINT8 *)(FlashConfig1.Version))[Loop1UInt16] = FlashBaseAddress[FLASH_CONFIG1_OFFSET + Loop1UInt16];

  /* Optionally display raw configuration data retrieved from flash memory. */
  if (DebugBitMask & DEBUG_FLASH)
  {
    /* Use util_display_data() to prevent a crash if some value represent garbage (DayOfMonth, Month, etc...). */
    util_display_data(FlashConfig1.Version, sizeof(FlashConfig1));
  }

  /* Validate CRC16 read from flash. */
  Dum1UInt16 = util_crc16((UINT8 *)&FlashConfig1, (UINT32)&FlashConfig1.Crc16 - (UINT32)&FlashConfig1.Version);


  if (DebugBitMask & DEBUG_FLASH)
  {
    uart_send(__LINE__, __func__, "CRC16 saved in flash configuration 1: 0x%4.4X (%5u)\r", FlashConfig1.Crc16, FlashConfig1.Crc16);
    uart_send(__LINE__, __func__, "CRC16 computed from data read:        0x%4.4X (%5u)\r", Dum1UInt16, Dum1UInt16);
  }

  /* If CRC16 read from flash does not correspond to the data read, assign default values and save new configuration. */
  if (FlashConfig1.Crc16 == Dum1UInt16)
  {
    if (DebugBitMask & DEBUG_FLASH)
    {
      /* Optionally display formatted configuration data retrieved from flash memory. */
      uart_send(__LINE__, __func__, "Flash configuration 1 is valid.\r\r\r");
      uart_send(__LINE__, __func__, "Display RGB Matrix configuration 1 data retrieved from flash memory:\r");
      flash_display_config1();
    }

    return 0;
  }



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                   Flash uninitialized or corrupted.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (FlashConfig1.Crc16 == 0xFFFF)
  {
    /* CRC16 is not valid. If CRC16 read from flash is 0xFFFF, we assume that no configuration has ever been saved to flash memory.
       If so, assign default values to configuration parameters and save it to flash. */
    if (DebugBitMask & DEBUG_FLASH)
    {
      uart_send(__LINE__, __func__, "Flash configuration 1 has never been initialized...\r");
      uart_send(__LINE__, __func__, "Setting up and save a default configuration 1 to flash.\r\r\r");
    }
  }
  else
  {
    /* Configuration read from flash seems to be corrupted. Save a valid default configuration. */
    if (DebugBitMask & DEBUG_FLASH)
    {
      uart_send(__LINE__, __func__, "Flash configuration 1 seems to be corrupted...\r");
      uart_send(__LINE__, __func__, "Setting up and save a default configuration 1 to flash.\r\r\r");
    }
  }

  /* Assign default values and save a new configuration to flash. */
  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_VERSION_DIGITS; ++Loop1UInt16)
    FlashConfig1.Version[Loop1UInt16] = 0x20;
  FlashConfig1.Version[MAX_VERSION_DIGITS - 1] = 0x00;        // end-of-string.
  sprintf(FlashConfig1.Version, "%s", FIRMWARE_VERSION);      // firmware version number.

  FlashConfig1.FlagAutoBrightness    = FLAG_ON;               // flag indicating we are in "Auto Brightness" mode.
  FlashConfig1.BrightnessLoLimit     = 1;                     // (between 1 and 1000) 1    is the  lowest brightness level for LED matrix when in auto brightness mode.
  FlashConfig1.BrightnessHiLimit     = 500;                   // (between 1 and 1000) 1000 is the highest brightness level for LED matrix when in auto brightness mode.
  FlashConfig1.BrightnessLevel       = 400;                   // (between 1 and 1000) 400  is the  steady brightness level when not in auto brightness mode.
  FlashConfig1.ChimeMode             = CHIME_DEFAULT;         // chime mode (Off / On / Day).
  FlashConfig1.ChimeTimeOn           = CHIME_TIME_ON;         // hourly chime will begin at this hour.
  FlashConfig1.ChimeTimeOff          = CHIME_TIME_OFF;        // hourly chime will begin at this hour.
  FlashConfig1.ChimeLightMode        = CHIME_HALF_HOUR;       // half-hour light chime mode (Off / On / Day).
  FlashConfig1.FlagButtonFeedback    = FLAG_ON;               // flag for local buttons audible feedback ("button-press" tone)
  FlashConfig1.FlagIrFeedback        = FLAG_ON;               // flag for remote control button audible feedback ("remote button-press" tone)
  FlashConfig1.FlagGoldenAge         = FLAG_OFF;              // flag to set a few behaviors to help old persons.
  FlashConfig1.GoldenMorningStart    = 7;                     // hour considered "morning start".
  FlashConfig1.GoldenAfternoonStart  = 12;                    // hour considered "afternoon start".
  FlashConfig1.GoldenEveningStart    = 18;                    // hour considered "evening start".
  FlashConfig1.GoldenNightStart      = 21;                    // hour considered "night start".
  FlashConfig1.TimeDisplayMode       = TIME_DISPLAY_DEFAULT;  // H12 or H24 default value.
  FlashConfig1.DSTCountry            = DST_COUNTRY;           // specifies how to handle the daylight saving time depending of country (see User Guide).
  FlashConfig1.Timezone              = TIMEZONE;              // time difference between local time and Universal Coordinated Time.
  FlashConfig1.FlagSummerTime        = FLAG_ON;               // system will evaluate and overwrite this value on next power-up sequence.
  FlashConfig1.TemperatureUnit       = TEMPERATURE_DEFAULT;   // CELSIUS or FAHRENHEIT default value (see clock options above).
  FlashConfig1.WatchdogFlag          = FLAG_OFF;              // variable reserved for watchdog mechanism.
  FlashConfig1.WatchdogCounter       = 0;                     // variable to count cumulative number of start triggered by watchdog.
  FlashConfig1.Variable8FuturUse8    = 0;                     // placeholder  8-bits variable reserved for future use.
  FlashConfig1.Variable8FuturUse7    = 0;                     // placeholder  8-bits variable reserved for future use.
  FlashConfig1.Variable8FuturUse6    = 0;                     // placeholder  8-bits variable reserved for future use.
  FlashConfig1.Variable8FuturUse5    = 0;                     // placeholder  8-bits variable reserved for future use.
  FlashConfig1.Variable8FuturUse4    = 0;                     // placeholder  8-bits variable reserved for future use.
  FlashConfig1.Variable8FuturUse3    = 0;                     // placeholder  8-bits variable reserved for future use.
  FlashConfig1.Variable8FuturUse2    = 0;                     // placeholder  8-bits variable reserved for future use.
  FlashConfig1.Variable8FuturUse1    = 0;                     // placeholder  8-bits variable reserved for future use.
  FlashConfig1.Variable16FuturUse10  = 0;                     // placeholder 16-bits variable reserved for future use.
  FlashConfig1.Variable16FuturUse9   = 0;                     // placeholder 16-bits variable reserved for future use.
  FlashConfig1.Variable16FuturUse8   = 0;                     // placeholder 16-bits variable reserved for future use.
  FlashConfig1.Variable16FuturUse7   = 0;                     // placeholder 16-bits variable reserved for future use.
  FlashConfig1.Variable16FuturUse6   = 0;                     // placeholder 16-bits variable reserved for future use.
  FlashConfig1.Variable16FuturUse5   = 0;                     // placeholder 16-bits variable reserved for future use.
  FlashConfig1.Variable16FuturUse4   = 0;                     // placeholder 16-bits variable reserved for future use.
  FlashConfig1.Variable16FuturUse3   = 0;                     // placeholder 16-bits variable reserved for future use.
  FlashConfig1.Variable16FuturUse2   = 0;                     // placeholder 16-bits variable reserved for future use.
  FlashConfig1.Variable16FuturUse1   = 0;                     // placeholder 16-bits variable reserved for future use.
  FlashConfig1.Variable32FuturUse2   = 0l;                    // placeholder 32-bits variable reserved for future use.
  FlashConfig1.Variable32FuturUse1   = 0l;                    // placeholder 32-bits variable reserved for future use.


  /* Network credentials. */
  for (Loop1UInt16 = 0; Loop1UInt16 < sizeof(FlashConfig1.SSID); ++Loop1UInt16) FlashConfig1.SSID[Loop1UInt16] = 0x00;          // wipe current data.
  sprintf(FlashConfig1.SSID,     "MyNetworkName");
  for (Loop1UInt16 = 0; Loop1UInt16 < sizeof(FlashConfig1.Password); ++Loop1UInt16) FlashConfig1.Password[Loop1UInt16] = 0x00;  // wipe current data.
  sprintf(FlashConfig1.Password, "MyPassword");


  FlashConfig1.FlagDisplayAlarms    = FLAG_ON;        // flag indicating that we want to show alarms status on LED matrix.
  FlashConfig1.FlagDisplayAlarmDays = FLAG_ON;        // flag indicating that we want to show days with an active alarms on LED matrix.


  /* Default configuration for 9 alarms. Text may be changed for another 40-characters max string. */
  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_ALARMS; ++Loop1UInt16)
  {
    FlashConfig1.Alarm[Loop1UInt16].FlagStatus = FLAG_OFF; // all alarms set to Off in default configuration.
    FlashConfig1.Alarm[Loop1UInt16].Hour       = 14;
    FlashConfig1.Alarm[Loop1UInt16].Minute     = (Loop1UInt16 * 5);
    FlashConfig1.Alarm[Loop1UInt16].DayMask    = 0;  // will be assigned below.
    sprintf(FlashConfig1.Alarm[Loop1UInt16].Message, "This is Alarm Number %u", Loop1UInt16 + 1);  // string to be displayed / scrolled when alarm is triggered (ALARM TEXT).
    FlashConfig1.Alarm[Loop1UInt16].NumberOfScrolls = 1;              // number of scrolls for each ring.
    FlashConfig1.Alarm[Loop1UInt16].NumberOfBeeps = Loop1UInt16 + 1;  // number of beeps for each ring.
    FlashConfig1.Alarm[Loop1UInt16].BeepMSec      = 100;              // number of msec for each beep.
    FlashConfig1.Alarm[Loop1UInt16].RepeatPeriod  = 15;               // number of seconds before the "beeps" sound again.
    FlashConfig1.Alarm[Loop1UInt16].RingDuration  = 1800;             // number of seconds for total beeps duration (1800 = one half-hour).
  }

  /* Data specific to each of the 9 alarms. */
  FlashConfig1.Alarm[0].DayMask    = (1 << MON) + (1 << TUE) + (1 << WED) + (1 << THU) + (1 << FRI);
  FlashConfig1.Alarm[1].DayMask    = (1 << SAT) + (1 << SUN);
  FlashConfig1.Alarm[2].DayMask    = (1 << SUN);
  FlashConfig1.Alarm[3].DayMask    = (1 << MON);
  FlashConfig1.Alarm[4].DayMask    = (1 << TUE);
  FlashConfig1.Alarm[5].DayMask    = (1 << WED);
  FlashConfig1.Alarm[6].DayMask    = (1 << THU);
  FlashConfig1.Alarm[7].DayMask    = (1 << FRI);
  FlashConfig1.Alarm[8].DayMask    = (1 << SAT);


  /* Assign default values for auto-scroll. */
  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_AUTO_SCROLLS; ++Loop1UInt16)
  {
    FlashConfig1.AutoScroll[Loop1UInt16].Period = 0;  // auto-scroll will repeat at this number of minutes (0 = inactive).

    for (Loop2UInt16 = 0; Loop2UInt16 < MAX_ITEMS; ++Loop2UInt16)
    {
      FlashConfig1.AutoScroll[Loop1UInt16].FunctionId[Loop2UInt16] = 0;  // if FunctionId = 0 means "nothing to scroll".
    }
  }

  /* Make the first auto-scroll active with a few items. */
  FlashConfig1.AutoScroll[0].Period = 15;  // auto-scroll 0 will repeat every 15 minutes.
  FlashConfig1.AutoScroll[0].FunctionId[0] = 200;  // Firmware Version.
  FlashConfig1.AutoScroll[0].FunctionId[1] = 209;  // Daylight Saving Time and Timezone settings.
  FlashConfig1.AutoScroll[0].FunctionId[2] = 201;  // Microcontroller type and Unique ID.
  FlashConfig1.AutoScroll[0].FunctionId[3] = 216;  // RGB Matrix uptime
  FlashConfig1.AutoScroll[0].FunctionId[4] = 202;  // Temperature.


  /* Make provision for future parameters. */
  for (Loop1UInt16 = 0; Loop1UInt16 < sizeof(FlashConfig1.Reserved); ++Loop1UInt16)
    FlashConfig1.Reserved[Loop1UInt16] = 0xFF;


  /* Assign default calendar events. */
  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_EVENTS; ++Loop1UInt16)
  {
    FlashConfig1.Event[Loop1UInt16].Day    = 0;  // initialize as an invalid day-of-month so it has no impact.
    FlashConfig1.Event[Loop1UInt16].Month  = 1;
    FlashConfig1.Event[Loop1UInt16].Jingle = 0;
    sprintf(FlashConfig1.Event[Loop1UInt16].Message, "Calendar event number %u", Loop1UInt16 + 1);
  }


  /* Save the default configuration assigned above. */
  flash_save_config1();

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting flash_read_config1()\r");

  return 1;
}





/* $PAGE */
/* $TITLE=flash_read_config2() */
/* ============================================================================================================================================================= *\
                                                   Read RGB Matrix configuration 2 data from flash memory.
\* ============================================================================================================================================================= */
UINT8 flash_read_config2(void)
{
  UCHAR String[256];

  UINT8 *FlashBaseAddress;

  UINT16 Dum1UInt16;
  UINT16 Loop1UInt16;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering flash_read_config2()\r");

  if (DebugBitMask & DEBUG_FLASH)
  {
    uart_send(__LINE__, __func__, " =======================================================================================================================\r");
    uart_send(__LINE__, __func__, "     Entering flash_read_config2()\r");
    uart_send(__LINE__, __func__, "     Read current configuration 2 from Pico's flash\r");
    uart_send(__LINE__, __func__, "     XIP_BASE: 0x%8.8X     FLASH_CONFIG2_OFFSET: 0x%8.8X\r", XIP_BASE, FLASH_CONFIG2_OFFSET);
    uart_send(__LINE__, __func__, "     sizeof(FlashConfig2):    0x%4.4X (%u)\r", sizeof(FlashConfig2), sizeof(FlashConfig2));
    uart_send(__LINE__, __func__, "     sizeof(struct calendar): 0x%4.4X (%4u)     sizeof(struct reminder1): 0x%4.4X (%4u)\r", sizeof(struct event), sizeof(struct event), sizeof(struct reminder1), sizeof(struct reminder1));
    uart_send(__LINE__, __func__, " =======================================================================================================================\r");
  }

  /* Read RGB Matrix configuration 2 data from Pico's flash memory (as an array of UINT8). */
  FlashBaseAddress = (UINT8 *)(XIP_BASE);
  for (Loop1UInt16 = 0; Loop1UInt16 < sizeof(FlashConfig2); ++Loop1UInt16)
  {
    ((UINT8 *)(FlashConfig2.Version))[Loop1UInt16] = FlashBaseAddress[FLASH_CONFIG2_OFFSET + Loop1UInt16];
  }

  /* Optionally display configuration data 2 retrieved from flash memory. */
  if (DebugBitMask & DEBUG_FLASH)
  {
    /// uart_send(__LINE__, __func__, "Display RGB Matrix configuration 2 data retrieved from flash memory:\r");
    /// flash_display_config2();
    /* Use util_display_data() to prevent a crash if some value represent garbage (DayOfMonth, Month, etc...). */
    util_display_data(FlashConfig2.Version, sizeof(FlashConfig2));
  }


  /* Validate CRC16 read from flash. */
  Dum1UInt16 = util_crc16((UINT8 *)&FlashConfig2, ((UINT32)&FlashConfig2.Crc16 - (UINT32)&FlashConfig2.Version));



  /***
  FlashConfig2.Crc16 == 0xFFFF;  // force a default configuration 2 to be saved to flash.
  uart_send(__LINE__, __func__, " ----------==========<<<<<<<<<< FORCE A DEFAULT CONFIGURATION 2 TO BE SAVED TO FLASH >>>>>>>>>>==========----------\r\r\r\r\r\r\r");
  ***/



  if (DebugBitMask & DEBUG_FLASH)
  {
    uart_send(__LINE__, __func__, "CRC16 saved in flash configuration 2: 0x%4.4X (%5u)\r", FlashConfig2.Crc16, FlashConfig2.Crc16);
    uart_send(__LINE__, __func__, "CRC16 computed from data read:        0x%4.4X (%5u)\r", Dum1UInt16, Dum1UInt16);
  }

  /* If CRC16 read from flash does not correspond to the data read, assign default values and save new configuration. */
  if (FlashConfig2.Crc16 == Dum1UInt16)
  {
    if (DebugBitMask & DEBUG_FLASH)
      uart_send(__LINE__, __func__, "Flash configuration 2 is valid.\r\r\r");

    /* Optionally display configuration 2 data retrieved from flash memory. */
    if (DebugBitMask & DEBUG_FLASH)
    {
      uart_send(__LINE__, __func__, "Display RGB Matrix configuration 2 data retrieved from flash memory:\r");
      flash_display_config2();
    }

    return 0;
  }

  if (FlashConfig2.Crc16 == 0xFFFF)
  {
    /* CRC16 is not valid. If CRC16 read from flash is 0xFFFF, we assume that no configuration 2 has ever been saved to flash memory.
       If so, assign default values to configuration parameters and save it to flash. */
    if (DebugBitMask & DEBUG_FLASH)
    {
      uart_send(__LINE__, __func__, "Flash configuration 2 has never been initialized...\r");
      uart_send(__LINE__, __func__, "Save a default configuration 2 to flash.\r\r\r");
    }
  }
  else
  {
    /* Configuration read from flash seems to be corrupted. Save a valid default configuration. */
    if (DebugBitMask & DEBUG_FLASH)
    {
      uart_send(__LINE__, __func__, "Flash configuration 2 seems to be corrupted...\r");
      uart_send(__LINE__, __func__, "Save a default configuration 2 to flash.\r\r\r");
    }
  }

  /* Assign default values and save a new configuration to flash. */
  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_VERSION_DIGITS; ++Loop1UInt16)
    FlashConfig2.Version[Loop1UInt16] = 0x20;
  FlashConfig2.Version[MAX_VERSION_DIGITS - 1] = 0x00;     // end-of-string.
  sprintf(FlashConfig2.Version, "%s", FIRMWARE_VERSION);   // firmware version number.



  /* Assign default reminder1's. */
  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_REMINDERS1; ++Loop1UInt16)
  {
    FlashConfig2.Reminder1[Loop1UInt16].StartPeriodUnixTime      = 0ll;
    FlashConfig2.Reminder1[Loop1UInt16].EndPeriodUnixTime        = 0ll;
    FlashConfig2.Reminder1[Loop1UInt16].RingRepeatTimeSeconds    = 0ll;
    FlashConfig2.Reminder1[Loop1UInt16].RingDurationSeconds      = 0ll;
    FlashConfig2.Reminder1[Loop1UInt16].NextReminderDelaySeconds = 0ll;
    sprintf(FlashConfig2.Reminder1[Loop1UInt16].Message, "Reminder number %u", Loop1UInt16 + 1);
  }



  /* Make provision for future parameters. */
  for (Loop1UInt16 = 0; Loop1UInt16 < sizeof(FlashConfig2.Reserved); ++Loop1UInt16)
    FlashConfig2.Reserved[Loop1UInt16] = 0xFF;



  /* Save the default configuration assigned above. */
  flash_save_config2();

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting flash_read_config2()\r");

  return 1;
}





/* $PAGE */
/* $TITLE=flash_save_config1() */
/* ============================================================================================================================================================= *\
                                                  Save current RGB Matrix main configuration 1 data to flash.
\* ============================================================================================================================================================= */
UINT8 flash_save_config1(void)
{
  UINT16 PwmLevel;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering flash_save_config1()\r");

  /* Calculate CRC16 to include it in the data being flashed. */
  FlashConfig1.Crc16 = util_crc16((UINT8 *)&FlashConfig1, ((UINT32)&FlashConfig1.Crc16 - (UINT32)&FlashConfig1.Version));

  if (DebugBitMask & DEBUG_FLASH)
  {
    uart_send(__LINE__, __func__, "=========================================================================================================\r");
    uart_send(__LINE__, __func__, "                             SAVING current RGB Matrix configuration 1 data to flash.\r");
    uart_send(__LINE__, __func__, "                             XIP_BASE: 0x%X     FLASH_CONFIG1_OFFSET: 0x%X\r", XIP_BASE, FLASH_CONFIG1_OFFSET);
    uart_send(__LINE__, __func__, "                                        sizeof(FlashConfig1): 0x%4.4X (%u)\r", sizeof(FlashConfig1), sizeof(FlashConfig1));
    uart_send(__LINE__, __func__, "                                Pointer to FlashConfig1.Version: 0x%8.8X\r", FlashConfig1.Version);
    uart_send(__LINE__, __func__, "                                Pointer to FlashConfig1.Crc16:   0x%8.8X\r", &FlashConfig1.Crc16);
    uart_send(__LINE__, __func__, "                           &FlashConfig1.Crc16 - &FlashConfig1.Version: 0x%4.4X (%u)\r", (UINT32)&FlashConfig1.Crc16 - (UINT32)&FlashConfig1.Version, (UINT32)&FlashConfig1.Crc16 - (UINT32)&FlashConfig1.Version);
    uart_send(__LINE__, __func__, "=========================================================================================================\r");

    /* Display configuration being saved. */
    flash_display_config1();
  }

  /* Validate sizeof configuration 1. */
  if (sizeof(struct flash_config1) != 0x1000)
  {
    queue_add_active(200, 5);
    printf("\r\r\r\r\r");
    uart_send(__LINE__, __func__, "*******************************************************************************************************\r\r");
    uart_send(__LINE__, __func__, "FlashConfig1 has an invalid size: 0x%4.4X\r", sizeof(struct flash_config1));
    uart_send(__LINE__, __func__, "Fix this problem and rebuild the Firmware...\r\r", sizeof(struct flash_config1));
    uart_send(__LINE__, __func__, "*******************************************************************************************************\r\r\r\r\r");
  }

  /* Blank RGB matrix LED display so that we don't see frozen display with overbright LEDs while interrupts are disabled. */
  win_open(WIN_MESSAGE, FLAG_OFF);
  /// if (DataOffset == FLASH_CONFIG1_OFFSET) win_printf(WIN_MESSAGE, 1, 99, FONT_5x7, "FLASHING 1");
  win_printf(WIN_MESSAGE, 1, 99, FONT_5x7, "FLASHING 1");
  sleep_ms(5000);

  /// RGB_matrix_cls(FrameBuffer);  /// turning Off PWM Level should be enough.
  PwmLevel = Pwm[PWM_ID_BRIGHTNESS].Level;  // keep track of original PWM level.
  pwm_set_level(PWM_ID_BRIGHTNESS, 2000);   // blank LED matrix while interrupts are disabled.

  flash_write(FLASH_CONFIG1_OFFSET, (UINT8 *)FlashConfig1.Version, sizeof(FlashConfig1));

  /* Restore original PWM level when done. */
  pwm_set_level(PWM_ID_BRIGHTNESS, PwmLevel);

  /* Re-open usual windows. */
  win_close(WIN_MESSAGE);  // will automatically restore WIN_DATE window.

  /* Display flash configuration as saved. Will crash the firmware if done inside a callback. *
  if (DebugBitMask & DEBUG_FLASH)
  {
    uart_send(__LINE__, __func__, "Display flash configuration as saved (including 64 bytes guard before and 64 bytes guard after target area):\r");
    flash_display(( - 64), (4096 + 128));
  }
  */

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting flash_save_config1())\r");

  return 0;
}





/* $PAGE */
/* $TITLE=flash_save_config2() */
/* ============================================================================================================================================================= *\
                                                  Save current RGB Matrix main configuration 2 data to flash.
\* ============================================================================================================================================================= */
UINT8 flash_save_config2(void)
{
  UINT16 PwmLevel;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering flash_save_config2()\r");

  /* Calculate CRC16 to include it in the data being flashed. */
  FlashConfig2.Crc16 = util_crc16((UINT8 *)&FlashConfig2, ((UINT32)&FlashConfig2.Crc16 - (UINT32)&FlashConfig2.Version));

  if (DebugBitMask & DEBUG_FLASH)
  {
    uart_send(__LINE__, __func__, "=========================================================================================================\r");
    uart_send(__LINE__, __func__, "                             SAVING current RGB Matrix configuration 2 data to flash.\r");
    uart_send(__LINE__, __func__, "                             XIP_BASE: 0x%X     FLASH_CONFIG2_OFFSET: 0x%X\r", XIP_BASE, FLASH_CONFIG2_OFFSET);
    uart_send(__LINE__, __func__, "                                         sizeof(FlashConfig2): 0x%4.4X (%u)\r", sizeof(FlashConfig2), sizeof(FlashConfig2));
    uart_send(__LINE__, __func__, "                                Pointer to FlashConfig2.Version: 0x%8.8X\r", FlashConfig2.Version);
    uart_send(__LINE__, __func__, "                                Pointer to FlashConfig2.Crc16:   0x%8.8X\r", &FlashConfig2.Crc16);
    uart_send(__LINE__, __func__, "                           &FlashConfig2.Crc16 - &FlashConfig2.Version: 0x%4.4X (%u)\r", (UINT32)&FlashConfig2.Crc16 - (UINT32)&FlashConfig2.Version, (UINT32)&FlashConfig2.Crc16 - (UINT32)&FlashConfig2.Version);
    uart_send(__LINE__, __func__, "=========================================================================================================\r");

    /* Validate sizeof configuration 2. */
    if (sizeof(struct flash_config2) != 0x1000)
    {
      queue_add_active(200, 5);
      printf("\r\r\r\r\r");
      uart_send(__LINE__, __func__, "*******************************************************************************************************\r\r");
      uart_send(__LINE__, __func__, "                      FlashConfig2 has an invalid size: 0x%4.4X\r", sizeof(struct flash_config2));
      uart_send(__LINE__, __func__, "                      Fix this problem and rebuild the firmware...\r\r");
      uart_send(__LINE__, __func__, "*******************************************************************************************************\r\r\r\r\r");
    }

    /* Display configuration being saved. */
    flash_display_config2();
  }

  /* Blank RGB matrix LED display so that we don't see frozen display with overbright LEDs while interrupts are disabled. */
  win_open(WIN_MESSAGE, FLAG_OFF);
  /// if (DataOffset == FLASH_CONFIG1_OFFSET) win_printf(WIN_MESSAGE, 1, 99, FONT_5x7, "FLASHING 2");
  win_printf(WIN_MESSAGE, 1, 99, FONT_5x7, "FLASHING 2");
  sleep_ms(5000);
  /// RGB_matrix_cls(FrameBuffer);  /// turning Off PWM Level should be enough.

  PwmLevel = Pwm[PWM_ID_BRIGHTNESS].Level;  // keep track of original PWM level.
  pwm_set_level(PWM_ID_BRIGHTNESS, 2000);   // blank LED matrix while interrupts are disabled.

  flash_write(FLASH_CONFIG2_OFFSET, (UINT8 *)FlashConfig2.Version, sizeof(FlashConfig2));

  /* Restore original PWM level when done. */
  pwm_set_level(PWM_ID_BRIGHTNESS, PwmLevel);

  /* Re-open usual windows. */
  win_close(WIN_MESSAGE);  // will automatically restore WIN_DATE window.

  /* Display flash configuration 2 as saved. NOTE: Will crash the firmware if done in a callback context. *
  if (DebugBitMask & DEBUG_FLASH)
  {
    uart_send(__LINE__, __func__, "Display flash configuration 2 as saved (including 64 bytes guard before and 64 bytes guard after target area):\r");
    flash_display((FLASH_CONFIG2_OFFSET - 64), (4096 + 128));
  }
  */

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting flash_save_config1()\r");

  return 0;
}





/* $PAGE */
/* $TITLE=flash_write() */
/* ============================================================================================================================================================= *\
                                               Write RGB Matrix configuration data to Pico's flash memory.
\* ============================================================================================================================================================= */
UINT flash_write(UINT32 DataOffset, UINT8 NewData[], UINT16 NewDataSize)
{
  UCHAR String[256];

  UINT8  CurrentDutyCycle;
  UINT8 *FlashBaseAddress;
  UINT8  OriginalClockMode;

  UINT16 Loop1UInt16;
  UINT16 PwmLevel;

  UINT32 InterruptMask;


  if (DebugBitMask & DEBUG_FLASH)
  {
    uart_send(__LINE__, __func__, "     Entering flash_write() - Data offset: 0x%X   Data size: 0x%4.4X (%u)\r", DataOffset, NewDataSize, NewDataSize);
    uart_send(__LINE__, __func__, "     Displaying data to be written to flash.\r");
    util_display_data(NewData, NewDataSize);
  }


  if (DataOffset % FLASH_SECTOR_SIZE)
  {
    /* Data offset specified is not aligned on a sector boundary. */
    if (DebugBitMask & DEBUG_FLASH)
    {
      uart_send(__LINE__, __func__, "     Data offset specified (0x%6.6X) is not aligned on a sector boundary (multiple of 0x1000)\r", DataOffset);
      uart_send(__LINE__, __func__, "     Phased out by 0x%X (%u) bytes.\r", DataOffset % FLASH_SECTOR_SIZE, DataOffset % FLASH_SECTOR_SIZE);
      uart_send(__LINE__, __func__, "     Last three digits of DataOffset (in hex) must be 0x000.\r");
    }
  }


  /* A wear leveling algorithm has not been implemented since the flash usage for configuration parameters doesn't require it.
     However, flash write should not be use for intensive data logging without adding a wear leveling algorithm. */
  FlashBaseAddress = (UINT8 *)(XIP_BASE);

  /* Take a copy of current flash content. */
  for (Loop1UInt16 = 0; Loop1UInt16 < FLASH_SECTOR_SIZE; ++Loop1UInt16)
    FlashData[Loop1UInt16] = FlashBaseAddress[DataOffset + Loop1UInt16];

  if (DebugBitMask & DEBUG_FLASH)
  {
    uart_send(__LINE__, __func__, "     FlashBaseAddress: 0x%p   Data offset: 0x%6.6X\r", FlashBaseAddress, DataOffset);
    uart_send(__LINE__, __func__, "     Displaying original data retrieved from flash\r");
    util_display_data(FlashData, FLASH_SECTOR_SIZE);
  }

  /* Overwrite the memory area that we want to save. */
  memcpy(FlashData, NewData, NewDataSize);

  if (DebugBitMask & DEBUG_FLASH)
  {
    uart_send(__LINE__, __func__, "     Data to be written to flash offset %X:\r", DataOffset);
    util_display_data(FlashData, FLASH_SECTOR_SIZE);
  }


  /* Erase flash before reprogramming. */
  flash_erase(DataOffset);

  /* Keep track of interrupt mask and disable interrupts during flash writing. */
  InterruptMask = save_and_disable_interrupts();

  /* Save data to flash memory. */
  flash_range_program(DataOffset, FlashData, FLASH_SECTOR_SIZE);

  /* Restore original interrupt mask when done. */
  restore_interrupts(InterruptMask);

  if (DebugBitMask & DEBUG_FLASH) printf("Exiting flash_write()\r");

  return 0;
}





/* $PAGE */
/* $TITLE=function_alarms() */
/* ============================================================================================================================================================= *\
                                                                    Function to display alarms info.
\* ============================================================================================================================================================= */
void function_alarms(void)
{
  UCHAR String[512];

  UINT8 FlagFirst;
  UINT8 ActiveCounter;
  UINT8 Loop1UInt8;
  UINT8 Loop2UInt8;


  /* Initialize counter on entry. */
  ActiveCounter = 0;
  String[0]     = 0x00;  // initialize as null string.

  /* Scan all alarms and display parameters of those that are On. */
  for (Loop1UInt8 = 0; Loop1UInt8 < MAX_ALARMS; ++Loop1UInt8)
  {
    /* Check if this alarm is On. */
    if (FlashConfig1.Alarm[Loop1UInt8].FlagStatus == FLAG_ON)
    {
      /* This alarm is On, display its number, along with trigger hour and minute. */
      sprintf(String, "Alarm %u On  %2.2u:%2.2u   ", Loop1UInt8 + 1, FlashConfig1.Alarm[Loop1UInt8].Hour, FlashConfig1.Alarm[Loop1UInt8].Minute);

      /* Display all target days. */
      FlagFirst = FLAG_ON;
      for (Loop2UInt8 = 0; Loop2UInt8 < 7; ++Loop2UInt8)
      {
        if (FlashConfig1.Alarm[Loop1UInt8].DayMask & (0x01 << Loop2UInt8))
        {
          if (FlagFirst)
          {
            sprintf(&String[strlen(String)], "%s", ShortDay[Loop2UInt8]);
            FlagFirst = FLAG_OFF;
          }
          else
          {
            sprintf(&String[strlen(String)], " - %s", ShortDay[Loop2UInt8]);
          }
        }
      }

      /* Display alarm message. */
      sprintf(&String[strlen(String)], "   <%s>", FlashConfig1.Alarm[Loop1UInt8].Message);

      /* Display number of message scrolls per ring. */
      sprintf(&String[strlen(String)], "   Nb of scrolls: %u", FlashConfig1.Alarm[Loop1UInt8].NumberOfScrolls);

      /* Display number of beeps per ring. */
      sprintf(&String[strlen(String)], "   Nb of beeps: %u", FlashConfig1.Alarm[Loop1UInt8].NumberOfBeeps);

      /* Display beep duration. */
      sprintf(&String[strlen(String)], "   Beep msec: %u", FlashConfig1.Alarm[Loop1UInt8].BeepMSec);

      /* Display ring repeat period. */
      sprintf(&String[strlen(String)], "   Repeat period: %u sec", FlashConfig1.Alarm[Loop1UInt8].RepeatPeriod);

      /* Display global alarm duration. */
      sprintf(&String[strlen(String)], "   Ring duration: %u sec", FlashConfig1.Alarm[Loop1UInt8].RingDuration);

      ++ActiveCounter;  // one more alarm is active.
    }
  }

  switch (ActiveCounter)
  {
    case (0):
      sprintf(&String[strlen(String)], "All alarms are Off");
    break;

    case (1):
      sprintf(&String[strlen(String)], "    There is currently 1 alarm active");
    break;

    default:
      sprintf(&String[strlen(String)], "    There are currently %u alarms active", ActiveCounter);
    break;
  }

  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, String);

  return;
}





/* $PAGE */
/* $TITLE=function_alarm_set() */
/* ============================================================================================================================================================= *\
                                                                    Function for alarms setup.
\* ============================================================================================================================================================= */
void function_alarm_set(void)
{
  if (DebugBitMask & DEBUG_FLOW) printf("Entering function_alarm_set()\r");

  uart_send(__LINE__, __func__, "Entering function_alarm_set()\r");
  uart_send(__LINE__, __func__, "To be completed\r\r\r");

  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "alarm_set() - to be completed...");

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting function_alarm_set()\r");

  return;
}





/* $PAGE */
/* $TITLE=function_auto_scroll() */
/* ============================================================================================================================================================= *\
                                                            Function to display auto-scrolls info.
\* ============================================================================================================================================================= */
void function_auto_scroll(void)
{
  UCHAR FunctionName[31];
  UCHAR String[512];

  UINT8 Loop1UInt8;
  UINT8 Loop2UInt8;

  UINT16 FunctionNumber;


  for (Loop1UInt8 = 0; Loop1UInt8 < MAX_AUTO_SCROLLS; ++Loop1UInt8)
  {
    /* Check if this auto-scroll is active. */
    if (FlashConfig1.AutoScroll[Loop1UInt8].Period != 0)
    {
      sprintf(String, "Auto-scroll %u active - Period: %u minutes   ", Loop1UInt8 + 1, FlashConfig1.AutoScroll[Loop1UInt8].Period);

      /* Scroll this part on first line of LED matrix. */
      win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "%s", String);
      String[0] = 0x00;  // reinitialize as null string.

      for (Loop2UInt8 = 0; Loop2UInt8 < MAX_ITEMS; ++Loop2UInt8)
      {
        if (FlashConfig1.AutoScroll[Loop1UInt8].FunctionId[Loop2UInt8] == 0) continue;
        FunctionNumber = get_function_number(FlashConfig1.AutoScroll[Loop1UInt8].FunctionId[Loop2UInt8], FunctionName);
        if (FunctionNumber != MAX_FUNCTIONS) sprintf(&String[strlen(String)], "Item %u - %s   ", Loop2UInt8 + 1, Function[FunctionNumber].Name);
      }

      /* Add this text to the text sent to the scroll engine above. */
      win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "%s", String);
    }
  }

  return;
}





/* $PAGE */
/* $TITLE=function_bootsel() */
/* ============================================================================================================================================================= *\
                                                       Reset Pico in bootsel mode to upload a new Formware.
\* ============================================================================================================================================================= */
void function_bootsel(void)
{
  UINT32 IdleTime;


  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "Make a long press on the <Set> button to reset the Pico in bootsel mode");

  /* Initializations. */
  IdleTime = 0l;


  uart_send(__LINE__, __func__, "Before entering while loop. IdleTime: %lu   (BUTTON_TIME_OUT * 1000): %u\r", IdleTime, (BUTTON_TIME_OUT * 1000));
  while (IdleTime < (BUTTON_TIME_OUT * 1000))
  {
    /* Local button superseeds remote control buttons. */
    if (ButtonBuffer[0] != BUTTON_NONE)
    {
      uart_send(__LINE__, __func__, "Assigning %u to IrBuffer[0]\r", ButtonBuffer[0]);
      IrBuffer[0] = ButtonBuffer[0];
    }


    switch (IrBuffer[0])
    {
      case (BUTTON_SET_LONG):
        uart_send(__LINE__, __func__, "Entering case BUTTON_LONG_SET\r");
        RGB_matrix_cls(FrameBuffer);
        reset_usb_boot(0l, 0l);
      break;

      default:
        uart_send(__LINE__, __func__, "User pressed button %s (%u). Cancel Pico bootsel...\r", ButtonName[IrBuffer[0]], IrBuffer[0]);
        IrBuffer[0]     = BUTTON_NONE;  // reset IR buffer to receive next data.
        ButtonBuffer[0] = BUTTON_NONE;
     break;

     case (BUTTON_NONE):
        /* No new remote control button received, pause and increment idle time. */
        sleep_ms(300);
        IdleTime += 300l;

        if (DebugBitMask & DEBUG_IR)
        {
          if ((IdleTime % 1000l) == 0)
            uart_send(__LINE__, __func__, "IrBuffer[0] is BUTTON_NONE... Increment current timeout delay...(%5lu VS %5u)\r", IdleTime, (BUTTON_TIME_OUT * 1000));
        }
      break;
    }
  }

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting function_bootsel()\r");

  return;
}





/* $PAGE */
/* $TITLE=function_brightness() */
/* ============================================================================================================================================================= *\
                                                    Function to display brightness and ambient light information.
\* ============================================================================================================================================================= */
void function_brightness(void)
{
  UCHAR String[256];


  if (DebugBitMask & DEBUG_FLOW) printf("Entering function_brightness()\r");  ///

  /* Scroll current setting for auto-brightness. */
  if (FlashConfig1.FlagAutoBrightness == FLAG_ON)
    sprintf(String, "Auto brightness On   ");
  else
    sprintf(String, "Auto brightness Off   ");


  /* Add current PWM level. */
  sprintf(&String[strlen(String)], "PWM Level: %u   ", Pwm[PWM_ID_BRIGHTNESS] .Level);


  /* Add instant ambient light value and hysteresis value. */
  sprintf(&String[strlen(String)], "Instant light: %u   Hysteresis: %u   ", get_light_value(), AverageAmbientLight);


  /* Add brightness settings (Low / High / Steady). */
  sprintf(&String[strlen(String)], "Brightness settings - Low: %u   High: %u   Steady: %u",
          FlashConfig1.BrightnessLoLimit, FlashConfig1.BrightnessHiLimit, FlashConfig1.BrightnessLevel);


  /* Scroll everything on first line of LED matrix. */
  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "%s", String);

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting function_brightness()\r");  ///

  return;
}





/* $PAGE */
/* $TITLE=function_brightness_set() */
/* ============================================================================================================================================================= *\
                                                                   Function to ajust brightness.
\* ============================================================================================================================================================= */
void function_brightness_set(void)
{
  if (DebugBitMask & DEBUG_FLOW) printf("Entering function_brightness_set()\r");

  uart_send(__LINE__, __func__, "Entering function_brightness_set()\r");
  uart_send(__LINE__, __func__, "To be completed\r\r\r");

  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "function_brightness_set() - to be completed.");

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting function_brightness_set()\r");

  return;
}





/* $PAGE */
/* $TITLE=function_chime_set() */
/* ============================================================================================================================================================= *\
                                                          Function to ajust hourly and half-hour chimes.
\* ============================================================================================================================================================= */
void function_chime_set(void)
{
  if (DebugBitMask & DEBUG_FLOW) printf("Entering function_chime_set()\r");

  uart_send(__LINE__, __func__, "Entering function_chime_set()\r");
  uart_send(__LINE__, __func__, "To be completed\r\r\r");

  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "function_chime_set() - to be completed.");

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting function_chime_set()\r");

  return;
}





/* $PAGE */
/* $TITLE=function_countdown_timer() */
/* ============================================================================================================================================================= *\
                                                               Function to launch countdown timer.
\* ============================================================================================================================================================= */
void function_countdown_timer(void)
{
  if (DebugBitMask & DEBUG_FLOW) printf("Entering function_countdown_time()\r");

  uart_send(__LINE__, __func__, "Entering function_countdown_timer()\r");
  uart_send(__LINE__, __func__, "To be completed\r\r\r");

  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "function_countdown_timer() - to be completed.");

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting function_countdown_timer()\r");

  return;
}





/* $PAGE */
/* $TITLE=function_countup_timer() */
/* ============================================================================================================================================================= *\
                                                                Function to launch countup timer.
\* ============================================================================================================================================================= */
void function_countup_timer(void)
{
  if (DebugBitMask & DEBUG_FLOW) printf("Entering function_countup_timer()\r");

  uart_send(__LINE__, __func__, "Entering function_countup_timer()\r");
  uart_send(__LINE__, __func__, "To be completed\r\r\r");

  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "function_countup_timer() - to be completed.");

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting function_countup_timer()\r");

  return;
}






/* $PAGE */
/* $TITLE=function_date_set() */
/* ============================================================================================================================================================= *\
                                                                  Function to ajust the date.
\* ============================================================================================================================================================= */
void function_date_set(void)
{
  if (DebugBitMask & DEBUG_FLOW) printf("Entering function_date_set()\r");


  win_printf(WIN_SETUP, 1, 99, FONT_5x7, Function[1].Name);

  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "function_date_set() - to be completed.");

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting function_date_set()\r");

  return;
}





/* $PAGE */
/* $TITLE=function_display() */
/* ============================================================================================================================================================= *\
                                                                  Display function data on external terminal.
\* ============================================================================================================================================================= */
void function_display(UINT16 FunctionNumber)
{
  if (DebugBitMask & DEBUG_FLOW) printf("Entering function_display()\r");

  if (FunctionNumber >= MAX_FUNCTIONS)
  {
    uart_send(__LINE__, __func__, "Invalid function number passed to function_display(): %u\r", FunctionNumber);
    return;
  }

  uart_send(__LINE__, __func__, "-----------------------------------------------------------------------------------------------------------------------------\r");
  uart_send(__LINE__, __func__, "Function number: %u   ID: %3.3u   Name: <%s>   Pointer: %X\r",
            FunctionNumber, Function[FunctionNumber].Id, Function[FunctionNumber].Name, Function[FunctionNumber].Pointer);
  uart_send(__LINE__, __func__, "-----------------------------------------------------------------------------------------------------------------------------\r");
  printf("\r\r\r");

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting function_display()\r");

  return;
}





/* $PAGE */
/* $TITLE=function_dst() */
/* ============================================================================================================================================================= *\
                                                           Function to display Daylight Saving Time info.
\* ============================================================================================================================================================= */
void function_dst(void)
{
  /* Scroll current settings for daylight saving time (DST) and timezone. */
  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "Daylight Saving Time country: %u   Timezone: %d", FlashConfig1.DSTCountry, FlashConfig1.Timezone);

  return;
}





/* $PAGE */
/* $TITLE=function_dst_set() */
/* ============================================================================================================================================================= *\
                                                             Function to ajust the daylight saving time.
\* ============================================================================================================================================================= */
void function_dst_set(void)
{
  UCHAR Dum1UChar[5];

  UINT8 EndColumnDST;
  UINT8 EndColumnTimezone;
  UINT8 EndRow;
  UINT8 FlagRefresh;
  UINT8 ItemNumber;
  UINT8 Loop1UInt8;
  UINT8 PreviousItemNumber;
  UINT8 StartColumnDST;
  UINT8 StartColumnTimezone;
  UINT8 StartRow;

  UINT32 IdleTime;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering function_dst_set()\r");


  /* Initializations. */
  FlagRefresh = FLAG_ON;
  IdleTime    = 0l;
  ItemNumber  = 0;  // hour is setup first.
  PreviousItemNumber  = 99;  // to force entry in switch case below.
  StartRow            = 20;
  EndRow              = 29;
  StartColumnDST      = 24;
  StartColumnTimezone = 21;
  EndColumnDST        = 41;
  EndColumnTimezone   = 43;


  uart_send(__LINE__, __func__, "Before win_cls(202)\r");
  win_cls(WIN_SETUP);
  /// win_scroll_off(WIN_SETUP);
  win_printf(WIN_SETUP, 201, 99, FONT_5x7, "%s", $SET);

  uart_send(__LINE__, __func__, "Before entering while loop. IdleTime: %lu   (BUTTON_TIME_OUT * 1000): %u\r", IdleTime, (BUTTON_TIME_OUT * 1000));
  while (IdleTime < (BUTTON_TIME_OUT * 1000))
  {
    /// uart_send(__LINE__, __func__, "IrBuffer[0]: %u   ButtonBuffer[0]: %u\r", IrBuffer[0], ButtonBuffer[0]);

    /* Blink item being adjusted. */
    if (ItemNumber != PreviousItemNumber)
    {
      PreviousItemNumber = ItemNumber;
      switch (ItemNumber)
      {
        case (0):
          /* First cycle, blink <DST Country> being adjusted. */
          uart_send(__LINE__, __func__, "Blinking DST Country\r");
          win_part_cls(WIN_SETUP, 18, 29);  // include lines containing infrared pilot.
          win_part_cls(WIN_SETUP, 202, 202);
          win_printf(WIN_SETUP, 202, 99, FONT_5x7, "%s", $COUNTRY_CODE);
          // win_printf(WIN_SETUP, 203, StartColumnDST, FONT_8x10, "%2.2u", FlashConfig1.DSTCountry);
          win_blink(WIN_SETUP, StartRow, StartColumnDST, EndRow, EndColumnDST);
        break;

        case (1):
          /* Blink <Timezone> being adjusted. */
          uart_send(__LINE__, __func__, "Blinking timezone\r");
          win_part_cls(WIN_SETUP, 18, 29);  // include lines containing infrared pilot.
          win_part_cls(WIN_SETUP, 202, 202);
          win_printf(WIN_SETUP, 202, 99, FONT_5x7, "%s", $TIMEZONE);
          // win_printf(WIN_SETUP, 203, StartColumnTimezone, FONT_8x10, "%3.3d", FlashConfig1.Timezone);
          win_blink(WIN_SETUP, StartRow, StartColumnTimezone, EndRow, EndColumnTimezone);
        break;
      }
    }


    /* Local button superseeds remote control buttons. */
    if (ButtonBuffer[0] != BUTTON_NONE)
    {
      uart_send(__LINE__, __func__, "Assigning %u to IrBuffer[0]\r", ButtonBuffer[0]);
      IrBuffer[0] = ButtonBuffer[0];
    }


    /* Check if there is a need to refresh the display. */
    if (FlagRefresh)
    {
      /* Reset IrBuffer and ButtonBuffer to receive next data. */
      IrBuffer[0]     = BUTTON_NONE;
      ButtonBuffer[0] = BUTTON_NONE;

      switch (ItemNumber)
      {
        case (0):
          /* Update what should be displayed in the blink buffer. */
          win_part_cls(WIN_SETUP, 18, 29);  // include the pilot IR line .
          /// RGB_matrix_printf(BlinkBuffer, StartRow, StartColumnDST, FONT_8x10, "%2.2u", FlashConfig1.DSTCountry);
          RGB_matrix_cls(BlinkBuffer);  // clear the blink buffer before putting next data to blink.
          RGB_matrix_printf(BlinkBuffer, StartRow, 99, FONT_8x10, "%u", FlashConfig1.DSTCountry);
        break;

        case (1):
          /* Update what should be displayed in the blink buffer. */
          win_part_cls(WIN_SETUP, 18, 29);  // include the pilot IR line .
          RGB_matrix_cls(BlinkBuffer);  // clear the blink buffer before putting next data to blink.
          if (FlashConfig1.Timezone > 0)
          {
            /// RGB_matrix_printf(BlinkBuffer, StartRow, StartColumnTimezone, FONT_8x10, "+%d", FlashConfig1.Timezone);
            RGB_matrix_printf(BlinkBuffer, StartRow, 99, FONT_8x10, "+%d", FlashConfig1.Timezone);
          }
          else
          {
            /// RGB_matrix_printf(BlinkBuffer, StartRow, StartColumnTimezone, FONT_8x10, "%d", FlashConfig1.Timezone);
            RGB_matrix_printf(BlinkBuffer, StartRow, 99, FONT_8x10, "%d", FlashConfig1.Timezone);
          }
        break;
      }
      FlagRefresh = FLAG_OFF;
    }


    switch (IrBuffer[0])
    {
      case (BUTTON_SET_LONG):
        win_blink_off(WIN_SETUP, 0, 0, 31, 63);
        IdleTime = (BUTTON_TIME_OUT * 1000);  // force an exit from while loop.
      break;

      case (BUTTON_SET):
        uart_send(__LINE__, __func__, "Entering case BUTTON_SET\r");
        IdleTime = 0l;              // reset idle time when receiving a new infrared data stream.
        ++ItemNumber;               // switch to next item to be adjusted: hour / minute / second
        if (ItemNumber == 2) ItemNumber = 0;  // revert to country code when we're done with timezone.
        FlagRefresh = FLAG_ON;
        while ((Window[WIN_SETUP].BlinkOnTimer == 0l) || ((time_us_32() - Window[WIN_SETUP].BlinkOnTimer) > 900000));  // make sure we change between Hour / Minute / Second only when current digit is turned On.
      break;

      case (IR_0):
      case (IR_1):
      case (IR_2):
      case (IR_3):
      case (IR_4):
      case (IR_5):
      case (IR_6):
      case (IR_7):
      case (IR_8):
      case (IR_9):
        uart_send(__LINE__, __func__, "Entering numeric digit from remote control.\r");
        IdleTime = 0l;  // reset idle time when receiving a new infrared data stream.

        switch (ItemNumber)
        {
          case (0):
            /* Initialize Dum1UChar with digits of current DST Country. */
            sprintf(Dum1UChar, "%2.2u", FlashConfig1.DSTCountry);

            /* Slide digits one position to the left and make digit just received the last digit of item being entered. */
            Dum1UChar[0] = Dum1UChar[1];
            Dum1UChar[1] = IrBuffer[0] + 36;  // convert IR code received to ASCII digit.

            FlashConfig1.DSTCountry = atoi(Dum1UChar);
          break;

          case (1):
            /* Initialize Dum1UChar with digits of current Timezone. */
            sprintf(Dum1UChar, "%3.2d", FlashConfig1.Timezone);

            /* Slide digits one position to the left and make digit just received the last digit of item being entered. */
            Dum1UChar[0] = Dum1UChar[1];
            Dum1UChar[1] = IrBuffer[0] + 36;  // convert IR code received to ASCII digit.

            FlashConfig1.Timezone = atoi(Dum1UChar);
          break;
        }

        uart_send(__LINE__, __func__, "Current status of item digits: %u %u %u %u %u\r", Dum1UChar[0], Dum1UChar[1], Dum1UChar[2], Dum1UChar[3], Dum1UChar[4]);
        uart_send(__LINE__, __func__, "Received digit IR%u\r", (IrBuffer[0]));
        uart_send(__LINE__, __func__, "Received one more digit: %u  (0x%2.2X)   Item: %u\r", IrBuffer[0], IrBuffer[0], atoi(Dum1UChar));

        FlagRefresh = FLAG_ON;
      break;

      case (BUTTON_UP):
        /* User pressed the <Up> button ("CH+"). */
        IdleTime = 0l;  // reset idle time when receiving a new infrared data stream.

        uart_send(__LINE__, __func__, "User pressed <BUTTON_UP>\r");
        if (ItemNumber == 0)
        {
          ++FlashConfig1.DSTCountry;
          if (FlashConfig1.DSTCountry >= DST_HI_LIMIT) FlashConfig1.DSTCountry = DST_NONE;
          uart_send(__LINE__, __func__, "Change DST Country to %2.2u\r", FlashConfig1.DSTCountry);
        }

        if (ItemNumber == 1)
        {
          ++FlashConfig1.Timezone;
          if (FlashConfig1.Timezone > 14) FlashConfig1.Timezone = -12;
          uart_send(__LINE__, __func__, "Change timezone to %3d\r", FlashConfig1.Timezone);
        }
        FlagRefresh = FLAG_ON;
      break;

      case (BUTTON_DOWN):
        /* User pressed <Down> button ("CH-"). */
        IdleTime = 0l;  // reset idle time when receiving a new infrared data stream.

        uart_send(__LINE__, __func__, "User pressed <BUTTON_DOWN>\r");
        if (ItemNumber == 0)
        {
          --FlashConfig1.DSTCountry;
          /* If we go under zero, revert to highest value. */
          if (FlashConfig1.DSTCountry > DST_HI_LIMIT) FlashConfig1.DSTCountry = DST_HI_LIMIT - 1;
          uart_send(__LINE__, __func__, "Change DST Country to %2.2u\r", FlashConfig1.DSTCountry);
        }

        if (ItemNumber == 1)
        {
          --FlashConfig1.Timezone;
          if (FlashConfig1.Timezone < -12) FlashConfig1.Timezone = 14;
          uart_send(__LINE__, __func__, "Change timezone to %3.2d\r", FlashConfig1.Timezone);
        }
        FlagRefresh = FLAG_ON;
      break;

      default:
      case (BUTTON_NONE):
        /* No new remote control button received, pause and increment idle time. */
        sleep_ms(300);
        IdleTime += 300l;

        if (DebugBitMask & DEBUG_IR)
        {
          if ((IdleTime % 1000l) == 0)
            uart_send(__LINE__, __func__, "IrBuffer[0] is BUTTON_NONE... Increment current timeout delay...(%5lu VS %5u)\r", IdleTime, (BUTTON_TIME_OUT * 1000));
        }
      break;
    }
  }

  win_close(WIN_SETUP);

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting function_dst_set())\r");

  return;
}





/* $PAGE */
/* $TITLE=function_event_set() */
/* ============================================================================================================================================================= *\
                                                               Function for calendar events setup.
\* ============================================================================================================================================================= */
void function_event_set(void)
{
  if (DebugBitMask & DEBUG_FLOW) printf("Entering function_event_set()\r");

  uart_send(__LINE__, __func__, "Entering function_event_set()\r");
  uart_send(__LINE__, __func__, "To be completed\r\r\r");

  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "function_event_set() - to be completed.");

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting function_event_set()\r");

  return;
}





/* $PAGE */
/* $TITLE=function_events_day() */
/* ============================================================================================================================================================= *\
                                                         Function to display calendar events of the day.
\* ============================================================================================================================================================= */
void function_events_day(void)
{
  UCHAR String[65];

  UINT16 EventCounter16;
  UINT16 Loop1UInt16;


  EventCounter16 = 0;
  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_EVENTS; ++Loop1UInt16)
  {
    String[0] = 0x00;  // initialize as null string.
    if (FlashConfig1.Event[Loop1UInt16].Day   != CurrentTime.DayOfMonth) continue;
    if (FlashConfig1.Event[Loop1UInt16].Month != CurrentTime.Month)  continue;
    ++EventCounter16;
    sprintf(String, "%2.2u-%s: %s   ", FlashConfig1.Event[Loop1UInt16].Day, ShortMonth[FlashConfig1.Event[Loop1UInt16].Month], FlashConfig1.Event[Loop1UInt16].Jingle, FlashConfig1.Event[Loop1UInt16].Message);
    win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "%s", String);
  }

  switch (EventCounter16)
  {
    case (0):
      sprintf(String, "No event defined for today");
    break;

    case (1):
      sprintf(String, "1 event defined for today");
    break;

    default:
      sprintf(String, "%u events defined for today", EventCounter16);
    break;
  }
  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "%s", String);

  return;
}





/* $PAGE */
/* $TITLE=function_events_month() */
/* ============================================================================================================================================================= *\
                                                         Function to display calendar events of the month.
\* ============================================================================================================================================================= */
void function_events_month(void)
{
 UCHAR String[65];

  UINT16 EventCounter16;
  UINT16 Loop1UInt16;


  EventCounter16 = 0;
  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_EVENTS; ++Loop1UInt16)
  {
    String[0] = 0x00;  // initialize as null string.
    if (FlashConfig1.Event[Loop1UInt16].Month != CurrentTime.Month)  continue;
    ++EventCounter16;
    sprintf(String, "%2.2u-%s: %s   ", FlashConfig1.Event[Loop1UInt16].Day, ShortMonth[FlashConfig1.Event[Loop1UInt16].Month], FlashConfig1.Event[Loop1UInt16].Message);
    win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "%s", String);
  }

  switch (EventCounter16)
  {
    case (0):
      sprintf(String, "No event defined for this month");
    break;

    case (1):
      sprintf(String, "1 event defined for this month");
    break;

    default:
      sprintf(String, "%u events defined for this month", EventCounter16);
    break;
  }
  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "%s", String);

  return;
}





/* $PAGE */
/* $TITLE=function_events_week() */
/* ============================================================================================================================================================= *\
                                                         Function to display calendar events of the week.
\* ============================================================================================================================================================= */
void function_events_week(void)
{
  UCHAR String[128];

  UINT16 EventCounter16;
  UINT16 Loop1UInt16;
  UINT16 Loop2UInt16;

  struct human_time HumanTime;


  /* Get a working copy of current date. */
  ds3231_get_time(&HumanTime);

  /* Retrieve beginning of the week (previous Sunday). */
  while (HumanTime.DayOfWeek != SUN)
  {
    --HumanTime.DayOfMonth;  // back one day.
    if (HumanTime.DayOfMonth == 0)
    {
      /* If we passed beginning of month, revert to previous month. */
      --HumanTime.Month;
      if (HumanTime.Month == 0)
      {
        /* If we passed first day of the year, revert to previous year. */
        --HumanTime.Year;
        HumanTime.Month = 12;
      }
      /* if we revert to previous month, assign last day of this month. */
      HumanTime.DayOfMonth = get_month_days(HumanTime.Month, HumanTime.Year);
    }

    /* Now that we get back one day, get day-of-week. */
    HumanTime.DayOfWeek = get_day_of_week(HumanTime.DayOfMonth, HumanTime.Month, HumanTime.Year);

    if (DebugBitMask & DEBUG_EVENT)
      uart_send(__LINE__, __func__, "Back one day: %9s [%u] %2u-%s-%4.4u\r", DayName[HumanTime.DayOfWeek], HumanTime.DayOfWeek, HumanTime.DayOfMonth, ShortMonth[HumanTime.Month], HumanTime.Year);
  }

  /* Scroll the date of the first day of the week (Sunday) that has been found. */
  sprintf(String, "Events of week beginning %s %2.2u-%s-%u", DayName[HumanTime.DayOfWeek], HumanTime.DayOfMonth, ShortMonth[HumanTime.Month], HumanTime.Year);
  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "%s", String);



  /* And display all events for each of the next seven days, starting with this Sunday. */
  EventCounter16 = 0;
  String[0] = 0x00;  // reset as null string (shouldn't be required).

  /* Scan each day of current week. */
  for (Loop1UInt16 = 0; Loop1UInt16 < 7; ++Loop1UInt16)
  {
    if (DebugBitMask & DEBUG_EVENT) uart_send(__LINE__, __func__, "Checking date:  %s %2u-%s-%4.4u\r", DayName[HumanTime.DayOfWeek], HumanTime.DayOfMonth, ShortMonth[HumanTime.Month], HumanTime.Year);

    /* Check every calendar event to find a match with date under evaluation. */
    for (Loop2UInt16 = 0; Loop2UInt16 < MAX_EVENTS; ++Loop2UInt16)
    {
      if (DebugBitMask & DEBUG_EVENT) uart_send(__LINE__, __func__, "Checking event number %2u  %2u-%3s [%s]\r", Loop2UInt16, FlashConfig1.Event[Loop2UInt16].Day, ShortMonth[FlashConfig1.Event[Loop2UInt16].Month], FlashConfig1.Event[Loop2UInt16].Message);

      if ((FlashConfig1.Event[Loop2UInt16].Day == HumanTime.DayOfMonth) && (FlashConfig1.Event[Loop2UInt16].Month == HumanTime.Month))
      {
        if (DebugBitMask & DEBUG_EVENT) uart_send(__LINE__, __func__, "Match found !\r");
        sprintf(String, "%s %2.2u-%s  %s", DayName[HumanTime.DayOfWeek], FlashConfig1.Event[Loop2UInt16].Day, ShortMonth[FlashConfig1.Event[Loop2UInt16].Month], FlashConfig1.Event[Loop2UInt16].Message);
        ++EventCounter16;  // one more event found.
        win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "%s", String);
      }
    }
    if (DebugBitMask & DEBUG_EVENT) printf("\r\r");

    /* Check next date. */
    ++HumanTime.DayOfWeek;
    ++HumanTime.DayOfMonth;
    if (HumanTime.DayOfMonth > get_month_days(HumanTime.Month, HumanTime.Year))
    {
      HumanTime.DayOfMonth = 1;
      ++HumanTime.Month;
      if (HumanTime.Month > 12)
      {
        HumanTime.Month = 1;
        ++HumanTime.Year;
      }
    }
  }
  printf("\r");

  switch (EventCounter16)
  {
    case (0):
      sprintf(String, "No event defined for this week");
    break;

    case (1):
      sprintf(String, "1 event defined for this week");
    break;

    default:
      sprintf(String, "%u events defined for this week\r", EventCounter16);
    break;
  }

  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "%s", String);

  return;
}





/* $PAGE */
/* $TITLE=function_events_year() */
/* ============================================================================================================================================================= *\
                                                         Function to display all defined calendar events.
\* ============================================================================================================================================================= */
void function_events_year(void)
{
 UCHAR String[65];

  UINT16 EventCounter16;
  UINT16 Loop1UInt16;


  EventCounter16 = 0;
  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_EVENTS; ++Loop1UInt16)
  {
    if (FlashConfig1.Event[Loop1UInt16].Day == 0) continue;  // do not scroll events currently suspended and / or undefined.
    String[0] = 0x00;  // initialize as null string.
    ++EventCounter16;
    sprintf(String, "%2.2u-%s: %s   ", FlashConfig1.Event[Loop1UInt16].Day, ShortMonth[FlashConfig1.Event[Loop1UInt16].Month], FlashConfig1.Event[Loop1UInt16].Message);
    win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "%s", String);
  }

  switch (EventCounter16)
  {
    case (0):
      sprintf(String, "No event defined in the system");
    break;

    case (1):
      sprintf(String, "1 event defined in the system");
    break;

    default:
      sprintf(String, "%u events defined in the system", EventCounter16);
    break;
  }
  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "%s", String);

  return;
}





/* $PAGE */
/* $TITLE=function_firmware_version() */
/* ============================================================================================================================================================= *\
                                                               Function to scroll Firmware Version.
\* ============================================================================================================================================================= */
void function_firmware_version(void)
{
  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "%s %s", $FIRMWARE_VERSION, FIRMWARE_VERSION);

  return;
}





/* $PAGE */
/* $TITLE=function_free_heap() */
/* ============================================================================================================================================================= *\
                                                          Function to display next free heap location.
\* ============================================================================================================================================================= */
void function_free_heap(void)
{
  UINT8 *Dum1Ptr;


  /* Find first free heap memory chunk. */
  Dum1Ptr = (UINT8 *)malloc(sizeof(struct active_scroll));
  free(Dum1Ptr);
  /// uart_send(__LINE__, __func__, "First free heap memory chunk: 0x%p\r\r\r", Dum1Ptr);

  /* Scroll first free heap memory chunk pointer on WinTop window. */
  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "Free heap pointer 0x%p", Dum1Ptr);

  return;
}





/* $PAGE */
/* $TITLE=function_golden_set() */
/* ============================================================================================================================================================= *\
                                                               Function for golden age mode setup.
\* ============================================================================================================================================================= */
void function_golden_set(void)
{
  UCHAR Dum1UChar[5];

  UINT8 FlagRefresh;
  UINT8 Loop1UInt8;
  UINT8 StartColumn;
  UINT8 EndColumn;
  UINT8 StartRow;
  UINT8 EndRow;

  UINT32 IdleTime;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering function_golden_set()\r");

  /* Initializations. */
  FlagRefresh = FLAG_ON;
  IdleTime    = 0l;
  StartRow            = 20;
  EndRow              = 29;
  StartColumn         = 20;
  EndColumn           = 43;


  uart_send(__LINE__, __func__, "Before entering while loop. IdleTime: %lu   (BUTTON_TIME_OUT * 1000): %u\r", IdleTime, (BUTTON_TIME_OUT * 1000));

  while (IdleTime < (BUTTON_TIME_OUT * 1000))
  {
    win_printf(WIN_SETUP, 201, 99, FONT_5x7, "%s", $SET);
    win_printf(WIN_SETUP, 202, 99, FONT_5x7, "%s", $GOLDEN_AGE_SET);

    /* Display current setting of Golden Age option. */
    if (FlashConfig1.FlagGoldenAge)
      win_printf(WIN_SETUP, 203, 99, FONT_8x10, "ON");
    else
      win_printf(WIN_SETUP, 203, 99, FONT_8x10, "OFF");

    /* Blink item being adjusted. */
    RGB_matrix_cls(BlinkBuffer);
    win_blink(WIN_SETUP, StartRow, StartColumn, EndRow, EndColumn);


    /* Local button superseeds remote control buttons. */
    if (ButtonBuffer[0] != BUTTON_NONE)
    {
      uart_send(__LINE__, __func__, "Assigning %u to IrBuffer[0]\r", ButtonBuffer[0]);
      IrBuffer[0] = ButtonBuffer[0];
    }


    switch (IrBuffer[0])
    {
      case (BUTTON_SET):
      case (BUTTON_SET_LONG):
        uart_send(__LINE__, __func__, "Entering case BUTTON_SET\r");
        win_blink_off(WIN_SETUP, 0, 0, 31, 63);
        IdleTime = (BUTTON_TIME_OUT * 1000);  // force an exit from while loop.
        while ((Window[WIN_SETUP].BlinkOnTimer == 0l) || ((time_us_32() - Window[WIN_SETUP].BlinkOnTimer) > 900000));  // make sure we change between Hour / Minute / Second only when current digit is turned On.
      break;

      case (BUTTON_UP):
      case (BUTTON_DOWN):
        /* User pressed the <Up> or <Down> button ("CH+"). */
        IdleTime = 0l;  // reset idle time when receiving a new infrared data stream.

        /* Reset IrBuffer and ButtonBuffer to receive next data. */
        IrBuffer[0]     = BUTTON_NONE;
        ButtonBuffer[0] = BUTTON_NONE;

        uart_send(__LINE__, __func__, "User pressed <BUTTON_UP>\r");
        if (FlashConfig1.FlagGoldenAge)
          FlashConfig1.FlagGoldenAge = FLAG_OFF;
        else
          FlashConfig1.FlagGoldenAge = FLAG_ON;

        FlagRefresh = FLAG_ON;
      break;


      default:
      case (BUTTON_NONE):
        /* No new remote control button received, pause and increment idle time. */
        sleep_ms(300);
        IdleTime += 300l;

        if (DebugBitMask & DEBUG_IR)
        {
          if ((IdleTime % 1000l) == 0)
            uart_send(__LINE__, __func__, "IrBuffer[0] is BUTTON_NONE... Increment current timeout delay...(%5lu VS %5u)\r", IdleTime, (BUTTON_TIME_OUT * 1000));
        }
      break;
    }
  }

  win_close(WIN_SETUP);

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting function_golden_set()\r");

  return;
}





/* $PAGE */
/* $TITLE=function_idle_time() */
/* ============================================================================================================================================================= *\
                                                      Function to display system idle time monitor info.
\* ============================================================================================================================================================= */
void function_idle_time(void)
{
  UCHAR String[128];


  /* Scroll current setting for daylight saving time and timezone. */
  sprintf(String, "System Idle Time Monitor - to be implemented");

  /* Scroll the info on WinFunction window. */
  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "%s", String);

  return;
}





/* $PAGE */
/* $TITLE=function_init() */
/* ============================================================================================================================================================= *\
                                                                  Function to ajust the time.
\* ============================================================================================================================================================= */
void function_init(void)
{
  UINT16 CounterFunction;
  UINT16 CounterId;
  UINT16 Loop1UInt16;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering function_init()\r");

  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_FUNCTIONS; ++Loop1UInt16)
  {
    Function[Loop1UInt16].Id      = 0;
    Function[Loop1UInt16].Number  = Loop1UInt16;
    Function[Loop1UInt16].Pointer = 0x00;
    sprintf(Function[Loop1UInt16].Name, "Invalid");
  }



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                          Setup functions.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  CounterId = 1;
  CounterFunction = 0;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $TIME_SET);
  Function[CounterFunction].Pointer = function_time_set;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $DATE_SET);
  Function[CounterFunction].Pointer = function_date_set;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $ALARM_SET);
  Function[CounterFunction].Pointer = function_alarm_set;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $NETWORK_SET);
  Function[CounterFunction].Pointer = function_network_set;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $BRIGHTNESS_SET);
  Function[CounterFunction].Pointer = function_brightness_set;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $EVENT_SET);
  Function[CounterFunction].Pointer = function_event_set;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $CHIME_SET);
  Function[CounterFunction].Pointer = function_chime_set;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $DST_SET);
  Function[CounterFunction].Pointer = function_dst_set;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $GOLDEN_AGE_SET);
  Function[CounterFunction].Pointer = function_golden_set;

  CounterId = 111;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $BOOTSEL);
  Function[CounterFunction].Pointer = function_bootsel;



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                        Information functions.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  CounterId = 200;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $FIRMWARE_VERSION);
  Function[CounterFunction].Pointer = function_firmware_version;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $PICO_TYPE);
  Function[CounterFunction].Pointer = function_pico_type;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $TEMPERATURE);
  Function[CounterFunction].Pointer = function_temperature;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $BRIGHTNESS);
  Function[CounterFunction].Pointer = function_brightness;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $ALARMS);
  Function[CounterFunction].Pointer = function_alarms;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $EVENTS_DAY);
  Function[CounterFunction].Pointer = function_events_day;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $EVENTS_WEEK);
  Function[CounterFunction].Pointer = function_events_week;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $EVENTS_MONTH);
  Function[CounterFunction].Pointer = function_events_month;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $EVENTS_YEAR);
  Function[CounterFunction].Pointer = function_events_year;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $DST);
  Function[CounterFunction].Pointer = function_dst;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $NETWORK_CREDENTIALS);
  Function[CounterFunction].Pointer = function_network_credentials;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $NETWORK);
  Function[CounterFunction].Pointer = function_network_data;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $IDLE_TIME);
  Function[CounterFunction].Pointer = function_idle_time;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $SILENCE);
  Function[CounterFunction].Pointer = function_silence;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $FREE_HEAP);
  Function[CounterFunction].Pointer = function_free_heap;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $AUTO_SCROLL);
  Function[CounterFunction].Pointer = function_auto_scroll;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $UP_TIME);
  Function[CounterFunction].Pointer = function_up_time;



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                          Operation functions.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  CounterId = 400;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $POLICE);
  Function[CounterFunction].Pointer = function_police;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $INTEGRITY_CHECK);
  Function[CounterFunction].Pointer = function_integrity_check;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $COUNT_UP);
  Function[CounterFunction].Pointer = function_countup_timer;

  ++CounterId;
  ++CounterFunction;
  Function[CounterFunction].Id = CounterId;
  sprintf(Function[CounterFunction].Name, $COUNT_DOWN);
  Function[CounterFunction].Pointer = function_countdown_timer;

  FunctionHiLimit = CounterFunction + 1;

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting function_init()\r");

  return;
}





/* $PAGE */
/* $TITLE=function_integrity_check() */
/* ============================================================================================================================================================= *\
                                                      Function to sequentially check all LEDs of RGB Matrix.
\* ============================================================================================================================================================= */
void function_integrity_check(void)
{
  RGB_matrix_integrity_check(FLAG_OFF);

  return;
}





/* $PAGE */
/* $TITLE=function_network_credentials() */
/* ============================================================================================================================================================= *\
                                                          Function to display network flashed credentials.
                                                            (extra network information is also scrolled)
\* ============================================================================================================================================================= */
void function_network_credentials(void)
{
  UCHAR String[128];


#ifndef NTP_SUPPORT
  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "Network not supported in this version of Firmware");
#else  // NTP_SUPPORT
  /* Scroll current network credentials (SSID and password) on WinFunction window. */
  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "Network credentials - SSID: <%s>   Password: <%s>", FlashConfig1.SSID, FlashConfig1.Password);
#endif  // NTP_SUPPORT

  return;
}





/* $PAGE */
/* $TITLE=function_network_data() */
/* ============================================================================================================================================================= *\
                                                                  Function to display network info.
\* ============================================================================================================================================================= */
void function_network_data(void)
{
  UCHAR String[128];


#ifndef NTP_SUPPORT
  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "Network not supported in this version of Firmware");
#else  // NTP_SUPPORT
  /* Scroll network health status. */
  if (NTPData.FlagNTPHistory == 0x01)
    strcpy(String, "Good");
  else
    strcpy(String, "Problem");

  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "Network health: %s - %lu - %lu - %lu", String, NTPData.NTPErrors, NTPData.NTPReadCycles, NTPData.NTPPollCycles);
#endif  // NTP_SUPPORT

  return;
}





/* $PAGE */
/* $TITLE=function_network_set() */
/* ============================================================================================================================================================= *\
                                                                     Function for network setup.
\* ============================================================================================================================================================= */
void function_network_set(void)
{
  if (DebugBitMask & DEBUG_FLOW) printf("Entering function_network_set()\r");

#ifndef NTP_SUPPORT
  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "Network not supported in this version of Firmware");
#else  // NTP_SUPPORT
  uart_send(__LINE__, __func__, "Entering function_network_set()\r");
  uart_send(__LINE__, __func__, "To be completed\r\r\r");

  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "function_network_set() - to be completed.");
#endif  // NTP_SUPPORT
  if (DebugBitMask & DEBUG_FLOW) printf("Exiting function_network_set()\r");

  return;
}





/* $PAGE */
/* $TITLE=function_pico_type() */
/* ============================================================================================================================================================= *\
                                                         Function to scroll the type of microcontroller.
\* ============================================================================================================================================================= */
void function_pico_type(void)
{
  UCHAR String[128];


  if (DebugBitMask & DEBUG_FLOW) uart_send(__LINE__, __func__, "Entering function_pico_type()...\r");  ///

  if (get_pico_type() == TYPE_PICO)
    sprintf(String, "%s Pico", $PICO_TYPE);
  else
    sprintf(String, "%s PicoW", $PICO_TYPE);

  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "%s   Pico Unique ID:  %s", String, PicoUniqueId);

  if (DebugBitMask & DEBUG_FLOW) uart_send(__LINE__, __func__, "Exiting function_pico_type()...\r");  ///

  return;
}





/* $PAGE */
/* $TITLE=function_police() */
/* ============================================================================================================================================================= *\
                                                              Function to simulate police lights.
\* ============================================================================================================================================================= */
void function_police(void)
{
  UINT8 Flashes;
  UINT8 FlipFlop;
  UINT8 Loop1UInt8;
  UINT8 Loop2UInt8;

  UINT32 StartTimer;


  win_open(WIN_TEST, FLAG_OFF);
  win_cls(WIN_TEST);

  FlipFlop   = 0;
  StartTimer = time_us_32();

  RGB_matrix_set_color( 0,  0, 15, 63,  RED);  // top half.
  RGB_matrix_set_color(16,  0, 31, 63, BLUE);  // bottom half.


  while (1)
  {
    /* Randomize a number of flashes between 1 and 3. */
    /// Flashes = ((rand() % 3) + 1);
    Flashes = 2;

    /* Alternate between red and blue. */
    if (FlipFlop)
    {
      /* Strobe the top half matrix. */
      FlipFlop = 0;

      for (Loop2UInt8 = 0; Loop2UInt8 < (MAX_ROWS / 2); ++Loop2UInt8)
      {
        FrameBuffer[Loop2UInt8] = 0xFFFFFFFFFFFFFFFFll;
        FrameBuffer[(MAX_ROWS / 2) + Loop2UInt8] = 0x00ll;
      }
    }
    else
    {
      /* Strobe the bottom half matrix. */
      FlipFlop = 1;

      for (Loop2UInt8 = 0; Loop2UInt8 < (MAX_ROWS / 2); ++Loop2UInt8)
      {
        FrameBuffer[Loop2UInt8] = 0x00ll;
        FrameBuffer[(MAX_ROWS / 2) + Loop2UInt8] = 0xFFFFFFFFFFFFFFFFll;
      }
    }

    for (Loop1UInt8 = 0; Loop1UInt8 < Flashes; ++Loop1UInt8)
    {
      pwm_set_level(PWM_ID_BRIGHTNESS, 1300);
      sleep_ms(120);
      pwm_set_level(PWM_ID_BRIGHTNESS, 2000);
      sleep_ms(120);
    }

    if (time_us_32() > (StartTimer + (3 * 60 * 1000000))) break;  // stop after 3 minutes.
    if (IrBuffer[0] != BUTTON_NONE) break;  // stop if user pressed a remote control button.
    sleep_ms(200);
  }

  win_close(WIN_TEST);

  return;
}





/* $PAGE */
/* $TITLE=function_search() */
/* ============================================================================================================================================================= *\
                                                    Find the function number correspondint to this function ID.
\* ============================================================================================================================================================= */
UINT16 function_search(UINT16 FunctionId)
{
  UINT16 FunctionNumber;
  UINT16 Loop1UInt16;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering function_search()\r");

  for(Loop1UInt16 = 0; Loop1UInt16 < MAX_FUNCTIONS; ++Loop1UInt16)
  {
    if (Function[Loop1UInt16].Id == FunctionId)
    {
      /* Function ID has been found, return function number. */
      return Loop1UInt16;
    }
  }

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting function_search()\r");

  /* Function ID has not been found, return MAX_FUNCTIONS as an error message. */
  return MAX_FUNCTIONS;
}





/* $PAGE */
/* $TITLE=function_silence() */
/* ============================================================================================================================================================= *\
                                                            Function to display silence period info.
\* ============================================================================================================================================================= */
void function_silence(void)
{
  UCHAR String[128];


  /* Scroll current setting for daylight saving time and timezone. */
  sprintf(String, "Silence period - to be implemented");

  /* Scroll the info on WinFunction window. */
  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "%s", String);

  return;
}





/* $PAGE */
/* $TITLE=function_temperature() */
/* ============================================================================================================================================================= *\
                                                      Function to display DS3231 & Pico internal temperature.
\* ============================================================================================================================================================= */
void function_temperature(void)
{
  UCHAR String[128];

  float DegreeC;
  float DegreeF;
  float Temperature;


  if (DebugBitMask & DEBUG_FLOW) uart_send(__LINE__, __func__, "Entering function_temperature()...\r");

  if (FlashConfig1.TemperatureUnit == CELSIUS)
  {
    get_pico_temp(&DegreeC, &DegreeF);
    sprintf(String, "Pico temp: %2.2f    ", DegreeC);

    ds3231_get_temperature(&DegreeC, &DegreeF);
    sprintf(&String[strlen(String)], "DS3231 temp: %2.2f", DegreeC);
  }
  else
  {
    get_pico_temp(&DegreeC, &DegreeF);
    sprintf(String, "Pico temp: %2.2f    ", DegreeF);

    ds3231_get_temperature(&DegreeC, &DegreeF);
    sprintf(&String[strlen(String)], "DS3231 temp: %2.2f", DegreeF);
  }

  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "%s", String);

  if (DebugBitMask & DEBUG_FLOW) uart_send(__LINE__, __func__, "Exiting function_temperature()...\r");

  return;
}





/* $PAGE */
/* $TITLE=function_time_set() */
/* ============================================================================================================================================================= *\
                                                                  Function to ajust the time.
\* ============================================================================================================================================================= */
void function_time_set(void)
{
  UCHAR Dum1UChar[5];

  UINT8 EndColumnHour;
  UINT8 EndColumnMinute;
  UINT8 EndColumnSecond;
  UINT8 FlagRefresh;
  UINT8 ItemNumber;
  UINT8 Loop1UInt8;
  UINT8 PreviousItemNumber;
  UINT8 StartColumnHour;
  UINT8 StartColumnMinute;
  UINT8 StartColumnSecond;
  UINT8 StartRow;

  UINT32 IdleTime;

  struct human_time HumanTime;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering function_time_set\r");

  /* Initializations. */
  FlagRefresh = FLAG_ON;
  IdleTime    = 0l;
  ItemNumber  = 0;  // hour is setup first.
  PreviousItemNumber = 99;  // to force entry in switch case below.
  StartRow           = 20;
  StartColumnHour    =  5;
  StartColumnMinute  = 24;
  StartColumnSecond  = 43;
  EndColumnHour      = 19;
  EndColumnMinute    = 38;
  EndColumnSecond    = 57;

  /* Get a working copy of current time. */
  ds3231_get_time(&HumanTime);


  /* Wipe line 2 before displaying current time. */
  uart_send(__LINE__, __func__, "Before win_part_cls(202)\r");
  win_cls(WIN_SETUP);
  /// win_scroll_off(WIN_SETUP);
  win_printf(WIN_SETUP, 201, 99, FONT_5x7, "%s", $SET);


  /* Display current time on entry. */
  uart_send(__LINE__, __func__, "Before printing current time\r");
  win_printf(WIN_SETUP, 20, 99, FONT_8x10, "%2.2u:%2.2u:%2.2u", HumanTime.Hour, HumanTime.Minute, HumanTime.Second);


  uart_send(__LINE__, __func__, "Before entering while loop. IdleTime: %lu   (BUTTON_TIME_OUT * 1000): %u\r", IdleTime, (BUTTON_TIME_OUT * 1000));
  while (IdleTime < (BUTTON_TIME_OUT * 1000))
  {
    /// uart_send(__LINE__, __func__, "IrBuffer[0]: %u   ButtonBuffer[0]: %u\r", IrBuffer[0], ButtonBuffer[0]);

    /* Blink item being adjusted. */
    if (ItemNumber != PreviousItemNumber)
    {
      PreviousItemNumber = ItemNumber;
      switch (ItemNumber)
      {
        case (0):
          /* First cycle, blink <Hour> being adjusted. */
          uart_send(__LINE__, __func__, "Blinking hour\r");
          win_part_cls(WIN_SETUP, 202, 202);
          win_printf(WIN_SETUP, 202, 99, FONT_5x7, "%s", $HOUR);
          win_blink_off(WIN_SETUP, 20, StartColumnSecond, 29, EndColumnSecond);  // make sure we don't blink seconds if we start over.
          win_blink(WIN_SETUP, 20, StartColumnHour, 29, EndColumnHour);
        break;

        case (1):
          /* Blink <Minute> being adjusted. */
          uart_send(__LINE__, __func__, "Blinking minutes\r");
          win_part_cls(WIN_SETUP, 202, 202);
          win_printf(WIN_SETUP, 202, 99, FONT_5x7, "%s", $MINUTE);
          win_blink_off(WIN_SETUP, 20, StartColumnHour, 29, EndColumnHour);
          win_blink(WIN_SETUP, 20, StartColumnMinute, 29, EndColumnMinute);
        break;

        case (2):
          /* Blink <Seconds> being adjusted. */
          uart_send(__LINE__, __func__, "Blinking seconds\r");
          win_part_cls(WIN_SETUP, 202, 202);
          win_printf(WIN_SETUP, 202, 99, FONT_5x7, "%s", $SECOND);
          win_blink_off(WIN_SETUP, 20, StartColumnMinute, 29, EndColumnMinute);
          win_blink(WIN_SETUP, 20, StartColumnSecond, 29, EndColumnSecond);
        break;
      }
    }


    /* Local button superseeds remote control buttons. */
    if (ButtonBuffer[0] != BUTTON_NONE)
    {
      uart_send(__LINE__, __func__, "Assigning %u to IrBuffer[0]\r", ButtonBuffer[0]);
      IrBuffer[0] = ButtonBuffer[0];
    }


    /* Check if there is a need to refresh the display. */
    if (FlagRefresh)
    {
      /* Reset IrBuffer and ButtonBuffer to receive next data. */
      IrBuffer[0]     = BUTTON_NONE;
      ButtonBuffer[0] = BUTTON_NONE;

      switch (ItemNumber)
      {
        case (0):
          /* Update what should be displayed in the blink buffer. */
          RGB_matrix_printf(BlinkBuffer, StartRow, StartColumnHour, FONT_8x10, "%2.2u", HumanTime.Hour);
        break;

        case (1):
          /* Update what should be displayed in the blink buffer. */
          RGB_matrix_printf(BlinkBuffer, StartRow, StartColumnMinute, FONT_8x10, "%2.2u", HumanTime.Minute);
        break;

        case (2):
          /* Update what should be displayed in the blink buffer. */
          RGB_matrix_printf(BlinkBuffer, StartRow, StartColumnSecond, FONT_8x10, "%2.2u", HumanTime.Second);
        break;
      }
      FlagRefresh = FLAG_OFF;
    }


    switch (IrBuffer[0])
    {
      case (BUTTON_SET_LONG):
        win_blink_off(WIN_SETUP, 0, 0, 31, 63);
        ds3231_set_time(&HumanTime);
        IdleTime = (BUTTON_TIME_OUT * 1000);  // force an exit from while loop.
      break;

      case (BUTTON_SET):
        uart_send(__LINE__, __func__, "Entering case BUTTON_SET\r");
        IdleTime = 0l;              // reset idle time when receiving a new infrared data stream.
        ++ItemNumber;               // switch to next item to be adjusted: hour / minute / second
        if (ItemNumber == 3) ItemNumber = 0;  // revert to hours when we're done with seconds.
        FlagRefresh = FLAG_ON;
        while ((Window[WIN_SETUP].BlinkOnTimer == 0l) || ((time_us_32() - Window[WIN_SETUP].BlinkOnTimer) > 900000));  // make sure we change between Hour / Minute / Second only when current digit is turned On.
      break;

      case (IR_0):
      case (IR_1):
      case (IR_2):
      case (IR_3):
      case (IR_4):
      case (IR_5):
      case (IR_6):
      case (IR_7):
      case (IR_8):
      case (IR_9):
        uart_send(__LINE__, __func__, "Entering numeric digit from remote control.\r");
        IdleTime = 0l;  // reset idle time when receiving a new infrared data stream.

        switch (ItemNumber)
        {
          case (0):
            /* Initialize Dum1UChar with digits of current hour. */
            sprintf(Dum1UChar, "%2.2u", HumanTime.Hour);

            /* Slide digits one position to the left and make digit just received the last digit of item being entered. */
            Dum1UChar[0] = Dum1UChar[1];
            Dum1UChar[1] = IrBuffer[0] + 36;  // convert IR code received to ASCII digit.

            HumanTime.Hour = atoi(Dum1UChar);
          break;

          case (1):
            /* Initialize Dum1UChar with digits of current minutes. */
            sprintf(Dum1UChar, "%2.2u", HumanTime.Minute);

            /* Slide digits one position to the left and make digit just received the last digit of minutes. */
            Dum1UChar[0] = Dum1UChar[1];
            Dum1UChar[1] = IrBuffer[0] + 36;  // convert IR code received to ASCII digit.

            HumanTime.Minute = atoi(Dum1UChar);
          break;

          case (2):
            /* Initialize Dum1UChar with digits of current seconds. */
            sprintf(Dum1UChar, "%2.2u", HumanTime.Second);

            /* Slide digits one position to the left and make digit just received the last digit of seconds. */
            Dum1UChar[0] = Dum1UChar[1];
            Dum1UChar[1] = IrBuffer[0] + 36;  // convert IR code received to ASCII digit.

            HumanTime.Second = atoi(Dum1UChar);
          break;
        }

        uart_send(__LINE__, __func__, "Current status of item digits: %u %u %u %u %u\r", Dum1UChar[0], Dum1UChar[1], Dum1UChar[2], Dum1UChar[3], Dum1UChar[4]);
        uart_send(__LINE__, __func__, "Received digit IR%u\r", (IrBuffer[0]));
        uart_send(__LINE__, __func__, "Received one more digit: %u  (0x%2.2X)   Item: %u\r", IrBuffer[0], IrBuffer[0], atoi(Dum1UChar));

        FlagRefresh = FLAG_ON;
      break;

      case (BUTTON_UP):
        /* User pressed the <Up> button ("CH+"). */
        IdleTime = 0l;  // reset idle time when receiving a new infrared data stream.

        uart_send(__LINE__, __func__, "User pressed <BUTTON_UP>\r");
        if (ItemNumber == 0)
        {
          ++HumanTime.Hour;
          if (HumanTime.Hour > 23) HumanTime.Hour = 0;
          uart_send(__LINE__, __func__, "Increment hour to %u\r", HumanTime.Hour);
        }

        if (ItemNumber == 1)
        {
          ++HumanTime.Minute;
          if (HumanTime.Minute > 59) HumanTime.Minute = 0;
          uart_send(__LINE__, __func__, "Increment minute to %u\r", HumanTime.Minute);
        }

        if (ItemNumber == 2)
        {
          ++HumanTime.Second;
          if (HumanTime.Second > 59) HumanTime.Second = 0;
          uart_send(__LINE__, __func__, "Increment second to %u\r", HumanTime.Second);
        }
        FlagRefresh = FLAG_ON;
      break;

      case (BUTTON_DOWN):
        /* User pressed <Down> button ("CH-"). */
        IdleTime = 0l;  // reset idle time when receiving a new infrared data stream.

        uart_send(__LINE__, __func__, "User pressed <BUTTON_DOWN>\r");
        if (ItemNumber == 0)
        {
          --HumanTime.Hour;
          if ((HumanTime.Hour < 0) || (HumanTime.Hour > 23)) HumanTime.Hour = 23;
          uart_send(__LINE__, __func__, "Decrement hour to %u\r", HumanTime.Hour);
        }

        if (ItemNumber == 1)
        {
          --HumanTime.Minute;
          if ((HumanTime.Minute < 0) || (HumanTime.Minute > 59)) HumanTime.Minute = 59;
          uart_send(__LINE__, __func__, "Decrement minute to %u\r", HumanTime.Minute);
        }

        if (ItemNumber == 2)
        {
          --HumanTime.Second;
          if ((HumanTime.Second < 0) || (HumanTime.Second > 59)) HumanTime.Second = 59;
          uart_send(__LINE__, __func__, "Decrement second to %u\r", HumanTime.Second);
        }
        FlagRefresh = FLAG_ON;
      break;

      default:
      case (BUTTON_NONE):
        /* No new remote control button received, pause and increment idle time. */
        sleep_ms(300);
        IdleTime += 300l;

        if (DebugBitMask & DEBUG_IR)
        {
          if ((IdleTime % 1000l) == 0)
            uart_send(__LINE__, __func__, "IrBuffer[0] is BUTTON_NONE... Increment current timeout delay...(%5lu VS %5u)\r", IdleTime, (BUTTON_TIME_OUT * 1000));
        }
      break;
    }
  }

  win_close(WIN_SETUP);

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting function_time_set())\r");

  return;
}





/* $PAGE */
/* $TITLE=function_up_time() */
/* ============================================================================================================================================================= *\
                                                          Function to scroll total RGB Matrix Up time.
\* ============================================================================================================================================================= */
void function_up_time(void)
{
  UCHAR String[131];

  UINT8 Loop1UInt8;
  UINT8 Hours;
  UINT8 Minutes;
  UINT8 Seconds;

  UINT16 Days;

  UINT64 Dum1UInt64;


  /* Initializations. */
  Days    = 0;
  Hours   = 0;
  Minutes = 0;
  Seconds = 0;

  /* Get total Up time in seconds. */
  Dum1UInt64 = time_us_64();
  Dum1UInt64 /= 1000000ll;

  /// printf("Total number of seconds: %llu\r\r", Dum1UInt64);

  /* Get number of seconds. */
  Seconds = Dum1UInt64 % 60ll;  // remaining seconds
  /// printf("Seconds: %2u\r", Seconds);


  /* Get number of minutes. */
  Dum1UInt64 /= 60ll;  // total minutes.
  /// printf("Remaining minutes: %2llu\r", Dum1UInt64);
  if (Dum1UInt64)
  {
    Minutes = Dum1UInt64 % 60ll;  // remaining minutes
    /// printf("Minutes: %2u\r", Minutes);
  }


  /* Get number of hours. */
  Dum1UInt64 /= 60ll;  // total hours.
  /// printf("Remaining hours:   %2llu\r", Dum1UInt64);
  if (Dum1UInt64)
  {
    Hours = Dum1UInt64 % 24ll;  // remaining hours.
    /// printf("Hours:   %2u\r", Hours);
  }

  /* Get number of days. */
  Dum1UInt64 /= 24ll;  // total days.
  /// printf("Remaining days:    %2llu\r", Dum1UInt64);
  if (Dum1UInt64)
  {
    Days = Dum1UInt64;
    /// printf("Days:   %4u\r\r\r", Days);
  }


  /* Scroll date and time of last power-on. */
  sprintf(String, "RGB Matrix On: %2.2u-%s-%4.4u at %2.2u:%2.2u:%2.2u",
          StartTime.DayOfMonth, ShortMonth[StartTime.Month], StartTime.Year, StartTime.Hour, StartTime.Minute, StartTime.Second);
  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "%s", String);



  /* Display total up time. */
  sprintf(String, "Total Up time:");

  if (Days == 1) sprintf(&String[strlen(String)], "  %u day",  Days);
  if (Days  > 1) sprintf(&String[strlen(String)], "  %u days", Days);

  if ((Hours == 0) && (Days != 0))
    sprintf(&String[strlen(String)],"  %u hour", Hours);
  else
  {
    if (Hours == 1) sprintf(&String[strlen(String)],"  %u hour",  Hours);
    if (Hours  > 1) sprintf(&String[strlen(String)],"  %u hours", Hours);
  }

  if ((Minutes == 0) && ((Hours != 0) || (Days != 0)))
    sprintf(&String[strlen(String)],"  %u min", Minutes);
  else
    if (Minutes > 0) sprintf(&String[strlen(String)],"  %u min", Minutes);

  sprintf(&String[strlen(String)], "  %u sec", Seconds);

  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "%s", String);

  if (DebugBitMask & DEBUG_FLOW) uart_send(__LINE__, __func__, "Exiting function_up_time()...\r");

  return;
}





/* $PAGE */
/* $TITLE=get_scroll_number() */
/* ============================================================================================================================================================= *\
                                                    Get scroll number of active scroll on current active window.
\* ============================================================================================================================================================= */
UINT8 get_scroll_number(void)
{
  UINT8 Loop1UInt8;


  for (Loop1UInt8 = 0; Loop1UInt8 < MAX_ACTIVE_SCROLL; ++Loop1UInt8)
  {
    if (ActiveScroll[Loop1UInt8])
    {
      // if (DebugBitMask & DEBUG_SCROLL) uart_send(__LINE__, __func__, "Verifying ActiveScroll number %u, Owner: %u %s\r", Loop1UInt8, ActiveScroll[Loop1UInt8]->Owner, Window[ActiveScroll[Loop1UInt8]->Owner].Name);

      if (ActiveScroll[Loop1UInt8]->Owner == WinTop) return Loop1UInt8;
      if (ActiveScroll[Loop1UInt8]->Owner == WinMid) return Loop1UInt8;
      if (ActiveScroll[Loop1UInt8]->Owner == WinBot) return Loop1UInt8;
    }
    else
    {
      // if (DebugBitMask & DEBUG_SCROLL) uart_send(__LINE__, __func__, "Verifying ActiveScroll number %u, NOT DEFINED\r");
    }
  }

  // if (DebugBitMask & DEBUG_SCROLL) uart_send(__LINE__, __func__, "Returning MAX_ACTIVE_SCROLL\r");

  return MAX_ACTIVE_SCROLL;
}





/* $PAGE */
/* $TITLE=get_day_of_week() */
/* ============================================================================================================================================================= *\
                                               Return the day-of-week for the specified date. Sunday =  (...) Saturday =
\* ============================================================================================================================================================= */
UINT8 get_day_of_week(UINT8 DayOfMonth, UINT8 Month, UINT16 Year)
{
  UINT8 DayOfWeek;
  UINT8 Table[12] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};


  Year -= Month < 3;
  DayOfWeek = ((Year + (Year / 4) - (Year / 100) + (Year / 400) + Table[Month - 1] + DayOfMonth) % 7);

  return DayOfWeek;;
}





/* $PAGE */
/* $TITLE=get_day_of_year() */
/* ============================================================================================================================================================= *\
                                                          Determine the day-of-year of date given in argument.
            NOTE: We shouldn't use uart_send() in this function since the timestamp is not available when get_day_of_year() is called from ds3231_init()
\* ============================================================================================================================================================= */
UINT16 get_day_of_year(UINT8 DayOfMonth, UINT8 Month, UINT16 Year)
{
  UINT8 Loop1UInt8;
  UINT8 MonthDays;

  UINT16 TargetDayOfYear;


  /// if (DebugBitMask & DEBUG_NTP) printf("[%4u]   DayOfMonth %u   Month: %u   Year: %u\r", __LINE__, DayOfMonth, Month, Year);
  if ((Month < 1)    || (Month > 12))   return 0;
  if ((Year  < 2000) || (Year  > 2100)) Year = 2024;


  /* Initializations. */
  TargetDayOfYear = 0;

  /* Add up all complete months. */
  for (Loop1UInt8 = 1; Loop1UInt8 < Month; ++Loop1UInt8)
  {
    MonthDays = get_month_days(Loop1UInt8, Year);
    TargetDayOfYear += MonthDays;

    /// if (DebugBitMask & DEBUG_NTP)
    ///   printf("[%4u]   Adding month %2u [%3s]   Number of days: %2u   (cumulative: %3u)\r", __LINE__, Loop1UInt8, ShortMonth[Loop1UInt8], MonthDays, TargetDayOfYear);
  }

  /* Then add days of the last, partial month. */
  /// if (DebugBitMask & DEBUG_NTP)
  ///   printf("[%4u]   Final DayNumber after adding final partial month: (%u + %u) = %3u\r\r\r", __LINE__, TargetDayOfYear, DayOfMonth, TargetDayOfYear + DayOfMonth);

  TargetDayOfYear += DayOfMonth;

  return TargetDayOfYear;
}





/* $TITLE=get_function_number() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                              Get function number and function name corresponding to this function ID.
\* ============================================================================================================================================================= */
UINT16 get_function_number(UINT16 FunctionId, UCHAR *FunctionName)
{
  UINT16 FunctionNumber;
  UINT16 Loop1UInt16;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering get_function_number()\r");

  /* Look for this function ID in the list of defined functions. */
  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_FUNCTIONS; ++Loop1UInt16)
  {
    if (Function[Loop1UInt16].Id == FunctionId)
    {
      /* Return function name... */
      strcpy(FunctionName, Function[Loop1UInt16].Name);

      /* ...and function number. */
      return Function[Loop1UInt16].Number;
    }
  }

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting display_function_name()\r");

  /* Function number not found. */
  return MAX_FUNCTIONS;
}





/* $PAGE */
/* $TITLE=get_light_value() */
/* ============================================================================================================================================================= *\
                                                                  Read ambient relative light value.
\* ============================================================================================================================================================= */
UINT16 get_light_value(void)
{
  UINT16 LightValue;


  /* Notes:
     ADC 0 (gpio 26)  is for photo-resistor (ambient light detector).
     ADC 1 (gpio 27)  not used for ADC (used for active buzzer instead).
     ADC 2 (gpio 28)  not used for ADC (used for infrared sensor).
     ADC 3 (gpio 29)  is for power supply voltage.
     ADC 4 (internal) is internally connected to read Pico's temperature. */

  if (DebugBitMask & DEBUG_FLOW) printf("Entering get_light_value()\r");

  /* Select ADC input 0 (gpio 26) for reading ambient light. */
  adc_select_input(0);

  /* Read gpio 26. Reverse light value so that more light means higher value. */
  LightValue = (1 << 12) - adc_read();

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting get_light_value()\r");

  return LightValue;
}





/* $PAGE */
/* $TITLE=get_month_days() */
/* ============================================================================================================================================================= *\
                            Return the number of days of a specific month, given the specified year (to know if it is a leap year or not).
\* ============================================================================================================================================================= */
UINT8 get_month_days(UINT8 MonthNumber, UINT16 TargetYear)
{
  UINT8 NumberOfDays;


  switch (MonthNumber)
  {
    case (1):
    case (3):
    case (5):
    case (7):
    case (8):
    case (10):
    case (12):
      NumberOfDays = 31;
    break;

    case (4):
    case (6):
    case (9):
    case (11):
      NumberOfDays = 30;
    break;

    case 2:
      /* February, we must check if it is a leap year. */
      if (((TargetYear % 4 == 0) && (TargetYear % 100 != 0)) || (TargetYear % 400 == 0))
      {
        /* This is a leap year. */
        NumberOfDays = 29;
      }
      else
      {
        /* Not a leap year. */
        NumberOfDays = 28;
      }
    break;
  }

  return NumberOfDays;
}





/* $PAGE */
/* $TITLE=get_pico_temp() */
/* ============================================================================================================================================================= *\
                                                Read Pico's internal temperature from Pico's analog-to-digital gpio.
\* ============================================================================================================================================================= */
void get_pico_temp(float *DegreeC, float *DegreeF)
{
  UINT8 Dum1UInt8;

  UINT16 AdcRawValue;

  float AdcVolts;


  /* Notes:
     Pico's ADCs (Analog-to-Digital Converters): 12-bit precision to read voltage from 0 to 3.3 Volts
     12-bit ADC range: 2^12 - 1 = 0 to 4095

     ADC 0 (gpio 26)  is for photo-resistor (ambient light detector).
     ADC 1 (gpio 27)  not used for ADC (used for active buzzer instead).
     ADC 2 (gpio 28)  not used for ADC (used for infrared sensor).
     ADC 3 (gpio 29)  is for power supply voltage.
     ADC 4 (internal connection only) is internally connected to a temperature sensor. */

  /* Make Pico's internal temperature readable from ADC. */
  adc_set_temp_sensor_enabled(true);

  adc_select_input(4);


  /* Get ADC raw value. */
  AdcRawValue = adc_read();


  /* Convert ADC raw value to volts. Reference voltage (3.3 volts). */
  AdcVolts = AdcRawValue * (3.28f / 4096);

  /* Return both Celsius and Fahrenheit values. Evaluate Pico internal temperature from voltage value.
     NOTE: 27 degree C gives a voltage of 0.706 volt and the slope is 1.721mV per degree (each additional degree C reduces the voltage by 0.001721 volt (1.721 mV)). */

  /* Compute Celsius value. */
  *DegreeC = (27 - ((AdcVolts - 0.706) / 0.001721));

  /* Compute Fahrenheit value. */
  *DegreeF = (((27 - ((AdcVolts - 0.706) / 0.001721)) * 9 / 5) + 32);

  /// uart_send(__LINE__, __func__, "AdcRawValue: %u     AdcVolts: %f     Delta with 0.706: %f\r", AdcRawValue, AdcVolts, AdcVolts - 0.706);
  /// uart_send(__LINE__, __func__, "Delta / 0.001721: %f     27 - previous:  %f\r", ((AdcVolts - 0.706) / 0.001721), (27 - ((AdcVolts - 0.706) / 0.001721)));
  /// uart_send(__LINE__, __func__, "DegreeC: %f   DegreeF: %f\r\r", DegreeC, DegreeF);

  return;
}





/* $PAGE */
/* $TITLE=get_pico_type() */
/* ============================================================================================================================================================= *\
                                                  Determine if the microcontroller is a Pico or a Pico W.
\* ============================================================================================================================================================= */
UINT8 get_pico_type(void)
{
  UCHAR String[128];

  UINT16 AdcValue1;
  UINT16 AdcValue2;

  float Volts1;
  float Volts2;


  /* Notes:
     ADC 0 (gpio 26)  is for photo-resistor (ambient light detector).
     ADC 1 (gpio 27)  not used for ADC (used for active buzzer instead).
     ADC 2 (gpio 28)  not used for ADC (used for infrared sensor).
     ADC 3 (gpio 29)  is for power supply voltage.
     ADC 4 (internal) is internally connected to read Pico's temperature. */

  /* Select power supply input. */
  adc_select_input(3);

  gpio_put(PICO_LED, 1);

  /* Read ADC converter raw value. */
  AdcValue1 = adc_read();

  /* Convert raw value to voltage value. */
  Volts1 = AdcValue1 * (3.3 / (1 << 12));



  /* The important power supply value to consider is when GPIO25 is Low. */
  gpio_put(PICO_LED, 0);

  /* Read ADC converter raw value. */
  AdcValue2 = adc_read();

  /* Convert raw value to voltage value. */
  Volts2 = AdcValue2 * (3.3 / (1 << 12));

  // uart_send(__LINE__, __func__, "AdcValue1: %4u   Volts1: %4.2f   AdcValue2: %4u   Volts2: %4.2f\r", AdcValue1, Volts1, AdcValue2, Volts2);
  /* Reading example for a Pico  =         1995            1.61             2010            1.62   */
  /* Reading example for a PicoW =         2395            1.93              120            0.10   */

  /* Determine the microcontroller type based on Volts2 value. */
  if (Volts2 > 1.0)
    return TYPE_PICO;
  else
    return TYPE_PICOW;
}





/* $PAGE */
/* $TITLE=get_pico_unique_id() */
/* ============================================================================================================================================================= *\
              Retrieve Pico's Unique ID from the flash IC. It's better to call this function during initialization phase, before core 1 begins to run.
\* ============================================================================================================================================================= */
void get_pico_unique_id(void)
{
  UINT8 Loop1UInt8;

  pico_unique_board_id_t board_id;


  /* Retrieve Pico Unique Number from flash memory IC. */
  pico_get_unique_board_id(&board_id);

  /* Build the Unique ID string in hex. */
  PicoUniqueId[0] = 0x00;  // initialize as null string on entry.
  for (Loop1UInt8 = 0; Loop1UInt8 < PICO_UNIQUE_BOARD_ID_SIZE_BYTES; ++Loop1UInt8)
  {
    sprintf(&PicoUniqueId[strlen(PicoUniqueId)], "%2.2X", board_id.id[Loop1UInt8]);
    if ((Loop1UInt8 % 2) && (Loop1UInt8 != 7)) sprintf(&PicoUniqueId[strlen(PicoUniqueId)], "-");
  }

  return;
}





/* $PAGE */
/* $TITLE=input_string() */
/* ============================================================================================================================================================= *\
                                                                            Read a string from stdin.
\* ============================================================================================================================================================= */
void input_string(UCHAR *String)
{
  INT8 DataInput;

  UINT8 Loop1UInt8;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering input_string()\r");

  Loop1UInt8 = 0;
  do
  {
    DataInput = getchar_timeout_us(50000);

    switch (DataInput)
    {
      case (PICO_ERROR_TIMEOUT):
      case (0):
        continue;
      break;

      case (8):
        /* <Backspace> */
        if (Loop1UInt8 > 0)
        {
          --Loop1UInt8;
          String[Loop1UInt8] = 0x00;
          printf("%c %c", 0x08, 0x08);  // erase character under the cursor.
        }
      break;

      case (27):
        /* <ESC> */
        if (Loop1UInt8 == 0)
        {
          String[Loop1UInt8++] = (UCHAR)DataInput;
          String[Loop1UInt8++] = 0x00;
        }
        printf("\r");
      break;

      case (0x0D):
        /* <Enter> */
        if (Loop1UInt8 == 0)
        {
          String[Loop1UInt8++] = (UCHAR)DataInput;
          String[Loop1UInt8++] = 0x00;
        }
        printf("\r");
      break;

      default:
        printf("%c", (UCHAR)DataInput);
        String[Loop1UInt8] = (UCHAR)DataInput;
        // printf("Loop1UInt8: %3u   %2.2X - %c\r", Loop1UInt8, DataInput, DataInput);  /// for debugging purposes.
        ++Loop1UInt8;
      break;
    }
  } while((Loop1UInt8 < 128) && (DataInput != 0x0D));

  String[Loop1UInt8] = '\0';  // end-of-string
  /// printf("\r\r\r");

  /* Optionally display each character entered. */
  /***
  for (Loop1UInt8 = 0; Loop1UInt8 < 10; ++Loop1UInt8)
    printf("%2u:[%2.2X]   ", Loop1UInt8, String[Loop1UInt8]);
  printf("\r");
  ***/

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting input_string()\r");

  return;
}





#ifdef REMOTE_SUPPORT
/* $TITLE=ir_decode_button() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                 Function to decode an infrared data stream received using the "Car MP3" RGB-Matrix remote control.
                                                                  Carrier: (37.9 kHz)
                                                  Pulse distance of the "get ready" signal: around 13600 usec.

                                                  32 Data bits made of:
                                                        bit 0 = pulse distance around: 1140 usec.
                                                        bit 1 = pulse distance around: 2280 usec.
\* ============================================================================================================================================================= */
UINT8 ir_decode_button(UINT8 *IrButton)
{
  UCHAR String[128];

  UINT8 BitNumber;
  UINT8 FlagError;          // indicate an error in remote control infrared data stream received.

  UINT16 Loop1UInt16;
  UINT16 Loop2UInt16;

  UINT64 DataBuffer;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering ir_decode_button()\r");

  /* Initialization. */
  DataBuffer = 0ll;          // data stream received from IR remote control.
 *IrButton   = IR_LO_LIMIT;  // initialize with invalid value on entry.
  FlagError  = FLAG_OFF;     // assume no error on entry.


  /* Optionally display infrared data stream timing informations. */
  /* NOTE: Can't be used while in callback thread context or it will crash the thread. */
  /***
  if (DebugBitMask & DEBUG_IR)
  {
    uart_send(__LINE__, __func__, "Entering ir_decode_button()\r");
    printf("\r");
    printf("Total number of logic steps: %2u\r\r", IrStepCount);

    // Display debug header.
    printf("Event   Bit   Logic    Level   Logic   Level      Pulse    Resulting\r");
    printf("number number level  duration  level  duration   distance    data\r");
  }
  ***/



  /* Process infrared data stream only if we received a minimum number of steps. */
  if (IrStepCount >= 67)
  {
    /* Analyze and process each pulse distance in remote control data stream. */
    for (Loop1UInt16 = 0; Loop1UInt16 < IrStepCount; Loop1UInt16 += 2)
    {
      BitNumber = (((Loop1UInt16 - 2) / 2) + 1);

      if (Loop1UInt16 < 2)
      {
        // Display <Get ready> levels & pulse distance from IR data stream.
        // if (DebugBitMask & DEBUG_IR)
        //   printf(" %3u     --     %u     %5lu      %u     %5lu     %7lu  <get ready>\r", Loop1UInt16, IrLevel[Loop1UInt16], IrResultValue[Loop1UInt16], IrLevel[Loop1UInt16 + 1], IrResultValue[Loop1UInt16 + 1], IrPulseDistance[Loop1UInt16 + 1]);
        continue;
      }


      if ((BitNumber > 0) && (BitNumber <= 32))
      {
        DataBuffer <<= 1ll;                                         // left-shift already received bits.
        if (IrPulseDistance[Loop1UInt16 + 1] > 1700) ++DataBuffer;  // if we just received a "1" bit, put it in bit 0 position.

        // Optionally display first 32 data bits.
        // if (DebugBitMask & DEBUG_IR)
        //   printf(" %3u    %3u     %u     %5lu      %u     %5lu     %7lu   0x%8.8llX\r", Loop1UInt16, BitNumber, IrLevel[Loop1UInt16], IrResultValue[Loop1UInt16], IrLevel[Loop1UInt16 + 1], IrResultValue[Loop1UInt16 + 1], IrPulseDistance[Loop1UInt16 + 1], DataBuffer);
      }


      if (BitNumber > 32)
      {
        // Optionally display extra bits (above 32 bits).
        // if (DebugBitMask & DEBUG_IR)
        //   printf(" %3u     --     %u     %5lu      %u     %5lu     %7lu\r", Loop1UInt16, IrLevel[Loop1UInt16], IrResultValue[Loop1UInt16], IrLevel[Loop1UInt16 + 1], IrResultValue[Loop1UInt16 + 1], IrPulseDistance[Loop1UInt16 + 1]);
      }


      /* When reading a value that makes no sense, assume that we passed the last valid value of the infrared data stream. */
      if ((IrResultValue[Loop1UInt16] > 10000l) || (IrResultValue[Loop1UInt16 + 1] > 10000l))
      {
        /* We reached end of IR data stream, get out of "for" loop. */
        break;

        // if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, " Reaching end of IR data stream > 10000 at IrStep %u\r", Loop1UInt16);
      }
    }
  }
  else
  {
    DataBuffer = 0ll;  // invalid infrared data stream.
    if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "Data stream rejected: %u steps\r", IrStepCount);
  }


  // if (DebugBitMask & DEBUG_IR)
  // {
  //   uart_send(__LINE__, __func__, "Final data: 0x%8.8llX\r", DataBuffer);
  //   uart_send(__LINE__, __func__, "Final step count: %2u (should be 67)\r\r", IrStepCount);
  // }


  /* Now that the command has been decoded, initalize variables to get ready for next infrared data stream reception. */
  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_IR_READINGS; ++Loop1UInt16)
  {
    IrInitialValue[Loop1UInt16]  = 0ll;
    IrFinalValue[Loop1UInt16]    = 0ll;
    IrResultValue[Loop1UInt16]   = 0l;
    IrPulseDistance[Loop1UInt16] = 0l;
    IrLevel[Loop1UInt16]         = 9;
  }
  IrStepCount = 0;


  switch (DataBuffer)
  {
    case (0x00FFA25D):
      /* Button "Down" ("Channel-" on the remote). */
      *IrButton = BUTTON_DOWN;
      if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "IR button decoded: %u <%s>   (<Channel-> on the remote)\r", *IrButton, ButtonName[*IrButton]);
    break;

    case (0x00FF629D):
      /* Button "Set/Function" ("Channel" on the remote). */
      *IrButton = BUTTON_SET;
      if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "IR button decoded: %u <%s>   (<Channel> on the remote)\r", *IrButton, ButtonName[*IrButton]);
    break;

    case (0x00FFE21D):
      /* Button "Up" ("Channel+" on the remote). */
      *IrButton = BUTTON_UP;
      if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "IR button decoded: %u <%s>   (<Channel+> on the remote)\r", *IrButton, ButtonName[*IrButton]);
    break;

    case (0x00FF22DD):
      /* Button "Previous" on the remote. */
      *IrButton = BUTTON_DOWN_LONG;
      if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "IR button decoded: %u <%s>   (<Prev> on the remote)\r", *IrButton, ButtonName[*IrButton]);
    break;

    case (0x00FF02FD):
      /* Button "Next" on the remote. */
      *IrButton = BUTTON_SET_LONG;
      if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "IR button decoded: %u <%s>   (<Next> on the remote)\r", *IrButton, ButtonName[*IrButton]);
    break;

    case (0x00FFC23D):
      /* Button "Up-Long" ("Play/Pause" on the remote). */
      *IrButton = BUTTON_UP_LONG;
      if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "IR button decoded: %u <%s>   (<Play/Pause> on the remote)\r", *IrButton, ButtonName[*IrButton]);
    break;

    case (0x00FFE01F):
      /* Button "Minus" ("Volume-" on the remote). */
      *IrButton = IR_VOL_MINUS;
      if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "IR button decoded: %u <%s>   (<Volume-> on the remote)\r", *IrButton, ButtonName[*IrButton]);
   break;

    case (0x00FFA857):
      /* Button "Plus" ("Volume+" on the remote). */
      *IrButton = IR_VOL_PLUS;
      if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "IR button decoded: %u <%s>   (<Volume+> on the remote)\r", *IrButton, ButtonName[*IrButton]);
    break;

    case (0x00FF906F):
      /* Button "Enter" ("EQ" on the remote). */
      *IrButton = IR_EQ;
      if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "IR button decoded: %u <%s>   (<EQ> on the remote)\r", *IrButton, ButtonName[*IrButton]);
    break;

    case (0x00FF9867):
      /* Button "100+". */
      *IrButton = IR_100;
      if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "IR button decoded: %u <%s>   (<100+> on the remote)\r", *IrButton, ButtonName[*IrButton]);
    break;

    case (0x00FFB04F):
      /* Button "200+". */
      *IrButton = IR_200;
      if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "IR button decoded: %u <%s>   (<200+> on the remote)\r", *IrButton, ButtonName[*IrButton]);
   break;

    case (0x00FF6897):
      /* Button "Digit-0". */
      *IrButton = IR_0;
      if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "IR button decoded: %u <%s> \r", *IrButton, ButtonName[*IrButton]);
    break;

    case (0x00FF30CF):
      /* Button "Digit-1". */
      *IrButton = IR_1;
      if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "IR button decoded: %u <%s> \r", *IrButton, ButtonName[*IrButton]);
    break;

    case (0x00FF18E7):
      /* Button "Digit-2". */
      *IrButton = IR_2;
      if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "IR button decoded: %u <%s> \r", *IrButton, ButtonName[*IrButton]);
    break;

    case (0x00FF7A85):
      /* Button "Digit-3". */
      *IrButton = IR_3;
      if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "IR button decoded: %u <%s> \r", *IrButton, ButtonName[*IrButton]);
    break;

    case (0x00FF10EF):
      /* Button "Digit-4". */
      *IrButton = IR_4;
      if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "IR button decoded: %u <%s> \r", *IrButton, ButtonName[*IrButton]);
    break;

    case (0x00FF38C7):
      /* Button "Digit-5". */
      *IrButton = IR_5;
      if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "IR button decoded: %u <%s> \r", *IrButton, ButtonName[*IrButton]);
    break;

    case (0x00FF5AA5):
      /* Button "Digit-6". */
      *IrButton = IR_6;
      if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "IR button decoded: %u <%s> \r", *IrButton, ButtonName[*IrButton]);
    break;

    case (0x00FF42BD):
      /* Button "Digit-7". */
      *IrButton = IR_7;
      if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "IR button decoded: %u <%s> \r", *IrButton, ButtonName[*IrButton]);
    break;

    case (0x00FF4AB5):
      /* Button "Digit-8". */
      *IrButton = IR_8;
      if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "IR button decoded: %u <%s> \r", *IrButton, ButtonName[*IrButton]);
    break;

    case (0x00FF52Ad):
      /* Button "Digit-9". */
      *IrButton = IR_9;
      if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "IR button decoded: %u <%s> \r", *IrButton, ButtonName[*IrButton]);
    break;

    case (0):
      /* Probably just received a spurious IR signal. Simply discard it. */
      if (DebugBitMask & DEBUG_IR)
        uart_send(__LINE__, __func__, "Spurious IR received\r");
    default:
      /* Unrecognized. */
      if (DebugBitMask & DEBUG_IR)
        uart_send(__LINE__, __func__, "Unrecognized IR command: 0x%8.8llX\r", DataBuffer);

      /* Visual feedback of an invalid command on RGB matrix. */
      RGB_matrix_set_color(IR_INDICATOR_START_ROW, IR_INDICATOR_START_COLUMN, IR_INDICATOR_END_ROW, IR_INDICATOR_END_COLUMN, RED);
      FlagError = FLAG_ON;
      return IR_HI_LIMIT;
    break;
  }

  /* Visual feedback of a valid command on RGB matrix. */
  RGB_matrix_set_color(IR_INDICATOR_START_ROW, IR_INDICATOR_START_COLUMN, IR_INDICATOR_END_ROW, IR_INDICATOR_END_COLUMN, GREEN);

  /* Audio feedback indicating we received a valid infrared button / command. */
  if (FlashConfig1.FlagIrFeedback)
  {
    queue_add_active(50, 1);
    queue_add_active(1000, SILENT);  // isolate this sound train from any subsequent sound train.
  }

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting ir_decode_button()\r");

  return 0;
}
#endif  // REMOTE_CONTROL_SUPPORT




#ifdef REMOTE_SUPPORT
/* $PAGE */
/* $TITLE=ir_display_timings() */
/* ============================================================================================================================================================= *\
                                                                Display timings of the infrared data stream.
\* ============================================================================================================================================================= */
void ir_display_timings(void)
{
  UINT8 BitNumber;

  UINT16 Loop1UInt16;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering ir_display_timings()\r");

  /* Display timing for every logic level change of last received infrared data stream. */
  printf("\r");
  printf("Event   Logic   Initial      Final     Result     Pulse\r");
  printf("number  level    timer       timer    duration   distance\r");
  for (Loop1UInt16 = 0; Loop1UInt16 < IrStepCount; ++Loop1UInt16)
  {
    printf(" %3u      %u   %10llu  %10llu   %5lu", Loop1UInt16, IrLevel[Loop1UInt16], IrInitialValue[Loop1UInt16], IrFinalValue[Loop1UInt16], IrResultValue[Loop1UInt16]);
    if (IrPulseDistance[Loop1UInt16] != 0)
      printf("     %7lu\r", IrPulseDistance[Loop1UInt16]);
    else
      printf("\r");
  }
  printf("Total number of logic level changes (IrStepCount): %u (0 to %u)\r", IrStepCount, (IrStepCount - 1));
  printf("\r\r");

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting ir_display_timings()\r");

  return;
}
#endif  // REMOTE_SUPPORT





/* $PAGE */
/* $TITLE=pilot_set_color() */
/* ============================================================================================================================================================= *\
                                                           Set color for endless loop pixel indicators pilot.
\* ============================================================================================================================================================= */
void pilot_set_color(UINT8 Color)
{
  /* Set color for left and right pixel indicators pilot. */
  RGB_matrix_set_color(0,  0,  0,  2, Color);  // left part.
  RGB_matrix_set_color(0, 61,  0, 63, Color);  // right part.

  return;
}





/* $PAGE */
/* $TITLE=pilot_toggle() */
/* ============================================================================================================================================================= *\
                                                       Toggle endless loop pixel indicators pilot On / Off.
                                                                      Double-dot pixels:
\* ============================================================================================================================================================= */
void pilot_toggle(void)
{
  UINT8 Loop1UInt8;

  static UINT8 FlagPilot;


  if (FlagPilot)
  {
    /* "Endless loop" pilot indicators are On, turn them Off on top of RGB-Matrix line, left and right. */
    FrameBuffer[0] &= ~(0xE000000000000007);
    FlagPilot = FLAG_OFF;
  }
  else
  {
    /* "Endless loop" pilot indicators are Off, turn them On on top of RGB-Matrix line, left and right. */
    FrameBuffer[0] |= 0xE000000000000007;
    FlagPilot = FLAG_ON;
  }

  return;
}





/* $PAGE */
/* $TITLE=process_button() */
/* ============================================================================================================================================================= *\
                      Function to process a button-press received either from local RGB matrix buttons and / of from remote control buttons.
\* ============================================================================================================================================================= */
void process_button(UINT8 CommandId)
{
  UCHAR String[31];            // command buffer (just as a security... should not really be required).
  UCHAR FunctionIdString[31];  // function Id digits components.

  UINT8 Counter;          // local counter of buttons (local or remote) received so far.
  UINT8 DisplayPoint;     // pointer to next character to be displayed on RGB matrix.
  UINT8 Dum1UInt8;
  UINT8 FlagFirstPass;    // first pass in while loop.

  UINT16 Dum1UInt16;
  UINT16 FunctionId;      // function ID entered by user.
  UINT16 FunctionNumber;  // function number corresponding to the function ID entered by user.
  UINT16 Loop1UInt16;

  UINT32 IdleTime;        // current delay waiting for next button (local or remote).


  if (DebugBitMask & DEBUG_FLOW) printf("Entering process_button()\r");

  if (DebugBitMask & DEBUG_IR)
  {
    printf("\r\r");
    uart_send(__LINE__, __func__, "Button pressed: <%s>   0x%2.2X\r", ButtonName[CommandId], CommandId);
  }


  Counter       = 0;
  FlagFirstPass = FLAG_ON;
  IdleTime      = 0l;

  do
  {
    switch (CommandId)
    {
      case (BUTTON_SET):
        IdleTime = 0l;  // reset idle time when receiving a new button press (local or remote).

        IrBuffer[0]     = BUTTON_NONE;  // reset IR buffer to receive function number or "Set" again to execute current function.
        ButtonBuffer[0] = BUTTON_NONE;  // reset local button buffer to prepare for next button press.
        CommandId       = BUTTON_NONE;
        if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "Received button <Function / Set>\r");

        if (FlagFirstPass)
        {
          FlagFirstPass = FLAG_OFF;
          if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "Turning Off FlagFirstPass\r");

          /* Assign initial function number. */
          FunctionId     = 200;  // function ID 200 is guaranteed to be defined ("time set").
          FunctionNumber = get_function_number(FunctionId, String);
          sprintf(FunctionIdString, "%3.3u", FunctionId);
          if (DebugBitMask & DEBUG_FUNCTION)
          {
            function_display(FunctionNumber);
            /// uart_send(__LINE__, __func__, "Current Function number: %u     Function ID: %3.3u     Function name: <%s>     Function pointer: %p\r", Number, Function[FunctionNumber].Id, Function[FunctionNumber].Name, Function[FunctionNumber].Pointer);
          }

          win_open(WIN_FUNCTION, FLAG_OFF);  // explode WIN_FUNCTION window.

          /* Display function name of current function ID on LED matrix. */
          win_part_cls(WIN_FUNCTION, 201, 201);
          if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "Before printing function name (length = %u)\r", RGB_matrix_pixel_length(FONT_5x7, "%s", Function[FunctionNumber].Name));
          win_scroll(WIN_FUNCTION, 201, 201, 1, 1, FONT_5x7, "%s", Function[FunctionNumber].Name);

          /* Display header <F-> to prepare reception of the function number. */
          if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "Before displaying function ID\r");
          win_printf(WIN_FUNCTION, 9, 19, FONT_5x7, "F-%3.3u  ", Function[FunctionNumber].Id);
        }
        else
        {
          /* User pressed "Function / Set" to enter currently displayed function. */
          if (DebugBitMask & DEBUG_FUNCTION)
          {
            function_display(FunctionNumber);
            /// uart_send(__LINE__, __func__, "Current FunctionNumber: %u     FunctionId: %3.3u     FunctionName: <%s>     FunctionPointer: %p\r", FunctionNumber, Function[FunctionNumber].FunctionId, Function[FunctionNumber].FunctionName, Function[FunctionNumber].FunctionPointer);
          }

          /* If there is currently a scroll going on, cancel it and add a few character spaces to introduce new scrolling. */
          win_scroll_cancel(WIN_FUNCTION, 201, 201);

          /* If we're going to execute a setup function, we'll use a full-matrix window. */
          if ((Function[FunctionNumber].Id > 0) && (Function[FunctionNumber].Id < 200))
          {
            /* This is a setup function, we'll use a "full-matrix window". */
            win_close(WIN_FUNCTION);
            win_open(WIN_SETUP, FLAG_OFF);
          }

          /* Execute this function (jump to function pointer). */
          Function[FunctionNumber].Pointer();
          IdleTime = (BUTTON_TIME_OUT * 1000);  // force a time-out to get out of switch and do loop.
        }
      break;

      case (IR_0):
      case (IR_1):
      case (IR_2):
      case (IR_3):
      case (IR_4):
      case (IR_5):
      case (IR_6):
      case (IR_7):
      case (IR_8):
      case (IR_9):
        IdleTime = 0l;  // reset idle time when receiving a new infrared data stream.

        /* Slide digits one position to the left and make digit just received the last digit of function ID. */
        if (DebugBitMask & DEBUG_FUNCTION) printf("Function ID before: %u  (%s)\r", FunctionId, FunctionIdString);
        for (Loop1UInt16 = 0; Loop1UInt16 < (MAX_FUNCTION_DIGITS - 1); ++Loop1UInt16)
          FunctionIdString[Loop1UInt16] = FunctionIdString[Loop1UInt16 + 1];
        FunctionIdString[MAX_FUNCTION_DIGITS - 1] = IrBuffer[0] + 36;  // convert IR code received to ASCII digit.
        FunctionId = atoi(FunctionIdString);      // translate current value of function ID.
        if (DebugBitMask & DEBUG_IR)
        {
          uart_send(__LINE__, __func__, "Function ID after: %u  (%s)\r", FunctionId, FunctionIdString);
          uart_send(__LINE__, __func__, "Current status of function digits: %u %u %u %u %u\r", FunctionIdString[0], FunctionIdString[1], FunctionIdString[2], FunctionIdString[3], FunctionIdString[4]);
          uart_send(__LINE__, __func__, "Received digit number %u: IR%u\r", Counter, (IrBuffer[0]));
          uart_send(__LINE__, __func__, "Received one more function digit: %u  (0x%2.2X)   FunctionId: %u\r", IrBuffer[0], IrBuffer[0], FunctionId);
        }

        IrBuffer[0]     = BUTTON_NONE;  // reset IR buffer to receive next data.
        ButtonBuffer[0] = BUTTON_NONE;  // reset local button buffer to prepare for next button press.
        CommandId       = BUTTON_NONE;
        DisplayPoint = win_printf(WIN_FUNCTION, 9, 19, FONT_5x7, "F-%3.3u  ", FunctionId);

        /* Check if this FunctionId exists. */
        FunctionNumber = function_search(FunctionId);
        if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "Found next valid function number: %u\r", FunctionNumber);

        if (FunctionNumber == MAX_FUNCTIONS)
        {
          win_part_cls(WIN_FUNCTION, 201, 201);
          win_printf(WIN_FUNCTION, 1, 99, FONT_5x7, "Not found");
        }
        else
        {
          /* Display function name of current function ID on LED matrix. */
          display_function_name(FunctionId);
        }
      break;

      case (BUTTON_DOWN):
        /* User pressed the <Down> button (on remote or local button). */
        IdleTime        = 0l;           // reset idle time when receiving a new button press.
        IrBuffer[0]     = BUTTON_NONE;  // reset IR buffer.
        ButtonBuffer[0] = BUTTON_NONE;  // reset local button buffer.
        CommandId       = BUTTON_NONE;  // reset CommandId.

        if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "User pressed <BUTTON_DOWN> while choice was function number %u <%s>\r", FunctionNumber, Function[FunctionNumber].Name);
        Dum1UInt16 = FunctionNumber;  // keep track of where we are in the function stack.
        do
        {
          /* Find previous valid function number and its corresponding ID and Name. */
          if (FunctionNumber == 0) FunctionNumber = MAX_FUNCTIONS;  // revert to the end of stack when reaching zero.
          --FunctionNumber;
          if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "Checking FunctionNumber: %3u   FunctionId: %3u\r", FunctionNumber, Function[FunctionNumber].Id);
          if (FunctionNumber == Dum1UInt16) break;  // we went back to the starting point without finding a valid function ID.
        } while (Function[FunctionNumber].Id == 0);


        /* If there is currently a scroll going on, cancel it and add a few character spaces to introduce new scrolling. */
        win_scroll_cancel(WIN_FUNCTION, 201, 201);

        /* Display function ID on RGB matrix. */
        if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "FunctionId: %u   FunctionNumber: %u   Dum1UInt16: %u\r", Function[FunctionNumber].Id, FunctionNumber, Dum1UInt16);
        win_part_cls(WIN_FUNCTION, 201, 201);
        if ((Function[FunctionNumber].Id == 9999) || (FunctionNumber == Dum1UInt16))
        {
          win_printf(WIN_FUNCTION, 1, 99, FONT_5x7, "Not found");
        }
        else
        {
          /* Display function name and current function ID on LED matrix. */
          display_function_name(Function[FunctionNumber].Id);
          win_printf(WIN_FUNCTION, 9, 19, FONT_5x7, "F-%3.3u  ", Function[FunctionNumber].Id);
        }
      break;

      case (BUTTON_UP):
        /* User pressed the <Up> button (on remote or local button). */
        IdleTime        = 0l;           // reset idle time when receiving a new button press.
        IrBuffer[0]     = BUTTON_NONE;  // reset IR buffer.
        ButtonBuffer[0] = BUTTON_NONE;  // reset local button buffer.
        CommandId       = BUTTON_NONE;

        if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "User pressed <BUTTON_UP> while choice was function number %u <%s>\r", FunctionNumber, Function[FunctionNumber].Name);
        Dum1UInt16 = FunctionNumber;  // keep track of where we are in the function stack.
        do
        {
          /* Find next valid function number and its corresponding ID and Name. */
          ++FunctionNumber;
          if (FunctionNumber >= MAX_FUNCTIONS) FunctionNumber = 0;  // revert to zero when reaching out-of-bound.
          if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "Checking FunctionNumber: %3u   FunctionId: %3u\r", FunctionNumber, Function[FunctionNumber].Id);
          if (FunctionNumber == Dum1UInt16) break;  // we went back to the starting point without finding a valid function ID.
        } while (Function[FunctionNumber].Id == 9999);


        /* If there is currently a scroll going on, cancel it and add a few character spaces to introduce new scrolling. */
        win_scroll_cancel(WIN_FUNCTION, 201, 201);

        /* Display function ID on RGB matrix. */
        if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "FunctionId: %u   FunctionNumber: %u   Dum1UInt16: %u\r", Function[FunctionNumber].Id, FunctionNumber, Dum1UInt16);
        win_part_cls(WIN_FUNCTION, 201, 201);
        if ((Function[FunctionNumber].Id == 9999) || (FunctionNumber == Dum1UInt16))
        {
          win_printf(WIN_FUNCTION, 1, 99, FONT_5x7, "Not found");
        }
        else
        {
          /* Display function name and current function ID on LED matrix. */
          display_function_name(Function[FunctionNumber].Id);
          win_printf(WIN_FUNCTION, 9, 19, FONT_5x7, "F-%3.3u  ", Function[FunctionNumber].Id);
        }
      break;

      case (BUTTON_UP_LONG):
        /* User made a long-press pressed on the Up button, add one minute to service light timer. */
        if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "User pressed <BUTTON_UP_LONG_PRESS>\r");
        IrBuffer[0]     = BUTTON_NONE;  // reset IR buffer to receive next data.
        ButtonBuffer[0] = BUTTON_NONE;  // reset local button buffer to prepare for next button press.
        CommandId       = BUTTON_NONE;
        if (ServiceLightTimer < 300) ServiceLightTimer += 60;
        pwm_set_level(PWM_ID_BRIGHTNESS, 1300);  // set RGB Matrix to highest brightness.
      break;

      case (IR_EQ):
        /* Received <Enter> from remote control, execute the decoded function. */
        IdleTime = 0l;  // reset idle time when receiving a new infrared data stream.

        if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "User pressed <IR_ENTER> (EQ)\r");
        IrBuffer[0]     = BUTTON_NONE;  // reset IR buffer to receive next data.
        ButtonBuffer[0] = BUTTON_NONE;  // reset local button buffer to prepare for next button press.
        CommandId       = BUTTON_NONE;
        Function[FunctionNumber].Pointer;
      break;

      case (IR_VOL_MINUS):
        /* Received <Minus> from remote control. */
        IdleTime = 0l;  // reset idle time when receiving a new infrared data stream.

        if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "User pressed <IR_MINUS>\r");
        IrBuffer[0]     = BUTTON_NONE;  // reset IR buffer to receive next data.
        ButtonBuffer[0] = BUTTON_NONE;  // reset local button buffer to prepare for next button press.
        CommandId       = BUTTON_NONE;
      break;

      case (IR_VOL_PLUS):
        /* Received <PLUS> from remote control. */
        IdleTime = 0l;  // reset idle time when receiving a new infrared data stream.

        if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "User pressed <IR_PLUS>\r");
        IrBuffer[0]     = BUTTON_NONE;  // reset IR buffer to receive next data.
        ButtonBuffer[0] = BUTTON_NONE;  // reset local button buffer to prepare for next button press.
        CommandId       = BUTTON_NONE;
      break;

      case (IR_100):
        /* Received <100+> from remote control. */
        IdleTime = 0l;  // reset idle time when receiving a new infrared data stream.

        if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "User pressed <IR_100> (100+)\r");
        IrBuffer[0]     = BUTTON_NONE;  // reset IR buffer to receive next data.
        ButtonBuffer[0] = BUTTON_NONE;  // reset local button buffer to prepare for next button press.
        CommandId       = BUTTON_NONE;

      break;

      case (IR_200):
        /* Received <200+> from remote control. */
        IdleTime = 0l;  // reset idle time when receiving a new infrared data stream.

        if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "User pressed <IR_200+>\r");
        IrBuffer[0]     = BUTTON_NONE;  // reset IR buffer to receive next data.
        ButtonBuffer[0] = BUTTON_NONE;  // reset local button buffer to prepare for next button press.
        CommandId       = BUTTON_NONE;
      break;

      case (IR_LO_LIMIT):
        /* No new remote control button received, pause and increment idle time. */
        sleep_ms(200);
        IdleTime += 200l;

        /***/
        if (DebugBitMask & DEBUG_IR)
          if ((IdleTime % 1000l) == 0)
            uart_send(__LINE__, __func__, "IrBuffer[Counter] is still IR_LO_LIMIT... Increment delay...(%5lu VS %5u)\r", IdleTime, (BUTTON_TIME_OUT * 1000));
        /// for (Loop1UInt8 = 0; Loop1UInt8 < sizeof(String); ++Loop1UInt8)
        /// {
        ///   printf("%2u - [%2.2u] <%c>   ", Loop1UInt8, String[Loop1UInt8], String[Loop1UInt8]);
        ///   if (((Loop1UInt8 + 1) % 8) == 0) printf("\r");
        /// }
        /// printf("\r\r");
        /***/
      break;
    }

    if (IdleTime >= (BUTTON_TIME_OUT * 1000))
    {
      if (DebugBitMask & DEBUG_IR) uart_send(__LINE__, __func__, "Reached IdleTime timeout...%u VS %u\r", IdleTime, (BUTTON_TIME_OUT * 1000));
      IrBuffer[0] = IR_LO_LIMIT;  // reset IR buffer to receive next data.

      /* Wait until current scroll is complete before closing <Function> window. */
      while ((Window[WinTop].FlagTopScroll) || (Window[WinMid].FlagMidScroll))
      {
        for (Loop1UInt16 = 0; Loop1UInt16 < MAX_ACTIVE_SCROLL; ++Loop1UInt16)
        {
          if (ActiveScroll[Loop1UInt16]->Owner == WinTop) Dum1UInt16 = ActiveScroll[Loop1UInt16]->PixelCountCurrent;
        }
        if (DebugBitMask & DEBUG_SCROLL) uart_send(__LINE__, __func__, "Waiting for scroll to complete... (%u pixels)\r", Dum1UInt16);
        sleep_ms(1000);
      }

      /* Waited IR_TIMEOUT seconds without receiving the <Enter> button (or time-out forced after function start). */
      win_close(WIN_FUNCTION);

      break;
    }

    /* Check if we received another command either from remote control, either from local buttons. */
    if (IrBuffer[0]     != BUTTON_NONE) CommandId = IrBuffer[0];
    if (ButtonBuffer[0] != BUTTON_NONE) CommandId = ButtonBuffer[0];
  } while (IdleTime < (BUTTON_TIME_OUT * 1000));

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting process_button()\r");

  return;
}





/* $PAGE */
/* $TITLE=isr_signal_trap() */
/* ============================================================================================================================================================= *\
                                                      Interrupt handler for signal received from remote control.
\* ============================================================================================================================================================= */
gpio_irq_callback_t isr_signal_trap(UINT8 gpio, UINT32 Events)
{
  static UINT32 ButtonSetOnTime;
  static UINT32 ButtonDownOnTime;
  static UINT32 ButtonUpOnTime;
  static UINT32 Dum1UInt32;


  /* Handle interrupts from infrared sensor. */
  if (gpio == IR_RX)
  {
    /* Check if interrupt is generated by the GPIO line changing from High to Low (38 kHz infrared burst changing from Off to On). */
    if (Events & GPIO_IRQ_EDGE_FALL)
    {
      /* This is the beginning of a 38kHz infrared burst. */
      if (IrStepCount > 0)
      {
        /* If this is not the first infrared burst for this data stream, keep track of timing information of the previous silent period length and pulse distance. */
        IrFinalValue[IrStepCount]  =  time_us_64();                                                      // this is the final timer value for the ending "silence"
        IrResultValue[IrStepCount] = (UINT32)(IrFinalValue[IrStepCount] - IrInitialValue[IrStepCount]);  // calculate duration of the ending "silence" period.
        IrLevel[IrStepCount]       = 1;                                                                  // ending period is a logical High level.

        /* Calculate the Pulse distance of the previous pulse that is ending now (Pulse distance = length of infrared burst + length of the "silence" period that follows). */
        IrPulseDistance[IrStepCount] = IrResultValue[IrStepCount - 1] + IrResultValue[IrStepCount];

        ++IrStepCount;  // start next logic level.
      }
      else
      {
        IrIndicator = 2;  // LED indicator on RGB matrix indicating we received an IR burst. */

        /* Turn On infrared LED indicators on RGB matrix when the infrared data stream begins. */
        RGB_matrix_set_pixel(FrameBuffer, IR_INDICATOR_START_ROW, IR_INDICATOR_START_COLUMN, IR_INDICATOR_END_ROW, IR_INDICATOR_END_COLUMN);
        RGB_matrix_set_color(IR_INDICATOR_START_ROW, IR_INDICATOR_START_COLUMN, IR_INDICATOR_END_ROW, IR_INDICATOR_END_COLUMN, BLUE);
      }

      IrInitialValue[IrStepCount] = time_us_64();  // this is also start timer of next Low level.

      gpio_acknowledge_irq(IR_RX, GPIO_IRQ_EDGE_FALL);
    }


    /* Check if interrupt is generated by the GPIO line changing from Low to High (38kHz infrared burst changing from On to Off). */
    if (Events & GPIO_IRQ_EDGE_RISE)
    {
      /* This is the end of a 38kHz infrared burst, the beginning of the "silence" period. */
      IrFinalValue[IrStepCount]  = time_us_64();                                                       // this is the final timer value for the ending infrared burst.
      IrResultValue[IrStepCount] = (UINT32)(IrFinalValue[IrStepCount] - IrInitialValue[IrStepCount]);  // calculate duration of the ending infrared burst.
      IrLevel[IrStepCount]       = 0;                                                                  // ending period is a logical Low level.
      ++IrStepCount;                                                                                   // go on to next logic level.

      /* Keep track of timer value at the beginning of this "silence" period. */
      IrInitialValue[IrStepCount] = time_us_64();                                                       // this is also start timer of next High level.

      gpio_acknowledge_irq(IR_RX, GPIO_IRQ_EDGE_RISE);
    }
  }



  /* Handle interrupts from RGB matrix <Set> buttons */
  if (gpio == BUTTON_SET_GPIO)
  {
    /* Check if interrupt is generated by the GPIO line changing from High to Low. */
    if (Events & GPIO_IRQ_EDGE_FALL)
    {
      /* This is the beginning of the <Set> button press. */
      ButtonSetOnTime = time_us_32();
      gpio_acknowledge_irq(BUTTON_SET_GPIO, GPIO_IRQ_EDGE_FALL);
    }


    /* Check if interrupt is generated by the GPIO line changing from Low to High. */
    if (Events & GPIO_IRQ_EDGE_RISE)
    {
      /* This is the end of the <Set> button press. */
      gpio_acknowledge_irq(BUTTON_SET_GPIO, GPIO_IRQ_EDGE_RISE);

      /* Determine if this <Set> button press is a quick press ("normal") or long press. */
      Dum1UInt32 = time_us_32();
      if ((Dum1UInt32 - ButtonSetOnTime) > BUTTON_LONG_PRESS_TIME)
      {
        ButtonBuffer[0] = BUTTON_SET_LONG;
        if (DebugBitMask & DEBUG_FUNCTION)
        {
          printf("\r");
          uart_send(__LINE__, __func__, "Button <Set>  On time: %10lu     Button <Set>  Off time: %10lu     Button <Set>  long press length: %8lu\r", ButtonSetOnTime, Dum1UInt32, (Dum1UInt32 - ButtonSetOnTime));
        }
      }
      else
      {
        ButtonBuffer[0] = BUTTON_SET;
        if (DebugBitMask & DEBUG_FUNCTION)
        {
          printf("\r");
          uart_send(__LINE__, __func__, "Button <Set>  On time: %10lu     Button <Set>  Off time: %10lu     Button <Set>  quick press length: %8lu\r", ButtonSetOnTime, Dum1UInt32, (Dum1UInt32 - ButtonSetOnTime));
        }
      }

      ButtonSetOnTime = 0l;  // reset button press On time.

      /* Button audible feedback. */
      if (FlashConfig1.FlagButtonFeedback == FLAG_ON)
        queue_add_active(50, 1);
    }
  }



  /* Handle interrupts from RGB matrix <Down> button. */
  if (gpio == BUTTON_DOWN_GPIO)
  {
    /* Check if interrupt is generated by the GPIO line changing from High to Low. */
    if (Events & GPIO_IRQ_EDGE_FALL)
    {
      /* This is the beginning of the <Down> button press. */
      ButtonDownOnTime = time_us_32();
      gpio_acknowledge_irq(BUTTON_DOWN_GPIO, GPIO_IRQ_EDGE_FALL);
    }


    /* Check if interrupt is generated by the GPIO line changing from Low to High. */
    if (Events & GPIO_IRQ_EDGE_RISE)
    {
      /* This is the end of the <Down> button press. */
      gpio_acknowledge_irq(BUTTON_DOWN_GPIO, GPIO_IRQ_EDGE_RISE);

      /* Determine if this <Down> button press is a quick press ("normal") or long press. */
      Dum1UInt32 = time_us_32();
      if ((Dum1UInt32 - ButtonDownOnTime) > BUTTON_LONG_PRESS_TIME)
      {
        ButtonBuffer[0] = BUTTON_DOWN_LONG;
        if (DebugBitMask & DEBUG_FUNCTION)
        {
          printf("\r");
          uart_send(__LINE__, __func__, "Button <Down> On time: %10lu     Button <Down> Off time: %10lu     Button <Down> long press length: %8lu\r", ButtonDownOnTime, Dum1UInt32, (Dum1UInt32 - ButtonDownOnTime));
        }
      }
      else
      {
        ButtonBuffer[0] = BUTTON_DOWN;
        if (DebugBitMask & DEBUG_FUNCTION)
        {
          printf("\r");
          uart_send(__LINE__, __func__, "Button <Down> On time: %10lu     Button <Down> Off time: %10lu     Button <Down> quick press length: %8lu\r", ButtonDownOnTime, Dum1UInt32, (Dum1UInt32 - ButtonDownOnTime));
        }
      }

      ButtonDownOnTime = 0l;

      /* Button audible feedback. */
      if (FlashConfig1.FlagButtonFeedback == FLAG_ON)
        queue_add_active(50, 1);
    }
  }



  /* Handle interrupts from RGB matrix <Up> button. */
  if (gpio == BUTTON_UP_GPIO)
  {
    /* Check if interrupt is generated by the GPIO line changing from High to Low. */
    if (Events & GPIO_IRQ_EDGE_FALL)
    {
      /* This is the beginning of a <Up> button press. */
      ButtonUpOnTime = time_us_32();
      gpio_acknowledge_irq(BUTTON_UP_GPIO, GPIO_IRQ_EDGE_FALL);
    }


    /* Check if interrupt is generated by the GPIO line changing from Low to High. */
    if (Events & GPIO_IRQ_EDGE_RISE)
    {
      /* This is the end of the <Up> button press. */
      gpio_acknowledge_irq(BUTTON_UP_GPIO, GPIO_IRQ_EDGE_RISE);

      /* Determine if this <Up> button press is a quick press ("normal") or long press. */
      Dum1UInt32 = time_us_32();
      if ((Dum1UInt32 - ButtonUpOnTime) > BUTTON_LONG_PRESS_TIME)
      {
        ButtonBuffer[0] = BUTTON_UP_LONG;
        if (DebugBitMask & DEBUG_FUNCTION)
        {
          printf("\r");
          uart_send(__LINE__, __func__, "Button <Up>   On time: %10lu     Button <Up>   Off time: %10lu     Button <Up>   long press length: %8lu\r", ButtonUpOnTime, Dum1UInt32, (Dum1UInt32 - ButtonUpOnTime));
        }
      }
      else
      {
        ButtonBuffer[0] = BUTTON_UP;
        if (DebugBitMask & DEBUG_FUNCTION)
        {
          printf("\r");
          uart_send(__LINE__, __func__, "Button <Up>   On time: %10lu     Button <Up>   Off time: %10lu     Button <Up>   quick press length: %8lu\r", ButtonUpOnTime, Dum1UInt32, (Dum1UInt32 - ButtonUpOnTime));
        }
      }

      ButtonUpOnTime = 0l;

      /* Button audible feedback. */
      if (FlashConfig1.FlagButtonFeedback == FLAG_ON)
        queue_add_active(50, 1);
    }
  }
}





#if 0
/* $PAGE */
/* $TITLE=isr_signal_trap() */
/* ============================================================================================================================================================= *\
                                                      Interrupt handler for signal received from remote control.
\* ============================================================================================================================================================= */
void isr_signal_trap(void)
{
  static UINT32 ButtonSetOnTime;
  static UINT32 ButtonDownOnTime;
  static UINT32 ButtonUpOnTime;
  static UINT32 Dum1UInt32;


  /* Handle interrupts from infrared sensor. */
  /* Check if interrupt is generated by the GPIO line changing from High to Low (38 kHz infrared burst changing from Off to On). */
  if (gpio_get_irq_event_mask(IR_RX) & GPIO_IRQ_EDGE_FALL)
  {
    /* This is the beginning of a 38kHz infrared burst. */
    if (IrStepCount > 0)
    {
      /* If this is not the first infrared burst for this data stream, keep track of timing information of the previous silent period length and pulse distance. */
      IrFinalValue[IrStepCount]  =  time_us_64();                                                      // this is the final timer value for the ending "silence"
      IrResultValue[IrStepCount] = (UINT32)(IrFinalValue[IrStepCount] - IrInitialValue[IrStepCount]);  // calculate duration of the ending "silence" period.
      IrLevel[IrStepCount]       = 1;                                                                  // ending period is a logical High level.

      /* Calculate the Pulse distance of the previous pulse that is ending now (Pulse distance = length of infrared burst + length of the "silence" period that follows). */
      IrPulseDistance[IrStepCount] = IrResultValue[IrStepCount - 1] + IrResultValue[IrStepCount];

      ++IrStepCount;  // start next logic level.
    }
    else
    {
      IrIndicator = 2;  // LED indicator on RGB matrix indicating we received an IR burst. */

      /* Turn On infrared LED indicators on RGB matrix when the infrared data stream begins. */
      RGB_matrix_set_pixel(FrameBuffer, IR_INDICATOR_START_ROW, IR_INDICATOR_START_COLUMN, IR_INDICATOR_END_ROW, IR_INDICATOR_END_COLUMN);
      RGB_matrix_set_color(IR_INDICATOR_START_ROW, IR_INDICATOR_START_COLUMN, IR_INDICATOR_END_ROW, IR_INDICATOR_END_COLUMN, BLUE);
    }

    IrInitialValue[IrStepCount] = time_us_64();  // this is also start timer of next Low level.

    gpio_acknowledge_irq(IR_RX, GPIO_IRQ_EDGE_FALL);
  }


  /* Check if interrupt is generated by the GPIO line changing from Low to High (38kHz infrared burst changing from On to Off). */
  if (gpio_get_irq_event_mask(IR_RX) & GPIO_IRQ_EDGE_FALL)
  {
    /* This is the end of a 38kHz infrared burst, the beginning of the "silence" period. */
    IrFinalValue[IrStepCount]  = time_us_64();                                                       // this is the final timer value for the ending infrared burst.
    IrResultValue[IrStepCount] = (UINT32)(IrFinalValue[IrStepCount] - IrInitialValue[IrStepCount]);  // calculate duration of the ending infrared burst.
    IrLevel[IrStepCount]       = 0;                                                                  // ending period is a logical Low level.
    ++IrStepCount;                                                                                   // go on to next logic level.

    /* Keep track of timer value at the beginning of this "silence" period. */
    IrInitialValue[IrStepCount] = time_us_64();                                                       // this is also start timer of next High level.

    gpio_acknowledge_irq(IR_RX, GPIO_IRQ_EDGE_RISE);
  }



  /* Handle interrupts from RGB matrix <Set> buttons */
  if (gpio == BUTTON_SET_GPIO)
  {
    /* Check if interrupt is generated by the GPIO line changing from High to Low. */
    if (Events & GPIO_IRQ_EDGE_FALL)
    {
      /* This is the beginning of the <Set> button press. */
      ButtonSetOnTime = time_us_32();
      gpio_acknowledge_irq(BUTTON_SET_GPIO, GPIO_IRQ_EDGE_FALL);
    }


    /* Check if interrupt is generated by the GPIO line changing from Low to High. */
    if (Events & GPIO_IRQ_EDGE_RISE)
    {
      /* This is the end of the <Set> button press. */
      gpio_acknowledge_irq(BUTTON_SET_GPIO, GPIO_IRQ_EDGE_RISE);

      /* Determine if this <Set> button press is a quick press ("normal") or long press. */
      Dum1UInt32 = time_us_32();
      if ((Dum1UInt32 - ButtonSetOnTime) > BUTTON_LONG_PRESS_TIME)
      {
        ButtonBuffer[0] = BUTTON_SET_LONG;
        if (DebugBitMask & DEBUG_FUNCTION)
        {
          printf("\r");
          uart_send(__LINE__, __func__, "Button <Set> On  time: %10lu     Button <Set> Off time: %10lu     Button <Set> long press length: %8lu\r", ButtonSetOnTime, Dum1UInt32, (Dum1UInt32 - ButtonSetOnTime));
        }
      }
      else
      {
        ButtonBuffer[0] = BUTTON_SET;
        if (DebugBitMask & DEBUG_FUNCTION)
        {
          printf("\r");
          uart_send(__LINE__, __func__, "Button <Set> On  time: %10lu     Button <Set> Off time: %10lu     Button <Set> quick press length: %8lu\r", ButtonSetOnTime, Dum1UInt32, (Dum1UInt32 - ButtonSetOnTime));
        }
      }

      ButtonSetOnTime = 0l;  // reset button press On time.

      /* Button audible feedback. */
      if (FlashConfig1.FlagButtonFeedback == FLAG_ON)
        queue_add_active(50, 1);
    }
  }



  /* Handle interrupts from RGB matrix <Down> button. */
  if (gpio == BUTTON_DOWN_GPIO)
  {
    /* Check if interrupt is generated by the GPIO line changing from High to Low. */
    if (Events & GPIO_IRQ_EDGE_FALL)
    {
      /* This is the beginning of the <Down> button press. */
      ButtonDownOnTime = time_us_32();
      gpio_acknowledge_irq(BUTTON_DOWN_GPIO, GPIO_IRQ_EDGE_FALL);
    }


    /* Check if interrupt is generated by the GPIO line changing from Low to High. */
    if (Events & GPIO_IRQ_EDGE_RISE)
    {
      /* This is the end of the <Down> button press. */
      gpio_acknowledge_irq(BUTTON_DOWN_GPIO, GPIO_IRQ_EDGE_RISE);

      /* Determine if this <Down> button press is a quick press ("normal") or long press. */
      Dum1UInt32 = time_us_32();
      if ((Dum1UInt32 - ButtonDownOnTime) > BUTTON_LONG_PRESS_TIME)
      {
        ButtonBuffer[0] = BUTTON_DOWN_LONG;
        printf("\r");
        uart_send(__LINE__, __func__, "Button <Down> On  time: %10lu     Button <Down> Off time: %10lu     Button <Down> long press length: %8lu\r", ButtonDownOnTime, Dum1UInt32, (Dum1UInt32 - ButtonDownOnTime));
      }
      else
      {
        ButtonBuffer[0] = BUTTON_DOWN;
        printf("\r");
        uart_send(__LINE__, __func__, "Button <Down> On  time: %10lu     Button <Down> Off time: %10lu     Button <Down> quick press length: %8lu\r", ButtonDownOnTime, Dum1UInt32, (Dum1UInt32 - ButtonDownOnTime));
      }

      ButtonDownOnTime = 0l;

      /* Button audible feedback. */
      if (FlashConfig1.FlagButtonFeedback == FLAG_ON)
        queue_add_active(50, 1);
    }
  }



  /* Handle interrupts from RGB matrix <Up> button. */
  if (gpio == BUTTON_UP_GPIO)
  {
    /* Check if interrupt is generated by the GPIO line changing from High to Low. */
    if (Events & GPIO_IRQ_EDGE_FALL)
    {
      /* This is the beginning of a <Up> button press. */
      ButtonUpOnTime = time_us_32();
      gpio_acknowledge_irq(BUTTON_UP_GPIO, GPIO_IRQ_EDGE_FALL);
    }


    /* Check if interrupt is generated by the GPIO line changing from Low to High. */
    if (Events & GPIO_IRQ_EDGE_RISE)
    {
      /* This is the end of the <Up> button press. */
      gpio_acknowledge_irq(BUTTON_UP_GPIO, GPIO_IRQ_EDGE_RISE);

      /* Determine if this <Up> button press is a quick press ("normal") or long press. */
      Dum1UInt32 = time_us_32();
      if ((Dum1UInt32 - ButtonUpOnTime) > BUTTON_LONG_PRESS_TIME)
      {
        ButtonBuffer[0] = BUTTON_UP_LONG;
        printf("\r");
        uart_send(__LINE__, __func__, "Button <Up> On  time: %10lu     Button <Up> Off time: %10lu     Button <Up> long press length: %8lu\r", ButtonUpOnTime, Dum1UInt32, (Dum1UInt32 - ButtonUpOnTime));
      }
      else
      {
        ButtonBuffer[0] = BUTTON_UP;
        printf("\r");
        uart_send(__LINE__, __func__, "Button <Up> On  time: %10lu     Button <Up> Off time: %10lu     Button <Up> quick press length: %8lu\r", ButtonUpOnTime, Dum1UInt32, (Dum1UInt32 - ButtonUpOnTime));
      }

      ButtonUpOnTime = 0l;

      /* Button audible feedback. */
      if (FlashConfig1.FlagButtonFeedback == FLAG_ON)
        queue_add_active(50, 1);
    }
  }
}
#endif  // 0





/* $PAGE */
/* $TITLE=process_function() */
/* ============================================================================================================================================================= *\
                                                                 Process a function number.
\* ============================================================================================================================================================= */
void process_function(UINT16 FunctionNumber)
{
  if (DebugBitMask & DEBUG_FLOW) printf("Entering process_function()\r");

  uart_send(__LINE__, __func__, "Entering process_function()\r\r");

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting process_function()\r");

  return;
}





/* $PAGE */
/* $TITLE=pwm_display_parameters() */
/* ============================================================================================================================================================= *\
                                            Display PWM parameters for all PWM signals used in the RGB Matrix project.
\* ============================================================================================================================================================= */
void pwm_display_parameters(void)
{
  UINT8 Loop1UInt8;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering pwm_display_parameters\r");

  for (Loop1UInt8 = 0; Loop1UInt8 < PWM_ID_HI_LIMIT; ++Loop1UInt8)
  {
    switch (Loop1UInt8)
    {
      case (PWM_ID_BRIGHTNESS):
        uart_send(__LINE__, __func__, "PWM for brightness control (PWM ID: %u)\r", PWM_ID_BRIGHTNESS);
      break;

      default:
        uart_send(__LINE__, __func__, "Undefined PWM (number %u)\r", Loop1UInt8);
      break;
    }

    uart_send(__LINE__, __func__, "SystemClock:    %lu\r",    clock_get_hz(clk_sys));
    uart_send(__LINE__, __func__, "PWM ID:         %u\r",     Loop1UInt8);
    uart_send(__LINE__, __func__, "GPIO:           %u\r",     Pwm[Loop1UInt8].Gpio);
    uart_send(__LINE__, __func__, "Slice:          %u\r",     Pwm[Loop1UInt8].Slice);
    uart_send(__LINE__, __func__, "Channel:        %u\r",     Pwm[Loop1UInt8].Channel);
    uart_send(__LINE__, __func__, "ClockDivider:   %f\r",     Pwm[Loop1UInt8].ClockDivider);
    uart_send(__LINE__, __func__, "Clock:          %lu\r",    Pwm[Loop1UInt8].Clock);
    uart_send(__LINE__, __func__, "Frequency:      %lu\r",    Pwm[Loop1UInt8].Frequency);
    uart_send(__LINE__, __func__, "Wrap:           %u\r",     Pwm[Loop1UInt8].Wrap);
    // uart_send(__LINE__, __func__, "DutyCycle:      %u\r",     Pwm[Loop1UInt8].DutyCycle);
    uart_send(__LINE__, __func__, "Level:          %u\r\r\r", Pwm[Loop1UInt8].Level);
  }

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting pwm_display_parameters()\r");

  return;
}




/* $PAGE */
/* $TITLE=pwm_initialize() */
/* ============================================================================================================================================================= *\
                                                                            Initialize PWM:
                                                                    For display matrix brightness.
\* ============================================================================================================================================================= */
void pwm_initialize(void)
{
  UCHAR String[128];

  UINT8 Loop1UInt8;

  UINT32 SystemClock;


  if (DebugBitMask & DEBUG_PWM) printf("Entering pwm_initialize()\r");

  /* Retrieve system clock (Pico is 125 MHz). */
  SystemClock = clock_get_hz(clk_sys);


  /* Initialize all GPIOs requiring PWM. */
  for (Loop1UInt8 = PWM_ID_LO_LIMIT; Loop1UInt8 < PWM_ID_HI_LIMIT; ++Loop1UInt8)
  {
    /* Initialize the specified GPIO as a PWM output. */
    gpio_init(Pwm[Loop1UInt8].Gpio);
    gpio_set_function(Pwm[Loop1UInt8].Gpio, GPIO_FUNC_PWM);

    /* Get PWM slice number for this PWM, depending on which GPIO it is connected to (for RGB Matrix, OE = GPIO 13 = Slice 6). */
    Pwm[Loop1UInt8].Slice = pwm_gpio_to_slice_num(Pwm[Loop1UInt8].Gpio);

    /* Get PWM channel number for this PWM, depending on which GPIO it is connected to (for RGB Matrix, OE = GPIO 13 = Channel 1). */
    Pwm[Loop1UInt8].Channel = pwm_gpio_to_channel(Pwm[Loop1UInt8].Gpio);


    /* For passive buzzer, slow down the 125 MHz system clock to get a PWM clock that is more flexible to reach lower frequencies for audio sounds with the counter / wrap. */
    Pwm[Loop1UInt8].ClockDivider = SystemClock / 100000000.0;  // ClockDivider will be 1.25000, ending up with a 100 MHz clock.
    Pwm[Loop1UInt8].Clock        = (UINT32)(SystemClock / Pwm[Loop1UInt8].ClockDivider);  // keep track of resulting clock value.
    pwm_set_clkdiv(Pwm[Loop1UInt8].Slice, Pwm[Loop1UInt8].ClockDivider);  // slow down the clock for this PWM slice.


    /* Complete initialization specific with each PWM. */
    switch (Loop1UInt8)
    {
      case (PWM_ID_BRIGHTNESS):
        /* Set current values in PWM structure. */
        Pwm[Loop1UInt8].Frequency = 50000;
        Pwm[Loop1UInt8].Wrap      = (UINT16)(Pwm[Loop1UInt8].Clock / (Pwm[Loop1UInt8].Frequency - 1));  // counter will wrap around to provide the wanted frequency.
        Pwm[Loop1UInt8].DutyCycle = 0;  // set duty cycle at 0% (blank) on entry, during power-up sequence.
        Pwm[Loop1UInt8].Level     = (UINT16)(Pwm[Loop1UInt8].Wrap * ((100 - Pwm[Loop1UInt8].DutyCycle) / 100.0));  // since OE is active low, reverse the duty cycle.

        CLK_HIGH;

        /* Set PWM frequency by setting a counter wrap value. */
        pwm_set_wrap(Pwm[Loop1UInt8].Slice, Pwm[Loop1UInt8].Wrap);

        /* Set PWM duty cycle given the Divider, Counter and Frequency current values. */
        pwm_set_chan_level(Pwm[Loop1UInt8].Slice, Pwm[Loop1UInt8].Channel, Pwm[Loop1UInt8].Level);

        /* Start PWM. */
        pwm_set_enabled(Pwm[Loop1UInt8].Slice, TRUE);

        CLK_LOW;
      break;
    }
  }

  if (DebugBitMask & DEBUG_PWM) printf("Exiting pwm_initialize()\r");

  return;
}





/* $PAGE */
/* $TITLE=pwm_on_off() */
/* ============================================================================================================================================================= *\
                                                       Turn On or Off the PWM signal specified in argument.
\* ============================================================================================================================================================= */
void pwm_on_off(UINT8 PwmNumber, UINT8 FlagSwitch)
{
  UCHAR String[256];


  if (DebugBitMask & DEBUG_FLOW) printf("Entering pwm_on_off()\r");

  Pwm[PwmNumber].OnOff = FlagSwitch;

  if (Pwm[PwmNumber].OnOff == FLAG_ON)
  {
    /* Turn On PWM signal. */
    /// uart_send(__LINE__, __func__, "Turning On  PWM signal %u  Slice: %u\r", PwmNumber, Pwm[PwmNumber].Slice);
    pwm_set_enabled(Pwm[PwmNumber].Slice, TRUE);
  }
  else
  {
    /* Turn Off PWM signal. */
    /// uart_send(__LINE__, __func__, "Turning Off PWM signal %u  Slice: %u\r", PwmNumber, Pwm[PwmNumber].Slice);
    pwm_set_enabled(Pwm[PwmNumber].Slice, FALSE);
  }

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting pwm_on_off()\r");

  return;
}





/* $PAGE */
/* $TITLE=pwm_set_duty_cycle() */
/* ============================================================================================================================================================= *\
                                             Set the duty cycle for the PWM controlling the display matrix brightness.
                            NOTES:
                            Duty cycle is given as a percentage. For example, a value of "50" means 50% for duty cycle
                            Since "Output Enable" (OE) is active low, a duty cycle of 75% means that PWM signal means that PWM line must be "Low@
                            for 25% of its time. So, we have to reverse the value sent to the Pico.
\* ============================================================================================================================================================= */
void pwm_set_duty_cycle(UINT8 DutyCycle)
{
  UINT8 Loop1UInt8;


  /* Validate value specified for duty cycle. */
  if (DutyCycle > 99) DutyCycle = 99;

  /* Compute the PWM Level required to have the desired duty cycle. */
  Pwm[PWM_ID_BRIGHTNESS].Level = (UINT16)(Pwm[PWM_ID_BRIGHTNESS].Wrap * ((100 - DutyCycle) / 100.0));

  CLK_HIGH;

  /* Set PWM duty cycle given the Divider, Counter and Frequency current values. */
  pwm_set_chan_level(Pwm[PWM_ID_BRIGHTNESS].Slice, Pwm[PWM_ID_BRIGHTNESS].Channel, Pwm[PWM_ID_BRIGHTNESS].Level);

  /* Set new values in PWM structure. */
  Pwm[PWM_ID_BRIGHTNESS].DutyCycle = DutyCycle;

  CLK_LOW;

  return;
}





/* $PAGE */
/* $TITLE=pwm_set_frequency() */
/* ============================================================================================================================================================= *\
                                                          Set the frequency for the specified PWM.
\* ============================================================================================================================================================= */
void pwm_set_frequency(UINT8 PwmNumber, UINT32 Frequency)
{
  /* Indicate current PWM values. */
  Pwm[PwmNumber].Frequency = Frequency;
  Pwm[PwmNumber].Wrap      = (UINT16)(Pwm[PwmNumber].Clock / Pwm[PwmNumber].Frequency);

  /* Set PWM frequency by setting a counter wrap value. */
  pwm_set_wrap(Pwm[PwmNumber].Slice, Pwm[PwmNumber].Wrap);

  /* Since we change the frequency, adjust duty cycle accordingly. */
  Pwm[PwmNumber].Level = (UINT16)(Pwm[PwmNumber].Wrap * (Pwm[PwmNumber].DutyCycle / 100.0));

  /* Set PWM duty cycle given the Divider, Counter and Frequency current values. */
  pwm_set_chan_level(Pwm[PwmNumber].Slice, Pwm[PwmNumber].Channel, Pwm[PwmNumber].Level);

  return;
}




/* $PAGE */
/* $TITLE=pwm_set_level() */
/* ============================================================================================================================================================= *\
                                                          Set the PWM "Level" for the specified PWM.
\* ============================================================================================================================================================= */
void pwm_set_level(UINT8 PwmNumber, UINT16 Level)
{
  /* Validation of Level value given in argument. */
  if (Level < PWM_LO_LIMIT) Level = PWM_LO_LIMIT;
  if (Level > PWM_HI_LIMIT) Level = PWM_HI_LIMIT;

  /* Indicate current PWM values. */
  Pwm[PwmNumber].Level = Level;

  /* Set PWM level. */
  CLK_HIGH;
  pwm_set_chan_level(Pwm[PwmNumber].Slice, Pwm[PwmNumber].Channel, Pwm[PwmNumber].Level);
  CLK_LOW;

  return;
}




/* $TITLE=queue_add_active() */
/* $PAGE */
/* ============================================================================================================================================================= *\
        Queue the given sound in the active buzzer sound queue. Use the queue algorithm where one slot is lost. This prevents the use of a lock mechanism.
\* ============================================================================================================================================================= */
UINT16 queue_add_active(UINT16 MSeconds, UINT16 RepeatCount)
{
  UCHAR String[256];


  /* Trap circular buffer corruption. */
  /***/
  if ((QueueActiveSound.Head > MAX_ACTIVE_SOUND_QUEUE) || (QueueActiveSound.Tail > MAX_ACTIVE_SOUND_QUEUE))
  {
    if (DebugBitMask & DEBUG_SOUND_QUEUE)
      uart_send(__LINE__, __func__, "- A-Corrupted:        %5u   %5u\r", QueueActiveSound.Head, QueueActiveSound.Tail);

    QueueActiveSound.Head = 0;
    QueueActiveSound.Tail = 0;

    return 0;
  }
  /***/


  /* Check if the active buzzer sound queue is full. */
  if (((QueueActiveSound.Tail > 0) && (QueueActiveSound.Head == QueueActiveSound.Tail - 1)) || ((QueueActiveSound.Tail == 0) && (QueueActiveSound.Head == MAX_ACTIVE_SOUND_QUEUE - 1)))
  {
    /* Sound queue is full, return error code. */
    return MAX_ACTIVE_SOUND_QUEUE;
  }

  /* If there is at least one slot available in the queue, insert the sound to be played. */
  QueueActiveSound.Element[QueueActiveSound.Head].MSec        = MSeconds;
  QueueActiveSound.Element[QueueActiveSound.Head].RepeatCount = RepeatCount;
  ++QueueActiveSound.Head;

  if (DebugBitMask & DEBUG_SOUND_QUEUE)
    uart_send(__LINE__, __func__, "- A-Queueing:            %5u   %5u\r", MSeconds, RepeatCount);


  /* If reaching end of circular buffer, revert to beginning. */
  if (QueueActiveSound.Head >= MAX_ACTIVE_SOUND_QUEUE) QueueActiveSound.Head = 0;

  return 0;
}





/* $PAGE */
/* $TITLE=queue_free_active() */
/* ============================================================================================================================================================= *\
                                                    Return the number of free slots in active sound queue.
\* ============================================================================================================================================================= */
UINT8 queue_free_active(void)
{
  UINT8 RemainingSlots;


  if (QueueActiveSound.Head > QueueActiveSound.Tail)
  {
    RemainingSlots = MAX_ACTIVE_SOUND_QUEUE - QueueActiveSound.Head + QueueActiveSound.Tail - 1;
    if (DebugBitMask & DEBUG_SOUND_QUEUE)
      uart_send(__LINE__, __func__, "Active queue remaining space: %5u  Head: %3u  Tail: %3u\r", RemainingSlots, QueueActiveSound.Head, QueueActiveSound.Tail);
  }

  if (QueueActiveSound.Head == QueueActiveSound.Tail)
  {
    RemainingSlots = MAX_ACTIVE_SOUND_QUEUE - 1;
    if (DebugBitMask & DEBUG_SOUND_QUEUE)
      uart_send(__LINE__, __func__, "Active queue remaining space: %5u  Head: %3u  Tail: %3u\r", RemainingSlots, QueueActiveSound.Head, QueueActiveSound.Tail);
  }

  if (QueueActiveSound.Head < QueueActiveSound.Tail)
  {
    RemainingSlots = QueueActiveSound.Tail - QueueActiveSound.Head - 1;
    if (DebugBitMask & DEBUG_SOUND_QUEUE)
      uart_send(__LINE__, __func__, "Active queue remaining space: %5u  Head: %3u  Tail: %3u\r", RemainingSlots, QueueActiveSound.Head, QueueActiveSound.Tail);
  }

  return RemainingSlots;
}





/* $PAGE */
/* $TITLE=queue_remove_active() */
/* ============================================================================================================================================================= *\
                                                    Unqueue next sound from the active buzzer sound queue.
\* ============================================================================================================================================================= */
UINT8 queue_remove_active(UINT16 *MSeconds, UINT16 *RepeatCount)
{
  UINT16 Loop1UInt16;

  UINT32 CurrentTail;


  /* Trap circular buffer corruption. */
  if ((QueueActiveSound.Head > MAX_ACTIVE_SOUND_QUEUE) || (QueueActiveSound.Tail > MAX_ACTIVE_SOUND_QUEUE))
  {
    if (DebugBitMask & DEBUG_SOUND_QUEUE)
    {
      uart_send(__LINE__, __func__, "- A-Corrupted:        %5u   %5u\r", QueueActiveSound.Head, QueueActiveSound.Tail);
      uart_send(__LINE__, __func__, "MAX_ACTIVE_SOUND_QUEUE: (%u)   Head: %4u   Tail: %4u\r", MAX_ACTIVE_SOUND_QUEUE, QueueActiveSound.Head, QueueActiveSound.Tail);
      uart_send(__LINE__, __func__, "          MSec    Repeat\r");

      for (Loop1UInt16 = 0; Loop1UInt16 < MAX_ACTIVE_SOUND_QUEUE; ++Loop1UInt16)
        uart_send(__LINE__, __func__, " %4u-   %5u     %5u\r", Loop1UInt16, QueueActiveSound.Element[Loop1UInt16].MSec, QueueActiveSound.Element[Loop1UInt16].RepeatCount);
    }

    QueueActiveSound.Head = 0;
    QueueActiveSound.Tail = 0;

    return 0xFF;
  }


  /* Check if active sound queue is empty. */
  if (QueueActiveSound.Head == QueueActiveSound.Tail)
  {
    /* In case of empty queue or queue error, return 0 as milliseconds and repeat count. */
    *MSeconds    = 0;
    *RepeatCount = 0;

    /***
    if (DebugBitMask & DEBUG_SOUND_QUEUE)
      uart_send(__LINE__, "- A-Empty:               %5u   %5u\r", QueueActiveSound.Head, QueueActiveSound.Tail);
    ***/

    return 0xFF;
  }
  else
  {
    /* Active sound queue is not empty. */
    if (DebugBitMask & DEBUG_SOUND_QUEUE)
      uart_send(__LINE__, __func__, "- A-NotEmpty:            %5u   %5u\r", QueueActiveSound.Head, QueueActiveSound.Tail);

    /* Extract data for next sound to play. */
    *MSeconds    = QueueActiveSound.Element[QueueActiveSound.Tail].MSec;
    *RepeatCount = QueueActiveSound.Element[QueueActiveSound.Tail].RepeatCount;


    /* And reset this slot in the sound queue. */
    QueueActiveSound.Element[QueueActiveSound.Tail].MSec        = 0;
    QueueActiveSound.Element[QueueActiveSound.Tail].RepeatCount = 0;


    /***
    if (DebugBitMask & DEBUG_SOUND_QUEUE)
      uart_send(__LINE__, __func__, "- A-Unqueuing %3u:        %5u   %5u\r", QueueActiveSound.Tail, *MSeconds, *RepeatCount);
    ***/


    if ((*MSeconds != 0) && (*RepeatCount <= 100))
    {
      /* The sound found in this slot is valid. Point to next slot for next cycle. */
      ++QueueActiveSound.Tail;

      /* If reaching end of circular buffer, revert to beginning. */
      if (QueueActiveSound.Tail >= MAX_ACTIVE_SOUND_QUEUE) QueueActiveSound.Tail = 0;

      return 0;
    }
    else
    {
      /* Sound in this slot was invalid. */
      if (DebugBitMask & DEBUG_SOUND_QUEUE)
      {
        uart_send(__LINE__, __func__, "- A-Invalid slot: %3u\r", QueueActiveSound.Tail);
        uart_send(__LINE__, __func__, "- MSec: %3u   RepeatCount: %3u\r", *MSeconds, *RepeatCount);
      }

      /* If this slot was corrupted, make some housekeeping. */
      CurrentTail = QueueActiveSound.Tail; // keep track of starting position in the queue.

      /* In case of empty queue or queue error, return 0 as milliseconds and repeat count. */
      *MSeconds    = 0;
      *RepeatCount = 0;


      /* Clean all active sound queue. */
      do
      {
        QueueActiveSound.Element[QueueActiveSound.Tail].MSec        = 0;
        QueueActiveSound.Element[QueueActiveSound.Tail].RepeatCount = 0;


        ++QueueActiveSound.Tail;

        /* If reaching end of circular buffer, revert to beginning. */
        if (QueueActiveSound.Tail >= MAX_ACTIVE_SOUND_QUEUE) QueueActiveSound.Tail = 0;
      } while (QueueActiveSound.Tail != CurrentTail);  // if we come back to where we started from, sound queue has all been cleaned up.

      QueueActiveSound.Head = QueueActiveSound.Tail;

      if (DebugBitMask & DEBUG_SOUND_QUEUE)
        uart_send(__LINE__, __func__, "- A-Done:                %5u   %5u\r", QueueActiveSound.Head, QueueActiveSound.Tail);

      return 0xFF;
    }
  }
}





/* $TITLE=reminder1_check() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                            Scan all reminders1 to find those that must be triggered.
\* ============================================================================================================================================================= */
void reminder1_check(void)
{
#ifdef RELEASE_VERSION
  UINT8 FlagLocalDebug = FLAG_OFF;  // should be OFF all times
#else  // RELEASE_VERSION
  UINT8 FlagLocalDebug = FLAG_ON;
#endif  // RELEASE_VERSION

  UCHAR String[31];

  UINT8 TotalActiveReminders;

  UINT16 Loop1UInt16;

  time_t UnixTime;


  /// if (FlagLocalDebug) printf("Entering reminder1_check()\r");

  /* Initialize total number of active reminders. */
  TotalActiveReminders = 0;


  /* Find current Unix time. */
  UnixTime = convert_human_to_unix(&CurrentTime, FLAG_ON);

  /* Scan all reminders to find those matching current time and day-of-week. */
  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_REMINDERS1; ++Loop1UInt16)
  {
    if (FlashConfig2.Reminder1[Loop1UInt16].StartPeriodUnixTime >= UnixTime)
    {
      if ((Reminder1BitMask & (0x01ll << Loop1UInt16)) == 0)
      {
        /* This reminder1 was inactive and becomes active. */
        if (FlagLocalDebug) uart_send(__LINE__, __func__, "Reminder1 %u becomes active\r", Loop1UInt16 + 1);
        Reminder1BitMask |= (0x01ll << Loop1UInt16);
        continue;
      }
      else
      {
        /* This reminder1 was already active. */
        if (FlagLocalDebug) uart_send(__LINE__, __func__, "Reminder1 %u already active\r", Loop1UInt16 + 1);
      }
    }
    else
    {
      /* Display reminder1 status for debug purpose. */
      /// if (FlagLocalDebug) uart_send(__LINE__, __func__, "Reminder1 inactive\r", Loop1UInt16 + 1);
    }
  }


  /// if (FlagLocalDebug) printf("Exiting reminder1_check()\r");

  return;
}





/* $TITLE=reminder1_ring() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                     Feed reminder1 ringer for currently active ("triggered") reminders1.
\* ============================================================================================================================================================= */
void reminder1_ring(void)
{
#ifdef RELEASE_VERSION
  UINT8 FlagLocalDebug = FLAG_OFF;  // should be OFF all times
#else  // RELEASE_VERSION
  UINT8 FlagLocalDebug = FLAG_OFF;
#endif  // RELEASE_VERSION

  UINT16 Loop1UInt16;

  UINT32 CurrentTimer;


  /// if (FlagLocalDebug) printf("Entering reminder1_ring()\r");

  /* When a reminder1 becomes active, reminder_check() will raise its bit in Reminder1BitMask. */
  if (Reminder1BitMask)
  {
    /* If some reminders are active, we'll need current timer to evaluate if it is time to feed ringer. */
    CurrentTimer = time_us_32();

    /* Scan reminders1 to find those requiring ringer repeat. */
    for (Loop1UInt16 = 0; Loop1UInt16 < MAX_REMINDERS1; ++Loop1UInt16)
    {
      /* Check if this reminder1 is active (global ring duration). */
      if (ActiveReminder1[Loop1UInt16].CountDown > 0)
      {
        /* Display details for first active reminder1 number for debug purpose. */
        if (FlagLocalDebug) uart_send(__LINE__, __func__, "0x%llX - %u - %3u - %3u - %9lu - %9lu - %4u\r", Reminder1BitMask, Loop1UInt16, FlashConfig2.Reminder1[Loop1UInt16].StartPeriodUnixTime, ActiveReminder1[Loop1UInt16].CountDown, CurrentTimer, ActiveReminder1[Loop1UInt16].PreviousTimer, (UINT16)((CurrentTimer - ActiveReminder1[Loop1UInt16].PreviousTimer) / 1000000ll));

        if (((CurrentTimer - ActiveReminder1[Loop1UInt16].PreviousTimer) / 1000000ll) >= FlashConfig2.Reminder1[Loop1UInt16].RingRepeatTimeSeconds)
        {
          if (FlagLocalDebug) uart_send(__LINE__, __func__, "1) %4u - %3u", (UINT16)((CurrentTimer - ActiveReminder1[Loop1UInt16].PreviousTimer) / 1000000ll), FlashConfig2.Reminder1[Loop1UInt16].RingRepeatTimeSeconds);
          /* It is time to feed this ringer and repeat the scroll. */
          queue_add_active(150, 4);
          queue_add_active(2000, SILENT);
          win_scroll(WIN_DATE, 201, 201, 1, 1, FONT_5x7, "%s", FlashConfig2.Reminder1[Loop1UInt16].Message);


          if (FlashConfig2.Reminder1[Loop1UInt16].RingRepeatTimeSeconds > ActiveReminder1[Loop1UInt16].CountDown)
          {
            /* Next ring would be beyond the global ringing period, reset it after current ring above. */
            ActiveReminder1[Loop1UInt16].CountDown     = 0;
            ActiveReminder1[Loop1UInt16].PreviousTimer = 0l;
            Reminder1BitMask &= ~(0x01ll << Loop1UInt16);  // turn Off this reminder1 number in the bitmask of active reminders1.
          }
          else
          {
            ActiveReminder1[Loop1UInt16].CountDown -= FlashConfig2.Reminder1[Loop1UInt16].RingRepeatTimeSeconds;  // "FlashConfig2.Reminder1[Loop1UInt16].RingRepeatTimeSecond" seconds have elapsed since last pass.
            if (ActiveReminder1[Loop1UInt16].CountDown > 0)
            {
              /* There are more rings to come. */
              ActiveReminder1[Loop1UInt16].PreviousTimer = CurrentTimer;
            }
            else
            {
              /* This alarm is over now. */
              ActiveReminder1[Loop1UInt16].PreviousTimer = 0l;
              Reminder1BitMask &= ~(0x01ll << Loop1UInt16);  // turn Off this reminder1 number in the bitmask of active reminders1.
            }
          }
        }
      }
    }
  }

  /// if (FlagLocalDebug) printf("Exiting reminder1_ring()\r");

  return;
}





/* $TITLE=reminder1_update() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                               Check / update reminders of type 1.
\* ============================================================================================================================================================= */
void reminder1_update(void)
{
  UCHAR String[65];

  UINT16 Loop1UInt16;

  time_t UnixTime;

  struct human_time HumanTime;
  struct tm TempTime;


  ds3231_get_time(&HumanTime);
  UnixTime = convert_human_to_unix(&HumanTime, FLAG_ON);
  convert_unix_time(UnixTime, &TempTime, &HumanTime, FLAG_ON);

  if (DebugBitMask & DEBUG_REMINDER)
  {
    display_human_time("Current human time used to update reminders: \r\r", &HumanTime);
    printf("\r");
    uart_send(__LINE__, __func__, "Current UnixTime: %12llu\r\r", UnixTime);
  }

  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_REMINDERS1; ++Loop1UInt16)
  {
    if (DebugBitMask & DEBUG_REMINDER)
    {
      uart_send(__LINE__, __func__, "Verifying Reminder number %2u     StartPeriod: %12llu   EndPeriod: %12llu\r",
                Loop1UInt16 + 1, FlashConfig2.Reminder1[Loop1UInt16].StartPeriodUnixTime, FlashConfig2.Reminder1[Loop1UInt16].EndPeriodUnixTime);
      sleep_ms(50);  // prevent communication override.
    }


    /* Check if this reminder is initialized. */
    if (FlashConfig2.Reminder1[Loop1UInt16].StartPeriodUnixTime == 0ll)
    {
      if (DebugBitMask & DEBUG_REMINDER) printf("\r");
      continue;
    }

    /* Check if this reminder is over now. */
    if (UnixTime > FlashConfig2.Reminder1[Loop1UInt16].EndPeriodUnixTime)
    {
      if (DebugBitMask & DEBUG_REMINDER) uart_send(__LINE__, __func__, "Reminder %2u is over, adding NextReminderDelaySeconds %12llu\r", Loop1UInt16 + 1, FlashConfig2.Reminder1[Loop1UInt16].NextReminderDelaySeconds);
      FlashConfig2.Reminder1[Loop1UInt16].StartPeriodUnixTime += FlashConfig2.Reminder1[Loop1UInt16].NextReminderDelaySeconds;
      FlashConfig2.Reminder1[Loop1UInt16].EndPeriodUnixTime   += FlashConfig2.Reminder1[Loop1UInt16].NextReminderDelaySeconds;
    }

    /* Check if this reminder is valid. */
    if ((UnixTime > FlashConfig2.Reminder1[Loop1UInt16].StartPeriodUnixTime) && (UnixTime < FlashConfig2.Reminder1[Loop1UInt16].EndPeriodUnixTime))
    {
      if (DebugBitMask & DEBUG_REMINDER) uart_send(__LINE__, __func__, "Reminder %2u is valid, set its corresponding bit in the bitmask          ************\r\r", Loop1UInt16 + 1);
      Reminder1BitMask |= (0x01ll << Loop1UInt16);
    }
    else
    {
      if (DebugBitMask & DEBUG_REMINDER) uart_send(__LINE__, __func__, "Reminder %2u is not valid, reset its corresponding bit in the bitmask\r\r", Loop1UInt16 + 1);
      Reminder1BitMask &= ~(0x01ll << Loop1UInt16);
    }
  }

  if (DebugBitMask & DEBUG_REMINDER)
  {
    util_uint64_to_binary_string(Reminder1BitMask, 40, String);
    printf("\r");
    uart_send(__LINE__, __func__, "Reminder1BitMask while exiting reminder1_update(): 0x%10.10llX [%s]\r\r\r", Reminder1BitMask, String);
  }

  return;
}





/* $TITLE=remote_control_test() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                                      Test remote control.
\* ============================================================================================================================================================= */
void remote_control_test(void)
{
  UINT8 FlagDone;

  UINT16 SecondsLeft;


  printf("                 Remote control test.\r\r");
  printf("Press a remote control button to display its decimal code and button name...\r");
  printf("You can watch the RGB Matrix IR indicator to see if the infrared data stream is properly received.\r");
  printf("Press a local button to exit test before the timeout period.\r\r");

  FlagDone    = FLAG_OFF;
  SecondsLeft = WATCHDOG_SECONDS - WatchdogMiss;

  while (SecondsLeft)
  {
    if (IrBuffer[0] == IR_LO_LIMIT)
    {
      sleep_ms(100);
      SecondsLeft = WATCHDOG_SECONDS - WatchdogMiss;
      if ((SecondsLeft % 5) == 0)
      {
        if (FlagDone == FLAG_OFF)
        {
          FlagDone = FLAG_ON;
          printf("Time left: %4u seconds\r", SecondsLeft);
        }

        /* User may abort the test by pressing a local button on the RGB Matrix. */
        if (ButtonBuffer[0] != BUTTON_NONE)
        {
          ButtonBuffer[0] = BUTTON_NONE;
          return;
        }
      }
      else
      {
        FlagDone = FLAG_OFF;
      }
      continue;
    }
    else
    {
      printf("User pressed remote control code %u, button name: <%s>\r\r", IrBuffer[0], ButtonName[IrBuffer[0]]);
      IrBuffer[0] = BUTTON_NONE;
    }
  }

  return;
}





/* $TITLE=RGB_matrix_blink() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                             Blink all blinking areas in active windows.
\* ============================================================================================================================================================= */
void RGB_matrix_blink()
{
  static UINT16 CycleNumber;

  UINT16 ColumnNumber;
  UINT16 RowNumber;


  if (DebugBitMask & DEBUG_FLOW) printf("Entering RGB_matrix_blink()\r");


  if ((Window[WinTop].FlagBlink == FLAG_OFF) && (Window[WinMid].FlagBlink == FLAG_OFF) && (Window[WinBot].FlagBlink == FLAG_OFF))
  {
    if (DebugBitMask & DEBUG_BLINK) uart_send(__LINE__, __func__, "Entering RGB_matrix_blink(FLAG_OFF)\r");
    return;
  }


  if (DebugBitMask & DEBUG_BLINK)
  {
    if (CycleNumber % 2)
      uart_send(__LINE__, __func__, "Entering RGB_matrix_blink(blank)\r");
    else
      uart_send(__LINE__, __func__, "Entering RGB_matrix_blink(restore)\r");
  }


  if (CycleNumber % 2)
  {
    if (DebugBitMask & DEBUG_BLINK) printf("Cycle to blank blink area\r");

    /* It is time to blank the blink area. */
    for (RowNumber = 0; RowNumber < MAX_ROWS; ++RowNumber)
    {
      if (CheckBuffer[RowNumber] != 0xFFFFFFFFFFFFFFFFll)
      {
        /// printf("Acting on Row: %2u - Cols: ", RowNumber);
        /* There is an active blink area on this line. */
        for (ColumnNumber = 0; ColumnNumber < MAX_COLUMNS; ++ColumnNumber)
        {
          if ((CheckBuffer[RowNumber] & (0x1ll << ColumnNumber)) == 0)
          {
            /// printf("%2u - ", ColumnNumber);
            FrameBuffer[RowNumber] &= ~(0x1ll << ColumnNumber);
          }
        }
        /// printf("\r");
      }
    }
    if (Window[WinTop].FlagBlink == FLAG_ON) Window[WinTop].BlinkOnTimer = 0l;
    if (Window[WinMid].FlagBlink == FLAG_ON) Window[WinMid].BlinkOnTimer = 0l;
    if (Window[WinBot].FlagBlink == FLAG_ON) Window[WinBot].BlinkOnTimer = 0l;
  }
  else
  {
    if (DebugBitMask & DEBUG_BLINK) printf("Cycle to restore blink area\r");

    /* It is time to restore the blink area. */
    for (RowNumber = 0; RowNumber < MAX_ROWS; ++RowNumber)
    {
      if (CheckBuffer[RowNumber] != 0xFFFFFFFFFFFFFFFFll)
      {
        /// printf("Acting on Row: %2u - Cols: ", RowNumber);
        /* There is an active blink area on this line. */
        for (ColumnNumber = 0; ColumnNumber < MAX_COLUMNS; ++ColumnNumber)
        {
          if ((CheckBuffer[RowNumber] & (0x1ll << ColumnNumber)) == 0)
          {
            if (BlinkBuffer[RowNumber] & (0x1ll << ColumnNumber))
              FrameBuffer[RowNumber] |= (0x1ll << ColumnNumber);
            else
              FrameBuffer[RowNumber] &= ~(0x1ll << ColumnNumber);
          }
        }
        /// printf("\r");
      }
    }
    if (Window[WinTop].FlagBlink == FLAG_ON) Window[WinTop].BlinkOnTimer = time_us_32();
    if (Window[WinMid].FlagBlink == FLAG_ON) Window[WinMid].BlinkOnTimer = time_us_32();
    if (Window[WinBot].FlagBlink == FLAG_ON) Window[WinBot].BlinkOnTimer = time_us_32();
  }

  ++CycleNumber;

  if (DebugBitMask & DEBUG_FLOW) printf("Exiting RGB_matrix_blink()\r");

  return;
}





/* $TITLE=RGB_matrix_box() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                             Draw or erase a box with specified borders.
\* ============================================================================================================================================================= */
void RGB_matrix_box(UINT8 StartRow, UINT8 StartColumn, UINT8 EndRow, UINT8 EndColumn, UINT8 Color, UINT8 Action)
{
  UINT8  Loop1UInt8;
  UINT8  RowNumber;


  /* Validate provided coordinates. */
  RGB_matrix_check_coord(&StartRow, &StartColumn, &EndRow, &EndColumn);


  /* Set color for top border. */
  RGB_matrix_set_color(StartRow, StartColumn, StartRow, EndColumn, Color);

  /* Turn On or Off pixels for the top box border. */
  for (Loop1UInt8 = StartColumn; Loop1UInt8 < EndColumn; ++Loop1UInt8)
  {
    if (Action == ACTION_DRAW)
      FrameBuffer[StartRow] |=  (0x01ll << Loop1UInt8);
    else
      FrameBuffer[StartRow] &= ~(0x01ll << Loop1UInt8);
  }



  /* Set color for bottom border. */
  RGB_matrix_set_color(EndRow, StartColumn, EndRow, EndColumn, Color);

  /* Turn On or Off pixels for the bottom box border. */
  for (Loop1UInt8 = StartColumn; Loop1UInt8 < EndColumn; ++Loop1UInt8)
  {
    if (Action == ACTION_DRAW)
      FrameBuffer[EndRow] |= (0x01ll << Loop1UInt8);
    else
      FrameBuffer[EndRow] &= ~(0x01ll << Loop1UInt8);
  }



  /* Turn On or Off pixels for the left box border. */
  for (RowNumber = StartRow; RowNumber <= EndRow; ++RowNumber)
  {
    /* Set color for left border. */
    RGB_matrix_set_color(RowNumber, StartColumn, RowNumber, StartColumn, Color);

    if (Action == ACTION_DRAW)
      FrameBuffer[RowNumber] |= (0x01ll << StartColumn);
    else
      FrameBuffer[RowNumber] &= ~(0x01ll << StartColumn);
  }

  /* Turn On pixels for the right box border. */
  for (RowNumber = StartRow; RowNumber <= EndRow; ++RowNumber)
  {
    /* Set color for right border. */
    RGB_matrix_set_color(RowNumber, EndColumn, RowNumber, EndColumn, Color);

    if (Action == ACTION_DRAW)
      FrameBuffer[RowNumber] |= (0x01ll << EndColumn);
    else
      FrameBuffer[RowNumber] &= ~(0x01ll << EndColumn);
  }

  return;
}





/* $TITLE=RGB_matrix_check_coord() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                          Check order of coordinates (start and end Row and Column and re-order if required.
\* ============================================================================================================================================================= */
void RGB_matrix_check_coord(UINT8 *StartRow, UINT8 *StartColumn, UINT8 *EndRow, UINT8 *EndColumn)
{
  UINT8 TempValue;


  /* Extract / filter requests for standard window lines. */
  switch (*StartRow)
  {
    case (201):
      *StartRow = 1;
      *EndRow   = 7;
    break;

    case (202):
      *StartRow =  9;
      *EndRow   = 15;
    break;

    case (203):
      *StartRow = 20;
      *EndRow   = 29;
    break;
  }


  /* Re-order start and end points if required. */
  if (*EndRow < *StartRow)
  {
    TempValue  = *StartRow;
    *StartRow  = *EndRow;
    *EndRow    = TempValue;
  }

  if ((*StartColumn != 99) && (*EndColumn < *StartColumn))
  {
    TempValue    = *StartColumn;
    *StartColumn = *EndColumn;
    *EndColumn   = TempValue;
  }

  return;
}





/* $TITLE=RGB_matrix_clear_pixel() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                          Turn Off the pixels in the specified matrix area.
\* ============================================================================================================================================================= */
void RGB_matrix_clear_pixel(UINT64 *BufferPointer, UINT8 StartRow, UINT8 StartColumn, UINT8 EndRow, UINT8 EndColumn)
{
  UINT8  ColumnNumber;
  UINT8  RowNumber;


  /* Validate provided coordinates. */
  RGB_matrix_check_coord(&StartRow, &StartColumn, &EndRow, &EndColumn);

  /* Turn Off pixels in the specified matrix buffer area. */
  for (RowNumber = StartRow; RowNumber <= EndRow; ++RowNumber)
	{
    for (ColumnNumber = StartColumn; ColumnNumber <= EndColumn; ++ColumnNumber)
    {
      BufferPointer[RowNumber] &= (~(0x01ll << ColumnNumber));
    }
  }

  return;
}





/* $TITLE=RGB_matrix_cls() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                                        Clear LED matrix.
        NOTE: If there is an opened window with an "ACTION_DRAW" border, RGB_matrix_cls() will erase that border and it will have to be manuelly refreshed.
              Turning Off the brightness with PWM Level may be a better solution to consider in some cases.
\* ============================================================================================================================================================= */
void RGB_matrix_cls(UINT64 *FrameBuffer)
{
  /* Clear LED display matrix. */
  memset(FrameBuffer, 0x00, (MAX_ROWS * MAX_COLUMNS / 8));

  return;
}





/* $TITLE=RGB_matrix_device_init() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                      Initialize GPIO for most devices used in RGB matrix.
                                               NOTE: PWM for matrix brightness is initialized in its own routine.
\* ============================================================================================================================================================= */
void RGB_matrix_device_init(void)
{
  /* Initialize RGB matrix button devices and GPIOs. */
  gpio_init(BUTTON_SET_GPIO);
  gpio_init(BUTTON_DOWN_GPIO);
  gpio_init(BUTTON_UP_GPIO);

  gpio_set_dir(BUTTON_SET_GPIO,  GPIO_IN);
  gpio_set_dir(BUTTON_DOWN_GPIO, GPIO_IN);
  gpio_set_dir(BUTTON_UP_GPIO,   GPIO_IN);

  gpio_pull_up(BUTTON_SET_GPIO);
  gpio_pull_up(BUTTON_DOWN_GPIO);
  gpio_pull_up(BUTTON_UP_GPIO);


  /* Initialize on-board Pico LED gpio. */
  gpio_init(PICO_LED);
  gpio_set_dir(PICO_LED, GPIO_OUT);
  gpio_put(PICO_LED, 0);


  /* Initialize RGB matrix integrated active buzzer. */
#ifndef NO_SOUND
  gpio_init(BUZZER);
  gpio_set_dir(BUZZER, GPIO_OUT);
  gpio_put(BUZZER, 0);
#endif  // NO_SOUND


#ifdef REMOTE_SUPPORT
  /* Initialize infrared sensor gpio. */
  gpio_init(IR_RX);

  /* IR sensor will receive IR command from remote control. */
  gpio_set_dir(IR_RX, GPIO_IN);

  /* Line will remain at High level until a signal is received. */
  gpio_pull_up(IR_RX);
#endif  // REMOTE_SUPPORT



  /* Initialize Pico's UART0 used to send information to a terminal emulator for terminal menu and / or debugging purposes. */
  gpio_init(UART_TX_PIN);
  gpio_init(UART_RX_PIN);
  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
  uart_init(uart0, 921600);
  uart_set_format(uart0, 8, 1, UART_PARITY_NONE);



  /* LEDs matrix related devices and GPIOs. */
  gpio_init(R1);
  gpio_init(G1);
  gpio_init(B1);
  gpio_init(R2);
  gpio_init(G2);
  gpio_init(B2);

  gpio_init(A);
  gpio_init(B);
  gpio_init(C);
  gpio_init(D);
  gpio_init(E);

  gpio_init(CLK);

  gpio_init(STB);


  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                          Initialize PWM for clock display brightness.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  /* Set GPIO for RGB matrix brightness ("Output Enable"). */
  Pwm[PWM_ID_BRIGHTNESS].Gpio = OE;

  /* Initialize pulse-width-modulation (PWM) signals. */
  pwm_initialize();

  gpio_set_dir(R1,  GPIO_OUT);
  gpio_set_dir(G1,  GPIO_OUT);
  gpio_set_dir(B1,  GPIO_OUT);
  gpio_set_dir(R2,  GPIO_OUT);
  gpio_set_dir(G2,  GPIO_OUT);
  gpio_set_dir(B2,  GPIO_OUT);

  gpio_set_dir(A,   GPIO_OUT);
  gpio_set_dir(B,   GPIO_OUT);
  gpio_set_dir(C,   GPIO_OUT);
  gpio_set_dir(D,   GPIO_OUT);
  gpio_set_dir(E,   GPIO_OUT);
  gpio_set_dir(CLK, GPIO_OUT);
  gpio_set_dir(STB, GPIO_OUT);
  STB_LOW;
  CLK_LOW;

  int MaxLed = 64;

  int C12[16] = {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  int C13[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0};

  for (int l = 0; l < MaxLed; l++)
  {
    int y = l % 16;

    R1_LOW;
    G1_LOW;
    B1_LOW;
    R2_LOW;
    G2_LOW;
    B2_LOW;

    if (C12[y] == 1)
    {
      R1_HIGH;
      G1_HIGH;
      B1_HIGH;
      R2_HIGH;
      G2_HIGH;
      B2_HIGH;
    }

    if (l > MaxLed - 12)
    {
      STB_HIGH;
    }
    else
    {
      STB_LOW;
    }

    CLK_HIGH;
    sleep_us(2);
    CLK_LOW;
  }
  STB_LOW;
  CLK_LOW;

  /* Send Data to control register 12. */
  for (int l = 0; l < MaxLed; l++)
  {
    int y = l % 16;

    R1_LOW;
    G1_LOW;
    B1_LOW;
    R2_LOW;
    G2_LOW;
    B2_LOW;

    if (C13[y] == 1)
    {
      R1_HIGH;
      G1_HIGH;
      B1_HIGH;
      R2_HIGH;
      G2_HIGH;
      B2_HIGH;
    }

    if (l > MaxLed - 13)
    {
      STB_HIGH;
    }
    else
    {
      STB_LOW;
    }

    CLK_HIGH;
    sleep_us(2);
    CLK_LOW;
  }
  STB_LOW;
  CLK_LOW;


  /* Initialize analog-to-digital converter to read ambient light relative value and power supply info. */
  adc_init();

  adc_gpio_init(ADC_LIGHT_SENSOR);  // RGB-Matrix ambient light detector.
  adc_gpio_init(ADC_VCC);           // power supply voltage.

  adc_select_input(0);

  return;
}





/* $TITLE=RGB_matrix_display() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                   Display specified ASCII character, beginning at specified matrix location (pixel row and pixel column), using the specified font type.

                                  WARNING - WARNING - WARNING - WARNING - WARNING - WARNING - WARNING - WARNING - WARNING - WARNING
                                  - Debugging must be done only outside of callback functions, otherwise the firmware will crash.
                                    If required, disable the callback updating date and time and use this function from a test area.

                     NOTES:
                            1) Rows (in pixels) are numbered from 0 to MAX_ROWS and columns (in pixels) are numbered from 0 to MAX_COLUMNS.
                            2) StartRow and StartColumn correspond to the top-left position of the character to be displayed.
                            3) ASCII positions below 0x20 are used to generate special foreign language character bitmaps that do not exist
                               in the English language.
                            4) If FlagMore is != 0, it means that more characters are to be displayed to the right of current character...
                               in such a case, we make sure the next column to the right of current character is blank.
                            5) This function returns the start column for an eventual next character.
\* ============================================================================================================================================================= */
UINT8 RGB_matrix_display(UINT64 *DisplayBuffer, UINT8 StartRow, UINT8 StartColumn, UINT8 AsciiValue, UINT8 FontType, UINT8 FlagMore)
{
  UCHAR String[64];

  UINT8  CharHeight;
  UINT8  CharWidth;
  UINT8  ColumnNumber;
  UINT8  RowNumber;


  /* Initializations. */
  switch (FontType)
  {
    case (FONT_4x7):
      CharWidth = Font4x7[AsciiValue].Width;
      CharHeight = 7;
      if (AsciiValue > 0x7F) AsciiValue = 0;  // only first 128 ASCII characters are defined for 4x7 font.
    break;

    case (FONT_5x7):
      CharWidth = Font5x7[AsciiValue].Width;
      CharHeight = 7;
    break;

    case (FONT_8x10):
      CharWidth = Font8x10[AsciiValue].Width;
      CharHeight = 10;
      if (AsciiValue > 0x7F) AsciiValue = 0;  // only first 128 ASCII characters are defined for 8x10 font.
    break;
  }



  if (DebugBitMask & DEBUG_MATRIX)
  {
    uart_send(__LINE__, __func__, "AsciiValue: 0x%2.2X (%3u) ", AsciiValue, AsciiValue);

    /* If this is a printable ASCII character, display it to log file. */
    if (AsciiValue >= 0x20) uart_send(__LINE__, __func__, "- <%c> ", AsciiValue);

    uart_send(__LINE__, __func__, "- FlagMore: %2.2X (%u) (will be <0> or <!= 0> - not necessarily <1>)\r", FlagMore, FlagMore);
    uart_send(__LINE__, __func__, "Character StartRow: %2u     Character StartColumn: %2u\r", StartRow, StartColumn);

    /* Display bitmap for each character row. */
    for (RowNumber = 0; RowNumber < CharHeight; ++RowNumber)
    {
      switch (FontType)
      {
        case (FONT_4x7):
          if (RowNumber == 0) uart_send(__LINE__, __func__, "Character width: %u\r", Font4x7[AsciiValue].Width);
          util_uint64_to_binary_string((UINT64)Font4x7[AsciiValue].Row[RowNumber],   Font4x7[AsciiValue].Width, String);
          uart_send(__LINE__, __func__, "Row[%u]: 0x%2.2X   <%s>\r", RowNumber,      Font4x7[AsciiValue].Row[RowNumber], String);
        break;

        case (FONT_5x7):
          if (RowNumber == 0) uart_send(__LINE__, __func__, "Character width: %u\r", Font5x7[AsciiValue].Width);
          util_uint64_to_binary_string((UINT64)Font5x7[AsciiValue].Row[RowNumber],   Font5x7[AsciiValue].Width, String);
          uart_send(__LINE__, __func__, "Row[%u]: 0x%2.2X   <%s>\r", RowNumber,      Font5x7[AsciiValue].Row[RowNumber], String);
        break;

        case (FONT_8x10):
          if (RowNumber == 0) uart_send(__LINE__, __func__, "Character width: %u\r", Font8x10[AsciiValue].Width);
          util_uint64_to_binary_string((UINT64)Font8x10[AsciiValue].Row[RowNumber],  Font8x10[AsciiValue].Width, String);
          uart_send(__LINE__, __func__, "Row[%2u]: 0x%2.2X   <%s>\r", RowNumber,     Font8x10[AsciiValue].Row[RowNumber], String);
        break;
      }
    }
  }


  /* Check if we need to blank an extra column to the right of the character (because more characters will be displayed to the right).
     If there are more characters to come, simulate that the character is one more column than it actually is. */
  if (FlagMore) ++CharWidth;
  if (DebugBitMask & DEBUG_MATRIX) uart_send(__LINE__, __func__, "Adjusted character Width: %u\r", CharWidth);


  /* Set pixels in the target display buffer to match the bitmap of this ASCII character. */
  for (RowNumber = 0; RowNumber < CharHeight; ++RowNumber)
	{
    /// if (DebugBitMask & DEBUG_MATRIX) uart_send(__LINE__, __func__, "Row[%u] value: 0x%2.2X\r\r", RowNumber, Font5x7[AsciiValue].Row[RowNumber]);  /// font 5x7 hard-coded.

    for (ColumnNumber = 0; ColumnNumber < CharWidth; ++ColumnNumber)
    {
      if (DebugBitMask & DEBUG_MATRIX)
        uart_send(__LINE__, __func__, "StartColumn:  %3u     CharColumn:   %2u\r", StartColumn, ColumnNumber);


      switch (FontType)
      {
        case (FONT_4x7):
          if (Font4x7[AsciiValue].Row[RowNumber] & (0x01 << (Font4x7[AsciiValue].Width) - ColumnNumber - 1))
          {
            /* This pixel must be turned On. */
            DisplayBuffer[StartRow + RowNumber] |= (0x01ll << (StartColumn + ColumnNumber));
            if (DebugBitMask & DEBUG_MATRIX)
            {
              uart_send(__LINE__, __func__, "RowNumber: %2u     ColumnNumber: %2u   Pixel must be turned On\r", StartRow + RowNumber, StartColumn + ColumnNumber);
              uart_send(__LINE__, __func__, "Press <Enter> to continuer: ");
              input_string(String);
            }
          }
          else
          {
            /* This pixel must be turned Off. */
            DisplayBuffer[StartRow + RowNumber] &= ~(0x01ll << (StartColumn + ColumnNumber));
            if (DebugBitMask & DEBUG_MATRIX)
            {
              uart_send(__LINE__, __func__, "RowNumber: %2u     ColumnNumber: %2u   Pixel must be turned Off\r", StartRow + RowNumber, StartColumn + ColumnNumber);
              uart_send(__LINE__, __func__, "Press <Enter> to continuer: ");
              input_string(String);
            }
          }
        break;

        case (FONT_5x7):
          if (Font5x7[AsciiValue].Row[RowNumber] & (0x01 << (Font5x7[AsciiValue].Width) - ColumnNumber - 1))
          {
            /* This pixel must be turned On. */
            DisplayBuffer[StartRow + RowNumber] |= (0x01ll << (StartColumn + ColumnNumber));
            if (DebugBitMask & DEBUG_MATRIX)
            {
              uart_send(__LINE__, __func__, "RowNumber: %2u     ColumnNumber: %2u   Pixel must be turned On\r", StartRow + RowNumber, StartColumn + ColumnNumber);
              uart_send(__LINE__, __func__, "Press <Enter> to continuer: ");
              input_string(String);
            }
          }
          else
          {
            /* This pixel must be turned Off. */
            DisplayBuffer[StartRow + RowNumber] &= ~(0x01ll << (StartColumn + ColumnNumber));
            if (DebugBitMask & DEBUG_MATRIX)
            {
              uart_send(__LINE__, __func__, "RowNumber: %2u     ColumnNumber: %2u   Pixel must be turned Off\r", StartRow + RowNumber, StartColumn + ColumnNumber);
              uart_send(__LINE__, __func__, "Press <Enter> to continuer: ");
              input_string(String);
            }
          }
        break;

        case (FONT_8x10):
          if (Font8x10[AsciiValue].Row[RowNumber] & (0x01 << (Font8x10[AsciiValue].Width) - ColumnNumber - 1))
          {
            /* This pixel must be turned On. */
            DisplayBuffer[StartRow + RowNumber] |= (0x01ll << (StartColumn + ColumnNumber));
            if (DebugBitMask & DEBUG_MATRIX)
            {
              uart_send(__LINE__, __func__, "RowNumber: %2u     ColumnNumber: %2u   Pixel must be turned On\r", StartRow + RowNumber, StartColumn + ColumnNumber);
              uart_send(__LINE__, __func__, "Press <Enter> to continuer: ");
              input_string(String);
            }
          }
          else
          {
            /* This pixel must be turned Off. */
            DisplayBuffer[StartRow + RowNumber] &= ~(0x01ll << (StartColumn + ColumnNumber));
            if (DebugBitMask & DEBUG_MATRIX)
            {
              uart_send(__LINE__, __func__, "RowNumber: %2u     ColumnNumber: %2u   Pixel must be turned Off\r", StartRow + RowNumber, StartColumn + ColumnNumber);
              uart_send(__LINE__, __func__, "Press <Enter> to continuer: ");
              input_string(String);
            }
          }
        break;
      }
    }
  }

  return StartColumn + ColumnNumber;
}





/* $TITLE=RGB_matrix_display_time() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                               Display data and time on LED matrix (usual matrix display).
                                    NOTE: Matrix scanning will be stopped while data is sent through CDC USB
                                          It's Ok to do it for debugging purposes for short periods of time but it should
                                          not be done continuously because LEDs are then overbright (overvoltage).
                                          This will shorten the life span of the LEDs.
\* ============================================================================================================================================================= */
void RGB_matrix_display_time(void)
{
  static UINT8 CurrentColor;

  UINT8 FlagLocalDebug = FLAG_OFF;

  UINT16 Loop1UInt16;
  UINT16 PwmLevel;
  UINT16 TargetDays;

  struct tm TempTime;


  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                              Read time from real-time clock IC (DS3231).
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (FlagLocalDebug) printf("%u   Before ds3231_get_time()\r", __LINE__);
  ds3231_get_time(&CurrentTime);



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                             Update day-of-week on first line of WIN_DATE.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  /* Update date only if WIN_DATE is the WinTop active window and if no scrolling is active. */
  if (FlagLocalDebug) printf("%4u   Before updating day-of-week\r", __LINE__);
  if (Window[WIN_DATE].FlagTopScroll == FLAG_OFF)
  {
    if (WinTop == WIN_DATE)
    {
      /* Update day-of-week only if WIN_DATE is the WinTop active window. */
      ///// PwmLevel = Pwm[PWM_ID_BRIGHTNESS].Level;  // keep track of original PWM level on entry.
      ///// pwm_set_level(PWM_ID_BRIGHTNESS, 2000);   // blank LED matrix before matrix update.
      /// pwm_on_off(PWM_ID_BRIGHTNESS, FLAG_OFF);

      /// critical_section_enter_blocking(&ThreadLock);

      CLK_HIGH;

      ///// win_part_cls(WIN_DATE, 201, 201);
      win_printf(WIN_DATE, 201, 99, FONT_5x7, "%s", DayName[CurrentTime.DayOfWeek]);

      CLK_LOW;

      /// critical_section_exit(&ThreadLock);

      /* Restore original PWM level when done. */
      ///// pwm_set_level(PWM_ID_BRIGHTNESS, PwmLevel);
      /// pwm_on_off(PWM_ID_BRIGHTNESS, FLAG_ON);
    }
  }



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                               Update date on second line of WIN_DATE.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (FlagLocalDebug) printf("%4u   Before updating date\r", __LINE__);
  if (Window[WIN_DATE].FlagMidScroll == FLAG_OFF)
  {
    if (WinMid == WIN_DATE)
    {
      /* Update date only if WIN_DATE is the WinMid active window. */
      ///// PwmLevel = Pwm[PWM_ID_BRIGHTNESS].Level;  // keep track of original PWM level on entry.
      ///// pwm_set_level(PWM_ID_BRIGHTNESS, 2000);   // blank LED matrix before matrix update.
      /// pwm_on_off(PWM_ID_BRIGHTNESS, FLAG_OFF);

      /// critical_section_enter_blocking(&ThreadLock);

      CLK_HIGH;

      ///// win_part_cls(WIN_DATE, 202, 202);

      /* If golden age mode is On, alternate between date and period of the day. */
      if ((FlashConfig1.FlagGoldenAge) && ((CurrentTime.Second % 10) < 5))
      {
        /* Display period of the day. */
        if ((CurrentTime.Hour >= FlashConfig1.GoldenMorningStart)   && (CurrentTime.Hour < FlashConfig1.GoldenAfternoonStart))
          win_printf(WIN_DATE, 202, 99, FONT_5x7, "%s", DayPeriod[MORNING]);
        if ((CurrentTime.Hour >= FlashConfig1.GoldenAfternoonStart) && (CurrentTime.Hour < FlashConfig1.GoldenEveningStart))
          win_printf(WIN_DATE, 202, 99, FONT_5x7, "%s", DayPeriod[AFTERNOON]);
        if ((CurrentTime.Hour >= FlashConfig1.GoldenEveningStart)   && (CurrentTime.Hour < FlashConfig1.GoldenNightStart))
          win_printf(WIN_DATE, 202, 99, FONT_5x7, "%s", DayPeriod[EVENING]);
        if ((CurrentTime.Hour >= FlashConfig1.GoldenNightStart)     || (CurrentTime.Hour < FlashConfig1.GoldenMorningStart))
          win_printf(WIN_DATE, 202, 99, FONT_5x7, "%s", DayPeriod[NIGHT]);
      }
      else
      {
        /* If we were in golden age mode and we revert to "normal" mode, let's reset current color. */
        if (CurrentColor != 0) CurrentColor = 0;

        /* Display date on WIN_DATE window while centering it on the lines. */
        win_printf(WIN_DATE, 202, 99, FONT_5x7, "%2.2u-%3s-%4.4u", CurrentTime.DayOfMonth, ShortMonth[CurrentTime.Month], CurrentTime.Year);
      }

      CLK_LOW;

      /// critical_section_exit(&ThreadLock);

      /* Restore original PWM level when done. */
      ///// pwm_set_level(PWM_ID_BRIGHTNESS, PwmLevel);
      /// pwm_on_off(PWM_ID_BRIGHTNESS, FLAG_ON);
    }
  }



  /* --------------------------------------------------------------------------------------------------------------------------- *\
             Update Alarms indicators and Alarm target days indicators on unused top and bottom rows of WIN_DATE window.
             NOTE: Even if we are scrolling text, alarm indicators and target alarm day indicators should not interfere.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (FlagLocalDebug) printf("%4u   Before updating alarm and day-of-week indicators\r", __LINE__);
  if (WinTop == WIN_DATE)
  {
    /* Find all days-of-week that are target days for all active alarms. */
    TargetDays = 0;  // bitmask of all days that are target days in one or more alarms.
    for (Loop1UInt16 = 0; Loop1UInt16 < MAX_ALARMS; ++Loop1UInt16)
    {
      if (FlashConfig1.Alarm[Loop1UInt16].FlagStatus)
        TargetDays |= FlashConfig1.Alarm[Loop1UInt16].DayMask;
    }

    /* --------------- Update active / inactive alarm indicators. --------------- */
    if (FlashConfig1.FlagDisplayAlarms)
    {
      for (Loop1UInt16 = 0; Loop1UInt16 < MAX_ALARMS; ++Loop1UInt16)
      {
        RGB_matrix_set_pixel(FrameBuffer, 0, (3 + (Loop1UInt16 * 7)), 0, (4 + (Loop1UInt16 * 7)));
        if (FlashConfig1.Alarm[Loop1UInt16].FlagStatus)
          RGB_matrix_set_color(0, (3 + (Loop1UInt16 * 7)), 0, (4 + (Loop1UInt16 * 7)), GREEN);  // active alarms have green indicators.
        else
          RGB_matrix_set_color(0, (3 + (Loop1UInt16 * 7)), 0, (4 + (Loop1UInt16 * 7)), RED);    // inactive alarms have red indicators.
      }
    }

    /* --------------- Update active / inactive target alarm days. --------------- */
    if (FlashConfig1.FlagDisplayAlarmDays)
    {
      for (Loop1UInt16 = 0; Loop1UInt16 < 7; ++Loop1UInt16)
      {
        RGB_matrix_set_pixel(FrameBuffer, Window[WIN_DATE].EndRow, (Loop1UInt16 * 10), Window[WIN_DATE].EndRow, (3 + (Loop1UInt16 * 10)));
        if (TargetDays & (1 << Loop1UInt16))
          RGB_matrix_set_color(Window[WIN_DATE].EndRow, (Loop1UInt16 * 10), Window[WIN_DATE].EndRow, (3 + (Loop1UInt16 * 10)), GREEN);  // days-of-week that have an active alarm have a green indicator.
        else
          RGB_matrix_set_color(Window[WIN_DATE].EndRow, (Loop1UInt16 * 10), Window[WIN_DATE].EndRow, (3 + (Loop1UInt16 * 10)), RED);    // days-of-week that don't have any active alarm have a red indicator.
      }
    }
  }



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                          Update time on bottom half of RGB matrix LED display.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  if (FlagLocalDebug) printf("%u   Before updating time\r", __LINE__);
  if (Window[WIN_TIME].FlagBotScroll == FLAG_OFF)
  {
    /* Display time on WIN_TIME window. */
    if (WinBot == WIN_TIME)
    {
      /* Update time only if WIN_TIME is the WinBot active window. */
      ///// PwmLevel = Pwm[PWM_ID_BRIGHTNESS].Level;  // keep track of original PWM level on entry.
      ///// pwm_set_level(PWM_ID_BRIGHTNESS, 2000);   // blank LED matrix before matrix update.
      /// pwm_on_off(PWM_ID_BRIGHTNESS, FLAG_OFF);

      /// critical_section_enter_blocking(&ThreadLock);

      CLK_HIGH;

      /* Update time. */
      win_printf(WIN_TIME, 203, 99, FONT_8x10, "%2.2u:%2.2u:%2.2u", CurrentTime.Hour, CurrentTime.Minute, CurrentTime.Second);


      /* win_part_cls() will erase all the WIN_TIME window, except the border. If we want the "endless loop" pilot
         inside this window area, we need to proceed another way. */
      // win_part_cls(WIN_TIME, 203, 203);

      /* Update time while blinking the two double-dots. */
      // if (CurrentTime.Second % 2)
      //   win_printf(WIN_TIME, 203, 99, FONT_8x10, "%2.2u %2.2u %2.2u", CurrentTime.Hour, CurrentTime.Minute, CurrentTime.Second);
      // else
      //   win_printf(WIN_TIME, 203, 99, FONT_8x10, "%2.2u:%2.2u:%2.2u", CurrentTime.Hour, CurrentTime.Minute, CurrentTime.Second);

      /* If we are in golden age mode, set the color according to the period of the day (day or night) while leaving the WIN_TIME box,
         alarm indicators and target days indicators in their original colors. */
      if (FlashConfig1.FlagGoldenAge)
      {
        if ((CurrentTime.Hour >= FlashConfig1.GoldenMorningStart) && (CurrentTime.Hour < FlashConfig1.GoldenNightStart))
        {
          if (CurrentColor != YELLOW)
          {
            CurrentColor = YELLOW;

            /* Set colors of WIN_TIME window without altering alarm indicators and alarm target day indicators. */
            RGB_matrix_set_color(Window[WIN_DATE].StartRow + 1, Window[WIN_DATE].StartColumn, Window[WIN_DATE].EndRow - 1, Window[WIN_DATE].EndColumn, CurrentColor);
            /// win_set_color(WIN_DATE, CurrentColor, Window[WIN_DATE].BoxColor);  // WIN_DATE, except outside box.

            /* Set colors for WIN_TIME window without changing border color. */
            RGB_matrix_set_color(Window[WIN_TIME].StartRow + 1, Window[WIN_TIME].StartColumn + 1, Window[WIN_TIME].EndRow - 1, Window[WIN_TIME].EndColumn - 1, CurrentColor);
            /// win_set_color(WIN_TIME, CurrentColor, Window[WIN_TIME].BoxColor);
          }
        }
        else
        {
          if (CurrentColor != BLUE)
          {
            CurrentColor = BLUE;

            /* Set colors of WIN_TIME window without altering alarm indicators and alarm target day indicators. */
            RGB_matrix_set_color(Window[WIN_DATE].StartRow + 1, Window[WIN_DATE].StartColumn, Window[WIN_DATE].EndRow - 1, Window[WIN_DATE].EndColumn, CurrentColor);
            /// win_set_color(WIN_DATE, CurrentColor, Window[WIN_DATE].BoxColor);  // WIN_DATE, except outside box.

            /* Set colors for WIN_TIME window without changing border color. */
            RGB_matrix_set_color(Window[WIN_TIME].StartRow + 1, Window[WIN_TIME].StartColumn + 1, Window[WIN_TIME].EndRow - 1, Window[WIN_TIME].EndColumn - 1, CurrentColor);
            /// win_set_color(WIN_TIME, CurrentColor, Window[WIN_TIME].BoxColor);
          }
        }

        /* Draw border for WIN_TIME window. */
        /// RGB_matrix_box(Window[WIN_TIME].StartRow, Window[WIN_TIME].StartColumn, Window[WIN_TIME].EndRow, Window[WIN_TIME].EndColumn, RED, ACTION_DRAW);
      }

      CLK_LOW;

      /// critical_section_exit(&ThreadLock);

      /* Restore original PWM level when done. */
      ///// pwm_set_level(PWM_ID_BRIGHTNESS, PwmLevel);
      /// pwm_on_off(PWM_ID_BRIGHTNESS, FLAG_ON);
    }
  }

  if (FlagLocalDebug) printf("%4u   Exiting RGB_matrix_display_time()\r", __LINE__);

  return;
}





/* $TITLE=RGB_matrix_integrity_check() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                               LED matrix device integrity check
                                Turn On and Off each line of the LED matrix and for each color and combination of colors.
              NOTE: FlagTerminal indicates if caller is a terminal emulator or remote control. If remote control, the check is performed unattended.
\* ============================================================================================================================================================= */
void RGB_matrix_integrity_check(UINT8 FlagTerminal)
{
  UCHAR String[31];

  UINT8 Color;
  UINT8 ColumnNumber;
  UINT8 PassNumber;
  UINT8 RowNumber;
  UINT8 SectorNumber;


  printf("\r\r\r");
  if (FlagTerminal) uart_send(__LINE__, __func__, "%4u   Entering RGB Matrix device integrity check.\r", __LINE__);

  if (FlagTerminal)
  {
    if (stdio_usb_connected())
    {
      uart_send(__LINE__, __func__, "Press <Enter> to erase LED matrix: ");
      input_string(String);
    }
  }

  win_open(WIN_TEST, FLAG_OFF);
  win_printf(WIN_TEST,  2, 99, FONT_5x7, "Device");
  win_printf(WIN_TEST, 10, 99, FONT_5x7, "integrity");
  win_printf(WIN_TEST, 18, 99, FONT_5x7, "check");


  /* -------------------- Top down test -------------------- */
  /* Turn On every line of LED matrix and for each color combination (Red - Green - Blue - Yellow - Magenta - Cyan - White). */
  if (FlagTerminal)
  {
    if (stdio_usb_connected())
    {
      uart_send(__LINE__, __func__, "Press <Enter> to begin top-down integrity check: ");
      input_string(String);
      uart_send(__LINE__, __func__, "Running top-down integrity check.\r");
    }
  }
  else
    sleep_ms(2000);

  RGB_matrix_cls(FrameBuffer);


  /* Test all seven color combinations. */
  for (PassNumber = 0; PassNumber < 7; ++PassNumber)
  {
    if (FlagTerminal) uart_send(__LINE__, __func__, "Proceeding with top-down integrity check, pass number %u\r", PassNumber);
    switch (PassNumber)
    {
      case (0):
        Color = RED;
      break;

      case (1):
        Color = GREEN;
      break;

      case (2):
        Color = BLUE;
      break;

      case (3):
        Color = RED + GREEN;
      break;

      case (4):
        Color = RED + BLUE;
      break;

      case (5):
        Color = GREEN + BLUE;
      break;

      case (6):
      default:
        Color = RED + GREEN + BLUE;
      break;
    }

    /* Test the LED matrix with all color compinations above. */
    RGB_matrix_set_color(0, 0, 31, 63, Color);

    /* Sequentially turn On all 32 rows of 64 pixels. */
    /* NOTE: Pixel color has already been set above. */
    for (RowNumber = 0; RowNumber < MAX_ROWS; ++RowNumber)
    {
      FrameBuffer[RowNumber] = 0xFFFFFFFFFFFFFFFFll;  // turn On all 64 pixels of this row.

      if (RowNumber > 7)
      {
        /* When reaching 8th row (or more), turn Off the "oldest" row of pixels so that electronic will not be overloaded.
           This way, only 8 rows will be turned On at any single time. */
        FrameBuffer[RowNumber - 8] = 0x00ll;
      }
      sleep_ms(100);  // make a quick pause after each line.
    }

    /* When the 32nd row has been turned On, complete turning Off last 8 rows (row 24 to 31). */
    for (RowNumber = 24; RowNumber < MAX_ROWS; ++RowNumber)
    {
      FrameBuffer[RowNumber] = 0x00ll;
      sleep_ms(100);
    }

    if (FlagTerminal)
    {
      if (stdio_usb_connected())
      {
        uart_send(__LINE__, __func__, "Press <Enter> to proceed with next pass or <ESC> to exit this test: ");
        input_string(String);
      }
    }
    else
      sleep_ms(1000);

    RGB_matrix_cls(FrameBuffer);
    if (String[0] == 27) break;  // user pressed <ESC>, get out of <for> loop and exit top-down integrity check.
    if (IrBuffer[0] != BUTTON_NONE) break;  // user pressed a remote control button, get out of <for> loop and exit top-down integrity check.
  }
  if (FlagTerminal) printf("\r\r\r");



  /* ------------------ Left-to-right test ------------------ */
  /* Turn On every column of LED matrix and for each color combination (Red - Green - Blue - Yellow - Magenta - Cyan - White). */
  if (FlagTerminal)
  {
    if (stdio_usb_connected())
    {
      uart_send(__LINE__, __func__, "Press <Enter> to begin left-to-right integrity check: ");
      input_string(String);
      uart_send(__LINE__, __func__, "Running left-to-right integrity check.\r");
    }
  }
  else
    sleep_ms(2000);

  /* Test all seven color combinations. */
  for (PassNumber = 0; PassNumber < 7; ++PassNumber)
  {
    if (FlagTerminal) uart_send(__LINE__, __func__, "Proceeding with left-to-right check, pass number %u\r", PassNumber);
    switch (PassNumber)
    {
      case (0):
        Color = RED;
      break;

      case (1):
        Color = GREEN;
      break;

      case (2):
        Color = BLUE;
      break;

      case (3):
        Color = RED + GREEN;
      break;

      case (4):
        Color = RED + BLUE;
      break;

      case (5):
        Color = GREEN+ BLUE;
      break;

      case (6):
      default:
        Color = RED + GREEN + BLUE;
      break;
    }

    /* Test the matrix with all color compinations above. */
    RGB_matrix_set_color(0, 0, 31, 63, Color);

    /* Sequentially turn On each column of pixels. */
    for (ColumnNumber = 0; ColumnNumber < MAX_COLUMNS; ++ColumnNumber)
    {
      for (RowNumber = 0; RowNumber < MAX_ROWS; ++RowNumber)
      {
        FrameBuffer[RowNumber] |= (0x01ll << ColumnNumber);

        /* When reaching 8th column, turn Off the oldest columns of pixels so that electronic is not overloaded. */
        if (ColumnNumber > 7)
        {
          FrameBuffer[RowNumber] &= ~(0x01ll << (ColumnNumber - 8));
        }
      }
      sleep_ms(100);
    }

    /* Complete turning Off last pixel columns. */
    for (ColumnNumber = 56; ColumnNumber < MAX_COLUMNS; ++ColumnNumber)
    {
      for (RowNumber = 0; RowNumber < MAX_ROWS; ++RowNumber)
      {
        FrameBuffer[RowNumber] &= ~(0x01ll << ColumnNumber);
      }
      sleep_ms(100);
    }

    if (FlagTerminal)
    {
      if (stdio_usb_connected())
      {
        uart_send(__LINE__, __func__, "Press <Enter> to proceed with next pass or <ESC> to exit this test: ");
        input_string(String);
      }
    }
    else
      sleep_ms(1000);

    RGB_matrix_cls(FrameBuffer);
    if (String[0] == 27) break;  // user pressed <ESC> exit left-to-right integrity check.
    if (IrBuffer[0] != BUTTON_NONE) break;  // user pressed a remote control button, exit left-to-right integrity check.
  }
  if (FlagTerminal) printf("\r\r\r");

  win_close(WIN_TEST);

  return;
}





/* $TITLE=RGB_matrix_pixel_length() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                          Calculate the length of the string supplied when using the font type specified.
\* ============================================================================================================================================================= */
UINT8 RGB_matrix_pixel_length(UINT8 FontType, UCHAR *Format, ...)
{
  UCHAR String[256];

  UINT8 Loop1UInt8;
  UINT8 TotalColumns;

  va_list argp;


  /* Transfer the text to display to local variable <String>. */
  va_start(argp, Format);
  vsnprintf(String, sizeof(String), Format, argp);
  va_end(argp);


  /* Compute total number of pixels width for the whole string, adding one blank pixel column after each character. */
  TotalColumns = 0;
  for (Loop1UInt8 = 0; String[Loop1UInt8]; ++Loop1UInt8)
  {
    switch (FontType)
    {
      case (FONT_4x7):
        TotalColumns += Font4x7[String[Loop1UInt8]].Width + 1;
      break;

      default:
      case (FONT_5x7):
        TotalColumns += Font5x7[String[Loop1UInt8]].Width + 1;
      break;

      case (FONT_8x10):
        TotalColumns += Font8x10[String[Loop1UInt8]].Width + 1;
      break;
    }
  }

  return (TotalColumns - 1);  // no need for a blank pixel column after last character.
}





/* $TITLE=RGB_matrix_printf() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                          Display specified string, beginning at the specified pixel row and specified pixel column.
                                          NOTE: This function uses 5x7 variable-width character set.
                                          NOTE: If PixelColumn specified is 99, string will be centered on the line.
\* ============================================================================================================================================================= */
UINT8 RGB_matrix_printf(UINT64 *DisplayBuffer, UINT8 StartRow, UINT8 StartColumn, UINT8 FontType, UCHAR *Format, ...)
{
  UCHAR String[256];

  UINT8 CurrentColumn;
  UINT8 Loop1UInt8;
  UINT8 TotalColumns;

  va_list argp;


  /* Transfer the text to display to local variable <String>. */
  va_start(argp, Format);
  vsnprintf(String, sizeof(String), Format, argp);
  va_end(argp);


  if (StartColumn != 99)
  {
    /* No request to center text on the line... Start column will be as specified. */
    CurrentColumn = StartColumn;
  }
  else
  {
    /* Compute total pixel length of the string to be displayed in order to center the text on matrix display. */
    TotalColumns = RGB_matrix_pixel_length(FontType, String);

    /* Find start column depending on string length. */
    if (TotalColumns > MAX_COLUMNS)
    {
      /* The string exceeds the width of the display matrix. */
      CurrentColumn = 0;  // the string is too long, display what we can...
      TotalColumns  = 0;  // shorten the string so that it fits inside the RGB matrix.
      for (Loop1UInt8 = 0; String[Loop1UInt8]; ++Loop1UInt8)
      {
        /// TotalColumns +=
      }
    }
    else
      CurrentColumn = (64 - TotalColumns) / 2;
  }


  /* Display string. */
  for (Loop1UInt8 = 0; String[Loop1UInt8]; ++Loop1UInt8)
  {
    CurrentColumn = RGB_matrix_display(DisplayBuffer, StartRow, CurrentColumn, String[Loop1UInt8], FontType, String[Loop1UInt8 + 1]);
  }

  return CurrentColumn;
}





/* $TITLE=RGB_matrix_scroll() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                       Scroll the specified rows one pixel to the left and manage the scroll ASCII buffer.
\* ============================================================================================================================================================= */
void RGB_matrix_scroll(UINT8 ScrollNumber)
{
  static UINT8 CharWidth;

  UINT8 FlagLocalDebug = FLAG_OFF;
  UINT8 Loop1UInt8;
  UINT8 RowNumber;


  /* Make sure ActiveScroll pointer is valid. */
  if (ActiveScroll[ScrollNumber] == 0x00l)
  {
    printf("************************************************************* Invalid ActiveScroll pointer: 0x%p\r", ActiveScroll[Loop1UInt8]);
    return;
  }


  /* Scroll one pixel to the left on the LED matrix. */
  for (RowNumber = ActiveScroll[ScrollNumber]->StartRow; RowNumber <= ActiveScroll[ScrollNumber]->EndRow; ++RowNumber)
  {
    ///// if (FlagLocalDebug) printf("1) %u, %u to %u\r", ScrollNumber, ActiveScroll[ScrollNumber]->StartRow, ActiveScroll[ScrollNumber]->EndRow);

    FrameBuffer[RowNumber] >>= 1;

    /* Handle box border if there is one persistent on this window (ACTION_DRAW). */
    /*** to be completed ***/
    /// if (Window[WindowNumber].LastBoxState == ACTION_DRAW)
    /// {
    ///   FrameBuffer[RowNumber] |=  (0x1ll);                       // redraw left box border.
    ///   FrameBuffer[RowNumber] &= ~(0x1ll << (MAX_COLUMNS - 2));  // erase the right border that has just been scrolled.
    /// }


    /* We just scrolled one pixel left on LED matrix...
       if there are more pixels to be scrolled in the intermediate bitmap scroll buffer, transfer next pixel column. */
    if (ActiveScroll[ScrollNumber]->PixelCountBuffer)
    {
      /* Display message below only once. */
      /// if (RowNumber == ActiveScroll[ScrollNumber]->StartRow) printf("1) More to scroll: %u\r", ActiveScroll[ScrollNumber]->PixelCountBuffer);

      /* No matter how many "current" pixels were remaining to be scrolled before in the FrameBuffer,
         if we recharged the Bitmap Scroll Buffer, also recharge the count of current pixel scrolls. */
      ActiveScroll[ScrollNumber]->PixelCountCurrent = MAX_COLUMNS;

      ///// if (ActiveScroll[ScrollNumber]->BitmapBuffer[RowNumber] & (0x01ll << (CharWidth - ActiveScroll[ScrollNumber]->PixelCountBuffer)))
      if (ActiveScroll[ScrollNumber]->BitmapBuffer[RowNumber] & (0x01ll << (CharWidth - ActiveScroll[ScrollNumber]->PixelCountBuffer)))
      {
        FrameBuffer[RowNumber] |= (0x01ll << (MAX_COLUMNS - 1));
        /// printf("1a) %2u 0x%2.2llX %u %u Adding <1>\r", RowNumber, ActiveScroll[ScrollNumber]->BitmapBuffer[RowNumber], CharWidth, ActiveScroll[ScrollNumber]->PixelCountBuffer);
      }
      else
      {
        FrameBuffer[RowNumber] &= ~(0x01ll << (MAX_COLUMNS - 1));
        /// printf("1b) %2u 0x%2.2llX %u %u Adding <0>\r", RowNumber, ActiveScroll[ScrollNumber]->BitmapBuffer[RowNumber], CharWidth, ActiveScroll[ScrollNumber]->PixelCountBuffer);
      }

      /* When reaching the last row, one whole column of the Bitmap scroll buffer has been transferred to the FrameBuffer. */
      if (RowNumber == ActiveScroll[ScrollNumber]->EndRow)
      {
        --ActiveScroll[ScrollNumber]->PixelCountBuffer;
        /// printf("3) PixelCount low to %u\r", ActiveScroll[ScrollNumber]->PixelCountBuffer);
      }
    }
    else
    {
      /* No more data in the Bitmap scroll buffer, remaining scrolls of the actual FrameBuffer is then decremented by one. */
      if (RowNumber == ActiveScroll[ScrollNumber]->EndRow)
      {
        --ActiveScroll[ScrollNumber]->PixelCountCurrent;
        /// printf("2) NO more pixel %u\r", ActiveScroll[ScrollNumber]->PixelCountBuffer);
      }
    }
  }


  /// printf("10) Count current %u\r", ActiveScroll[ScrollNumber]->PixelCountCurrent);



  if (ActiveScroll[ScrollNumber]->PixelCountBuffer == 0)
  {
    /* If there are no more pixel to scroll in the bitmask scroll buffer, check if there are more characters in the ASCII scroll buffer. */
    /// printf("4) Buffer count zero: %u\r", ActiveScroll[ScrollNumber]->PixelCountBuffer);
    if (strlen(&ActiveScroll[ScrollNumber]->Message[ActiveScroll[ScrollNumber]->AsciiBufferPointer]) != 0)
    {
      /* There are more ASCII characters to scroll in the ASCII scroll buffer, convert the next one from ASCII to bitmask in the bitmask scroll buffer. */
      /// printf("5) More ASCII %u\r", strlen(&ActiveScroll[ScrollNumber]->Message[ActiveScroll[ScrollNumber]->AsciiBufferPointer]));

      /* Fill-up the Bitmask Scroll Buffer with next ASCII characters to scroll. */
      if (FlagLocalDebug) printf("6) Txfr %c\r", ActiveScroll[ScrollNumber]->Message[ActiveScroll[ScrollNumber]->AsciiBufferPointer]);

      if (ActiveScroll[ScrollNumber]->FontType == FONT_8x10)
        CharWidth = RGB_matrix_display(ActiveScroll[ScrollNumber]->BitmapBuffer, ActiveScroll[ScrollNumber]->StartRow, 0, ActiveScroll[ScrollNumber]->Message[ActiveScroll[ScrollNumber]->AsciiBufferPointer], FONT_8x10, ActiveScroll[ScrollNumber]->Message[ActiveScroll[ScrollNumber]->AsciiBufferPointer + 1]);
      else
        CharWidth = RGB_matrix_display(ActiveScroll[ScrollNumber]->BitmapBuffer, ActiveScroll[ScrollNumber]->StartRow, 0, ActiveScroll[ScrollNumber]->Message[ActiveScroll[ScrollNumber]->AsciiBufferPointer], FONT_5x7, ActiveScroll[ScrollNumber]->Message[ActiveScroll[ScrollNumber]->AsciiBufferPointer + 1]);

      ActiveScroll[ScrollNumber]->PixelCountBuffer = CharWidth;  // more pixels to be scrolled in bitmap scroll buffer.
      ++ActiveScroll[ScrollNumber]->AsciiBufferPointer;          // point to next character in ASCII scroll buffer.

      /// printf("7) Ascii ptr %u PixelCount %u after txfr\r", ActiveScroll[ScrollNumber]->AsciiBufferPointer, ActiveScroll[ScrollNumber]->PixelCountBuffer);

      /***
      for (RowNumber = ActiveScroll[ScrollNumber]->StartRow; RowNumber <= ActiveScroll[ScrollNumber]->EndRow; ++RowNumber)
        printf("8) [%u]: 0x%2.2X\r", RowNumber, ActiveScroll[ScrollNumber]->BitmapBuffer[RowNumber]);
      ***/
    }
    else
    {
      /// printf("9) No more ASCII %u\r", strlen(&ActiveScroll[ScrollNumber]->Message[ActiveScroll[ScrollNumber]->AsciiBufferPointer]));
      /* No more character in the ASCII scroll buffer... Check if more than one cycle have been requested. */
      if (ActiveScroll[ScrollNumber]->ScrollTimes > 0)
      {
        /// printf("9) Remain scrolls %u\r", ActiveScroll[ScrollNumber]->ScrollTimes);
        /* More than one cycle have been requested and count is not down to zero. Trigger another cycle. */
        --ActiveScroll[ScrollNumber]->ScrollTimes;
        ActiveScroll[ScrollNumber]->AsciiBufferPointer = 0;  // reset ASCII buffer pointer to zero to trigger a new cycle.
      }
      else
      {
        /// printf("9)No more scrolls %u\r", ActiveScroll[ScrollNumber]->ScrollTimes);
        /* Turning Off current scroll when done. */
        if (ActiveScroll[ScrollNumber]->PixelCountCurrent == 0)
        {
          /// if (DebugBitMask & DEBUG_SCROLL) printf("Free up scroll memory.\r");
          win_scroll_off(ScrollNumber);
        }
      }
    }
  }

  return;
}





/* $TITLE=RGB_matrix_set_color() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                          Set matrix display color for the specified area.
\* ============================================================================================================================================================= */
void RGB_matrix_set_color(UINT8 StartRow, UINT8 StartColumn, UINT8 EndRow, UINT8 EndColumn, UINT8 Color)
{
  UINT8 ColumnNumber;
  UINT8 RowNumber;


  /* Validate provided coordinates. */
  RGB_matrix_check_coord(&StartRow, &StartColumn, &EndRow, &EndColumn);

  for (RowNumber = StartRow; RowNumber <= EndRow; ++RowNumber)
  {
    for (ColumnNumber = StartColumn; ColumnNumber <= EndColumn; ++ColumnNumber)
    {
      /// uart_send(__LINE__, __func__, "RowNumber: %2u   ColumnNumber: %2u\r", RowNumber, ColumnNumber);
      if (RowNumber < 16)
        DisplayRGB[RowNumber][ColumnNumber] = ((DisplayRGB[RowNumber][ColumnNumber] & 0xF0) | Color);
      else
        DisplayRGB[RowNumber - 16][ColumnNumber] = ((DisplayRGB[RowNumber - 16][ColumnNumber] & 0x0F) | (Color << 4));
    }
  }

  return;
}





/* $TITLE=RGB_matrix_set_pixel() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                Turn On the pixels in the specified matrix area of the specified buffer.
\* ============================================================================================================================================================= */
void RGB_matrix_set_pixel(UINT64 *BufferPointer, UINT8 StartRow, UINT8 StartColumn, UINT8 EndRow, UINT8 EndColumn)
{
  UINT8  ColumnNumber;
  UINT8  RowNumber;


  /* Validate provided coordinates. */
  RGB_matrix_check_coord(&StartRow, &StartColumn, &EndRow, &EndColumn);

  /* Turn On pixels in the specified matrix buffer area. */
  for (RowNumber = StartRow; RowNumber <= EndRow; ++RowNumber)
	{
    for (ColumnNumber = StartColumn; ColumnNumber <= EndColumn; ++ColumnNumber)
    {
      BufferPointer[RowNumber] |= (0x01ll << ColumnNumber);
    }
  }

  return;
}





/* $TITLE=RGB_matrix_update() */
/* $PAGE */
/* ============================================================================================================================================================= *\

\* ============================================================================================================================================================= */
void RGB_matrix_update(UINT64 *FrameBuffer)
{
  /// UINT8  ColumnNumber;
  uint_fast32_t ColumnNumber;
  UINT8 *Framebuffer;  ///

  UINT16 PwmLevel;


  Framebuffer = (UINT8 *)FrameBuffer;  ///

  /* Simultaneously scan first 16 rows (top half) and next 16 rows (bottom half) and then, start again (RowScan goes from 0 to 15 and then, start over again). */
  ++RowScan;
  if (RowScan >= HALF_ROWS) RowScan = 0;

	FlagFrameBufferBusy = FLAG_ON;  // flag indicating that the FrameBuffer is currently being updated.

  /* Blank LED matrix before updating. */
  PwmLevel = Pwm[PWM_ID_BRIGHTNESS].Level;  // keep track of original PWM level on entry.
  pwm_set_level(PWM_ID_BRIGHTNESS, 2000);



  /// critical_section_enter_blocking(&ThreadLock);



  /* Scan all columns of the LED matrix. */
  for (ColumnNumber = 0; ColumnNumber < Matrix_COLS_BYTE; ++ColumnNumber)
  {
    /// printf("%3u   %3u   %2u\r", RowScan, (RowScan + HALF_ROWS), ColumnNumber % 8);  /// should go with the for loop in next paragraph.
    /// printf("%3u   %3u   %2u\r", RowScan * Matrix_COLS_BYTE + ColumnNumber, (RowScan + HALF_ROWS) * Matrix_COLS_BYTE + ColumnNumber, ColumnNumber);
    RGB_matrix_write_data(Framebuffer[RowScan * Matrix_COLS_BYTE + ColumnNumber], Framebuffer[(RowScan + HALF_ROWS) * Matrix_COLS_BYTE + ColumnNumber], ColumnNumber);
  }


  /* Scan all columns of the LED matrix. */
  /// for (ColumnNumber = 0; ColumnNumber < MAX_COLUMNS; ++ColumnNumber)
  //// {
  ////   printf("%3u   %3u   %2u\r", RowScan, (RowScan + HALF_ROWS), ColumnNumber % 8);
  ////   RGB_matrix_write_data(FrameBuffer[RowScan], Framebuffer[RowScan + HALF_ROWS], ColumnNumber % 8);
  //// }


  /* Successively scan all rows of the LED matrix. */
  if (RowScan & 0x01) A_HIGH; else A_LOW;
  if (RowScan & 0x02) B_HIGH; else B_LOW;
  if (RowScan & 0x04) C_HIGH; else C_LOW;
  if (RowScan & 0x08) D_HIGH; else D_LOW;
  if (RowScan & 0x10) E_HIGH; else E_LOW;


  /* Latch this value. */
  STB_HIGH;
  NOP;
  STB_LOW;


  /// critical_section_exit(&ThreadLock);



  FlagFrameBufferBusy = FLAG_OFF;  // we're done with FrameBuffer update.


  /* Restore original PWM level when done. */
  pwm_set_level(PWM_ID_BRIGHTNESS, PwmLevel);

  return;




#if 0
void RGBMatrixDeviceFlush(uint8_t *buf)
{
  uint8_t i;

  CS_cnt ++;
  if (CS_cnt > (Matrix_ROWS_SHOW - 1))
  {
    CS_cnt = 0;
  }

  UpdateVideoMemory = 1;

  OE_HIGH;

  for (i = 0; i < (Matrix_COLS_BYTE); ++i)
  {
    RGBMatrixWriteData(buf[CS_cnt * Matrix_COLS_BYTE + i], buf[(CS_cnt + Matrix_ROWS_SHOW) * Matrix_COLS_BYTE + i], i);
  }


  if (CS_cnt & 0x01) A_HIGH; else A_LOW;
  if (CS_cnt & 0x02) B_HIGH; else B_LOW;
  if (CS_cnt & 0x04) C_HIGH; else C_LOW;
  if (CS_cnt & 0x08) D_HIGH; else D_LOW;
  if (CS_cnt & 0x10) E_HIGH; else E_LOW;


  STB_HIGH;
  // sleep_us(1);

  STB_LOW;

  UpdateVideoMemory = 0;

  OE_LOW;

#ifndef TRUE
  if (FlagAutoBrightness != FLAG_OFF)
  {
    FlagAutoBrightness = FLAG_ON;
  }
  else
  {
    OE_LOW;
  }
#endif
}
#endif  // 0
}





//* $TITLE=RGB_matrix_write_data() */
/* $PAGE */
/* ============================================================================================================================================================= *\

\* ============================================================================================================================================================= */
void RGB_matrix_write_data(UINT8 MatrixTop, UINT8 MatrixBottom, UCHAR DisplayRGBCount)
{
  UINT8  j;
	UINT8 rgb;


  for (j = 0; j < 8; ++j)
  {
		rgb = DisplayRGB[RowScan][(8 * DisplayRGBCount) + j];
    /// printf("%u - %u\r", RowScan, (8 * DisplayRGBCount) + j);
    CLK_LOW;

    R1_LOW;
    G1_LOW;
    B1_LOW;

    R2_LOW;
    G2_LOW;
    B2_LOW;

    if (MatrixTop & 0x01)
    {
			if (rgb & 0x04) R1_HIGH;
      if (rgb & 0x02) G1_HIGH;
      if (rgb & 0x01) B1_HIGH;
    }

    if (MatrixBottom & 0x01)
    {
      if (rgb & 0x40) R2_HIGH;
      if (rgb & 0x20) G2_HIGH;
      if (rgb & 0x10) B2_HIGH;
    }

    MatrixTop    >>= 1;
    MatrixBottom >>= 1;

    CLK_HIGH;
  }

  return;
}





/* $TITLE=set_auto_brightness() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                         Manage ambient light history for automatic brightness.
\* ============================================================================================================================================================= */
void set_auto_brightness(void)
{
  static UINT16 Counter;
  UINT16 CurrentLightValue;
  UINT16 LightRange;
  UINT16 Loop1UInt16;
  UINT16 PwmHiLimit;
  UINT16 PwmLoLimit;
  UINT16 PwmLevel;

  UINT64 TotalValues;

  float  PwmRange;


  CurrentLightValue     = get_light_value();
  AmbientLight[Counter] = CurrentLightValue;  // keep history of ambient light for hysteresis.

  if ((Counter % 5) == 0)
  {
    /* Every 'BRIGHTNESS_HYSTERESIS_SECONDS', compute average ambient light value for the last hysteresis period. */
    TotalValues = 0ll;
    for (Loop1UInt16 = 0; Loop1UInt16 < BRIGHTNESS_HYSTERESIS_SECONDS; ++Loop1UInt16)
      TotalValues += AmbientLight[Loop1UInt16];

    AverageAmbientLight = (TotalValues / BRIGHTNESS_HYSTERESIS_SECONDS);
  }

  ++Counter;
  if (Counter >= BRIGHTNESS_HYSTERESIS_SECONDS) Counter = 0;


  /* --------------------------------------------------------------------------------------------------------------------------- *\
                           Manage LED matrix auto brightness when PWM brightness control option is turned On.

                    NOTES: Ambient light will usually go from 300 (almost no light) to 3500 (very bright light).
                           This gives a range of about 3200 levels (3500 - 300) to cover the whole range of
                           light intensities ("PWM_LOWEST_LEVEL" to "PWM_HIGHEST_LEVEL").
  \* --------------------------------------------------------------------------------------------------------------------------- */
  while ((CurrentTime.Second % 5) == 0)
  {
    /* Compute PWM lo and hi limits based on user preference for lowest and highest brightness limits set in device configuration. */
    /* Default brightness limits are from 1 to 1000 (range of 999) in device configuration and default PWM levels are from 1999 to 1300 (range of 699). */
    /* So, for each brightness degree that we change, PWM level will be changed by 0.7 (699 / 999). */
    PwmHiLimit = (PWM_HI_LIMIT - ((UINT16)(FlashConfig1.BrightnessLoLimit * 0.7)));  // removed <+0.5> at the end
    PwmLoLimit = (PWM_LO_LIMIT + ((UINT16)((1000 - FlashConfig1.BrightnessHiLimit) * 0.7)));

    LightRange = LIGHT_HI_LIMIT - LIGHT_LO_LIMIT;

    if (AverageAmbientLight <= LIGHT_LO_LIMIT)
    {
      /* If average ambient light is lower than the lowest limit, brightness is automatically set to its lowest limit (= highest PWM level). */
      pwm_set_level(PWM_ID_BRIGHTNESS, PwmHiLimit);
      if (DebugBitMask & DEBUG_BRIGHTNESS)
      {
        uart_send(__LINE__, __func__, "\r");
        /// uart_send(__LINE__, __func__, "Ambient light is lower that low limit - Instantaneous ambient light value: %4u   AverageAmbientLight: %4u   Level: %3u\r", CurrentLightValue, AverageAmbientLight, PwmHiLimit);
        uart_send(__LINE__, __func__, "PWM Level: %4u\r", PwmHiLimit);
      }
      break;
    }

    if (AverageAmbientLight >= LIGHT_HI_LIMIT)
    {
      /* If average ambient light is higher than the highest limit, brightness is automatically set to its highest limit (= lowest PWM level). */
      pwm_set_level(PWM_ID_BRIGHTNESS, PwmLoLimit);
      if (DebugBitMask & DEBUG_BRIGHTNESS)
      {
        uart_send(__LINE__, __func__, "\r");
        /// uart_send(__LINE__, __func__, "Ambient light is higher than high limit - Instantaneous ambient light value: %4u   AverageAmbientLight: %4u   Level: %3u\r", CurrentLightValue, AverageAmbientLight, PwmLoLimit);
        uart_send(__LINE__, __func__, "PWM Level: %4u\r", PwmLoLimit);
      }
      break;
    }

    if ((AverageAmbientLight > LIGHT_LO_LIMIT) && (AverageAmbientLight < LIGHT_HI_LIMIT))
    {
      /* If average ambient light is between the lowest and highest limits set by developer, brightness is automatically adjusted proportionally. */
      PwmRange = PwmHiLimit - PwmLoLimit;
      PwmLevel = PwmHiLimit - (UINT16)((AverageAmbientLight - LIGHT_LO_LIMIT) * (PwmRange / LIGHT_RANGE));
      pwm_set_level(PWM_ID_BRIGHTNESS, PwmLevel);
      break;
    }
  }

  return;
}





/* $TITLE=software_reset() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                        Restart the RGB Matrix Firmware by software reset.
\* ============================================================================================================================================================= */
void software_reset(void)
{
  UINT32 Loop1UInt32;


  /* Log the restart. */
  printf("\r\r\r\r\r");
  printf("%2.2u-%s-%4.4u - %2.2u:%2.2u:%2.2u - RGB Matrix restarted by watchdog...\r\r\r\r\r", CurrentTime.DayOfMonth, ShortMonth[CurrentTime.Month], CurrentTime.Year, CurrentTime.Hour, CurrentTime.Minute, CurrentTime.Second);

  /* Cancel 1-second callback that is calling the watchdog. */
  /* (cancelling the 1-second timer while we are executing from this thread would crash the Firmware). */
  cancel_repeating_timer(&Handle1000MSecTimer);

  /* Open a test window to prevent clock refresh on LED matrix
  /// win_open(WIN_TEST, FLAG_OFF);

  /* Clear LED display since matrix scan will stop during reboot. */
  RGB_matrix_cls(FrameBuffer);
  pwm_set_level(PWM_ID_BRIGHTNESS, 2000);

  /* Keep track of interrupt mask and disable interrupts during flash writing. */
  /// InterruptMask = save_and_disable_interrupts();

  /* Indicate that restart has been triggered by watchdog. */
  /// FlashConfig1.WatchdogFlag = FLAG_ON;
  /// ++FlashConfig1.WatchdogCounter;

  /* Make sure these values are saved to flash. */
  /// flash_write(FLASH_CONFIG1_OFFSET, (UINT8 *)FlashConfig1.Version, sizeof(FlashConfig1));
  /// flash_check_config(1);

  /* Restore original interrupt mask when done. */
  /// restore_interrupts(InterruptMask);

  /* Add a small delay - without using sleep_ms() - to make sure PWM has enough time to adjust the proper level. */
  for (Loop1UInt32 = 0; Loop1UInt32 < 100000; ++Loop1UInt32);

  /// watchdog_reboot(0, SRAM_END, 0);
  /// watchdog_reboot(0, 0x20042000, 0);
  /// while (1);

  /// AIRCR_Register = 0x5FA0004;  // this one can't be used for the Pico-RGB-Matrix since it doesn't reset core 1.

  watchdog_enable(1, 1);

  return;
}





/* $TITLE=term_alarm_setup() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                                 Terminal submenu for alarm setup.
\* ============================================================================================================================================================= */
void term_alarm_setup(void)
{
  UCHAR DayMask[16];
  UCHAR String[61];

  UINT8 AlarmNumber;
  UINT8 DayNumber;
  UINT8 Loop1UInt8;
  UINT8 Loop2UInt8;

  UINT16 ScrollDuration;


  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                          Flag to display alarm indicators on RGB matrix display.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  while (1)
  {
    if (FlashConfig1.FlagDisplayAlarms)
      printf("FlagDisplayAlarms currently On\r");
    else
      printf("FlagDisplayAlarms currently Off\r");
    printf("Press <c> to change this setting\r");
    printf("<Enter> to keep it this way\r");
    printf("<ESC> to exit alarm setup: ");

    input_string(String);
    if (String[0] == 0x0D) break;
    if (String[0] == 27)   return;
    if ((String[0] == 'C') || (String[0] == 'c'))
      FlashConfig1.FlagDisplayAlarms ^= 0x01;

    if (FlashConfig1.FlagDisplayAlarms == FLAG_OFF)
    {
      /* FlagDisplayAlarms has just been turned Off, restore original status of LED matrix. */
      RGB_matrix_box(Window[WIN_DATE].StartRow, Window[WIN_DATE].StartColumn, Window[WIN_DATE].StartRow, Window[WIN_DATE].EndColumn, Window[WIN_DATE].InsideColor, ACTION_ERASE);
    }
  }
  printf("\r\r");



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                      Flag to display target day indicators for all active alarms.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  while (1)
  {
    if (FlashConfig1.FlagDisplayAlarmDays)
      printf("FlagDisplayAlarmDays currently On\r");
    else
      printf("FlagDisplayAlarmDays currently Off\r");
    printf("Press <c> to change this setting\r");
    printf("<Enter> to keep it this way\r");
    printf("<ESC> to exit alarm setup: ");

    input_string(String);
    if (String[0] == 0x0D) break;
    if (String[0] == 27)   return;
    if ((String[0] == 'C') || (String[0] == 'c'))
      FlashConfig1.FlagDisplayAlarmDays ^= 0x01;

    if (FlashConfig1.FlagDisplayAlarmDays == FLAG_OFF)
    {
      /* FlagDisplayAlarmDays has just been turned Off, restore original status of LED matrix. */
      RGB_matrix_box(Window[WIN_DATE].EndRow, Window[WIN_DATE].StartColumn, Window[WIN_DATE].EndRow, Window[WIN_DATE].EndColumn, Window[WIN_DATE].InsideColor, ACTION_ERASE);
    }
  }
  printf("\r\r");



  while (1)
  {
    /* Display current alarm parameters for all alarms. */
    for (Loop1UInt8 = 0; Loop1UInt8 < MAX_ALARMS; ++Loop1UInt8)
    {
      display_alarm(Loop1UInt8);
    }



    /* --------------------------------------------------------------------------------------------------------------------------- *\
                                             Selection of an alarm number to review / change.
    \* --------------------------------------------------------------------------------------------------------------------------- */
    printf("\r\r");
    printf("Enter alarm number you want to change or <ESC> to exit alarm setup: ");
    input_string(String);
    if (String[0] == 27) return;
    AlarmNumber = atoi(String) - 1;
    while (AlarmNumber >= MAX_ALARMS)
    {
      printf("Invalid alarm. Enter alarm number (1 to %u) or <ESC> to exit alarm setup: ", MAX_ALARMS);
      input_string(String);
      if (String[0] == 27) return;
      AlarmNumber = atoi(String) - 1;
    }

    printf("\r\r\r");
    display_alarm(AlarmNumber);



   /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                          Alarm status (On / Off).
    \* --------------------------------------------------------------------------------------------------------------------------- */
    while (1)
    {
      if (FlashConfig1.Alarm[AlarmNumber].FlagStatus)
        printf("Alarm status is currently On\r");
      else
        printf("Alarm status is currently Off\r");
      printf("Press <c> to change this setting\r");
      printf("<Enter> to keep it this way\r");
      printf("<ESC> to exit alarm setup: ");

      input_string(String);
      if (String[0] == 0x0D) break;
      if (String[0] == 27)   return;
      if ((String[0] == 'C') || (String[0] == 'c'))
        FlashConfig1.Alarm[AlarmNumber].FlagStatus ^= 0x01;
    }
    printf("\r\r");



    /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                              Alarm hour.
    \* --------------------------------------------------------------------------------------------------------------------------- */
    while (1)
    {
      printf("Current hour for alarm is: %2u\r", FlashConfig1.Alarm[AlarmNumber].Hour);
      printf("Enter new value to change this setting (0 to 23)\r");
      printf("<Enter> to keep current setting\r");
      printf("<ESC> to exit alarm setup: ");

      input_string(String);
      if (String[0] == 0x0D) break;
      if (String[0] == 27)   return;
      FlashConfig1.Alarm[AlarmNumber].Hour = atoi(String);
      if (FlashConfig1.Alarm[AlarmNumber].Hour > 23) FlashConfig1.Alarm[AlarmNumber].Hour = 23;
    }
    printf("\r\r");



    /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                             Alarm minute.
    \* --------------------------------------------------------------------------------------------------------------------------- */
    while (1)
    {
      printf("Current minute for alarm is: %2.2u\r", FlashConfig1.Alarm[AlarmNumber].Minute);
      printf("Enter new value to change this setting (0 to 59)\r");
      printf("<Enter> to keep current setup\r");
      printf("<ESC> to exit alarm setup: ");

      input_string(String);
      if (String[0] == 0x0D) break;
      if (String[0] == 27)   return;
      FlashConfig1.Alarm[AlarmNumber].Minute = atoi(String);
      if (FlashConfig1.Alarm[AlarmNumber].Minute > 59) FlashConfig1.Alarm[AlarmNumber].Minute = 59;
    }
    printf("\r\r");



    /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                           Alarm days-of-week.
    \* --------------------------------------------------------------------------------------------------------------------------- */
    /* Display number and short name of all days-of-week. */
    printf("=======================================================================\r");
    printf("             Days of week are the following:\r");
    for (Loop1UInt8 = 0; Loop1UInt8 < 7; ++Loop1UInt8)
      printf("%2u) %s   ", Loop1UInt8, ShortDay[Loop1UInt8]);
    printf("\r");
    printf("=======================================================================\r");


    while (1)
    {
      /* Display days-of-week. */
      util_uint64_to_binary_string(FlashConfig1.Alarm[AlarmNumber].DayMask, 8, DayMask);

      printf("Current days selected for alarm %u are:\r", AlarmNumber);
      sprintf(String, "Alarm[%2.2u].DayMask:      %s     (0x%2.2X) ", AlarmNumber, DayMask, FlashConfig1.Alarm[AlarmNumber].DayMask);

      /* Display short name of all days-of-week selected for this alarms (those selected in the bit mask). */
      for (Loop1UInt8 = 0; Loop1UInt8 < 7; ++Loop1UInt8)
      {
        if (FlashConfig1.Alarm[AlarmNumber].DayMask & (1 << Loop1UInt8))
          sprintf(&String[strlen(String)], "%s ", ShortDay[Loop1UInt8]);
      }
      strcat(String, "\r");
      printf("%s", String);


      printf("Enter day number to add or removed (0 to 6)\r");
      printf("<Enter> to keep current setup\r");
      printf("<ESC> to exit alarm setup: ");

      input_string(String);
      if (String[0] == 0x0D) break;
      if (String[0] == 27)   return;
      DayNumber = atoi(String);
      while (DayNumber > 6)
      {
        printf("Invalid day number.\r");
        printf("Enter day number (0 to 6) or <ESC> to exit alarm setup: ");
        input_string(String);
        if (String[0] == 0x0D) break;
        if (String[0] == 27)   return;
        DayNumber = atoi(String);
      }

      if (FlashConfig1.Alarm[AlarmNumber].DayMask & (1 << DayNumber))
        FlashConfig1.Alarm[AlarmNumber].DayMask &= ~(1 << DayNumber);
      else
        FlashConfig1.Alarm[AlarmNumber].DayMask |= (1 << DayNumber);
    }
    printf("\r\r");



    /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                             Alarm message.
    \* --------------------------------------------------------------------------------------------------------------------------- */
    while (1)
    {
      /* Display alarm message for this alarm. */
      printf("Alarm[%2.2u].Message:\r", AlarmNumber);
      printf("<", FlashConfig1.Alarm[AlarmNumber].Message);
      for (Loop1UInt8 = 0; Loop1UInt8 < sizeof(FlashConfig1.Alarm[AlarmNumber].Message); ++Loop1UInt8)
      {
        if  (FlashConfig1.Alarm[AlarmNumber].Message[Loop1UInt8] == 0x00) break;  // get out of <for> loop when reaching end-of-string.
        if ((FlashConfig1.Alarm[AlarmNumber].Message[Loop1UInt8] >= 0x20) && (FlashConfig1.Alarm[AlarmNumber].Message[Loop1UInt8] <= 0x7E))
          printf("%c", FlashConfig1.Alarm[AlarmNumber].Message[Loop1UInt8]);
        else
          printf("?");
      }
      printf(">\r\r");

      printf("Enter new message for this alarm <Enter> to keep current message or <ESC> to exit alarm setup (maximum %u characters):\r");
      /// printf("<Enter> to keep current message\r");
      /// printf("or <ESC> to exit alarm setup\r");
      /// printf("(maximum %u characters):\r", sizeof(FlashConfig1.Alarm[AlarmNumber].Message));

      /* Display a template for maximum alarm message length. */
      printf("[");
      for (Loop1UInt8 = 0; Loop1UInt8 < sizeof(FlashConfig1.Alarm[AlarmNumber].Message); ++Loop1UInt8) printf("-");
      printf("]\r ");

      input_string(String);
      if (String[0] == 0x0D) break;
      if (String[0] == 27)   return;
      while (strlen(String) > sizeof(FlashConfig1.Alarm[AlarmNumber].Message))
      {
        printf("Message too long (by %u characters)\r", strlen(String) - sizeof(FlashConfig1.Alarm[AlarmNumber].Message));
        printf("Enter new message for this alarm <Enter> to keep current message or <ESC> to exit alarm setup (maximum %u characters):\r");
        /// printf("<Enter> to keep current message\r");
        /// printf("or <ESC> to exit alarm setup\r");
        /// printf("(maximum %u characters):\r", sizeof(FlashConfig1.Alarm[AlarmNumber].Message));

        /* Display a template for maximum alarm message length. */
        printf("[");
        for (Loop1UInt8 = 0; Loop1UInt8 < sizeof(FlashConfig1.Alarm[AlarmNumber].Message); ++Loop1UInt8) printf("-");
        printf("]\r ");

        input_string(String);
        if (String[0] == 0x0D) break;
        if (String[0] == 27)   return;
      }
      if (String[0] == 0x0D) break;
      sprintf(FlashConfig1.Alarm[AlarmNumber].Message, String);
    }



    /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                            Set number of scrolls
                          When the alarm is triggered, it will scroll the defined message this number of times.
       WARNING: The scroll must be completed before the "repeat rate" resume. Otherwise, the new message will be added at the end
                of the first message, the third one added at the end of the first two, and so on and so forth, and will end-up as
                a strange and probably unwanted behavior...
    \* --------------------------------------------------------------------------------------------------------------------------- */
    while (1)
    {
      printf("\r\r");
      printf("When triggered, alarm will scroll the message entered above %u times.\r", FlashConfig1.Alarm[AlarmNumber].NumberOfScrolls);

      ScrollDuration = (UINT16)(4 + (strlen(FlashConfig1.Alarm[AlarmNumber].Message) * 0.33));
      if ((ScrollDuration * FlashConfig1.Alarm[AlarmNumber].NumberOfScrolls) > FlashConfig1.Alarm[AlarmNumber].RepeatPeriod)
      {
        printf("*** WARNING *** WARNING *** WARNING *** WARNING *** WARNING *** WARNING\r");
        printf("Current message is made of %u characters and should take about %u seconds to scroll once.\r", strlen(FlashConfig1.Alarm[AlarmNumber].Message), ScrollDuration);
        printf("You asked to scroll it %u times at every <RepeatPeriod> (%u seconds), for a total of about %u seconds\r", FlashConfig1.Alarm[AlarmNumber].NumberOfScrolls, FlashConfig1.Alarm[AlarmNumber].RepeatPeriod, FlashConfig1.Alarm[AlarmNumber].NumberOfScrolls * ScrollDuration);
        printf("It may not have completed its scrolling when reaching the repeat period and the message queue will increase each time...\r");
        printf("...which is probably not what you want. You should either lower the number of scrolls and / or increase the repeat period.\r\r");
      }
      printf("Enter new value to change this setting (0 to 10)\r");
      printf("<Enter> to keep current setting\r");
      printf("<ESC> to exit alarm setup: ");

      input_string(String);
      if (String[0] == 0x0D) break;
      if (String[0] == 27)   return;
      FlashConfig1.Alarm[AlarmNumber].NumberOfScrolls = atoi(String);
      if (FlashConfig1.Alarm[AlarmNumber].NumberOfScrolls > 10) FlashConfig1.Alarm[AlarmNumber].NumberOfScrolls = 10;
    }
    printf("\r\r");



    /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                            Set number of beeps.
                            When the alarm is triggered, it will ring this number of "beeps" for each "ring".
    \* --------------------------------------------------------------------------------------------------------------------------- */
    while (1)
    {
      printf("\r\r");
      printf("When triggered, alarm will sound %u beeps for each ring.\r", FlashConfig1.Alarm[AlarmNumber].NumberOfBeeps);
      printf("Enter new value to change this setting (0 to 10)\r");
      printf("<Enter> to keep current setting\r");
      printf("<ESC> to exit alarm setup: ");

      input_string(String);
      if (String[0] == 0x0D) break;
      if (String[0] == 27)   return;
      FlashConfig1.Alarm[AlarmNumber].NumberOfBeeps = atoi(String);
      if (FlashConfig1.Alarm[AlarmNumber].NumberOfBeeps > 10) FlashConfig1.Alarm[AlarmNumber].NumberOfBeeps = 10;
    }
    printf("\r\r");



    /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                            Set beep duration
                              When the alarm is triggered, the length of each beep will be this number of msec".
    \* --------------------------------------------------------------------------------------------------------------------------- */
    while (1)
    {
      printf("\r\r");
      printf("When triggered, the length of each beep will be %u msec.\r", FlashConfig1.Alarm[AlarmNumber].BeepMSec);
      printf("Enter new value to change this setting (50 to 2000 -> 2000 means 2 seconds)\r");
      printf("<Enter> to keep current setting\r");
      printf("<ESC> to exit alarm setup: ");

      input_string(String);
      if (String[0] == 0x0D) break;
      if (String[0] == 27)   return;
      FlashConfig1.Alarm[AlarmNumber].BeepMSec = atoi(String);
      if (FlashConfig1.Alarm[AlarmNumber].BeepMSec > 2000) FlashConfig1.Alarm[AlarmNumber].BeepMSec = 2000;
    }
    printf("\r\r");



    /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                            Set "beeps" repeat period.
                    When the alarm is triggered, it will ring the specified number of beeps every so many seconds.
    \* --------------------------------------------------------------------------------------------------------------------------- */
    while (1)
    {
      printf("\r\r");
      printf("Alarm beeps will repeat every %u seconds\r", FlashConfig1.Alarm[AlarmNumber].RepeatPeriod);
      ScrollDuration = (UINT16)((4 + (strlen(FlashConfig1.Alarm[AlarmNumber].Message) * 0.33)) * FlashConfig1.Alarm[AlarmNumber].NumberOfScrolls);
      if (ScrollDuration > FlashConfig1.Alarm[AlarmNumber].RepeatPeriod)
      {
        printf("*** WARNING *** WARNING *** WARNING *** WARNING *** WARNING *** WARNING\r");
        printf("Current message is made of %u characters and should take about %s to scroll once, and\r", strlen(FlashConfig1.Alarm[AlarmNumber].Message), (UINT16)((4 + (strlen(FlashConfig1.Alarm[AlarmNumber].Message) * 0.33))));
        printf("you asked to scroll it %u times at every <ring time>, for a total of about %u seconds\r", FlashConfig1.Alarm[AlarmNumber].NumberOfScrolls, FlashConfig1.Alarm[AlarmNumber].NumberOfScrolls * ScrollDuration);
        printf("It may not have complete its scrolling when reaching the repeat period and the message queue will increase each time\r");
        printf("which is probaly not what you want. You should either lower the number of scrolls and / or increase the repeat period.\r\r");
      }
      printf("Enter new value to change this setting (0 to 3600 -> 3600 seconds represents 1 hours)\r");
      printf("<Enter> to keep current setting\r");
      printf("<ESC> to exit alarm setup: ");

      input_string(String);
      if (String[0] == 0x0D) break;
      if (String[0] == 27)   return;
      FlashConfig1.Alarm[AlarmNumber].RepeatPeriod = atoi(String);
      if (FlashConfig1.Alarm[AlarmNumber].RepeatPeriod > 3600) FlashConfig1.Alarm[AlarmNumber].RepeatPeriod = 3600;
    }
    printf("\r\r");



    /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                          Set global alarm duration.
                  When the alarm is triggered, it will ring this total number of seconds if it is not shut off by user.
    \* --------------------------------------------------------------------------------------------------------------------------- */
    while (1)
    {
      printf("\r\r");
      printf("Alarm duration is now set to %u seconds (%3.0f minutes)\r", FlashConfig1.Alarm[AlarmNumber].RingDuration, FlashConfig1.Alarm[AlarmNumber].RingDuration / 60.0);
      printf("Enter new value to change this setting (0 to 60000 -> 60000 represents 10 hours)\r");
      printf("<Enter> to keep current setting\r");
      printf("<ESC> to exit alarm setup: ");

      input_string(String);
      if (String[0] == 0x0D) break;
      if (String[0] == 27)   return;
      FlashConfig1.Alarm[AlarmNumber].RingDuration = atoi(String);
      if (FlashConfig1.Alarm[AlarmNumber].RingDuration > 60000) FlashConfig1.Alarm[AlarmNumber].RingDuration = 60000;
      if (FlashConfig1.Alarm[AlarmNumber].RingDuration < FlashConfig1.Alarm[AlarmNumber].RepeatPeriod)
      {
        queue_add_active(300, 3);
        printf("*** WARNING *** WARNING *** WARNING *** WARNING *** WARNING *** WARNING\r");
        printf("Global / total alarm duration is %u seconds, but repeat period exceeds this time.\r", FlashConfig1.Alarm[AlarmNumber].RingDuration);
        printf("When it will be time to ring again, global time will already be elapsed...\r");
        printf("Make sure this is what you want or consult the User Guide for more information.\r");
        printf("Press <Enter> to continue: ");
        input_string(String);
      }
    }
    printf("\r\r");
  }

  return;
}





/* $TITLE=term_auto_scroll() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                              Terminal submenu for auto scroll setup.
\* ============================================================================================================================================================= */
void term_auto_scroll(void)
{
  UCHAR String[65];

  UINT8 AutoScrollNumber;
  UINT8 ItemNumber;

  UINT16 Loop1UInt16;
  UINT16 StartFunctionInfo;
  UINT16 EndFunctionInfo;


  /* --------------------------------------------------------------------------------------------------------------------------- *\
        Find the first and the last FunctionNumber of category "info". This category is the only one eligible for auto-scroll
                                              since it does not require any user operation.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  StartFunctionInfo = 9999;  // assign invalid value on entry.

  /* Find the first and last FunctionNumber whose FunctionId are in the category "info". */
  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_FUNCTIONS; ++Loop1UInt16)
  {
    if ((Function[Loop1UInt16].Id >= FUNCTION_INFO_START) && (Function[Loop1UInt16].Id <= FUNCTION_INFO_END))
    {
      if (StartFunctionInfo == 9999) StartFunctionInfo = Loop1UInt16;  // just found the first FunctionNumber for an "info" function.
      EndFunctionInfo = Loop1UInt16;  // last FunctionNumber assigned will be the last FunctionNumber for an "info" function.
    }
  }


  while (1)
  {
    printf("\r\r\r\r");
    printf(" ---------------------------------- Auto scroll setup ----------------------------------\r\r");
    printf("This section allows you to trigger one or more automatic scrolling on the RGB-Matrix\r");
    printf("You first decide the period at which the scrolling will be triggered (in minutes) and then,\r");
    printf("you select which functions you want to scroll when the auto scroll time period has elapsed.\r\r");


    /* Display current auto scrolls configuration. */
    for (Loop1UInt16 = 0; Loop1UInt16 < MAX_AUTO_SCROLLS; ++Loop1UInt16)
    {
      display_auto_scroll(Loop1UInt16);
    }



    /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                 Selection of auto scroll number to change.
    \* --------------------------------------------------------------------------------------------------------------------------- */
    printf("Enter auto scroll number you want to change or <ESC> to exit auto scroll setup: ");
    input_string(String);
    if (String[0] == 27) break;
    AutoScrollNumber = atoi(String) - 1;
    while (AutoScrollNumber > MAX_AUTO_SCROLLS)
    {
      printf("Invalid auto scroll number.\r");
      printf("Enter auto scroll number (1 to %u) or <ESC> to exit auto scroll setup: ", MAX_AUTO_SCROLLS);
      input_string(String);
      if (String[0] == 27)
      {
        printf(("Updating auto scrolls...\r\r"));
        sleep_ms(3000);
        return;
      }
      AutoScrollNumber = atoi(String) - 1;
    }
    printf("\r\r\r");

    /* Display selected auto-scroll number. */
    display_auto_scroll(AutoScrollNumber);



    /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                           Auto scroll period
    \* --------------------------------------------------------------------------------------------------------------------------- */
    while (1)
    {
      printf("Automatic scroll period is defined as %u minutes.\r", FlashConfig1.AutoScroll[AutoScrollNumber].Period);
      printf("NOTE: A period of 0 minute will disable this auto-scroll.\r");
      printf("Enter new value to change this setting (1 to 10080) - (10080 minutes corresponds to 1 week)\r");
      printf("<Enter> to keep current value\r");
      printf("<ESC> to exit auto-scroll setup: ");

      input_string(String);
      if (String[0] == 0x0D) break;
      if (String[0] == 27)   return;
      FlashConfig1.AutoScroll[AutoScrollNumber].Period = atoi(String);
      if (FlashConfig1.AutoScroll[AutoScrollNumber].Period > 10080) FlashConfig1.AutoScroll[AutoScrollNumber].Period = 10080;
    }
    printf("\r\r");



    /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                        Selection of items to scroll.
    \* --------------------------------------------------------------------------------------------------------------------------- */
    while (1)
    {
      /* Display current status of this auto-scroll. */
      display_auto_scroll(AutoScrollNumber);

      /* Display all valid function ID selected for scrolling (functions "info"). */
      printf("Functions that you may select for scrolling:\r");
      printf("--------------------------------------------\r\r");
      printf("Function   Function     Function\r");
      printf("   ID       Number        Name\r");
      printf("---------------------------------------\r");
      for (Loop1UInt16 = StartFunctionInfo; Loop1UInt16 <= EndFunctionInfo; ++Loop1UInt16)
        printf("   %3u      (%3u)     %s\r", Function[Loop1UInt16].Id, Function[Loop1UInt16].Number, Function[Loop1UInt16].Name);
      printf("\r\r");

      printf("First, enter the item number that you want to change in the auto-scroll list at the top of the screen\r");
      printf("<ESC> to exit auto-scroll setup: ");
      input_string(String);
      ItemNumber = atoi(String);
      if (String[0] == 27)   return;
      while ((ItemNumber < 1) || (ItemNumber > MAX_ITEMS))
      {
        printf("Invalid choice. You must select between 1 and %u\r", MAX_ITEMS);
        printf("Please re-enter a valid number\r");
        printf("<ESC> to exit auto-scroll setup: ");
        input_string(String);
        ItemNumber = atoi(String);
        if (String[0] == 27)   return;
      }
      printf("\r");



      printf("Now, enter the function ID for the item number that you selected %u (see list above)\r", ItemNumber);
      printf("<0> to set a dummy entry\r");
      printf("<Enter> to keep current FunctionNumber\r");
      printf("<ESC> to exit auto-scroll setup: ");

      input_string(String);
      if (String[0] == 27) return;
      FlashConfig1.AutoScroll[AutoScrollNumber].FunctionId[ItemNumber - 1] = atoi(String);
    }
  }

  return;
}





/* $TITLE=term_brightness_setup()) */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                              Terminal submenu for brightness setup.
\* ============================================================================================================================================================= */
void term_brightness_setup(void)
{
  UCHAR String[31];

  UINT16 LightRange;
  UINT16 PwmHiLimit;
  UINT16 PwmLoLimit;
  UINT16 PwmLevel;

  float  PwmRange;



  printf("\r\r\r\r");
  printf("      Brightness setup\r\r");

  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                        Automatic brightness.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  while (1)
  {
    if (FlashConfig1.FlagAutoBrightness)
      printf("Automatic brightness is now turned On.\r");
    else
      printf("Automatic brightness is now turned Off.\r");
    printf("Press <c> to change this setting\r");
    printf("<Enter> to keep it this way\r");
    printf("<ESC> to exit brightness setup: ");

    input_string(String);
    if (String[0] == 0x0D) break;
    if (String[0] == 27)   return;
    if ((String[0] == 'C') || (String[0] == 'c'))
      FlashConfig1.FlagAutoBrightness ^= 0x01;

    /* If brightness has been changed from "automatic" to "steady", adjust current brightness to match its setting. */
    if (FlashConfig1.FlagAutoBrightness == FLAG_OFF)
    {
      PwmRange = PWM_HI_LIMIT - PWM_LO_LIMIT;
      PwmLevel = PWM_HI_LIMIT - (UINT16)(FlashConfig1.BrightnessLevel * (PwmRange / 1000));
      pwm_set_level(PWM_ID_BRIGHTNESS, PwmLevel);
      printf("Brightness level has been set to its steady configuration value: %u (level: %u)\r", FlashConfig1.BrightnessLevel, PwmLevel);
    }
  }
  printf("\r\r");



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                    Automatic brightness lowest level.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  while (1)
  {
    printf("Lowest level for auto brightness is: %u\r", FlashConfig1.BrightnessLoLimit);
    printf("Enter new value to change this setting (1 to 1000)\r");
    printf("<Enter> to keep it this way\r");
    printf("<ESC> to exit brightness setup: ");

    input_string(String);
    if (String[0] == 0x0D) break;
    if (String[0] == 27)   return;
    FlashConfig1.BrightnessLoLimit = atoi(String);
    if (FlashConfig1.BrightnessLoLimit <= 0)   FlashConfig1.BrightnessLoLimit = 1;
    if (FlashConfig1.BrightnessLoLimit > 1000) FlashConfig1.BrightnessLoLimit = 1000;
  }
  printf("\r\r");



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                   Automatic brightness highest level.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  while (1)
  {
    printf("Highest level for auto brightness is: %u\r",            FlashConfig1.BrightnessHiLimit);
    printf("Enter new value to change this setting (%u to 1000)\r", FlashConfig1.BrightnessLoLimit);
    printf("<Enter> to keep it this way\r");
    printf("<ESC> to exit brightness setup: ");

    input_string(String);
    if (String[0] == 0x0D) break;
    if (String[0] == 27)   return;
    FlashConfig1.BrightnessHiLimit = atoi(String);
    if (FlashConfig1.BrightnessHiLimit < FlashConfig1.BrightnessLoLimit) FlashConfig1.BrightnessHiLimit = FlashConfig1.BrightnessLoLimit;
    if (FlashConfig1.BrightnessHiLimit > 1000) FlashConfig1.BrightnessHiLimit = 1000;

  }
  printf("\r\r");



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                       Steady brightness level.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  while (1)
  {
    printf("Steady brightness setting is: %u\r", FlashConfig1.BrightnessLevel);
    printf("Enter new value to change this setting (1 to 1000)\r");
    printf("<Enter> to keep it this way\r");
    printf("<ESC> to exit brightness setup: ");

    input_string(String);
    if (String[0] == 0x0D) break;
    if (String[0] == 27)   return;
    FlashConfig1.BrightnessLevel = atoi(String);
    if (FlashConfig1.BrightnessLevel <= 0)   FlashConfig1.BrightnessLevel = 1;
    if (FlashConfig1.BrightnessLevel > 1000) FlashConfig1.BrightnessLevel = 1000;

    /* NOTE: Steady brightness setting does not take care of limits setup for automatic brightness control.
             User is free to set the steady brightness setting inside the whole range of brightness (1 to 1000). */
    /// PwmLoLimit = (PWM_LO_LIMIT + ((UINT16)((1000 - FlashConfig1.BrightnessHiLimit) * 0.7)));
    /// PwmHiLimit = (PWM_HI_LIMIT - ((UINT16)((FlashConfig1.BrightnessLoLimit * 0.7) + 0.5)));
    PwmRange = PWM_HI_LIMIT - PWM_LO_LIMIT;
    PwmLevel = PWM_HI_LIMIT - (UINT16)(FlashConfig1.BrightnessLevel * (PwmRange / 1000));

    if (DebugBitMask & DEBUG_BRIGHTNESS)
    {
      uart_send(__LINE__, __func__, "PWM_HI_LIMIT:        %4u   PWM_LO_LIMIT:       %4u\r", PWM_HI_LIMIT, PWM_LO_LIMIT);
      uart_send(__LINE__, __func__, "BrightnessLoLimit:   %4u   BrightnessHiLimit:  %4u   User range: %4u\r", FlashConfig1.BrightnessLoLimit, FlashConfig1.BrightnessHiLimit, FlashConfig1.BrightnessHiLimit - FlashConfig1.BrightnessLoLimit);
      uart_send(__LINE__, __func__, "PwmRange / 1000:     %4f   FlashConfig1.BrightnessLevel * (PwmRange / 1000): %f\r", (PwmRange / 1000), (FlashConfig1.BrightnessLevel * (PwmRange / 1000)));
    }

#if 0
    if (PwmLevel < PWM_LO_LIMIT)
    {
      /* If brightness value entered is lower than the lowest limit, brightness is automatically set to its lowest limit (= highest PWM level). */
      if (DebugBitMask & DEBUG_BRIGHTNESS)
        uart_send(__LINE__, __func__, "Brightness value entered is lower that low limit - Brightness has been set to 1 (VERY dim)\r");
      PwmLevel = PWM_LO_LIMIT;
    }

    if (PwmLevel >= PWM_HI_LIMIT)
    {
      /* If brightness value entered is higher than the highest limit, brightness is automatically set to its highest limit (= lowest PWM level). */
      if (DebugBitMask & DEBUG_BRIGHTNESS)
        uart_send(__LINE__, __func__, "Brightness value entered is higher that high limit - Brightness has been set to 1000 (VERY bright)\r");
      PwmLevel = PWM_HI_LIMIT;
    }
#endif  // 0

    /// if ((PwmLevel > PWM_LO_LIMIT) && (PwmLevel < PWM_HI_LIMIT))
    /// {
      /* If brightness value entered is between the lowest and highest limits, brightness is automatically adjusted proportionally. */
      PwmLevel = PWM_HI_LIMIT - (UINT16)(FlashConfig1.BrightnessLevel * (PwmRange / 1000));
      if (DebugBitMask & DEBUG_BRIGHTNESS)
        uart_send(__LINE__, __func__, "Setting PwmLevel to: %4u\r", PwmLevel);
    /// }

    pwm_set_level(PWM_ID_BRIGHTNESS, PwmLevel);
  }
  printf("\r\r");

  return;
}





/* $TITLE=term_button_feedback_setup()) */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                              Terminal submenu for button feedback setup.
\* ============================================================================================================================================================= */
void term_button_feedback_setup(void)
{
  UCHAR String[31];


  while (1)
  {
    if (FlashConfig1.FlagButtonFeedback == FLAG_ON)
      printf("Button audible feedback is now turned On.\r");
    else
      printf("Button audible feedback is now turned Off.\r");
    printf("Press <c> to change this setting\r");
    printf("<Enter> to keep it this way\r");
    printf("<ESC> to exit button feedback setup: ");

    input_string(String);
    if (String[0] == 0x0D) break;
    if (String[0] == 27)   return;
    if ((String[0] == 'C') || (String[0] == 'c'))
      FlashConfig1.FlagButtonFeedback ^= 0x01;
  }
  printf("\r\r");

  return;
}





/* $TITLE=term_date_setup()) */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                                  Terminal submenu for date setup.
\* ============================================================================================================================================================= */
void term_date_setup(void)
{
  UCHAR String[31];

  UINT8 Loop1UInt8;

  struct human_time HumanTime;
  struct tm SetupTime;


  /* First, get current time from real-time clock IC (DS3231). */
  ds3231_get_time(&HumanTime);

  /***
  uart_send(__LINE__, __func__, "DayName:\r");
  for (Loop1UInt8 = 0; Loop1UInt8 < 7; ++Loop1UInt8)
    printf("%u - %s (%s)\r", Loop1UInt8, DayName[Loop1UInt8], ShortDay[Loop1UInt8]);
  printf("\r\r\r");

  uart_send(__LINE__, __func__, "ShortMonth:\r");
  for (Loop1UInt8 = 0; Loop1UInt8 < 12; ++Loop1UInt8)
    printf("%u - %s (%s)\r", Loop1UInt8, ShortMonth[Loop1UInt8], ShortMonth[Loop1UInt8]);
  printf("\r\r\r");

  printf("Time.tm_hour:   %2u\r", SetupTime.tm_hour);  // tm_hour: 0 to 23
  printf("Time.tm_min:    %2u\r", SetupTime.tm_min);   // tm_min:  0 to 59
  printf("Time.tm_sec:    %2u\r", SetupTime.tm_sec);   // tm_sec:  0 to 59
  printf("Time.tm_wday:   %2u\r", SetupTime.tm_wday);  // tm_wday: 0-SUN   1-MON   2-TUE   3-WED   4-THU   5-FRI   6-SAT
  printf("Time.tm_mday:   %2u\r", SetupTime.tm_mday);  // tm_mday: 1 to 31
  printf("Time.tm_mon:    %2u\r", SetupTime.tm_mon);   // tm_mon:  0 to 11
  printf("Time.tm_year: %4u\r",   SetupTime.tm_year);  // tm_year: 2000 and up...
  printf("\r\r\r");
  ***/


  printf("\r\r\r\r");
  printf("            Date setup\r\r");
  printf("Current date is: %s   %2.2u-%s-%2.2u\r\r", DayName[HumanTime.DayOfWeek], HumanTime.DayOfMonth, ShortMonth[HumanTime.Month], HumanTime.Year);


  printf("Note: Weekdays are:  ");
  for (Loop1UInt8 = 0; Loop1UInt8 < 7; ++Loop1UInt8)
    printf("%u-%s   ", Loop1UInt8 + 1, ShortDay[Loop1UInt8]);
  printf("\r");
  printf("Enter number for day-of-week (or <Enter> for no change) [%2u]: ", HumanTime.DayOfWeek + 1);
  input_string(String);
  if (String[0] != 0x0D) HumanTime.DayOfWeek = (atoi(String) - 1);
  printf("\r");


  printf("Enter day-of-month (or <Enter> for no change)           [%2u]: ", HumanTime.DayOfMonth);
  input_string(String);
  if (String[0] != 0x0D) HumanTime.DayOfMonth = atoi(String);
  printf("\r");


  printf("Note: ");
  for (Loop1UInt8 = 1; Loop1UInt8 < 13; ++Loop1UInt8)
    printf("%u-%s   ", Loop1UInt8, ShortMonth[Loop1UInt8]);
  printf("\r");
  printf("Enter number for Month (or <Enter> for no change)       [%2u]: ", HumanTime.Month);
  input_string(String);
  if (String[0] != 0x0D) HumanTime.Month = atoi(String);
  printf("\r");


  printf("Enter Year (or <Enter> for no change)                   [%4u]: ", HumanTime.Year);
  input_string(String);
  if (String[0] != 0x0D) HumanTime.Year = atoi(String);
  printf("\r");


  ds3231_set_time(&HumanTime);
  printf("Real time clock IC has been set to: %s   %2.2u-%s-%2.2u\r", DayName[HumanTime.DayOfWeek], HumanTime.DayOfMonth, ShortMonth[HumanTime.Month], HumanTime.Year);
  printf("Press <Enter> to continue: ");
  input_string(String);

  return;
}





/* $TITLE=term_debug() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                         Terminal submenu to toggle On or Off debug flags.
\* ============================================================================================================================================================= */
void term_debug(void)
{
  UCHAR String[11];

  UINT8 DebugNumber;

  UINT16 Loop1UInt16;

  UINT64 TargetDebug;


  printf("    Terminal submenu to toggle debug flags On or Off.\r\r");
  printf("    Current debug flags status:\r\r");
  while (1)
  {
    for (Loop1UInt16 = 0; Loop1UInt16 < 64; ++Loop1UInt16)
    {
      switch (0x01 << Loop1UInt16)
      {
        case DEBUG_ALARM:
          if (DebugBitMask & (0x01 << Loop1UInt16))
            uart_send(__LINE__, __func__, "%2u - DEBUG_ALARM       is On     *****\r", Loop1UInt16);
          else
            uart_send(__LINE__, __func__, "%2u - DEBUG_ALARM       is Off.\r", Loop1UInt16);
        break;

        case DEBUG_BLINK:
          if (DebugBitMask & (0x01 << Loop1UInt16))
            uart_send(__LINE__, __func__, "%2u - DEBUG_BLINK       is On     *****\r", Loop1UInt16);
          else
            uart_send(__LINE__, __func__, "%2u - DEBUG_BLINK       is Off.\r", Loop1UInt16);
        break;

        case DEBUG_BLUETOOTH:
          if (DebugBitMask & (0x01 << Loop1UInt16))
            uart_send(__LINE__, __func__, "%2u - DEBUG_BLUETOOTH   is On     *****\r", Loop1UInt16);
          else
            uart_send(__LINE__, __func__, "%2u - DEBUG_BLUETOOTH   is Off.\r", Loop1UInt16);
        break;

        case DEBUG_BOX:
          if (DebugBitMask & (0x01 << Loop1UInt16))
            uart_send(__LINE__, __func__, "%2u - DEBUG_BOX         is On     *****\r", Loop1UInt16);
          else
            uart_send(__LINE__, __func__, "%2u - DEBUG_BOX         is Off.\r", Loop1UInt16);
        break;

        case DEBUG_BRIGHTNESS:
          if (DebugBitMask & (0x01 << Loop1UInt16))
            uart_send(__LINE__, __func__, "%2u - DEBUG_BRIGHTNESS  is On     *****\r", Loop1UInt16);
          else
            uart_send(__LINE__, __func__, "%2u - DEBUG_BRIGHTNESS  is Off.\r", Loop1UInt16);
        break;

        case DEBUG_BUTTON:
          if (DebugBitMask & (0x01 << Loop1UInt16))
            uart_send(__LINE__, __func__, "%2u - DEBUG_BUTTON      is On     *****\r", Loop1UInt16);
          else
            uart_send(__LINE__, __func__, "%2u - DEBUG_BUTTON      is Off.\r", Loop1UInt16);
        break;

        case DEBUG_CORE:
          if (DebugBitMask & (0x01 << Loop1UInt16))
            uart_send(__LINE__, __func__, "%2u - DEBUG_CORE        is On     *****\r", Loop1UInt16);
          else
            uart_send(__LINE__, __func__, "%2u - DEBUG_CORE        is Off.\r", Loop1UInt16);
        break;

        case DEBUG_DS3231:
          if (DebugBitMask & (0x01 << Loop1UInt16))
            uart_send(__LINE__, __func__, "%2u - DEBUG_DS3231      is On     *****\r", Loop1UInt16);
          else
            uart_send(__LINE__, __func__, "%2u - DEBUG_DS3231      is Off.\r", Loop1UInt16);
        break;

        case DEBUG_EVENT:
          if (DebugBitMask & (0x01 << Loop1UInt16))
            uart_send(__LINE__, __func__, "%2u - DEBUG_EVENT       is On     *****\r", Loop1UInt16);
          else
            uart_send(__LINE__, __func__, "%2u - DEBUG_EVENT       is Off.\r", Loop1UInt16);
        break;

        case DEBUG_FLASH:
          if (DebugBitMask & (0x01 << Loop1UInt16))
            uart_send(__LINE__, __func__, "%2u - DEBUG_FLASH       is On     *****\r", Loop1UInt16);
          else
            uart_send(__LINE__, __func__, "%2u - DEBUG_FLASH       is Off.\r", Loop1UInt16);
        break;

        case DEBUG_FLOW:
          if (DebugBitMask & (0x01 << Loop1UInt16))
            uart_send(__LINE__, __func__, "%2u - DEBUG_FLOW        is On     *****\r", Loop1UInt16);
          else
            uart_send(__LINE__, __func__, "%2u - DEBUG_FLOW        is Off.\r", Loop1UInt16);
        break;

        case DEBUG_FUNCTION:
          if (DebugBitMask & (0x01 << Loop1UInt16))
            uart_send(__LINE__, __func__, "%2u - DEBUG_FUNCTION    is On     *****\r", Loop1UInt16);
          else
            uart_send(__LINE__, __func__, "%2u - DEBUG_FUNCTION    is Off.\r", Loop1UInt16);
        break;

        case DEBUG_IR:
          if (DebugBitMask & (0x01 << Loop1UInt16))
            uart_send(__LINE__, __func__, "%2u - DEBUG_IR          is On     *****\r", Loop1UInt16);
          else
            uart_send(__LINE__, __func__, "%2u - DEBUG_IR          is Off.\r", Loop1UInt16);
        break;

        case DEBUG_MATRIX:
          if (DebugBitMask & (0x01 << Loop1UInt16))
            uart_send(__LINE__, __func__, "%2u - DEBUG_MATRIX      is On     *****\r", Loop1UInt16);
          else
            uart_send(__LINE__, __func__, "%2u - DEBUG_MATRIX      is Off.\r", Loop1UInt16);
        break;

        case DEBUG_NTP:
          if (DebugBitMask & (0x01 << Loop1UInt16))
            uart_send(__LINE__, __func__, "%2u - DEBUG_NTP         is On     *****\r", Loop1UInt16);
          else
            uart_send(__LINE__, __func__, "%2u - DEBUG_NTP         is Off.\r", Loop1UInt16);
        break;

        case DEBUG_PWM:
          if (DebugBitMask & (0x01 << Loop1UInt16))
            uart_send(__LINE__, __func__, "%2u - DEBUG_PWM         is On     *****\r", Loop1UInt16);
          else
            uart_send(__LINE__, __func__, "%2u - DEBUG_PWM         is Off.\r", Loop1UInt16);
        break;

        case DEBUG_REMINDER:
          if (DebugBitMask & (0x01 << Loop1UInt16))
            uart_send(__LINE__, __func__, "%2u - DEBUG_REMINDER    is On     *****\r", Loop1UInt16);
          else
            uart_send(__LINE__, __func__, "%2u - DEBUG_REMINDER    is Off.\r", Loop1UInt16);
        break;

        case DEBUG_SCROLL:
          if (DebugBitMask & (0x01 << Loop1UInt16))
            uart_send(__LINE__, __func__, "%2u - DEBUG_SCROLL      is On     *****\r", Loop1UInt16);
          else
            uart_send(__LINE__, __func__, "%2u - DEBUG_SCROLL      is Off.\r", Loop1UInt16);
        break;

        case DEBUG_SOUND_QUEUE:
          if (DebugBitMask & (0x01 << Loop1UInt16))
            uart_send(__LINE__, __func__, "%2u - DEBUG_SOUND_QUEUE is On     *****\r", Loop1UInt16);
          else
            uart_send(__LINE__, __func__, "%2u - DEBUG_SOUND_QUEUE is Off.\r", Loop1UInt16);
        break;

        case DEBUG_STARTUP:
          if (DebugBitMask & (0x01 << Loop1UInt16))
            uart_send(__LINE__, __func__, "%2u - DEBUG_STARTUP     is On     *****\r", Loop1UInt16);
          else
            uart_send(__LINE__, __func__, "%2u - DEBUG_STARTUP     is Off.\r", Loop1UInt16);
        break;

        case DEBUG_SUMMER_TIME:
          if (DebugBitMask & (0x01 << Loop1UInt16))
            uart_send(__LINE__, __func__, "%2u - DEBUG_SUMMER_TIME is On     *****\r", Loop1UInt16);
          else
            uart_send(__LINE__, __func__, "%2u - DEBUG_SUMMER_TIME is Off.\r", Loop1UInt16);
        break;

        case DEBUG_TEST:
          if (DebugBitMask & (0x01 << Loop1UInt16))
            uart_send(__LINE__, __func__, "%2u - DEBUG_TEST        is On     *****\r", Loop1UInt16);
          else
            uart_send(__LINE__, __func__, "%2u - DEBUG_TEST        is Off.\r", Loop1UInt16);
        break;

        case DEBUG_WATCHDOG:
          if (DebugBitMask & (0x01 << Loop1UInt16))
            uart_send(__LINE__, __func__, "%2u - DEBUG_WATCHDOG    is On     *****\r", Loop1UInt16);
          else
            uart_send(__LINE__, __func__, "%2u - DEBUG_WATCHDOG    is Off.\r", Loop1UInt16);
        break;

        case DEBUG_WIFI:
          if (DebugBitMask & (0x01 << Loop1UInt16))
            uart_send(__LINE__, __func__, "%2u - DEBUG_WIFI        is On     *****\r", Loop1UInt16);
          else
            uart_send(__LINE__, __func__, "%2u - DEBUG_WIFI        is Off.\r", Loop1UInt16);
        break;

        case DEBUG_WINDOW:
          if (DebugBitMask & (0x01 << Loop1UInt16))
            uart_send(__LINE__, __func__, "%2u - DEBUG_WINDOW      is On     *****\r", Loop1UInt16);
          else
            uart_send(__LINE__, __func__, "%2u - DEBUG_WINDOW      is Off.\r", Loop1UInt16);
        break;

        default:
          if (DebugBitMask & (0x01 << Loop1UInt16))
            uart_send(__LINE__, __func__, "%2u - Debug section %2u  is On    *****\r", Loop1UInt16, Loop1UInt16);
          else
            uart_send(__LINE__, __func__, "%2u - Debug section %2u  is Off.\r", Loop1UInt16, Loop1UInt16);
        break;
      }
    }

    printf("\r\r");
    printf("Enter the debug number you want to toggle\r");
    printf("<ESC> to exit debug flag setup: ");
    input_string(String);
    if (String[0] == 27) return;
    if (String[0] == 0x0D) continue;
    DebugNumber = atoi(String);
    if (DebugBitMask & (0x01 << DebugNumber))
      DebugBitMask &= ~(0x01 << DebugNumber);
    else
      DebugBitMask |= (0x01 << DebugNumber);
  }
}





/* $TITLE=term_dst_setup() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                   Terminal submenu for daylight saving time and time zone setup.
\* ============================================================================================================================================================= */
void term_dst_setup(void)
{
  UCHAR String[31];

  INT8 Dum1Int8;

  UINT8 Dum1UInt8;


  printf("    --------------------------------------------------------------\r");
  printf("    Terminal submenu for Daylight Saving Time (DST) and time zone.\r\r");
  printf("    Daylight Saving Time country setting is currently: %u\r",   FlashConfig1.DSTCountry);
  printf("    Time zone setting is currently:                    %d\r\r", FlashConfig1.Timezone);
  printf("    --------------------------------------------------------------\r\r");


    /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                  Setting Daylight Saving Time country.
    \* --------------------------------------------------------------------------------------------------------------------------- */
  printf("    What setting do you want for Daylight Saving Time country (refer to User Guide for details)\r");
  printf("    (<ESC> to keep current value): ");
  input_string(String);
  if ((String[0] != 0x1B) && (String[0] != 0x0D))
  {
    Dum1UInt8 = atoi(String);
    while (Dum1UInt8 > 12)
    {
      printf("    Invalid setting. Please enter a value between 0 and 12: ");
      input_string(String);
      if ((String[0] == 0x1B) || (String[0] == 0x0D)) break;
      Dum1UInt8 = atoi(String);
    }
  }
  if ((String[0] != 0x1B) && (String[0] != 0x0D)) FlashConfig1.DSTCountry = Dum1UInt8;
  printf("\r\r");


    /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                           Setting Time zone.
    \* --------------------------------------------------------------------------------------------------------------------------- */
  printf("    What setting do you want for Time zone.\r");
  printf("    (<ESC> to keep current valuie): ");
  input_string(String);
  if ((String[0] != 0x1B) && (String[0] != 0x0D))
  {
    Dum1Int8 = atoi(String);
    while ((Dum1Int8 < -12) || (Dum1Int8 > 14))
    {
      printf("    Invalid setting. Please enter a value between -12 and 14: ");
      input_string(String);
      if ((String[0] == 0x1B) || (String[0] == 0x0D)) break;
      Dum1Int8 = atoi(String);
    }
  }
  if ((String[0] != 0x1B) && (String[0] != 0x0D)) FlashConfig1.Timezone = Dum1Int8;
  printf("\r\r");

  printf("    --------------------------------------------------------------\r");
  printf("    Daylight Saving Time and Time zone have been set as follow:\r\r");
  printf("    Daylight Saving Time country setting is currently: %u\r",   FlashConfig1.DSTCountry);
  printf("    Time zone setting is currently:                    %d\r\r", FlashConfig1.Timezone);
  printf("    --------------------------------------------------------------\r\r");

#ifdef NTP_SUPPORT
  NTPData.FlagNTPResync = FLAG_ON;  // force re-sync when implemented.
#endif  // NTP_SUPPORT

  return;
}





/* $TITLE=term_erase_flash() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                    Terminal submenu for "erase flash configuration".
\* ============================================================================================================================================================= */
void term_erase_flash(void)
{
  UCHAR String[31];

  UINT16 PwmLevel;


  printf("\r\r\r\r");
  printf("                         Erase flash configuration\r\r");
  printf("WARNING - WARNING - WARNING - WARNING - WARNING - WARNING - WARNING - WARNING - WARNING - WARNING\r\r");
  printf("This operation will completely erase the configuration saved in RGB Matrix Pico's non-volatile memory...\r");
  printf("You will have to manually reconfigure all settings...\r");
  printf("Are you sure you want to proceed <Y/N> (or <ESC> to exit this option without change): ");
  input_string(String);
  if (String[0] == 27) return;
  if ((String[0] == 'Y') || (String[0] == 'y'))
  {
    uart_send(__LINE__, __func__, "Erasing configuration from Pico's flash memory to force generating a default configuration.\r");
    win_open(WIN_MESSAGE, FLAG_OFF);
    win_printf(WIN_MESSAGE, 1, 99, FONT_5x7, "ERASING");
    win_printf(WIN_MESSAGE, 9, 99, FONT_5x7, "FLASH");
    sleep_ms(5000);

    PwmLevel = Pwm[PWM_ID_BRIGHTNESS].Level;  // keep track of original PWM level.
    pwm_set_level(PWM_ID_BRIGHTNESS, 2000);   // blank LED matrix while interrupts are disabled.
    flash_erase(0x1FF000);   // erase configuration 1 (most configuration settings).
    flash_erase(0x1FE000);   // erase configuration 2 (Reminders1).
    win_close(WIN_MESSAGE);  // restore backlink windows.
    pwm_set_level(PWM_ID_BRIGHTNESS, PwmLevel);  // restore brightness level when done

    /* Check flash configurations 1 and 2. Since we just erased both, two default configurations will be saved, preventing flash_read_config() to crash. */
    /// flash_check_config(1);
    /// flash_check_config(2);
    flash_read_config1();
    flash_read_config2();
  }

  return;
}





/* $TITLE=term_events_setup()) */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                              Terminal submenu for calendar events setup.
\* ============================================================================================================================================================= */
void term_events_setup(void)
{
  UCHAR String[61];

  UINT16 EventNumber;
  UINT16 Loop1UInt16;
  UINT16 Loop2UInt16;


  while (1)
  {
    /* Display calendar events. */
    for (Loop1UInt16 = 0; Loop1UInt16 < MAX_EVENTS; ++Loop1UInt16)
    {
      display_event(Loop1UInt16);
    }



    /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                          Selection of event number.
    \* --------------------------------------------------------------------------------------------------------------------------- */
    printf("Enter event number you want to change or <ESC> to exit event setup: ");
    input_string(String);
    if (String[0] == 27) break;
    EventNumber = atoi(String) - 1;
    while (EventNumber > MAX_EVENTS)
    {
      printf("Invalid event number.\r");
      printf("Enter event number (1 to %u) or <ESC> to exit event setup: ", MAX_EVENTS);
      input_string(String);
      if (String[0] == 27)
      {
        printf(("Updating calendar events...\r\r"));
        sleep_ms(3000);
        event_check();
        return;
      }
      EventNumber = atoi(String) - 1;
    }



    display_event(EventNumber);

    /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                              Event day.
                                      NOTE: No validation is done on values entered.
                                            This allows to temporarily suspend an event without erasing
                                            it by assigning an unreachable value for day and / or month.
    \* --------------------------------------------------------------------------------------------------------------------------- */
    while (1)
    {
      printf("Current day of month for event number %u is: %2u\r", EventNumber + 1, FlashConfig1.Event[EventNumber].Day);
      printf("Enter new value to change this setting: ");
      printf("<Enter> to keep current setting\r");
      printf("<ESC> to exit event setup: ");

      input_string(String);
      if (String[0] == 0x0D) break;
      if (String[0] == 27)
      {
        printf(("Updating calendar events...\r\r"));
        sleep_ms(3000);
        event_check();
        return;
      }
      FlashConfig1.Event[EventNumber].Day = atoi(String);
    }
    printf("\r\r");



    /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                              Event month.
    \* --------------------------------------------------------------------------------------------------------------------------- */
    while (1)
    {
      printf("Current month for event %u is: %2.2u\r", EventNumber + 1, FlashConfig1.Event[EventNumber].Month);
      printf("Enter new value to change this setting\r");
      printf("<Enter> to keep current setup\r");
      printf("<ESC> to exit event setup: ");

      input_string(String);
      if (String[0] == 0x0D) break;
      if (String[0] == 27)
      {
        printf(("Updating calendar events...\r\r"));
        sleep_ms(3000);
        event_check();
        return;
      }
      FlashConfig1.Event[EventNumber].Month = atoi(String);
    }
    printf("\r\r");



    /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                           Optional event jingle.
               NOTE: This requires installation of a passive buzzer which is not installed originally in the RGB matrix.
    \* --------------------------------------------------------------------------------------------------------------------------- */
    while (1)
    {
      printf("Current jingle number event %u is: %2.2u\r", EventNumber + 1, FlashConfig1.Event[EventNumber].Jingle);
      printf("Enter new value to change this setting\r");
      printf("<Enter> to keep current setup\r");
      printf("<ESC> to exit event setup: ");

      input_string(String);
      if (String[0] == 0x0D) break;
      if (String[0] == 27)
      {
        printf(("Updating calendar events...\r\r"));
        sleep_ms(3000);
        event_check();
        return;
      }
      FlashConfig1.Event[EventNumber].Jingle = atoi(String);
    }
    printf("\r\r");




    /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                             Event message.
    \* --------------------------------------------------------------------------------------------------------------------------- */
    /* Display message for this event. */
    printf("Event[%2.2u].Message:  <", EventNumber + 1, FlashConfig1.Event[EventNumber].Message);
    for (Loop1UInt16 = 0; Loop1UInt16 < sizeof(FlashConfig1.Event[EventNumber].Message); ++Loop1UInt16)
    {
      if  (FlashConfig1.Event[EventNumber].Message[Loop1UInt16] == 0x00) break;  // get out of <for> loop when reaching end-of-string.
      if ((FlashConfig1.Event[EventNumber].Message[Loop1UInt16] >= 0x20) && (FlashConfig1.Event[EventNumber].Message[Loop1UInt16] <= 0x7E))
        printf("%c", FlashConfig1.Event[EventNumber].Message[Loop1UInt16]);
      else
        printf("?");
    }
    printf(">\r\r");

    printf("Enter new message for this event\r");
    printf("<Enter> to keep current message\r");
    printf("or <ESC> to exit event setup\r");
    printf("(maximum %u characters):\r", sizeof(FlashConfig1.Event[EventNumber].Message));

    /* Display a template for maximum event message length. */
    printf("[");
    for (Loop1UInt16 = 0; Loop1UInt16 < sizeof(FlashConfig1.Event[EventNumber].Message); ++Loop1UInt16) printf("-");
    printf("]\r ");

    input_string(String);
    if (String[0] == 0x0D) break;
    if (String[0] == 27)
    {
      printf(("Updating calendar events...\r\r"));
      sleep_ms(3000);
      event_check();
      return;
    }

    while (strlen(String) > sizeof(FlashConfig1.Event[EventNumber].Message))
    {
      printf("Message too long (by %u characters)\r", strlen(String) - sizeof(FlashConfig1.Event[EventNumber].Message));
      printf("Enter new message for this event\r");
      printf("<Enter> to keep current message\r");
      printf("or <ESC> to exit event setup\r");
      printf("(maximum %u characters):\r", sizeof(FlashConfig1.Event[EventNumber].Message));
      input_string(String);
      if (String[0] == 0x0D) break;
      if (String[0] == 27)
      {
        printf(("Updating calendar events...\r\r"));
        sleep_ms(3000);
        event_check();
        return;
      }
    }
    sprintf(FlashConfig1.Event[EventNumber].Message, String);
  }

  /* Check if we added a new calendar event that must be active today. */
  event_check();

  return;
}





/* $TITLE=term_golden_age()) */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                              Terminal submenu for golden age mode.
\* ============================================================================================================================================================= */
void term_golden_age(void)
{
  UCHAR String[31];

  UINT16 Dum1UInt16;


  printf("\r\r\r\r");
  printf("             Golden age setup\r\r");

  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                        Flag golden age status.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  while (1)
  {
    if (FlashConfig1.FlagGoldenAge)
      printf("Golden age mode is now turned On.\r");
    else
      printf("Golden age mode is now turned Off.\r");
    printf("Press <c> to change this setting\r");
    printf("<Enter> to keep it this way\r");
    printf("<ESC> to exit golden age setup: ");

    input_string(String);
    if (String[0] == 0x0D) break;
    if (String[0] == 27)   return;
    if ((String[0] == 'C') || (String[0] == 'c'))
      FlashConfig1.FlagGoldenAge ^= 0x01;

    if (FlashConfig1.FlagGoldenAge == FLAG_OFF)
    {
      /* Golden age mode has just been turned Off, restore original colors for WIN_DATE and WIN_TIME windows. */
      /* NOTE: If we turn golden mode On, colors will be managed in RGB_matrix_display_time(), depending on the period of the day. */
      win_set_color(WIN_TIME, Window[WIN_TIME].InsideColor, Window[WIN_TIME].BorderColor);  // WIN_TIME, except alarm and target days indicators.

      /* Do not use win_set_color() for WIN_DATE , so that we do not change alarm indicators and alarm target day indicators. */
      RGB_matrix_set_color(Window[WIN_DATE].StartRow + 1, Window[WIN_DATE].StartColumn, Window[WIN_DATE].EndRow - 1, Window[WIN_DATE].EndColumn, Window[WIN_DATE].InsideColor);
    }
  }
  printf("\r\r");



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                          Hour considered morning start for golden age mode.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  while(1)
  {
    printf("Hour considered morning start is currently set to %u\r", FlashConfig1.GoldenMorningStart);
    printf("Enter new value (0 to %u)\r", FlashConfig1.GoldenAfternoonStart - 1);
    printf("<Enter> to keep current setting\r");
    printf("<ESC> to exit golden age setup: ");

    input_string(String);
    if (String[0] == 0x0D) break;
    if (String[0] == 27)   return;
    Dum1UInt16 = atoi(String);

    while (Dum1UInt16 >= FlashConfig1.GoldenAfternoonStart)
    {
      printf("Hour for <morning start> must be smaller than <afternoon start> (%u).\r", FlashConfig1.GoldenAfternoonStart);
      printf("Enter new value (0 to %u)\r", FlashConfig1.GoldenAfternoonStart - 1);
      printf("<ESC> to exit hourly golden age setup: ");

      input_string(String);
      if (String[0] == 0x0D) break;
      if (String[0] == 27)   return;
      Dum1UInt16 = atoi(String);
    }
    if (Dum1UInt16 < FlashConfig1.GoldenAfternoonStart) FlashConfig1.GoldenMorningStart = atoi(String);
  }
  printf("\r\r");



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                          Hour considered afternoon start for golden age mode.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  while(1)
  {
    printf("Hour considered afternoon start is currently set to %u\r", FlashConfig1.GoldenAfternoonStart);
    printf("Enter new value (%u to 23)\r", FlashConfig1.GoldenMorningStart + 1);
    printf("<Enter> to keep current setting\r");
    printf("<ESC> to exit hourly golden age setup: ");

    input_string(String);
    if (String[0] == 0x0D) break;
    if (String[0] == 27)   return;
    Dum1UInt16 = atoi(String);

    while ((Dum1UInt16 <= FlashConfig1.GoldenMorningStart) || (Dum1UInt16 >= FlashConfig1.GoldenEveningStart))
    {
      printf("Hour for <afternoon start> must be greater than <morning start> (%u) and smaller than <evening start> (%u).\r", FlashConfig1.GoldenMorningStart, FlashConfig1.GoldenEveningStart);
      printf("Enter new value (%u to %u)\r", FlashConfig1.GoldenMorningStart + 1, FlashConfig1.GoldenEveningStart - 1);
      printf("<ESC> to exit hourly golden age setup: ");

      input_string(String);
      if (String[0] == 0x0D) break;
      if (String[0] == 27)   return;
      Dum1UInt16 = atoi(String);
    }
    if ((Dum1UInt16 > FlashConfig1.GoldenMorningStart) && (Dum1UInt16 < FlashConfig1.GoldenEveningStart)) FlashConfig1.GoldenAfternoonStart = Dum1UInt16;
  }
  printf("\r\r");



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                            Hour considered evening start for golden age mode.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  while(1)
  {
    printf("Hour considered evening start is currently set to %u\r", FlashConfig1.GoldenEveningStart);
    printf("Enter new value (%u to 23)\r", FlashConfig1.GoldenAfternoonStart + 1);
    printf("<Enter> to keep current setting\r");
    printf("<ESC> to exit hourly golden age setup: ");

    input_string(String);
    if (String[0] == 0x0D) break;
    if (String[0] == 27)   return;
    Dum1UInt16 = atoi(String);

    while ((Dum1UInt16 <= FlashConfig1.GoldenAfternoonStart) || (Dum1UInt16 >= FlashConfig1.GoldenNightStart))
    {
      printf("Hour for <evening start> must be greater than <afternoon start> (%u) and smaller than <night start> (%u).\r", FlashConfig1.GoldenAfternoonStart, FlashConfig1.GoldenNightStart);
      printf("Enter new value (%u to %u)\r", FlashConfig1.GoldenAfternoonStart + 1, FlashConfig1.GoldenNightStart - 1);
      printf("<ESC> to exit hourly golden age setup: ");

      input_string(String);
      if (String[0] == 0x0D) break;
      if (String[0] == 27)   return;
      Dum1UInt16 = atoi(String);
    }
    if ((Dum1UInt16 > FlashConfig1.GoldenAfternoonStart) && (Dum1UInt16 < FlashConfig1.GoldenNightStart)) FlashConfig1.GoldenEveningStart = Dum1UInt16;
  }
  printf("\r\r");



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                            Hour considered night start for golden age mode.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  while(1)
  {
    printf("Hour considered night start is currently set to %u\r", FlashConfig1.GoldenNightStart);
    printf("Enter new value (%u to 23)\r", FlashConfig1.GoldenEveningStart + 1);
    printf("<Enter> to keep current setting\r");
    printf("<ESC> to exit hourly golden age setup: ");

    input_string(String);
    if (String[0] == 0x0D) break;
    if (String[0] == 27)   return;
    Dum1UInt16 = atoi(String);

    while (Dum1UInt16 <= FlashConfig1.GoldenEveningStart)
    {
      printf("Hour for <night start> must be greater than <evening start> (%u).\r", FlashConfig1.GoldenEveningStart);
      printf("Enter new value (%u to 23)\r", FlashConfig1.GoldenEveningStart + 1);
      printf("<ESC> to exit hourly golden age setup: ");

      input_string(String);
      if (String[0] == 0x0D) break;
      if (String[0] == 27)   return;
      Dum1UInt16 = atoi(String);
    }
    if ((Dum1UInt16 > FlashConfig1.GoldenEveningStart) && (Dum1UInt16 < 24)) FlashConfig1.GoldenNightStart = Dum1UInt16;
  }
  printf("\r\r");

  return;
}



/* $TITLE=term_hourly_chime_setup()) */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                         Terminal menu for hourly chime setup.
\* ============================================================================================================================================================= */
void term_hourly_chime_setup(void)
{
  UCHAR String[31];


  printf("\r\r\r\r");
  printf("                     Hourly chime setup\r\r");
  printf("IMPORTANT:\r");
  printf("----------\r");
  printf("Be aware that <Half-hour light chime>, <Calendar events> and <Reminders1>\r");
  printf("will all follow the same rules (On / Off / Day) and same hours than hourly chime.\r\r");



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                           Hourly chime mode.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  while (1)
  {
    switch (FlashConfig1.ChimeMode)
    {
      case (FLAG_OFF):
        printf("Hourly chime is currently Off\r");
      break;

      case (FLAG_ON):
        printf("Hourly chime is currently On\r");
      break;

      case(FLAG_DAY):
        printf("Hourly chime is currently set for intermittent operation\r");
        printf("based on On and Off hours you specify.\r");
      break;
    }
    printf("Press <c> to change this setting\r");
    printf("<Enter> to keep it this way\r");
    printf("<ESC> to exit hourly chime setup: ");

    input_string(String);
    if (String[0] == 0x0D) break;
    if (String[0] == 27)   return;
    if ((String[0] == 'C') || (String[0] == 'c'))
    {
      if (FlashConfig1.ChimeMode == FLAG_OFF)
      {
        FlashConfig1.ChimeMode = FLAG_ON;
        continue;
      }

      if (FlashConfig1.ChimeMode == FLAG_ON)
      {
        FlashConfig1.ChimeMode = FLAG_DAY;
        continue;
      }

      if (FlashConfig1.ChimeMode == FLAG_DAY)
      {
        FlashConfig1.ChimeMode = FLAG_OFF;
        continue;
      }
    }
  }
  printf("\r\r");



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                           Half-hour light chime mode.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  while (1)
  {
    switch (FlashConfig1.ChimeLightMode)
    {
      case (FLAG_OFF):
        printf("Half hour light chime is currently Off\r");
      break;

      case (FLAG_ON):
        printf("Half hour light chime is currently On\r");
      break;

      case(FLAG_DAY):
        printf("Half hour light chime is currently set for intermittent operation\r");
        printf("NOTE: On and Off hours will be the same as for hourly chime.\r");
      break;
    }
    printf("Press <c> to change this setting\r");
    printf("<Enter> to keep it this way\r");
    printf("<ESC> to exit hourly chime setup: ");

    input_string(String);
    if (String[0] == 0x0D) break;
    if (String[0] == 27)   return;
    if ((String[0] == 'C') || (String[0] == 'c'))
    {
      if (FlashConfig1.ChimeLightMode == FLAG_OFF)
      {
        FlashConfig1.ChimeLightMode = FLAG_ON;
        continue;
      }

      if (FlashConfig1.ChimeLightMode == FLAG_ON)
      {
        FlashConfig1.ChimeLightMode = FLAG_DAY;
        continue;
      }

      if (FlashConfig1.ChimeLightMode == FLAG_DAY)
      {
        FlashConfig1.ChimeLightMode = FLAG_OFF;
        continue;
      }
    }
  }
  printf("\r\r");



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                           Hourly chime On time.
                        NOTE: On and Off time only apply when hourly chime mode is set to "Day" (intermittent).
  \* --------------------------------------------------------------------------------------------------------------------------- */
  while(1)
  {
    printf("Hourly chime On time is currently set to %u\r", FlashConfig1.ChimeTimeOn);
    printf("Enter new value (0 to 24)\r");
    printf("<Enter> to keep current setting\r");
    printf("<ESC> to exit hourly chime setup: ");

    input_string(String);
    if (String[0] == 0x0D) break;
    if (String[0] == 27)   return;
    FlashConfig1.ChimeTimeOn = atoi(String);
    if (FlashConfig1.ChimeTimeOn > 23) FlashConfig1.ChimeTimeOn = 23;
  }
  printf("\r\r");



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                           Hourly chime Off time.
                        NOTE: On and Off time only apply when hourly chime mode is set to "Day" (intermittent).
  \* --------------------------------------------------------------------------------------------------------------------------- */
  while(1)
  {
    printf("Hourly chime Off time is currently set to %u\r", FlashConfig1.ChimeTimeOff);
    printf("Enter new value (0 to 23)\r");
    printf("<Enter> to keep current setting\r");
    printf("<ESC> to exit hourly chime setup: ");

    input_string(String);
    if (String[0] == 0x0D) break;
    if (String[0] == 27)   return;
    FlashConfig1.ChimeTimeOff = atoi(String);
    if (FlashConfig1.ChimeTimeOff > 23) FlashConfig1.ChimeTimeOff = 23;
  }
  printf("\r\r");

  if (FlashConfig1.ChimeTimeOff < FlashConfig1.ChimeTimeOn)
  {
    printf("WARNING:\r");
    printf("Be aware that the time Off setting has been set BEFORE the time On setting...\r");
    printf("This is usually the settings for night time workers...\r");
    printf("Make sure this is what you want.\r\r");
    printf("Press <Enter> to exit hourly chime settings.");
    input_string(String);
  }

  return;
}





/* $TITLE=term_info() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                             Terminal submenu for <info> functions.
\* ============================================================================================================================================================= */
void term_info(void)
{
  UCHAR String[128];

  UINT8 *Dum1Ptr8;
  UINT8  Menu;
  UINT8  PicoType;

  UINT16 Dum1UInt16;
  UINT16 Loop1UInt16;
  UINT16 Loop2UInt16;

  UINT64 TotalValues;

  float DegreeC;
  float DegreeF;
  float Temperature;

  struct human_time HumanTime;
  struct tm TempTime;

  absolute_time_t AbsoluteTime;


  while (1)
  {
    printf("\r\r\r\r");
    printf("                       Functions <INFO> submenu\r\r");
    printf("               1) - Firmware version.\r");
    printf("               2) - Microcontroller type and Unique ID.\r");
    printf("               3) - DS3231 & Pico internal temperature.\r");
    printf("               4) - Brightness and ambient light info.\r");
    printf("               5) - Alarms info.\r");
    printf("               6) - Calendar events of the day.\r");
    printf("               7) - Calendar events of the week.\r");
    printf("               8) - Calendar events of the month.\r");
    printf("               9) - Calendar events of the year (all).\r");
    printf("              10) - Daylight Saving Time info.\r");
    printf("              11) - Network credentials and NTP info.\r");
    printf("              12) - Idle time monitor info.\r");
    printf("              13) - Silence period info.\r");
    printf("              14) - First free heap memory pointer.\r");
    printf("              15) - Flash configuration display.\r");
    printf("              16) - Windows status info.\r");
    printf("              17) - Auto-scrolls info.\r");
    printf("              18) - Active scrolls status.\r");
    printf("              19) - Display available functions.\r");
    printf("              20) - Display current display buffers.\r");
    printf("              21) - Display total RGB Matrix Up time.\r");
    printf("             ESC) - Return to previous menu.\r\r");

    printf("                    Enter your choice: ");
    input_string(String);

    /* If user pressed <Enter> only, loop back to menu. */
    if (String[0] == 0x0D) continue;

    /* If user pressed <ESC> return to previous menu. */
    if (String[0] == 27)
    {
      String[0] = 0x00;
      printf("\r\r\r");

      return;
    }

    /* User pressed a menu option, execute it. */
    Menu = atoi(String);


    switch(Menu)
    {
      case (1):
        /* Firmware Version. */
        printf("\r\r");
        printf("Current firmware version: %s\r\r", FIRMWARE_VERSION);
        printf("Press <Enter> to continue: ");
        input_string(String);
        printf("\r\r");
      break;

      case (2):
        /* Microcontroller type and Unique ID. */
        printf("\r\r");
        PicoType = get_pico_type();
        if (PicoType == TYPE_PICO)
          printf("Microcontroller: Raspberry Pi Pico\r");
        else
          printf("Microcontroller: Raspberry Pi PicoW\r");
        printf("Pico Unique ID:  %s\r\r", PicoUniqueId);
        printf("Press <Enter> to continue: ");
        input_string(String);
        printf("\r\r");
      break;

      case (3):
        /* DS3231 & Pico internal temperature. */
        printf("\r\r");
        if (FlashConfig1.TemperatureUnit == CELSIUS)
        {
          get_pico_temp(&DegreeC, &DegreeF);
          printf("Pico internal temperature:   %2.2f\r", DegreeC);

          ds3231_get_temperature(&DegreeC, &DegreeF);
          printf("DS3231 internal temperature: %2.2f\r\r", DegreeC);
        }
        else
        {
          get_pico_temp(&DegreeC, &DegreeF);
          printf("Pico internal temperature:   %2.2f\r", DegreeF);

          ds3231_get_temperature(&DegreeC, &DegreeF);
          printf("DS3231 internal temperature: %2.2f\r\r", DegreeF);
        }

        printf("Press <Enter> to continue: ");
        input_string(String);
        printf("\r\r");
      break;


      case (4):
        /* Brightness and ambient light info. */
        printf("\r\r");
        if (FlashConfig1.FlagAutoBrightness == FLAG_ON)
          printf("Auto brightness is On\r");
        else
          printf("Auto brightness is On\r");
        printf("Brightness settings (from    0 to 1000) - Low: %u   High: %u   Steady: %4u\r", FlashConfig1.BrightnessLoLimit, FlashConfig1.BrightnessHiLimit, FlashConfig1.BrightnessLevel);
        printf("PWM current level   (from %4u to %4u) -                     Current: %4u\r", PWM_HI_LIMIT, PWM_LO_LIMIT, Pwm[PWM_ID_BRIGHTNESS].Level);
        printf("Ambient light current value: %u   Hysteresis value: %u\r", get_light_value(), AverageAmbientLight);
        printf("Ambient light readings for the last hysteresis period (%u seconds):\r", BRIGHTNESS_HYSTERESIS_SECONDS);
        TotalValues = 0ll;
        for (Loop1UInt16 = 0; Loop1UInt16 < BRIGHTNESS_HYSTERESIS_SECONDS; ++Loop1UInt16)
        {
          printf("[%3u]- %4u  ", Loop1UInt16, AmbientLight[Loop1UInt16]);
          if (((Loop1UInt16 + 1) % 16) == 0) printf("\r");
          TotalValues += AmbientLight[Loop1UInt16];
        }
        printf("\r");
        Dum1UInt16 = (UINT16)(TotalValues / BRIGHTNESS_HYSTERESIS_SECONDS);
        printf("Sum: %llu     Average: %u\r\r", TotalValues, Dum1UInt16);
        printf("Press <Enter> to continue: ");
        input_string(String);
        printf("\r\r");
      break;

      case (5):
        /* Alarms. */
        printf("\r\r");
        printf("     ========================= Display alarms =========================\r\r");
        for (Loop1UInt16 = 0; Loop1UInt16 < MAX_ALARMS; ++Loop1UInt16)
          display_alarm(Loop1UInt16);
        printf("Press <Enter> to continue: ");
        input_string(String);
        printf("\r\r");
      break;

      case (6):
        /* Calendar events of the day. */
        printf("\r\r");
        printf("========================= Display calendar events for today %u-%s-%4u =========================\r\r",
               CurrentTime.DayOfMonth, ShortMonth[CurrentTime.Month], CurrentTime.Year);
        printf("Event   Day     Month     Jingle          Message\r\r");
        Dum1UInt16 = 0;
        for (Loop1UInt16 = 0; Loop1UInt16 < MAX_EVENTS; ++Loop1UInt16)
        {
          if (FlashConfig1.Event[Loop1UInt16].Day   != CurrentTime.DayOfMonth) continue;
          if (FlashConfig1.Event[Loop1UInt16].Month != CurrentTime.Month)  continue;
          ++Dum1UInt16;
          printf("  %2u     %2u       %2u        %2u    <%s>\r", Loop1UInt16, FlashConfig1.Event[Loop1UInt16].Day, FlashConfig1.Event[Loop1UInt16].Month, FlashConfig1.Event[Loop1UInt16].Jingle, FlashConfig1.Event[Loop1UInt16].Message);
        }
        printf("\r");

        switch (Dum1UInt16)
        {
          case (0):
            printf("No event defined for today\r\r");
          break;

          case (1):
            printf("1 event defined for today\r\r");
          break;

          default:
            printf("%u events defined for today\r\r", Dum1UInt16);
          break;
        }
        printf("\r");
        printf("Press <Enter> to continue: ");
        input_string(String);
        printf("\r\r");
      break;

      case (7):
        /* Calendar events for current week. Get a working copy of current date. */
        ds3231_get_time(&HumanTime);
        if (DebugBitMask & DEBUG_EVENT)
          uart_send(__LINE__, __func__, "Today's date is %9s [%u] %2u-%s-%4.4u\r", DayName[HumanTime.DayOfWeek], HumanTime.DayOfWeek, HumanTime.DayOfMonth, ShortMonth[HumanTime.Month], HumanTime.Year);

        /* Retrieve beginning of week (previous Sunday). */
        while (HumanTime.DayOfWeek != SUN)
        {
          --HumanTime.DayOfMonth;  // back one day.
          if (HumanTime.DayOfMonth == 0)
          {
            /* If we passed beginning of month, revert to previous month. */
            --HumanTime.Month;
            if (HumanTime.Month == 0)
            {
              /* If we passed first day of the year, revert to previous year. */
              --HumanTime.Year;
              HumanTime.Month = 12;
            }
            /* if we revert to previous month, assign last day of this month. */
            HumanTime.DayOfMonth = get_month_days(HumanTime.Month, HumanTime.Year);
          }

          /* Get day-of-week once we get back one day. */
          HumanTime.DayOfWeek = get_day_of_week(HumanTime.DayOfMonth, HumanTime.Month, HumanTime.Year);

          if (DebugBitMask & DEBUG_EVENT)
            uart_send(__LINE__, __func__, "Back one day:   %9s [%u] %2u-%s-%4.4u\r", DayName[HumanTime.DayOfWeek], HumanTime.DayOfWeek, HumanTime.DayOfMonth, ShortMonth[HumanTime.Month], HumanTime.Year);
        }

        /* Display header, including date of the first day of the week (Sunday) that has been found. */
        printf("\r\r");
        printf("====================== Display calendar events for this week, beginning %s %2.2u-%s-%u ======================\r\r", DayName[HumanTime.DayOfWeek], HumanTime.DayOfMonth, ShortMonth[HumanTime.Month], HumanTime.Year);
        printf("Event   Day     Month     Jingle          Message\r\r");

        /* And display all events for each of the next seven days, starting with this Sunday. */
        Dum1UInt16 = 0;

        /* Scan each day of current week. */
        for (Loop1UInt16 = 0; Loop1UInt16 < 7; ++Loop1UInt16)
        {
          if (DebugBitMask & DEBUG_EVENT)
          {
            uart_send(__LINE__, __func__, " Checking date:  %9s %2u-%s-%4.4u\r", DayName[Loop1UInt16], HumanTime.DayOfMonth, ShortMonth[HumanTime.Month], HumanTime.Year);
            uart_send(__LINE__, __func__, " -------------------------------------------\r\r");
          }

          /* Check every calendar event to find a match with date under evaluation. */
          for (Loop2UInt16 = 0; Loop2UInt16 < MAX_EVENTS; ++Loop2UInt16)
          {
            if (DebugBitMask & DEBUG_EVENT)
            {
              uart_send(__LINE__, __func__, "Checking event number %2u  %2u-%3s [%s]\r", Loop2UInt16, FlashConfig1.Event[Loop2UInt16].Day, ShortMonth[FlashConfig1.Event[Loop2UInt16].Month], FlashConfig1.Event[Loop2UInt16].Message);
              sleep_ms(20);  // prevent communication override.
            }

            if ((FlashConfig1.Event[Loop2UInt16].Day == HumanTime.DayOfMonth) && (FlashConfig1.Event[Loop2UInt16].Month == HumanTime.Month))
            {
              if (DebugBitMask & DEBUG_EVENT) uart_send(__LINE__, __func__, "Match found !\r");
              printf("  %2u     %2u       %2u        %2u    <%s>\r", Loop2UInt16, FlashConfig1.Event[Loop2UInt16].Day, FlashConfig1.Event[Loop2UInt16].Month, FlashConfig1.Event[Loop1UInt16].Jingle, FlashConfig1.Event[Loop2UInt16].Message);
              ++Dum1UInt16;  // one more event found.
            }
          }

          /* Check next date. */
          ++HumanTime.DayOfMonth;
          if (HumanTime.DayOfMonth > get_month_days(HumanTime.Month, HumanTime.Year))
          {
            HumanTime.DayOfMonth = 1;
            ++HumanTime.Month;
            if (HumanTime.Month > 12)
            {
              HumanTime.Month = 1;
              ++HumanTime.Year;
            }
          }
          printf("\r\r");
        }

        switch (Dum1UInt16)
        {
          case (0):
            printf("No event defined for this week\r\r");
          break;

          case (1):
            printf("1 event defined for this week\r\r");
          break;

          default:
            printf("%u events defined for this week\r\r", Dum1UInt16);
          break;
        }
        printf("\r");
        printf("Press <Enter> to continue: ");
        input_string(String);
        printf("\r\r");
      break;

      case (8):
        /* Calendar events of current month. */
        printf("\r\r");
        printf("========================= Display calendar events for %s-%4.4u =========================\r\r", ShortMonth[CurrentTime.Month], CurrentTime.Year);
        printf("Event   Day     Month     Jingle          Message\r\r");
        Dum1UInt16 = 0;
        for (Loop1UInt16 = 0; Loop1UInt16 < MAX_EVENTS; ++Loop1UInt16)
        {
          if (FlashConfig1.Event[Loop1UInt16].Day   == 0) continue;  // skip unused calendar events that are initialized as 0-JAN-2100
          if (FlashConfig1.Event[Loop1UInt16].Month != CurrentTime.Month) continue;
          ++Dum1UInt16;
          printf("  %2u     %2u       %2u        %2u    <%s>\r", Loop1UInt16, FlashConfig1.Event[Loop1UInt16].Day, FlashConfig1.Event[Loop1UInt16].Month, FlashConfig1.Event[Loop1UInt16].Jingle, FlashConfig1.Event[Loop1UInt16].Message);
        }
        printf("\r");

        switch (Dum1UInt16)
        {
          case (0):
            printf("No event defined for current month\r\r");
          break;

          case (1):
            printf("1 event defined for current month\r\r");
          break;

          default:
            printf("%u events defined for current month\r\r", Dum1UInt16);
          break;
        }
        printf("\r");
        printf("Press <Enter> to continue: ");
        input_string(String);
        printf("\r\r");
      break;

      case (9):
        /* Calendar events of the year (all). */
        printf("\r\r");
        printf("========================= Display all calendar events =========================\r\r");
        util_uint64_to_binary_string(EventBitMask, 64, String);
        printf("EventBitMask: 0x%16.16X   %s\r\r", EventBitMask, String);
        printf("Event   Day     Month     Jingle          Message\r\r");
        Dum1UInt16 = 0;
        for (Loop1UInt16 = 0; Loop1UInt16 < MAX_EVENTS; ++Loop1UInt16)
        {
          ++Dum1UInt16;
          printf("  %2u     %2u       %2u        %2u    <%s>\r", Loop1UInt16, FlashConfig1.Event[Loop1UInt16].Day, FlashConfig1.Event[Loop1UInt16].Month, FlashConfig1.Event[Loop1UInt16].Jingle, FlashConfig1.Event[Loop1UInt16].Message);
        }
        printf("\r");

        switch (Dum1UInt16)
        {
          case (0):
            printf("No event defined in the system.\r\r");
          break;

          case (1):
            printf("1 event defined in the system.\r\r");
          break;

          default:
            printf("%u events defined in the system.\r\r", Dum1UInt16);
          break;
        }
        printf("\r");
        printf("Press <Enter> to continue: ");
        input_string(String);
        printf("\r\r");
      break;

      case (10):
        /* Daylight Saving Time info. */
        printf("\r\r");
        printf("Daylight Saving Time info:\r");
        printf("==========================\r");
        printf("Daylight Saving Time (DST) country setting:   %2u   (refer to User Guide for details)\r", FlashConfig1.DSTCountry);
        printf("Coordinated Universal Time (UTC) / Timezone: %3d\r\r", FlashConfig1.Timezone);
        printf("Press <Enter> to continue: ");
        input_string(String);
        printf("\r\r");
      break;

      case (11):
#ifndef NTP_SUPPORT
        printf("\r\r");
        uart_send(__LINE__, __func__, "Reserved for Firmware with network time protocol (with a PicoW)\r");
#else  // NTP_SUPPORT
        /* Network credentials and Network Time Protocol info. */
        printf("\r\r");
        uart_send(__LINE__, __func__, "=========================================================\r");
        uart_send(__LINE__, __func__, "   Network credentials and Network Time Protocol info\r");
        uart_send(__LINE__, __func__, "=========================================================\r");
        uart_send(__LINE__, __func__, "   Network name (SSID): <%s>\r", FlashConfig1.SSID);
        uart_send(__LINE__, __func__, "   Network password:    <%s>\r", FlashConfig1.Password);
        display_ntp_info();
        printf("Press <Enter> to continue: ");
        input_string(String);
        printf("\r\r");
#endif  // NTP_SUPPORT
      break;

      case (12):
        /* Idle Time Monitor info. */
        printf("\r\r");
        printf("Idle Time Monitor info - To be implemented...\r");
        printf("=============================================\r\r");
        printf("Press <Enter> to continue: ");
        input_string(String);
        printf("\r\r");
      break;

      case (13):
        /* Silence period info. */
        printf("\r\r");
        printf("Silence period info - To be implemented...\r");
        printf("==========================================\r\r");
        printf("Press <Enter> to continue: ");
        input_string(String);
        printf("\r\r");
      break;

      case (14):
        /* Find first free memory chunk in the heap. */
        Dum1Ptr8 = (UINT8 *)malloc(sizeof(struct active_scroll));
        free(Dum1Ptr8);

        /* Display first free memory chunk in the heap. */
        printf("\r\r");
        printf("First free memory area in the heap: 0x%p\r\r", Dum1Ptr8);
        printf("Press <Enter> to continue: ");
        input_string(String);
        printf("\r\r");
      break;

      case (15):
        /* Flash configuration display. */
        printf("\r\r");
        flash_display_config1();
        flash_display_config2();
        printf("Press <Enter> to continue: ");
        input_string(String);
        printf("\r\r");
      break;

      case (16):
        /* Windows status info. */
        printf("\r\r");
        printf("     ========================= Display windows =========================\r\r");
        display_windows();
        printf("Press <Enter> to continue: ");
        input_string(String);
        printf("\r\r");
      break;

      case (17):
        /* Auto-scrolls. */
        printf("\r\r");
        printf(" ========================= Display auto-scrolls =========================\r\r");
        for (Loop1UInt16 = 0; Loop1UInt16 < MAX_AUTO_SCROLLS; ++Loop1UInt16)
          display_auto_scroll(Loop1UInt16);
        printf("Press <Enter> to continue: ");
        input_string(String);
        printf("\r\r");
      break;

      case (18):
        /* Active scrolls status. */
        printf("\r\r");
        uart_send(__LINE__, __func__, " ============================== Display scrolls ===============================\r\r");
        display_scroll();
        printf("Press <Enter> to continue: ");
        input_string(String);
        printf("\r\r");
      break;

      case (19):
        /* Display available functions. */
        printf("\r\r");
        printf(" ======================== Display functions =========================\r\r");
        for (Loop1UInt16 = 0; Loop1UInt16 < MAX_FUNCTIONS; ++Loop1UInt16)
          display_function(Loop1UInt16);
        printf("Press <Enter> to continue: ");
        input_string(String);
        printf("\r\r");
      break;

      case (20):
        /* Display current display buffers content. */
        printf("\r\r");
        printf(" ======================== Display current buffer content =========================\r\r");
        printf(" ---------------------------------- FrameBuffer ----------------------------------\r\r");
        display_matrix_buffer(FrameBuffer);
        printf("\r\r");
        printf(" ---------------------------------- CheckBuffer ----------------------------------\r\r");
        display_matrix_buffer(CheckBuffer);
        printf("\r\r");
        printf(" ---------------------------------- BlinkBuffer ----------------------------------\r\r");
        display_matrix_buffer(BlinkBuffer);
        printf("\r\r");
        printf("Press <Enter> to continue: ");
        input_string(String);
        printf("\r\r");
      break;

      case (21):
        /* Display total RGB Matrix Up time. */
        printf("\r\r");
        printf(" ======================== Display total RGB Matrix Up time =========================\r\r");
        display_up_time();
        printf("\r");
        printf("Press <Enter> to continue: ");
        input_string(String);
        printf("\r\r");
      break;

      case (22):
        /*  */
        printf("\r\r");
        printf("To be implemented...\r\r");
        printf("Press <Enter> to continue: ");
        input_string(String);
        printf("\r\r");
      break;

      case (23):
        /*  */
        printf("\r\r");
        printf("To be implemented...\r\r");
        printf("Press <Enter> to continue: ");
        input_string(String);
        printf("\r\r");
      break;

      default:
        printf("\r\r");
        printf("                    Invalid choice... please re-enter [%s]  [%u]\r\r\r\r\r", String, Menu);
        printf("\r\r");
      break;
    }
  }

  return;
}





/* $TITLE=term_ir_feedback_setup()) */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                      Terminal submenu for remote control audible feedback setup.
\* ============================================================================================================================================================= */
void term_ir_feedback_setup(void)
{
  UCHAR String[31];


  while (1)
  {
    if (FlashConfig1.FlagIrFeedback == FLAG_ON)
      printf("Remote control audible feedback is now turned On.\r");
    else
      printf("Remote control audible feedback is now turned Off.\r");
    printf("Press <c> to change this setting\r");
    printf("<Enter> to keep it this way\r");
    printf("<ESC> to exit button feedback setup: ");

    input_string(String);
    if (String[0] == 0x0D) break;
    if (String[0] == 27)   return;
    if ((String[0] == 'C') || (String[0] == 'c'))
      FlashConfig1.FlagIrFeedback ^= 0x01;
  }
  printf("\r\r");

  return;
}





/* $TITLE=term_display_memory() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                  Terminal submenu to display a section of Pico's memory.
\* ============================================================================================================================================================= */
void term_memory_display(void)
{
  UCHAR String[31];

  UINT32 StartAddress;
  UINT32 DisplayLength;


  printf("     Display a section of Pico's memory.\r");
  printf("   ---------------------------------------\r\r");
  printf("Pico's Flash memory address space: 0x10000000 to 0x10200000\r");
  printf("Pico's RAM   memory address space: 0x20000000 to 0x20042000\r");


  printf("Enter start memory address to display (in Hex)\r");
  printf("<ESC> to exit memory display submenu: ");
  input_string(String);
  if (String[0] == 27) return;
  StartAddress = strtol(String, NULL, 16);

  printf("Enter the length of the memory to display (in Hex)\r");
  printf("<ESC> to exit memory display submenu: ");
  input_string(String);
  if (String[0] == 27) return;
  DisplayLength = strtol(String, NULL, 16);
  printf("\r\r\r");

  util_display_data((UCHAR *)StartAddress, DisplayLength);

  return;
}





/* $TITLE=term_menu()) */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                          Terminal menu when a CDC USB connection is detected during power up sequence.
\* ============================================================================================================================================================= */
void term_menu(void)
{
  UCHAR String[128];

  UINT8 Loop1UInt8;
  UINT8 Menu;


  while (1)
  {
    printf("\r\r\r\r");
    printf("                         Terminal menu\r\r");
    printf("               1) - Functions SETUP submenu.\r");
    printf("               2) - Functions INFO submenu.\r");
    printf("               3) - Functions TOOLS submenu.\r");
    printf("               4) - Erase flash configuration.\r");
    printf("               5) - Select a specific test to execute.\r");
    printf("               6) - Turn On or Off debug flags.\r");
    printf("               7) - Display a section of Pico's memory.\r");
    printf("               8) - Display current Unix time\r");
    printf("               9) - Display one-second callback duration history\r");
    printf("              10) - Remote control tests\r");
    printf("              11) - Test software reset\r");
    printf("             ESC) - Switch to clock normal behavior.\r\r");

    printf("            Remember that main system loop is suspended\r");
    printf("         while you navigate in terminal menus and submenus.\r");
    if (FlagEndlessLoop == FLAG_ON)
      printf("                 Main system loop has been started.\r\r");
    else
      printf("             Main system loop has not been started yet.\r\r");
    printf("                       Enter your choice: ");
    input_string(String);

    /* If user pressed <Enter> only, loop back to menu. */
    if (String[0] == 0x0D) continue;

    /* If user pressed <ESC> restart clock and continue with normal clock behavior. */
    if (String[0] == 27)
    {
      String[0] = 0x00;
      printf("\r\r\r");

      if (WinTop == WIN_TEST)
      {
        win_close(WIN_TEST);
        win_open(WIN_TIME, FLAG_ON);
      }

      TermModeTimer = time_us_64();  // keep track of last time we exited from terminal menu.

      return;
    }


    /* User pressed a menu option, execute it. */
    Menu = atoi(String);

    switch(Menu)
    {
      case (1):
        /* Functions setup submenu. */
        printf("\r\r");
        term_setup();
        printf("\r\r");
      break;

      case (2):
        /* Functions info submenu. */
        printf("\r\r");
        term_info();
        printf("\r\r");
      break;

      case (3):
        /* Functions tools submenu. */
        printf("\r\r");
        term_tools();
        printf("\r\r");
      break;

      case (4):
        /* Flash configuration erasing. */
        printf("\r\r");
        term_erase_flash();
        printf("\r\r");
      break;

      case (5):
        /* Test menu. */
        printf("\r\r");
        term_test();
        printf("\r\r");
      break;

      case (6):
        /* Debug flags. */
        printf("\r\r");
        term_debug();
        printf("\r\r");
      break;

      case (7):
        /* Display a section of memory. */
        printf("\r\r");
        term_memory_display();
        printf("\r\r");
      break;

      case (8):
        /* Display current Unix time. */
        printf("\r\r");
        display_current_unix_time();
        printf("\r\r");
      break;

      case (9):
        /* One-second callback duration history. */
        printf("\r\r");
        display_one_second_callback();
        printf("\r\r");
      break;

      case (10):
        /* Remote control test. */
        printf("\r\r");
        remote_control_test();
        printf("\r\r");
      break;

      case (11):
        /* Test software reset. */
        printf("\r\r");
        printf("This option us used to test RGB Matrix software reset.\r\r");
        printf("===== WARNING   WARNING   WARNING   WARNING   WARNING =====\r\r");
        printf("By pressing <G>, you will reset (restart) the RGB Matrix.\r\r");
        printf("Press <G> to reset the RGB Matrix, or <ESC> to return to menu without restarting: ");
        input_string(String);
        if ((String[0] == 'G') || (String[0] == 'g'))
        {
          software_reset();
        }
        printf("\r\r");
      break;

      case (12):
        /* Test debug_pixel() */
        printf("\r\r");
        printf("Test debug_pixel()");
        for (Loop1UInt8 = 0; Loop1UInt8 < MAX_COLUMNS; ++Loop1UInt8)
        {
          debug_pixel(31, Loop1UInt8, BLUE);
          sleep_ms(500);
        }
        sleep_ms(2000);
        printf("Press <Enter> to continue: ");
        input_string(String);
      break;

      case (13):
        /*   */
        printf("\r\r");
        printf("Undefined...");
        sleep_ms(3000);
        printf("\r\r");
      break;

      case (14):
        /*   */
        printf("\r\r");
        printf("Undefined...");
        sleep_ms(3000);
        printf("\r\r");
      break;

      case (15):
        /*   */
        printf("\r\r");
        printf("Undefined...");
        sleep_ms(3000);
        printf("\r\r");
      break;

      case (16):
        /*   */
        printf("\r\r");
        printf("Undefined...");
        sleep_ms(3000);
        printf("\r\r");
      break;

      case (17):
        /*   */
        printf("\r\r");
        printf("Undefined...");
        sleep_ms(3000);
        printf("\r\r");
      break;

      case (18):
        /*   */
        printf("\r\r");
        printf("Undefined...");
        sleep_ms(3000);
        printf("\r\r");
      break;

      case (19):
        /*   */
        printf("\r\r");
        printf("Undefined...");
        sleep_ms(3000);
        printf("\r\r");
      break;

      case (20):
        /*   */
        printf("\r\r");
        printf("Undefined...");
        sleep_ms(3000);
        printf("\r\r");
      break;

      default:
        printf("\r\r");
        printf("                    Invalid choice... please re-enter [%s]  [%u]\r\r\r\r\r", String, Menu);
        printf("\r\r");
      break;
    }
  }

  return;
}





/* $TITLE=term_network_setup()) */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                             Terminal submenu for network credentials setup.
\* ============================================================================================================================================================= */
void term_network_setup(void)
{
  UCHAR String[81];

  UINT16 Loop1UInt16;


#ifndef NTP_SUPPORT
  win_scroll(WinTop, 201, 201, 1, 1, FONT_5x7, "Network not supported in this version of Firmware");
#else  // NTP_SUPPORT
  /* User may change and validate credentials, so trigger a new NTP cycle. */
  NTPData.NTPUpdateTime = make_timeout_time_ms(NTPData.NTPRefresh * 1000);


  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                    Enter network SSID (network name).
  \* --------------------------------------------------------------------------------------------------------------------------- */
  NTPData.FlagNTPResync = FLAG_ON;
  while (1)
  {
    /* Display current SSID (network name), but only displayable characters. */
    printf("Current SSID (network name):  <");
    for (Loop1UInt16 = 0; Loop1UInt16 < sizeof(FlashConfig1.SSID); ++Loop1UInt16)
    {
      if  (FlashConfig1.SSID[Loop1UInt16] == 0x00) break;  // get out of <for> loop when reaching end-of-string.
      if ((FlashConfig1.SSID[Loop1UInt16] >= 0x20) && (FlashConfig1.SSID[Loop1UInt16] <= 0x7E))
        printf("%c", FlashConfig1.SSID[Loop1UInt16]);
      else
        printf("?");
    }
    printf(">\r\r");

    printf("Enter new SSID (network name), <Enter> to keep current one or <ESC> to exit network setup\r");
    printf("(maximum %u characters):\r", (sizeof(FlashConfig1.SSID) - 1));

    /* Display a template for maximum SSID length. */
    printf("[");
    for (Loop1UInt16 = 0; Loop1UInt16 < (sizeof(FlashConfig1.SSID) - 1); ++Loop1UInt16) printf("-");
    printf("]\r ");

    input_string(String);
    if (String[0] == 0x0D) break;
    if (String[0] == 0x1B) return;

    while (strlen(String) > sizeof(FlashConfig1.SSID))
    {
      printf("SSID too long (by %u characters)\r", strlen(String) - sizeof(FlashConfig1.SSID) - 1);
      printf("Enter new SSID (network name), <Enter> to keep current one or <ESC> to exit network setup\r");
      printf("(maximum %u characters):\r", sizeof(FlashConfig1.SSID) - 1);
      input_string(String);
      if (String[0] == 0x0D) break;
      if (String[0] == 0x1B) return;
    }
    for (Loop1UInt16 = 0; Loop1UInt16 < sizeof(FlashConfig1.SSID); ++Loop1UInt16) FlashConfig1.SSID[Loop1UInt16] = 0x00;  // wipe current data.
    sprintf(FlashConfig1.SSID, String);  // then write new SSID
  }



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                           Enter password.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  while (1)
  {
    /* Display current network password, but only displayable characters. */
    printf("Current network password:  <");
    for (Loop1UInt16 = 0; Loop1UInt16 < sizeof(FlashConfig1.Password); ++Loop1UInt16)
    {
      if  (FlashConfig1.Password[Loop1UInt16] == 0x00) break;  // get out of <for> loop when reaching end-of-string.
      if ((FlashConfig1.Password[Loop1UInt16] >= 0x20) && (FlashConfig1.Password[Loop1UInt16] <= 0x7E))
        printf("%c", FlashConfig1.Password[Loop1UInt16]);
      else
        printf("?");
    }
    printf(">\r\r");

    printf("Enter new network password, <Enter> to keep current one or <ESC> to exit network setup\r");
    printf("(maximum %u characters):\r", (sizeof(FlashConfig1.Password) - 1));

    /* Display a template for maximum Password length. */
    printf("[");
    for (Loop1UInt16 = 0; Loop1UInt16 < (sizeof(FlashConfig1.Password) - 1); ++Loop1UInt16) printf("-");
    printf("]\r ");

    input_string(String);
    if (String[0] == 0x0D) break;
    if (String[0] == 0x1B) return;

    while (strlen(String) > sizeof(FlashConfig1.Password))
    {
      printf("Password too long (by %u characters)\r", strlen(String) - sizeof(FlashConfig1.Password) - 1);
      printf("Enter new network password, <Enter> to keep current one or <ESC> to exit network setup\r");
      printf("(maximum %u characters):\r", sizeof(FlashConfig1.Password) - 1);
      input_string(String);
      if (String[0] == 0x0D) break;
      if (String[0] == 0x1B) return;
    }
    for (Loop1UInt16 = 0; Loop1UInt16 < sizeof(FlashConfig1.Password); ++Loop1UInt16) FlashConfig1.Password[Loop1UInt16] = 0x00;  // wipe current data.
    sprintf(FlashConfig1.Password, String);
  }
#endif  // NTP_SUPPORT

  return;
}





/* $TITLE=term_reminder1_setup()) */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                            Terminal submenu for reminders of type 1 setup.
\* ============================================================================================================================================================= */
void term_reminder1_setup(void)
{
  UCHAR String[65];

  UINT8 ReminderNumber;

  UINT16 Loop1UInt16;

  UINT64 Dum1UInt64;

  time_t UnixTime;

  struct human_time HumanTime;
  struct tm TempTime;


  printf("-------------------------------------- Reminders setup -------------------------------------- \r\r");

  while (1)
  {
    /* Display current reminders of type 1 settings for all reminders1. */
    for (Loop1UInt16 = 0; Loop1UInt16 < MAX_REMINDERS1; ++Loop1UInt16)
    {
      display_reminder1(Loop1UInt16);
    }



    /* --------------------------------------------------------------------------------------------------------------------------- *\
                                            Selection of a reminder number to review / change.
    \* --------------------------------------------------------------------------------------------------------------------------- */
    printf("\r\r");
    printf("Enter reminder number you want to change or <ESC> to exit reminder setup: ");
    input_string(String);
    if (String[0] == 0x1B) return;
    ReminderNumber = atoi(String) - 1;
    while (ReminderNumber >= MAX_REMINDERS1)
    {
      printf("Invalid reminder number. Enter reminder number (1 to %u) or <ESC> to exit reminder setup: ", MAX_REMINDERS1);
      input_string(String);
      if (String[0] == 0x1B) return;
      ReminderNumber = atoi(String) - 1;
    }
    printf("\r\r\r");



    while (1)
    {
      display_reminder1(ReminderNumber);

      /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                        Enter date and time for start period.
                                        NOTE: Start period also corresponds to the time of first alarm ring.
      \* --------------------------------------------------------------------------------------------------------------------------- */
      if (FlashConfig2.Reminder1[ReminderNumber].StartPeriodUnixTime != 0ll)
      {
        convert_unix_time(FlashConfig2.Reminder1[ReminderNumber].StartPeriodUnixTime, &TempTime, &HumanTime, FLAG_ON);
      }
      else
      {
        /* Initialize with default meaningless values. */
        HumanTime.Hour       = 0;
        HumanTime.Minute     = 0;
        HumanTime.Second     = 0;
        HumanTime.DayOfMonth = 1;
        HumanTime.Month      = 1;
        HumanTime.Year       = 2100;
        HumanTime.DayOfWeek  = 0;
        HumanTime.DayOfYear  = 1;
        HumanTime.FlagDst    = 0;
      }

      printf("------------------------------------------------------------------------------\r");
      printf("                     Enter date and time for start period\r");
      printf("------------------------------------------------------------------------------\r");
      enter_human_time(&HumanTime, FLAG_ON, FLAG_ON);

      display_human_time("Start period - Display data entered before converting to Unix time:", &HumanTime);
      UnixTime = convert_human_to_unix(&HumanTime, FLAG_ON);

      /* Set StartPeriod Unix time to the equivalent of the human time. */
      FlashConfig2.Reminder1[ReminderNumber].StartPeriodUnixTime = (UnixTime - (FlashConfig1.Timezone * 60 * 60));
      if (DebugBitMask & DEBUG_NTP) uart_send(__LINE__, __func__, "UnixTime for start period: %llu\r", FlashConfig2.Reminder1[ReminderNumber].StartPeriodUnixTime );





      /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                         Enter date and time for end period.
      \* --------------------------------------------------------------------------------------------------------------------------- */
      if (FlashConfig2.Reminder1[ReminderNumber].EndPeriodUnixTime != 0ll)
      {
        convert_unix_time(FlashConfig2.Reminder1[ReminderNumber].EndPeriodUnixTime, &TempTime, &HumanTime, FLAG_ON);
      }
      else
      {
        /* Initialize with default meaningless values. */
        HumanTime.Hour       = 0;
        HumanTime.Minute     = 0;
        HumanTime.Second     = 0;
        HumanTime.DayOfMonth = 1;
        HumanTime.Month      = 1;
        HumanTime.Year       = 2100;
        HumanTime.DayOfWeek  = 0;
        HumanTime.DayOfYear  = 1;
        HumanTime.FlagDst    = 0;
      }

      printf("------------------------------------------------------------------------------\r");
      printf("                      Enter date and time for end period\r");
      printf("------------------------------------------------------------------------------\r");
      enter_human_time(&HumanTime, FLAG_ON, FLAG_ON);

      display_human_time("End period - Display data entered before converting to Unix time:", &HumanTime);
      UnixTime = convert_human_to_unix(&HumanTime, FLAG_ON);

      /* Set EndPeriod Unix time to the equivalent human time. */
      FlashConfig2.Reminder1[ReminderNumber].EndPeriodUnixTime = (UnixTime - (FlashConfig1.Timezone * 60 * 60));
      if (DebugBitMask & DEBUG_NTP) uart_send(__LINE__, __func__, "UnixTime for end period: %llu\r", FlashConfig2.Reminder1[ReminderNumber].EndPeriodUnixTime );





      /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                        Enter ring repeat time seconds.
      \* --------------------------------------------------------------------------------------------------------------------------- */
      while (1)
      {
        printf("-------------------------------------------------------------------------------\r");
        printf("     Enter ring repeat time in seconds (0 to 86400 where 86400 = 24 hours)\r");
        printf("-------------------------------------------------------------------------------\r");
        printf("Current value for ring repeat time is: %llu seconds\r", FlashConfig2.Reminder1[ReminderNumber].RingRepeatTimeSeconds);
        printf("Enter ring repeat time, <Enter> to keep current value or <ESC> to exit reminders setup: ");
        input_string(String);
        if (String[0] == 0x1B) return;
        if (String[0] == 0x0D) break;
        Dum1UInt64 = atoi(String);
        while (Dum1UInt64 > 86400)
        {
          printf("Invalid value, please re-enter (0 to 86400): ");
          input_string(String);
          if (String[0] == 0x1B) return;
          Dum1UInt64 = atoi(String);
        }
        FlashConfig2.Reminder1[ReminderNumber].RingRepeatTimeSeconds = Dum1UInt64;
      }
      printf("\r\r");





      /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                     Enter global ring duration seconds.
      \* --------------------------------------------------------------------------------------------------------------------------- */
      while (1)
      {
        printf("-------------------------------------------------------------------------------\r");
        printf("   Enter global ring duration in seconds (0 to 86400 where 86400 = 24 hours)\r");
        printf("-------------------------------------------------------------------------------\r");
        printf("Current value for ring duration is: %llu seconds\r", FlashConfig2.Reminder1[ReminderNumber].RingDurationSeconds);
        printf("Enter ring duration, <Enter> to keep current value or <ESC> to exit reminders setup: ");
        input_string(String);
        if (String[0] == 0x1B) return;
        if (String[0] == 0x0D) break;
        Dum1UInt64 = atoi(String);
        while (Dum1UInt64 > 86400)
        {
          printf("Invalid value, please re-enter (0 to 86400): ");
          input_string(String);
          if (String[0] == 0x1B) return;
          Dum1UInt64 = atoi(String);
        }
        FlashConfig2.Reminder1[ReminderNumber].RingDurationSeconds = Dum1UInt64;
      }
      printf("\r\r");





      /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                       Delay for next reminder (in seconds).
      \* --------------------------------------------------------------------------------------------------------------------------- */
      while (1)
      {
        printf("------------------------------------------------------------------------------\r");
        printf("                  Enter delay for next reminder (in seconds)\r");
        printf("------------------------------------------------------------------------------\r");
        printf("     (3600 = 1 hour   86400 = 1 day   604800 = 1 week   31449600 = 1 year)\r");
        printf("Current value for next reminder delay is: %llu seconds\r", FlashConfig2.Reminder1[ReminderNumber].NextReminderDelaySeconds);
        printf("Enter next reminder delay, <Enter> to keep current value or <ESC> to exit reminders setup: ");
        input_string(String);
        if (String[0] == 0x1B) return;
        if (String[0] == 0x0D) break;
        Dum1UInt64 = atoi(String);
        while (Dum1UInt64 > 31449600)
        {
          printf("Invalid value, please re-enter (0 to 31449600): ");
          input_string(String);
          if (String[0] == 0x1B) return;
          Dum1UInt64 = atoi(String);
        }
        FlashConfig2.Reminder1[ReminderNumber].NextReminderDelaySeconds = Dum1UInt64;
      }
      printf("\r\r");





      /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                            Enter reminder message.
      \* --------------------------------------------------------------------------------------------------------------------------- */
      /* Display message for this event. */
      printf("Message:  <");
      for (Loop1UInt16 = 0; Loop1UInt16 < sizeof(FlashConfig2.Reminder1[ReminderNumber].Message); ++Loop1UInt16)
      {
        if  (FlashConfig2.Reminder1[ReminderNumber].Message[Loop1UInt16] == 0x00) break;  // get out of <for> loop when reaching end-of-string.
        if ((FlashConfig2.Reminder1[ReminderNumber].Message[Loop1UInt16] >= 0x20) && (FlashConfig2.Reminder1[ReminderNumber].Message[Loop1UInt16] <= 0x7E))
          printf("%c", FlashConfig2.Reminder1[ReminderNumber].Message[Loop1UInt16]);
        else
          printf("?");
      }
      printf(">\r\r");

      printf("Enter new message for this reminder\r");
      printf("<Enter> to keep current message\r");
      printf("or <ESC> to exit reminders setup\r");
      printf("(maximum %u characters):\r", (sizeof(FlashConfig2.Reminder1[ReminderNumber].Message) - 1));

      /* Display a template for maximum reminder1 message length. */
      printf("[");
      for (Loop1UInt16 = 0; Loop1UInt16 < (sizeof(FlashConfig2.Reminder1[ReminderNumber].Message) - 1); ++Loop1UInt16) printf("-");
      printf("]\r ");

      input_string(String);
      if (String[0] == 0x0D) break;
      if (String[0] == 0x1B)
      {
        printf(("Updating reminders...\r\r"));
        sleep_ms(3000);
        reminder1_check();
        return;
      }

      while (strlen(String) > sizeof(FlashConfig2.Reminder1[ReminderNumber].Message))
      {
        printf("Message too long (by %u characters)\r", strlen(String) - sizeof(FlashConfig2.Reminder1[ReminderNumber].Message));
        printf("Enter new message for this reminder\r");
        printf("<Enter> to keep current message\r");
        printf("or <ESC> to exit reminders setup\r");
        printf("(maximum %u characters):\r", sizeof(FlashConfig2.Reminder1[ReminderNumber].Message));
        input_string(String);
        if (String[0] == 0x0D) break;
        if (String[0] == 0x1B)
        {
          printf(("Updating reminders...\r\r"));
          sleep_ms(3000);
          reminder1_check();
          return;
        }
      }
      sprintf(FlashConfig2.Reminder1[ReminderNumber].Message, String);
    }
  }

  return;
}





/* $TITLE=term_setup()) */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                                   Terminal submenu for RGB matrix setup.
\* ============================================================================================================================================================= */
void term_setup(void)
{
  UCHAR String[32];

  UINT8 Menu;


  while (1)
  {
    printf("\r\r\r\r");
    printf("                         Setup section\r\r");
    printf("               1) - Time setup.\r");
    printf("               2) - Date setup.\r");
    printf("               3) - Hourly chime setup.\r");
    printf("               4) - Buttons audio feedback setup.\r");
    printf("               5) - Remote audio feedback setup.\r");
    printf("               6) - Brightness setup.\r");
    printf("               7) - Alarms setup.\r");
    printf("               8) - Temperature unit setup.\r");
    printf("               9) - Daylight saving time setup.\r");
    printf("              10) - Network credentials setup.\r");
    printf("              11) - Golden age setup.\r");
    printf("              12) - Auto-scroll setup.\r");
    printf("              13) - Calendar events setup.\r");
    printf("              14) - Reminders of type 1 setup.\r");
    printf("             ESC) - Return to main terminal menu.\r\r");

    printf("                    Enter your choice: ");
    input_string(String);

    /* If user pressed <Enter> only, loop back to menu. */
    if (String[0] == 0x0D) continue;

    /* If user pressed <ESC>, return to term_menu(). */
    if (String[0] == 27) return;

    /* User pressed a menu option, execute it. */
    Menu = atoi(String);


    switch(Menu)
    {
      case (1):
        /* Time setup. */
        printf("\r\r");
        term_time_setup();
        printf("\r\r");
      break;

      case (2):
        /* Date setup. */
        printf("\r\r");
        term_date_setup();
        printf("\r\r");
      break;

      case (3):
        /* Hourly chime setup. */
        printf("\r\r");
        term_hourly_chime_setup();
        printf("\r\r");
      break;

      case (4):
        /* Buttons audio feedback setup. */
        printf("\r\r");
        term_button_feedback_setup();
        printf("\r\r");
      break;

      case (5):
        /* Remote control audio feedback setup. */
        printf("\r\r");
        term_ir_feedback_setup();
        printf("\r\r");
      break;

      case (6):
        /* Brightness setup. */
        printf("\r\r");
        term_brightness_setup();
        printf("\r\r");
      break;

      case (7):
        /* Alarm setup. */
        printf("\r\r");
        term_alarm_setup();
        printf("\r\r");
      break;

      case (8):
        /* Temperature unit setup. */
        printf("\r\r");
        term_temperature_setup();
        printf("\r\r");
      break;

      case (9):
        /* Daylight saving time setup. */
        printf("\r\r");
        term_dst_setup();
        printf("\r\r");
      break;

      case (10):
        /* Network credentials setup. */
        printf("\r\r");
        term_network_setup();
        printf("\r\r");
      break;

      case (11):
        /* Golden age setup */
        printf("\r\r");
        term_golden_age();
        printf("\r\r");
      break;

      case (12):
        /* Auto-scroll setup. */
        printf("\r\r");
        term_auto_scroll();
        printf("\r\r");
      break;

      case (13):
        /* Calendar events setup. */
        printf("\r\r");
        term_events_setup();
        printf("\r\r");
      break;

      case (14):
        /* Reminder1 setup. */
        printf("\r\r");
        term_reminder1_setup();
        printf("\r\r");
      break;

      case (15):
        /*  */
        printf("\r\r");
        printf("To be implemented...\r");
        sleep_ms(3000);
        printf("\r\r");
      break;

      case (16):
        /*  */
        printf("\r\r");
        printf("To be implemented...\r");
        sleep_ms(3000);
        printf("\r\r");
      break;

      case (17):
        /*  */
        printf("\r\r");
        printf("To be implemented...\r");
        sleep_ms(3000);
        printf("\r\r");
      break;

      case (18):
        /*  */
        printf("\r\r");
        printf("To be implemented...\r");
        sleep_ms(3000);
        printf("\r\r");
      break;

      case (19):
        /*  */
        printf("\r\r");
        printf("To be implemented...\r");
        sleep_ms(3000);
        printf("\r\r");
      break;

      case (20):
        /*  */
        printf("\r\r");
        printf("To be implemented...\r");
        sleep_ms(3000);
        printf("\r\r");
      break;

      case (21):
        /*  */
        printf("\r\r");
        printf("To be implemented...\r");
        sleep_ms(3000);
        printf("\r\r");
      break;

      default:
        printf("\r\r");
        printf("                    Invalid choice... please re-enter [%s]  [%u]\r\r\r\r\r", String, Menu);
        printf("\r\r");
      break;
    }
  }
}





/* $TITLE=term_temperature_setup() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                               Terminal submenu for temperature setup.
\* ============================================================================================================================================================= */
void term_temperature_setup(void)
{
  UCHAR String[31];


  printf("\r\r\r\r");
  printf("             Temperature unit setup\r\r");

  while (1)
  {
    switch (FlashConfig1.TemperatureUnit)
    {
      case (FAHRENHEIT):
        printf("Current temperature unit is Fahrenheit.\r");
      break;

      case (CELSIUS):
        printf("Current temperature unit is Celsius.\r");
      break;
    }

    printf("Press <c> to change this setting\r");
    printf("<Enter> to keep it this way\r");
    printf("<ESC> to exit golden age setup: ");

    input_string(String);
    if (String[0] == 0x0D) break;
    if (String[0] == 27)   return;
    if ((String[0] == 'C') || (String[0] == 'c'))
    {
      if (FlashConfig1.TemperatureUnit == FAHRENHEIT)
        FlashConfig1.TemperatureUnit = CELSIUS;
      else
        FlashConfig1.TemperatureUnit = FAHRENHEIT;
    }
  }
  printf("\r\r");

  return;
}





/* $TITLE=term_test() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                                   Terminal submenu for tests.
\* ============================================================================================================================================================= */
void term_test(void)
{
  UCHAR String[32];

  UINT8 Menu;


  win_open(WIN_TEST, FLAG_OFF);
  win_printf(WIN_TEST, 2, 99, FONT_5x7, "TEST");


  while (1)
  {
    printf("\r\r\r\r");
    printf("                         Test section\r\r");
    printf("               1) - Pixel set / Pixel clear.\r");
    printf("               2) - Character set bitmap tests.\r");
    printf("               3) - LED matrix brightness tests.\r");
    printf("               4) - Scrolling tests.\r");
    printf("               5) - Color setting tests.\r");
    printf("               6) - Time display layout tests.\r");
    printf("               7) - Box and Window algorithms.\r");
    printf("               8) - Undefined.\r");
    printf("               9) - Power supply requirement tests.\r");
    printf("              10) - Active buzzer sound queue.\r");
    printf("              11) - Trigger bootsel by software.\r");
    printf("             ESC) - Switch to clock normal behavior.\r\r");

    printf("                    Enter the test option you want: ");
    input_string(String);

    /* If user pressed <Enter> only, loop back to menu. */
    if (String[0] == 0x0D) continue;

    /* If user pressed <ESC>, return to term_menu(). */
    if (String[0] == 27)
    {
      win_close(WIN_TEST);
      return;
    }

    /* User pressed a menu option, execute it. */
    Menu = atoi(String);


    switch(Menu)
    {
      case (1):
        /* Pixel set / Pixel clear. */
        printf("\r\r");
        test_zone(1);
        printf("\r\r");
      break;

      case (2):
        /* Character set bitmap tests. */
        printf("\r\r");
        test_zone(2);
        printf("\r\r");
      break;

      case (3):
        /* LED matrix brightness tests. */
        printf("\r\r");
        test_zone(3);
        printf("\r\r");
      break;

      case (4):
        /* Scrolling tests. */
        printf("\r\r");
        test_zone(4);
        printf("\r\r");
      break;

      case (5):
        /* Color setting tests. */
        printf("\r\r");
        test_zone(5);
        printf("\r\r");
      break;

      case (6):
        /* Time display layout tests. */
        printf("\r\r");
        test_zone(6);
        printf("\r\r");
      break;

      case (7):
        /* Window algorithm. */
        printf("\r\r");
        test_zone(7);
        printf("\r\r");
      break;

      case (8):
        /* Power supply requirement tests. */
        printf("\r\r");
        test_zone(8);
        printf("\r\r");
      break;

      case (9):
        /* Active buzzer sound queue. */
        printf("\r\r");
        test_zone(9);
        printf("\r\r");
      break;

      case (10):
        /* Active buzzer sound queue. */
        printf("\r\r");
        test_zone(10);
        printf("\r\r");
      break;

      case (11):
        /* Trigger bootsel by software. */
        printf("\r\r");
        test_zone(11);
        printf("\r\r");
      break;

      default:
        printf("\r\r");
        printf("                    Invalid choice... please re-enter [%s]  [%u]\r\r\r\r\r", String, Menu);
        printf("\r\r");
      break;
    }
  }
}





/* $TITLE=term_time_setup()) */
/* $PAGE */
/* ============================================================================================================================================================ *\
                                                                  Terminal submenu for time setup.
\* ============================================================================================================================================================ */
void term_time_setup(void)
{
  UCHAR String[31];

  UINT8 Dum1UInt8[10];  ///

  struct human_time HumanTime;
  struct tm SetupTime;


  /* First, get current time from real-time clock IC (DS3231). */
  ds3231_get_time(&HumanTime);

  printf("\r\r\r\r");
  printf("        Time setup\r\r");
  printf("Current time is: %2.2u:%2.2u:%2.2u\r", HumanTime.Hour, HumanTime.Minute, HumanTime.Second);

  printf("Enter Hour (or <Enter> for no change)   [%2u]: ", HumanTime.Hour);
  input_string(String);
  if (String[0] != 0x0D) HumanTime.Hour = atoi(String);


  printf("Enter Minute (or <Enter> for no change) [%2.2u]: ", HumanTime.Minute);
  input_string(String);
  if (String[0] != 0x0D) HumanTime.Minute = atoi(String);


  printf("Enter Second (or <Enter> for no change) [%2.2u]: ", HumanTime.Second);
  input_string(String);
  if (String[0] != 0x0D) HumanTime.Second = atoi(String);

  ds3231_set_time(&HumanTime);
  printf("Real time clock IC has been set to: %2.2u:%2.2u:%2.2u\r", HumanTime.Hour, HumanTime.Minute, HumanTime.Second);
  printf("Press <Enter> to continue: ");
  input_string(String);

  return;
}





/* $TITLE=term_tools() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                             Terminal submenu for <tools> functions.
\* ============================================================================================================================================================= */
void term_tools(void)
{
  UCHAR String[128];

  UINT8 Menu;


  while (1)
  {
    printf("\r\r\r\r");
    printf("                       Functions <tools> submenu\r\r");
    printf("               1) - LED matrix integrity check.\r");
    printf("               2) - Count-down timer.\r");
    printf("               3) - Count-up timer.\r");
    printf("               4) - Dices rolling.\r");
    printf("               5) - Waiting queue.\r");
    printf("               6) - Store OPENED.\r");
    printf("               7) - Ambiance light.\r");
    printf("               8) - Temporary bright light.\r");
    printf("               9) - Falling snow animation.\r");
    printf("              10) - Random pixels twingling.\r");
    printf("              11) - Full RGB Matrix demo.\r");
    printf("             ESC) - Return to previous menu.\r\r");

    printf("                    Enter your choice: ");
    input_string(String);

    /* If user pressed <Enter> only, loop back to menu. */
    if (String[0] == 0x0D) continue;

    /* If user pressed <ESC> return to previous menu. */
    if (String[0] == 27)
    {
      String[0] = 0x00;
      printf("\r\r\r");

      return;
    }

    /* User pressed a menu option, execute it. */
    Menu = atoi(String);


    switch(Menu)
    {
      case (1):
        /* LED matrix integrity check. */
        printf("\r\r");
        printf("LED matrix integrity check...\r");
        RGB_matrix_integrity_check(FLAG_ON);
        printf("\r\r");
      break;

      case (2):
        /* Count-down timer. */
        printf("\r\r");
        printf("Count-down timer\r");
        printf("To be implemented...\r");
        sleep_ms(3000);
        printf("\r\r");
      break;

      case (3):
        /* Count-up timer. */
        printf("\r\r");
        printf("Count-up timer\r");
        printf("To be implemented...\r");
        sleep_ms(3000);
        printf("\r\r");
      break;

      case (4):
        /* Dice rolling. */
        printf("\r\r");
        printf("Dices rolling - To be implemented...\r");
        sleep_ms(3000);
        printf("\r\r");
      break;

      case (5):
        /* Waiting queue. */
        printf("\r\r");
        printf("Waiting queue - To be implemented...\r");
        sleep_ms(3000);
        printf("\r\r");
      break;

      case (6):
        /* Store OPENED. */
        printf("\r\r");
        printf("Store OPENED - To be implemented...\r");
        sleep_ms(3000);
        printf("\r\r");
      break;

      case (7):
        /* Ambiance light. */
        printf("\r\r");
        printf("Ambiance light - To be implemented...\r");
        sleep_ms(3000);
        printf("\r\r");
      break;

      case (8):
        /* Temporary bright light. */
        printf("\r\r");
        printf("Temporary bright light - To be implemented...\r");
        sleep_ms(3000);
        printf("\r\r");
      break;

      case (9):
        /* Falling snow animation. */
        printf("\r\r");
        printf("Falling snow animation - To be implemented...\r");
        sleep_ms(3000);
        printf("\r\r");
      break;

      case (10):
        /* Random pixels twingling. */
        printf("\r\r");
        printf("Randon pixels twingling - To be implemented...\r");
        sleep_ms(3000);
        printf("\r\r");
      break;

      case (11):
        /* Full RGB matrix demo. */
        printf("\r\r");
        printf("Full RGB Matrix demo - To be implemented...\r");
        sleep_ms(3000);
        printf("\r\r");
      break;

      default:
        printf("\r\r");
        printf("                    Invalid choice... please re-enter [%s]  [%u]\r\r\r\r\r", String, Menu);
        printf("\r\r");
      break;
    }
  }

  return;
}





/* $TITLE=test_zone() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                                        Test chunks of code.
\* ============================================================================================================================================================= */
void test_zone(UINT TestNumber)
{
  UCHAR  String[128];

  UCHAR Dum1UChar;

  UINT  Dum1UInt;
  UINT  Loop1UInt;
  UINT  Loop2UInt;

  UINT8 AsciiValue;
  UINT8 Color;
  UINT8 ColumnNumber;
  UINT8 DutyCycle;
  UINT8 EndColumn;
  UINT8 EndRow;
  UINT8 Loop1UInt8;
  UINT8 Loop2UInt8;
  UINT8 NbRows;
  UINT8 PassNumber;
  UINT8 RowNumber;
  UINT8 ScrollNumber;
  UINT8 StartColumn;
  UINT8 StartRow;
  UINT8 WindowNumber;

  UINT16 Length;
  UINT16 Loop1UInt16;
  UINT16 Loop2UInt16;
  UINT16 PwmLevel;
  UINT16 RepeatCount;

  UINT32 Frequency;
  UINT32 SystemClock;

  UINT64 Dum1UInt64;

  float  Dum1Float;


  switch (TestNumber)
  {
    case (1):
      goto Test1;
    break;

    case (2):
      goto Test2;
    break;

    case (3):
      goto Test3;
    break;

    case (4):
      goto Test4;
    break;

    case (5):
      goto Test5;
    break;

    case (6):
      goto Test6;
    break;

    case (7):
      goto Test7;
    break;

    case (8):
      goto Test8;
    break;

    case (9):
      goto Test9;
    break;

    case (10):
      goto Test10;
    break;

    case (11):
      goto Test11;
    break;

    case (12):
      goto Test12;
    break;

    case (13):
      goto Test13;
    break;

    case (14):
      goto Test14;
    break;

    case (15):
      goto Test15;
    break;

    case (16):
      goto Test16;
    break;

    case (17):
      goto Test17;
    break;

    case (18):
      goto Test18;
    break;

    case (19):
      goto Test9;
    break;

    case (20):
      goto Test20;
    break;

    default:
      goto Test1;
    break;
  }



  /* $TITLE=Test1 */
  /* $PAGE */
  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                  Test 1 - Pixel set / Pixel clear.
  \* --------------------------------------------------------------------------------------------------------------------------- */
Test1:
  printf("\r\r\r");
  uart_send(__LINE__, __func__, "Entering Test number 1\r");
  uart_send(__LINE__, __func__, "Pixel set / Pixel clear.\r");

  /* Announce test number. */
  win_cls(WIN_TEST);
  RGB_matrix_set_color(0, 0, 31, 63, BLUE);  // use blue color since this is the one drawing the least current.
  win_printf(WIN_TEST, 2, 99, FONT_5x7, "Test 1");


  /* --------------------------------------------------------------------------------------------------------------------------- *\
                             Turn On pixels row by row and column by column on the whole LED matrix.
                                            (using function RGB_matrix_set_pixel()).
  \* --------------------------------------------------------------------------------------------------------------------------- */
  uart_send(__LINE__, __func__, "Press <Enter> to begin RGB_matrix_set_pixel() test\r");
  uart_send(__LINE__, __func__, "or <ESC> to return to menu: ");
  input_string(String);
  if (String[0] == 27) return;
  RGB_matrix_cls(FrameBuffer);  // turn Off all LEDs on entry.

  for (Loop1UInt8 = 0; Loop1UInt8 < MAX_ROWS; ++Loop1UInt8)
  {
    for (Loop2UInt8 = 0; Loop2UInt8 < MAX_COLUMNS; ++Loop2UInt8)
    {
      uart_send(__LINE__, __func__, "RGB_matrix_set_pixel() - Row: %2u   Column: %2u   ", Loop1UInt8, Loop2UInt8);
      RGB_matrix_set_pixel(FrameBuffer, Loop1UInt8, Loop2UInt8, Loop1UInt8, Loop2UInt8);
      uart_send(__LINE__, __func__, "- Press <Enter> to continue or <ESC> to exit test: ");
      sleep_ms(100);  // to prevent communication override if user keep pressing <Enter>.
      input_string(String);
      if (String[0] == 27) break;
    }
    printf("\r");
    if (String[0] == 27) break;
  }
  printf("\r\r\r");



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                             Turn Off pixels row by row and column by column on the whole LED matrix.
                                           (using function RGB_matrix_clear_pixel()).
  \* --------------------------------------------------------------------------------------------------------------------------- */
  uart_send(__LINE__, __func__, "Press <Enter> to begin RGB_matrix_clear_pixel() test\r");
  uart_send(__LINE__, __func__, "or <ESC> to return to menu: ");
  input_string(String);
  if (String[0] == 27) return;
  memset(FrameBuffer, 0xFF, 256);  // turn On all LEDs on entry.

  for (Loop1UInt8 = 0; Loop1UInt8 < MAX_ROWS; ++Loop1UInt8)
  {
    for (Loop2UInt8 = 0; Loop2UInt8 < MAX_COLUMNS; ++Loop2UInt8)
    {
      uart_send(__LINE__, __func__, "RGB_matrix_clear_pixel() - Row %2u   Column: %2u   ", Loop1UInt8, Loop2UInt8);
      RGB_matrix_clear_pixel(FrameBuffer, Loop1UInt8, Loop2UInt8, Loop1UInt8, Loop2UInt8);
      uart_send(__LINE__, __func__, "- Press <Enter> to continue or <ESC> to exit test: ");
      sleep_ms(100);  // to prevent communication override if user keep pressing <Enter>.
      input_string(String);
      if (String[0] == 27) break;
    }
    printf("\r");
    if (String[0] == 27) break;
  }
  printf("\r\r\r");



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                          Turn On pixels row by row and column by column for the specified LED matrix area.
                                                (using direct FrameBuffer access).
  \* --------------------------------------------------------------------------------------------------------------------------- */
  uart_send(__LINE__, __func__, "Press <Enter> to turn On pixels one by one using direct FrameBuffer access\r");
  uart_send(__LINE__, __func__, "or <ESC> to return to menu: ");
  input_string(String);
  if (String[0] == 27) return;

  /* Determine the matrix area to be turned On. */
  StartRow    =  9;
  EndRow      = 22;
  StartColumn = 20;
  EndColumn   = 43;

  /* Turn Off all LEDs on entry. */
  RGB_matrix_cls(FrameBuffer);

  /* Turn On pixels in the specified matrix area, row by row and column by column. */
  uart_send(__LINE__, __func__, "LED matrix area defined to be turned On: StartRow:  %2u     StartColumn:  %2u     EndRow:  %2u     EndColumn:  %2u\r\r", StartRow, StartColumn, EndRow, EndColumn);

  for (RowNumber = StartRow; RowNumber <= EndRow; ++RowNumber)
	{
    for (ColumnNumber = StartColumn; ColumnNumber <= EndColumn; ++ColumnNumber)
    {
      uart_send(__LINE__, __func__, "Turn On pixel - Row: %2u   Column: %2u   ", RowNumber, ColumnNumber);
      FrameBuffer[RowNumber] |= (0x01ll << ColumnNumber);
      uart_send(__LINE__, __func__, "- Press <Enter> to turn On next pixel or <ESC> to exit test: ");
      sleep_ms(100);  // to prevent communication override if user keep pressing <Enter>.
      input_string(String);
      if (String[0] == 27)
      {
        /* Erase LED matrix when done. */
        RGB_matrix_cls(FrameBuffer);
        break;
      }
    }
    printf("\r");
    if (String[0] == 27) break;  // exit outer loop.
  }
  printf("\r\r\r");



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                       Turn Off pixels row by row and column by column for the specified LED matrix area.
                                                (using direct FrameBuffer access).
  \* --------------------------------------------------------------------------------------------------------------------------- */
  uart_send(__LINE__, __func__, "Press <Enter> to turn Off pixels one by one using direct FrameBuffer access\r");
  uart_send(__LINE__, __func__, "or <ESC> to return to menu: ");
  input_string(String);
  if (String[0] == 27) return;

  /* Determine the matrix area to be turned Off. */
  StartRow    =  9;
  EndRow      = 22;
  StartColumn = 20;
  EndColumn   = 43;

  /* Turn On all LEDs on entry. */
  memset(FrameBuffer, 0xFF, 256);

  /* Turn Off pixels in the specified matrix area, row by row and column by column. */
  uart_send(__LINE__, __func__, "LED matrix area defined to be turned Off: StartRow:  %2u     StartColumn:  %2u     EndRow:  %2u     EndColumn:  %2u\r\r", StartRow, StartColumn, EndRow, EndColumn);

  for (RowNumber = StartRow; RowNumber <= EndRow; ++RowNumber)
	{
    for (ColumnNumber = StartColumn; ColumnNumber <= EndColumn; ++ColumnNumber)
    {
      uart_send(__LINE__, __func__, "Turn Off pixel - Row: %2u   Column: %2u   ", RowNumber, ColumnNumber);
      FrameBuffer[RowNumber] &= ~(0x01ll << ColumnNumber);
      uart_send(__LINE__, __func__, "- Press <Enter> to turn Off next pixel or <ESC> to exit test: ");
      sleep_ms(100);  // to prevent communication override if user keep pressing <Enter>.
      input_string(String);
      if (String[0] == 27)
      {
        /* Erase LED matrix before exiting. */
        RGB_matrix_cls(FrameBuffer);
        break;
      }
    }
    printf("\r");
    if (String[0] == 27) break;  // exit outer loop.
  }
  printf("\r\r\r");



  /* --------------------------------------------------------------------------------------------------------------------------- *\
                            Turn On pixels for the specified LED matrix area using RGB_matrix_set_pixel().
  \* --------------------------------------------------------------------------------------------------------------------------- */
  uart_send(__LINE__, __func__, "Press <Enter> to begin RGB_matrix_set_pixel() tests for specific LED matrix areas\r");
  uart_send(__LINE__, __func__, "or <ESC> to return to menu: ");
  input_string(String);
  if (String[0] == 27) return;

  /* Turn Off LED matrix on entry. */
  RGB_matrix_cls(FrameBuffer);


  uart_send(__LINE__, __func__, "Press <Enter> to set matrix  5, 5 to 10,10 or <ESC> to exit test: ");
  input_string(String);
  if (String[0] == 27)
  {
    RGB_matrix_cls(FrameBuffer);
    return;
  }
  RGB_matrix_set_pixel(FrameBuffer, 5, 5, 10, 10);


  uart_send(__LINE__, __func__, "Press <Enter> to set matrix  5,53 to 10,58 or <ESC> to exit test: ");
  input_string(String);
  if (String[0] == 27)
  {
    RGB_matrix_cls(FrameBuffer);
    return;
  }
  RGB_matrix_set_pixel(FrameBuffer, 5, 53, 10, 58);


  uart_send(__LINE__, __func__, "Press <Enter> to set matrix 15,10 to 18,20 or <ESC> to exit test: ");
  input_string(String);
  if (String[0] == 27)
  {
    RGB_matrix_cls(FrameBuffer);
    return;
  }
  RGB_matrix_set_pixel(FrameBuffer, 15, 10, 18, 20);


  uart_send(__LINE__, __func__, "Press <Enter> to set matrix 15,42 to 18,53 or <ESC> to exit test: ");
  input_string(String);
  if (String[0] == 27)
  {
    RGB_matrix_cls(FrameBuffer);
    return;
  }
  RGB_matrix_set_pixel(FrameBuffer, 15, 42, 18, 53);


  uart_send(__LINE__, __func__, "Press <Enter> to set matrix 30,10 to 30,53 or <ESC> to exit test: ");
  input_string(String);
  if (String[0] == 27)
  {
    RGB_matrix_cls(FrameBuffer);
    return;
  }
  RGB_matrix_set_pixel(FrameBuffer, 30, 10, 30, 53);


  uart_send(__LINE__, __func__, "Press <Enter> to set matrix 28,10 to 28,53 or <ESC> to exit test: ");
  input_string(String);
  if (String[0] == 27)
  {
    RGB_matrix_cls(FrameBuffer);
    return;
  }
  RGB_matrix_set_pixel(FrameBuffer, 28, 10, 28, 53);


  uart_send(__LINE__, __func__, "Press <Enter> to set matrix  4, 4 to 12,12 or <ESC> to exit test: ");
  input_string(String);
  if (String[0] == 27)
  {
    RGB_matrix_cls(FrameBuffer);
    return;
  }
  RGB_matrix_set_pixel(FrameBuffer, 4, 4, 12, 12);


  uart_send(__LINE__, __func__, "Press <Enter> to set matrix  4,51 to 12,59 or <ESC> to exit test: ");
  input_string(String);
  if (String[0] == 27)
  {
    RGB_matrix_cls(FrameBuffer);
    return;
  }
  RGB_matrix_set_pixel(FrameBuffer, 4, 51, 12, 59);


  uart_send(__LINE__, __func__, "Press <Enter> to set matrix 22, 3 to 25,31 or <ESC> to exit test: ");
  input_string(String);
  if (String[0] == 27)
  {
    RGB_matrix_cls(FrameBuffer);
    return;
  }
  RGB_matrix_set_pixel(FrameBuffer, 22, 3, 25, 31);


  uart_send(__LINE__, __func__, "Press <Enter> to set matrix 22, 3 to 25,31 or <ESC> to exit test: ");
  input_string(String);
  if (String[0] == 27)
  {
    RGB_matrix_cls(FrameBuffer);
    return;
  }
  RGB_matrix_set_pixel(FrameBuffer, 22, 32, 25, 60);


  uart_send(__LINE__, __func__, "Press <Enter> to set matrix  8,19 to 23,43 or <ESC> to exit test: ");
  input_string(String);
  if (String[0] == 27)
  {
    RGB_matrix_cls(FrameBuffer);
    return;
  }
  RGB_matrix_set_pixel(FrameBuffer, 8, 19, 23, 43);


  uart_send(__LINE__, __func__, "Press <Enter> to set matrix 31,63 to 20,40 or <ESC> to exit test\r");
  uart_send(__LINE__, __func__, "NOTE: Coordinates have volontarily been inverted: ");
  input_string(String);
  if (String[0] == 27)
  {
    RGB_matrix_cls(FrameBuffer);
    return;
  }
  RGB_matrix_set_pixel(FrameBuffer, 31, 63, 20, 40);


  uart_send(__LINE__, __func__, "Press <Enter> to set matrix 31,63 to 15,25 or <ESC> to exit test\r");
  uart_send(__LINE__, __func__, "NOTE: Coordinates have volontarily been inverted: ");
  input_string(String);
  if (String[0] == 27)
  {
    RGB_matrix_cls(FrameBuffer);
    return;
  }
  RGB_matrix_set_pixel(FrameBuffer, 31, 63, 15, 25);


  uart_send(__LINE__, __func__, "Test completed. Press <Enter> to exit test: ");
  input_string(String);
  if (String[0] == 27)
  {
    RGB_matrix_cls(FrameBuffer);
    return;
  }
  RGB_matrix_cls(FrameBuffer);

  printf("\r\r\r");

  return;





  /* $TITLE=Test2 */
  /* $PAGE */
  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                 Test number 2 - Character set tests.
  \* --------------------------------------------------------------------------------------------------------------------------- */
Test2:
  printf("\r\r\r");
  uart_send(__LINE__, __func__, "Entering Test number 2\r");
  uart_send(__LINE__, __func__, "Character set tests.\r");

  /* Announce test number. */
  win_cls(WIN_TEST);
  RGB_matrix_set_color(0, 0, 31, 63, BLUE);  // use blue color since this is the one drawing the least current.
  win_printf(WIN_TEST, 2, 99, FONT_5x7, "Test 2");


  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                            5x7 variable-width character set test.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  uart_send(__LINE__, __func__, "Press <Enter> to begin 5x7 character set test\r");
  uart_send(__LINE__, __func__, "or <ESC> to return to menu: ");
  input_string(String);
  if (String[0] == 27) return;
  win_part_cls(WIN_TEST, 11, 31);  // erase botton part of the RGB matrix.

  for (AsciiValue = 0x00; AsciiValue <= 0xFF; ++AsciiValue)
  {
    /* Display specifications of this ASCII character. */
    uart_send(__LINE__, __func__, "Processing ASCII character: 0x%2.2X (%3u)   ", AsciiValue, AsciiValue);

    /* If this ASCII character is displayable, display it. */
    if (AsciiValue >= 0x20)
      printf("<%c>\r", AsciiValue);
    else
      printf("(non printable)\r");

    /* Optionally display bitmap of each row. */
    if (DebugBitMask & DEBUG_MATRIX)
    {
      for (RowNumber = 0; RowNumber < 7; ++RowNumber)
      {
        uart_send(__LINE__, __func__, "Row[%u]:  0x%2.2X\r", RowNumber, Font5x7[AsciiValue].Row[RowNumber]);
      }
      uart_send(__LINE__, __func__, "Width:      %u\r\r", Font5x7[AsciiValue].Width);
    }


    /* Draw a box of blue pixels and another smaller box of green pixels to see "how the character fits" inside it. */
    /* There should be a border of 2 green pixels and 2 blue pixels all around the character. */
    /* Character height will always be 7 pixels. */
    StartRow    = 12;
    EndRow      = 12 + 2 + 2 + 7 + 2 + 2 - 1;
    StartColumn =  3;
    EndColumn   = (3 + 2 + 2 + Font5x7[AsciiValue].Width + 2 + 2 - 1);

    if (DebugBitMask & DEBUG_MATRIX)
    {
      uart_send(__LINE__, __func__, "Draw BLUE, GREEN and RED positioning boxes.\r");
      uart_send(__LINE__, __func__, "BLUE box StartRow: %2u     BLUE box StartColumn: %2u\r",   StartRow, StartColumn);
      uart_send(__LINE__, __func__, "BLUE box EndRow:   %2u     BLUE box EndColumn:   %2u\r\r", EndRow,   EndColumn);
      uart_send(__LINE__, __func__, "Press <Enter> to draw color boxes and character bitmap: ");
      input_string(String);
    }

    PwmLevel = Pwm[PWM_ID_BRIGHTNESS].Level;  // keep track of original PWM level on entry.
    pwm_set_level(PWM_ID_BRIGHTNESS, 2000);

	  /* Display ASCII value on LED display. */
    RGB_matrix_printf(FrameBuffer, 17, 40, FONT_5x7, "%3.3u", AsciiValue);

    /* Draw a bigger blue box. */
    RGB_matrix_set_color(StartRow, StartColumn, EndRow, EndColumn, BLUE);
    RGB_matrix_set_pixel(FrameBuffer, StartRow, StartColumn, EndRow, EndColumn);

    /* Then, draw a smaller green box inside the blue box. */
    RGB_matrix_set_color(StartRow + 2, StartColumn + 2, EndRow - 2, EndColumn - 2, GREEN);
    RGB_matrix_set_pixel(FrameBuffer, StartRow + 2, StartColumn + 2, EndRow - 2, EndColumn - 2);

    /* Draw a red box for character bitmap width only. */
    RGB_matrix_set_color(StartRow + 4, StartColumn + 4, EndRow - 4, EndColumn - 4, RED);
    RGB_matrix_set_pixel(FrameBuffer, StartRow + 4, StartColumn + 4, EndRow - 4, EndColumn - 4);

    /* Finally, display ASCII character in inside the red box. */
    RGB_matrix_display(FrameBuffer, StartRow + 4, StartColumn + 4, AsciiValue, FONT_5x7, 0);

    /* Display the same ASCII character to the right of the larger blue box. */
    RGB_matrix_display(FrameBuffer, StartRow + 4, StartColumn + 22, AsciiValue, FONT_5x7, 0);

    /* Restore original PWM level when done. */
    pwm_set_level(PWM_ID_BRIGHTNESS, PwmLevel);

    sleep_ms(100);  // slow-down display to prevent communication override if user keeps a finger on <Enter> key.
    uart_send(__LINE__, __func__, "Press <Enter> to display next ASCII character 0x%2.2X (%3u) or <ESC> to exit test: ", AsciiValue + 1, AsciiValue + 1);
    input_string(String);
    if ((String[0] == 27) || (AsciiValue == 255)) break;  // AsciiValue is a UINT8 (will never go higher than 255).

    /* Clear display matrix and proceed to next character or exit test. */
    win_part_cls(WIN_TEST, 11, 31);  // erase botton part of the RGB matrix.
  }
  printf("\r\r\r");





  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                            8x10 variable-width character set test.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  uart_send(__LINE__, __func__, "Press <Enter> to begin 8x10 character set test\r");
  uart_send(__LINE__, __func__, "or <ESC> to return to menu: ");
  input_string(String);
  if (String[0] == 27)
  {
    RGB_matrix_cls(FrameBuffer);
    return;
  }

  win_part_cls(WIN_TEST, 11, 31);  // erase botton part of the RGB matrix.
  RGB_matrix_set_color(0, 0, 31, 63, BLUE);


  for (AsciiValue = 0x00; AsciiValue <= 0x7F; ++AsciiValue)
  {
    /* Display specifications of this ASCII character. */
    uart_send(__LINE__, __func__, "Processing ASCII character: 0x%2.2X (%3u)   ", AsciiValue, AsciiValue);

    /* If this ASCII character is displayable, display it. */
    if (AsciiValue >= 0x20)
      printf("<%c>\r", AsciiValue);
    else
      printf("(non printable)\r");

    /* Optionally display bitmap of each row. */
    if (DebugBitMask & DEBUG_MATRIX)
    {
      for (RowNumber = 0; RowNumber < 10; ++RowNumber)
      {
        uart_send(__LINE__, __func__, "Row[%u]:  0x%2.2X\r", RowNumber, Font8x10[AsciiValue].Row[RowNumber]);
      }
      uart_send(__LINE__, __func__, "Width:      %u\r\r", Font8x10[AsciiValue].Width);
    }


    /* Draw a box of blue pixels and another smaller box of green pixels to see "how the character fits" inside it. */
    /* There should be a border of 2 green pixels and 2 blue pixels all around the character. */
    /* Character height will always be 10 pixels. */
    StartRow    = 11;
    EndRow      = 11 + 2 + 2 + 10 + 2 + 2 - 1;
    StartColumn =  3;
    EndColumn   = (3 + 2 + 2 + Font8x10[AsciiValue].Width + 2 + 2 - 1);

    if (DebugBitMask & DEBUG_MATRIX)
    {
      uart_send(__LINE__, __func__, "Draw BLUE, GREEN and RED positioning boxes.\r");
      uart_send(__LINE__, __func__, "BLUE box StartRow: %2u     BLUE box StartColumn: %2u\r",   StartRow, StartColumn);
      uart_send(__LINE__, __func__, "BLUE box EndRow:   %2u     BLUE box EndColumn:   %2u\r\r", EndRow,   EndColumn);
      uart_send(__LINE__, __func__, "Press <Enter> to draw color boxes and character bitmap: ");
      input_string(String);
    }

    PwmLevel = Pwm[PWM_ID_BRIGHTNESS].Level;  // keep track of original PWM level on entry.
    pwm_set_level(PWM_ID_BRIGHTNESS, 2000);

	  /* Display ASCII value on LED display. */
    RGB_matrix_printf(FrameBuffer, 17, 40, FONT_5x7, "%3.3u", AsciiValue);

    /* Draw a bigger blue box. */
    RGB_matrix_set_color(StartRow, StartColumn, EndRow, EndColumn, BLUE);
    RGB_matrix_set_pixel(FrameBuffer, StartRow, StartColumn, EndRow, EndColumn);

    /* Then, draw a smaller green box inside the blue box. */
    RGB_matrix_set_color(StartRow + 2, StartColumn + 2, EndRow - 2, EndColumn - 2, GREEN);
    RGB_matrix_set_pixel(FrameBuffer, StartRow + 2, StartColumn + 2, EndRow - 2, EndColumn - 2);

    /* Draw a red box for character bitmap width only. */
    RGB_matrix_set_color(StartRow + 4, StartColumn + 4, EndRow - 4, EndColumn - 4, RED);
    RGB_matrix_set_pixel(FrameBuffer, StartRow + 4, StartColumn + 4, EndRow - 4, EndColumn - 4);

    /* Finally, display ASCII character in inside the red box. */
    RGB_matrix_display(FrameBuffer, StartRow + 4, StartColumn + 4, AsciiValue, FONT_8x10, 0);

    /* Display the same ASCII character to the right of the larger blue box. */
    RGB_matrix_display(FrameBuffer, StartRow + 4, StartColumn + 22, AsciiValue, FONT_8x10, 0);

    /* Restore original PWM level when done. */
    pwm_set_level(PWM_ID_BRIGHTNESS, PwmLevel);

    sleep_ms(100);  // slow-down display to prevent communication override if user keeps a finger on <Enter> key.
    uart_send(__LINE__, __func__, "Press <Enter> to display next ASCII character 0x%2.2X (%3u) or <ESC> to exit test: ", AsciiValue + 1, AsciiValue + 1);
    input_string(String);
    if (String[0] == 27) break;

    /* Clear display matrix and proceed to next character or exit test. */
    win_part_cls(WIN_TEST, 11, 31);  // erase botton part of the RGB matrix.
  }
  printf("\r\r\r");





  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                            4x7 variable-width character set test.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  uart_send(__LINE__, __func__, "Press <Enter> to begin 4x7 character set test\r");
  uart_send(__LINE__, __func__, "or <ESC> to return to menu: ");
  input_string(String);
  if (String[0] == 27)
  {
    RGB_matrix_cls(FrameBuffer);
    return;
  }

  win_part_cls(WIN_TEST, 11, 31);  // erase botton part of the RGB matrix.
  RGB_matrix_set_color(0, 0, 31, 63, BLUE);


  for (AsciiValue = 0x00; AsciiValue <= 0x7F; ++AsciiValue)
  {
    /* Display specifications of this ASCII character. */
    uart_send(__LINE__, __func__, "Processing ASCII character: 0x%2.2X (%3u)   ", AsciiValue, AsciiValue);

    /* If this ASCII character is displayable, display it. */
    if (AsciiValue >= 0x20)
      printf("<%c>\r", AsciiValue);
    else
      printf("(non printable)\r");

    /* Optionally display bitmap of each row. */
    if (DebugBitMask & DEBUG_MATRIX)
    {
      for (RowNumber = 0; RowNumber < 7; ++RowNumber)
      {
        uart_send(__LINE__, __func__, "Row[%u]:  0x%2.2X\r", RowNumber, Font4x7[AsciiValue].Row[RowNumber]);
      }
      uart_send(__LINE__, __func__, "Width:      %u\r\r", Font4x7[AsciiValue].Width);
    }


    /* Draw a box of blue pixels and another smaller box of green pixels to see "how the character fits" inside it. */
    /* There should be a border of 2 green pixels and 2 blue pixels all around the character. */
    /* Character height will always be 7 pixels. */
    StartRow    = 12;
    EndRow      = 12 + 2 + 2 + 7 + 2 + 2 - 1;
    StartColumn =  3;
    EndColumn   = (3 + 2 + 2 + Font4x7[AsciiValue].Width + 2 + 2 - 1);

    if (DebugBitMask & DEBUG_MATRIX)
    {
      uart_send(__LINE__, __func__, "Draw BLUE, GREEN and RED positioning boxes.\r");
      uart_send(__LINE__, __func__, "BLUE box StartRow: %2u     BLUE box StartColumn: %2u\r",   StartRow, StartColumn);
      uart_send(__LINE__, __func__, "BLUE box EndRow:   %2u     BLUE box EndColumn:   %2u\r\r", EndRow,   EndColumn);
      uart_send(__LINE__, __func__, "Press <Enter> to draw color boxes and character bitmap: ");
      input_string(String);
    }

    PwmLevel = Pwm[PWM_ID_BRIGHTNESS].Level;  // keep track of original PWM level on entry.
    pwm_set_level(PWM_ID_BRIGHTNESS, 2000);

	  /* Display ASCII value on LED display. */
    RGB_matrix_printf(FrameBuffer, 17, 40, FONT_5x7, "%3.3u", AsciiValue);

    /* Draw a bigger blue box. */
    RGB_matrix_set_color(StartRow, StartColumn, EndRow, EndColumn, BLUE);
    RGB_matrix_set_pixel(FrameBuffer, StartRow, StartColumn, EndRow, EndColumn);

    /* Then, draw a smaller green box inside the blue box. */
    RGB_matrix_set_color(StartRow + 2, StartColumn + 2, EndRow - 2, EndColumn - 2, GREEN);
    RGB_matrix_set_pixel(FrameBuffer, StartRow + 2, StartColumn + 2, EndRow - 2, EndColumn - 3);

    /* Draw a red box for character bitmap width only. */
    RGB_matrix_set_color(StartRow + 4, StartColumn + 4, EndRow - 4, EndColumn - 4, RED);
    RGB_matrix_set_pixel(FrameBuffer, StartRow + 4, StartColumn + 4, EndRow - 4, EndColumn - 4);

    /* Finally, display ASCII character in inside the red box. */
    RGB_matrix_display(FrameBuffer, StartRow + 4, StartColumn + 4, AsciiValue, FONT_4x7, 0);

    /* Display the same ASCII character to the right of the larger blue box. */
    RGB_matrix_display(FrameBuffer, StartRow + 4, StartColumn + 22, AsciiValue, FONT_4x7, 0);

    /* Restore original PWM level when done. */
    pwm_set_level(PWM_ID_BRIGHTNESS, PwmLevel);

    sleep_ms(100);  // slow-down display to prevent communication override if user keeps a finger on <Enter> key.
    uart_send(__LINE__, __func__, "Press <Enter> to display next ASCII character 0x%2.2X (%3u) or <ESC> to exit test: ", AsciiValue + 1, AsciiValue + 1);
    input_string(String);
    if (String[0] == 27) break;

    /* Clear display matrix and proceed to next character or exit test. */
    win_part_cls(WIN_TEST, 11, 31);  // erase botton part of the RGB matrix.
  }
  printf("\r\r\r");

  return;





  /* $TITLE=Test3 */
  /* $PAGE */
  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                               Test number 3 - LED matrix brightness tests.
  \* --------------------------------------------------------------------------------------------------------------------------- */
Test3:
  /* LED matrix brightness tests. */
  printf("\r\r\r");
  uart_send(__LINE__, __func__, "Entering Test number 3\r");
  uart_send(__LINE__, __func__, "LED matrix brightness tests.\r");

  /* Announce test number. */
  win_part_cls(WIN_TEST, 1, 30);
  RGB_matrix_set_color(0, 0, 31, 63, RED);  // red seems more subject to spurious interference on the display.
  win_printf(WIN_TEST, 2, 99, FONT_5x7, "Test 3");

  uart_send(__LINE__, __func__, "Press <Enter> to begin brightness test\r");
  uart_send(__LINE__, __func__, "or <ESC> to return to menu: ");
  input_string(String);
  if (String[0] == 27) return;


  win_printf(WIN_TEST, 13, 99, FONT_5x7, "ABCDEFGHIJ");
  win_printf(WIN_TEST, 23, 99, FONT_5x7, "KLMNOPQRST");



  /* Retrieve system clock (Pico is 125 MHz). */
  SystemClock = clock_get_hz(clk_sys);

  uart_send(__LINE__, __func__, "WARNING - - - WARNING - - - WARNING - - - WARNING - - - WARNING - - - WARNING - - - WARNING - - - WARNING - - - WARNING - - - WARNING\r\r");
  uart_send(__LINE__, __func__, "Be careful while changing PWM parameters. You could go beyond operational values and jeopardize the life span of the LEDS...\r");
  uart_send(__LINE__, __func__, "Be prepared to turn Off the Pico-RGB-matrix power supply if ever you realize that the LEDs are overbright and you can't quickly\r");
  uart_send(__LINE__, __func__, "restore the situation.\r\r");
  uart_send(__LINE__, __func__, "WARNING - - - WARNING - - - WARNING - - - WARNING - - - WARNING - - - WARNING - - - WARNING - - - WARNING - - - WARNING - - - WARNING\r\r\r\r");
  uart_send(__LINE__, __func__, "Press <Enter> to continue: ");
  input_string(String);


  FlashConfig1.FlagAutoBrightness = FLAG_OFF;
  uart_send(__LINE__, __func__, "FlagAutoBrightness has been turned Off: 0x%2.2X\r", FlashConfig1.FlagAutoBrightness);
  uart_send(__LINE__, __func__, "and DutyCycle has been set to: %2u\r\r\r", Pwm[PWM_ID_BRIGHTNESS].DutyCycle);


  while (1)
  {
    printf("\r");
    uart_send(__LINE__, __func__, "Current instantaneous value of ambient light: %4u\r",   get_light_value());
    uart_send(__LINE__, __func__, "AverageAmbientLight for the last period:      %4u\r\r", AverageAmbientLight);

    pwm_display_parameters();
    printf("\r\r");

    uart_send(__LINE__, __func__, "1) Set divider.\r");
    uart_send(__LINE__, __func__, "2) Set wrap.\r");
    uart_send(__LINE__, __func__, "3) Set level.\r");
    uart_send(__LINE__, __func__, "4) Set color.\r");
    uart_send(__LINE__, __func__, "5) Turn Off matrix.\r");
    uart_send(__LINE__, __func__, "6) Turn On matrix.\r");



    String[0] = '0';
    String[1] = 0x00;
    while (atoi(String) == 0)
    {
      uart_send(__LINE__, __func__, "Enter your choice or <ESC> to exit test: ");
      input_string(String);
      if (String[0] == 27) return;
      if ((atoi(String) == 0) || (atoi(String) > 6))
      {
        uart_send(__LINE__, __func__, "Invalid choice... try again\r");
        String[0] = '0';
        String[1] = 0x00;
      }
    }

    switch (atoi(String))
    {
      case (1):
        /* Set divider. */
        uart_send(__LINE__, __func__, "Enter clock divider value or <ESC> to return to test menu (float): ");
        input_string(String);
        if (String[0] == 27) break;
        if (String[0] != 0x0D)
        {
          Pwm[PWM_ID_BRIGHTNESS].ClockDivider = atof(String);
          uart_send(__LINE__, __func__, "Setting PWM divider to: %f\r", Pwm[PWM_ID_BRIGHTNESS].ClockDivider);
          Pwm[PWM_ID_BRIGHTNESS].Clock = (UINT32)(SystemClock / Pwm[PWM_ID_BRIGHTNESS].ClockDivider);  // keep track of resulting clock frequency.
          pwm_set_clkdiv(Pwm[PWM_ID_BRIGHTNESS].Slice, Pwm[PWM_ID_BRIGHTNESS].ClockDivider);
        }
      break;

      case (2):
        /* Set wrap. */
        uart_send(__LINE__, __func__, "Enter wrap value or <ESC> to return to test menu (UINT16): ");
        input_string(String);
        if (String[0] == 27) break;
        if (String[0] != 0x0D)
        {
          Pwm[PWM_ID_BRIGHTNESS].Wrap = atoi(String);
          uart_send(__LINE__, __func__, "Setting PWM wrap to: %u\r", Pwm[PWM_ID_BRIGHTNESS].Wrap);
          pwm_set_wrap(Pwm[PWM_ID_BRIGHTNESS].Slice, Pwm[PWM_ID_BRIGHTNESS].Wrap);
        }
      break;

      case (3):
        /* Set level. */
        uart_send(__LINE__, __func__, "Enter level or <ESC> to return to test menu (UINT16): ");
        input_string(String);
        if (String[0] == 27) break;
        if (String[0] != 0x0D)
        {
          Pwm[PWM_ID_BRIGHTNESS].Level = atoi(String);
          uart_send(__LINE__, __func__, "Setting PWM level to: %u\r", Pwm[PWM_ID_BRIGHTNESS].Level);
          pwm_set_level(PWM_ID_BRIGHTNESS, Pwm[PWM_ID_BRIGHTNESS].Level);
        }
      break;

      case (4):
        /* Set color. */
        uart_send(__LINE__, __func__, "Enter matrix color to set or <ESC> to return to test menu (UINT16): ");
        input_string(String);
        if (String[0] == 27) break;
        if (String[0] != 0x0D)
        {
          if ((strcmp(String, "RED")     == 0) || (strcmp(String, "red")     == 0)) RGB_matrix_set_color(0, 0, 31, 63, RED);
          if ((strcmp(String, "GREEN")   == 0) || (strcmp(String, "green")   == 0)) RGB_matrix_set_color(0, 0, 31, 63, GREEN);
          if ((strcmp(String, "BLUE")    == 0) || (strcmp(String, "blue")    == 0)) RGB_matrix_set_color(0, 0, 31, 63, BLUE);
          if ((strcmp(String, "CYAN")    == 0) || (strcmp(String, "cyan")    == 0)) RGB_matrix_set_color(0, 0, 31, 63, CYAN);
          if ((strcmp(String, "MAGENTA") == 0) || (strcmp(String, "magenta") == 0)) RGB_matrix_set_color(0, 0, 31, 63, MAGENTA);
          if ((strcmp(String, "YELLOW")  == 0) || (strcmp(String, "yellow")  == 0)) RGB_matrix_set_color(0, 0, 31, 63, YELLOW);
          if ((strcmp(String, "WHITE")   == 0) || (strcmp(String, "white")   == 0)) RGB_matrix_set_color(0, 0, 31, 63, WHITE);
        }
      break;

      case (5):
        /* Turn Off matrix. */
        uart_send(__LINE__, __func__, "Press <Enter> to blank LED matrix or <ESC> to exit test: ");
        input_string(String);
        if (String[0] == 27) break;
        RGB_matrix_cls(FrameBuffer);
      break;

      case (6):
        /* Turn On matrix. */
        uart_send(__LINE__, __func__, "Press <Enter> to turn On all LED matrix or <ESC> to exit test: ");
        input_string(String);
        if (String[0] == 27) break;
        memset(FrameBuffer, 0xFF, 256);
      break;
    }
  }
  printf("\r\r\r");



  uart_send(__LINE__, __func__, "FlashConfig1.FlagAutoBrightness: %4u\r", FlashConfig1.FlagAutoBrightness);
  uart_send(__LINE__, __func__, "Press <Enter> to read ambient light: ");
  input_string(String);


  while (1)
  {
    uart_send(__LINE__, __func__, "Current instantaneous value of ambient light: %4u\r", get_light_value());
    uart_send(__LINE__, __func__, "AverageAmbientLight for the last period:      %4u\r", AverageAmbientLight);
    for (Loop1UInt8 = 0; Loop1UInt8 < (BRIGHTNESS_HYSTERESIS_SECONDS / 4); ++Loop1UInt8)
    {
      printf("                       %3u - %4u     %3u - %4u     %3u - %4u     %3u - %4u\r", Loop1UInt8, AmbientLight[Loop1UInt8],
                                                                                             Loop1UInt8 +  (BRIGHTNESS_HYSTERESIS_SECONDS / 4),      AmbientLight[Loop1UInt8 +  (BRIGHTNESS_HYSTERESIS_SECONDS / 4)],
                                                                                             Loop1UInt8 + ((BRIGHTNESS_HYSTERESIS_SECONDS / 4) * 2), AmbientLight[Loop1UInt8 + ((BRIGHTNESS_HYSTERESIS_SECONDS / 4) * 2)],
                                                                                             Loop1UInt8 + ((BRIGHTNESS_HYSTERESIS_SECONDS / 4) * 3), AmbientLight[Loop1UInt8 + ((BRIGHTNESS_HYSTERESIS_SECONDS / 4) * 3)]);
    }
    uart_send(__LINE__, __func__, "Press <Enter> to update figures, or <ESC> to exit test: ");
    input_string(String);
    if (String[0] == 27) break;
  }
  printf("\r\r\r");

  return;





  /* $TITLE=Test4 */
  /* $PAGE */
  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                     Test number 4 - Scrolling tests.
  \* --------------------------------------------------------------------------------------------------------------------------- */
Test4:
  /* Scroll text on LED matrix. */
  printf("\r\r\r");
  uart_send(__LINE__, __func__, "Entering Test number 4\r");
  uart_send(__LINE__, __func__, "Scroll text on LED matrix.\r");

  /* Announce test number. */
  win_part_cls(WIN_TEST, 1, 30);
  RGB_matrix_set_color(0, 0, 31, 63, BLUE);  // use blue for tests since it is the color drawing the least current.
  win_printf(WIN_TEST, 2, 99, FONT_5x7, "Test 4");

  uart_send(__LINE__, __func__, "Press <Enter> to begin text scrolling test\r");
  uart_send(__LINE__, __func__, "or <ESC> to return to menu: ");
  input_string(String);
  if (String[0] == 27) return;



  /* Scroll text on a non-standard line (Rows 4 to 10). */
  RGB_matrix_cls(FrameBuffer);  /// erase LED matrix for now to erase box border... to be tested after box border support has been added.
  /// win_part_cls(WIN_TEST, 1, 30);
  ScrollNumber = win_scroll(WIN_TEST, 4, 10, 1, 1, FONT_5x7, "Test number 1: scrolling text once on rows 4 to 10 on the Pico-RGB-Matrix (non-standard rows)");
  uart_send(__LINE__, __func__, "Wait for scrolling to stop");
  while(ActiveScroll[ScrollNumber])
  {
    printf(".");
    sleep_ms(300);
  };  // wait for scrolling to stop.
  printf("\r");
  uart_send(__LINE__, __func__, "This example was done from Row 4 to Row 10 which are not standard...\r");
  uart_send(__LINE__, __func__, "Press <Enter> to continue or <ESC> to exit test: ");
  input_string(String);
  if (String[0] == 27) return;



  /* Scroll text on standard Top line (Rows 1 to 7). */
  RGB_matrix_cls(FrameBuffer);  /// erase LED matrix for now to erase box border... to be tested after box border support has been added.
  /// win_part_cls(WIN_TEST, 1, 30);
  ScrollNumber = win_scroll(WIN_TEST, 201, 201, 2, 1, FONT_5x7, "Test number 2: scrolling text twice on rows 1 to 7 (standard Line 1 of the Pico-RGB-Matrix)");
  uart_send(__LINE__, __func__, "Wait for scrolling to stop");
  while(ActiveScroll[ScrollNumber])
  {
    printf(".");
    sleep_ms(300);
  };  // wait for scrolling to stop.
  printf("\r");
  uart_send(__LINE__, __func__, "This example was done on standard Line 1 (Row 1 to Row 7).\r");
  uart_send(__LINE__, __func__, "Press <Enter> to continue or <ESC> to exit test: ");
  input_string(String);
  if (String[0] == 27) return;



  /* Scroll text on standard Mid Line (Rows 9 to 15). */
  RGB_matrix_cls(FrameBuffer);  /// erase LED matrix for now to erase box border... to be tested after box border support has been added.
  /// win_part_cls(WIN_TEST, 1, 30);
  ScrollNumber = win_scroll(WIN_TEST, 202, 202, 3, 1, FONT_5x7, "Test number 3: scrolling text three times on rows 9 to 15 (standard Line 2 of the Pico-RGB-Matrix)");
  uart_send(__LINE__, __func__, "Wait for scrolling to stop");
  while(ActiveScroll[ScrollNumber])
  {
    printf(".");
    sleep_ms(300);
  };  // wait for scrolling to stop.
  printf("\r");
  uart_send(__LINE__, __func__, "This example was done from standard Line 2 (Row 9 to Row 15).\r");
  uart_send(__LINE__, __func__, "Press <Enter> to continue or <ESC> to exit test: ");
  input_string(String);
  if (String[0] == 27) return;



  /* Scroll text on standard Bot Line (Rows 20 to 29). Note: For now, character set 8x10 includes only numbers. */
  RGB_matrix_cls(FrameBuffer);  /// erase LED matrix for now to erase box border... to be tested after box border support has been added.
  /// win_part_cls(WIN_TEST, 20, 29);
  printf("\r\r\r");
  uart_send(__LINE__, __func__, "This test will scroll numbers 1 to 10 four times in 8x10 font\r");
  uart_send(__LINE__, __func__, "since only numbers have been defined in this character set for now...\r");
  ScrollNumber = win_scroll(WIN_TEST, 203, 203, 4, 1, FONT_8x10, "1234567890");
  uart_send(__LINE__, __func__, "Wait for scrolling to stop");
  while(ActiveScroll[ScrollNumber])
  {
    printf(".");
    sleep_ms(300);
  };  // wait for scrolling to stop.
  printf("\r");
  uart_send(__LINE__, __func__, "This example was done from standard Line 3 (Row 20 to Row 29) with 8x10 characters.\r");
  uart_send(__LINE__, __func__, "Press <Enter> to continue: ");
  input_string(String);

  printf("\r\r\r");

  return;





  /* $TITLE=Test5 */
  /* $PAGE */
  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                 Test 5: RGB_matrix_set_color tests.
  \* --------------------------------------------------------------------------------------------------------------------------- */
Test5:
  printf("\r\r\r");
  uart_send(__LINE__, __func__, "Entering Test number 5\r");
  uart_send(__LINE__, __func__, "RGB_matrix_set_color() tests.\r");

  /* Announce test number. */
  win_cls(WIN_TEST);
  RGB_matrix_set_color(0, 0, 31, 63, BLUE);  // use blue as default color since this is the one drawing the least current.
  win_printf(WIN_TEST, 2, 99, FONT_5x7, "Test 5");


  /* --------------------------------------------------------------------------------------------------------------------------- *\
                           Change color on specific matrix areas to test RGB_matrix_set_color() function.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  uart_send(__LINE__, __func__, "Press <Enter> to begin RGB_matrix_set_color() test\r");
  uart_send(__LINE__, __func__, "or <ESC> to return to menu: ");
  input_string(String);
  if (String[0] == 27) return;
  RGB_matrix_set_pixel(FrameBuffer, 0, 0, 31, 63);

  uart_send(__LINE__, __func__, "Press <Enter> to set color red from  0, 0 to  5, 5 or <ESC> to exit test: ");
  input_string(String);
  if (String[0] == 27) return;
  RGB_matrix_set_color(0, 0, 5, 5, RED);

  uart_send(__LINE__, __func__, "Press <Enter> to set color red from  0,57 to  5,63 or <ESC> to exit test: ");
  input_string(String);
  if (String[0] == 27) return;
  RGB_matrix_set_color(0, 57, 5, 63, RED);

  uart_send(__LINE__, __func__, "Press <Enter> to set color green from 10, 5 to 20, 6 or <ESC> to exit test: ");
  input_string(String);
  if (String[0] == 27) return;
  RGB_matrix_set_color(10, 5, 20, 6, GREEN);

  uart_send(__LINE__, __func__, "Press <Enter> to set color green from 10, 56 to 20, 57 or <ESC> to exit test: ");
  input_string(String);
  if (String[0] == 27) return;
  RGB_matrix_set_color(10, 5, 20, 6, GREEN);

  uart_send(__LINE__, __func__, "Press <Enter> to set color yellow from 15,25 to 25,30 or <ESC> to exit test: ");
  input_string(String);
  if (String[0] == 27) return;
  RGB_matrix_set_color(8, 12, 31, 12, YELLOW);

  uart_send(__LINE__, __func__, "Press <Enter> to set color yellow from 15,32 to 25,37 or <ESC> to exit test: ");
  input_string(String);
  if (String[0] == 27) return;
  RGB_matrix_set_color(8, 12, 31, 12, YELLOW);

  uart_send(__LINE__, __func__, "Press <Enter> to set color magenta from 5,63 to 30,63 or <ESC> to exit test: ");
  input_string(String);
  if (String[0] == 27) return;
  RGB_matrix_set_color(5, 63, 30, 63, RED + BLUE);

  uart_send(__LINE__, __func__, "Test completed. Press <Enter> to return: ");
  input_string(String);
  if (String[0] == 27) return;

  printf("\r\r\r");

  return;





  /* $TITLE=Test6 */
  /* $PAGE */
  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                    Test 6 - Time display tests.
  \* --------------------------------------------------------------------------------------------------------------------------- */
Test6:
  printf("\r\r\r");
  uart_send(__LINE__, __func__, "Entering Test number 6\r");
  uart_send(__LINE__, __func__, "Time display tests.\r");

  /* Announce test number. */
  win_cls(WIN_TEST);
  RGB_matrix_set_color(0, 0, 31, 63, BLUE);  // use blue color since this is the one drawing the least current.
  win_printf(WIN_TEST, 2, 99, FONT_5x7, "Test 6");


  /* --------------------------------------------------------------------------------------------------------------------------- *\
                         WIN_TEST is the current active window, disabling date and time update on LED matrix.
            So, tests must be done on the best ways to display time and date on LED matrix, along with other indicators.
  \* --------------------------------------------------------------------------------------------------------------------------- */
  uart_send(__LINE__, __func__, "Press <Enter> to begin Date and Time display tests\r");
  uart_send(__LINE__, __func__, "or <ESC> to return to menu: ");
  input_string(String);
  if (String[0] == 27) return;
  RGB_matrix_cls(FrameBuffer);  // turn Off all LEDs on entry.
  RGB_matrix_display_time();  // display time, using current matrix format.

  uart_send(__LINE__, __func__, "Press <Enter> to continue or <ESC> to exit test: ");
  input_string(String);
  win_cls(WIN_TEST);
  if (String[0] == 27) return;

  /* This is the place to try different matrix formats to display time and date. */
  uart_send(__LINE__, __func__, "There are no more LED matrix formats to try for now...\r");
  uart_send(__LINE__, __func__, "Press <Enter> to continue or <ESC> to exit test: ");
  printf("\r\r\r");

  return;





  /* $TITLE=Test7 */
  /* $PAGE */
  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                   Test 7 - Window and Box algorithm.
  \* --------------------------------------------------------------------------------------------------------------------------- */
Test7:
  printf("\r\r\r");
  uart_send(__LINE__, __func__, "Entering Test number 7\r");
  uart_send(__LINE__, __func__, "Window and Box algorithm.\r");

  /* Announce test number. */
  win_cls(WIN_TEST);
  RGB_matrix_set_color(0, 0, 31, 63, BLUE);  // use blue color since this is the one drawing the least current.
  win_printf(WIN_TEST, 2, 99, FONT_5x7, "Test 7");


  uart_send(__LINE__, __func__, "Press <Enter> to begin Window and Box algorithm tests\r");
  uart_send(__LINE__, __func__, "or <ESC> to return to menu: ");
  input_string(String);
  if (String[0] == 27) return;
  RGB_matrix_cls(FrameBuffer);  // turn Off all LEDs on entry.


  uart_send(__LINE__, __func__, "Press <Enter> to draw a box with coordinates: (0, 0) - (31, 63)\r");
  uart_send(__LINE__, __func__, "or press <ESC> to return to menu: ");
  input_string(String);
  if (String[0] == 27) return;
  RGB_matrix_cls(FrameBuffer);
  RGB_matrix_box(0, 0, 31, 63, BLUE, ACTION_DRAW);


  uart_send(__LINE__, __func__, "Press <Enter> to draw a box with coordinates: (5, 5) - (13, 58)\r");
  uart_send(__LINE__, __func__, "or press <ESC> to return to menu: ");
  input_string(String);
  if (String[0] == 27) return;
  RGB_matrix_box(5, 5, 13, 58, RED, ACTION_DRAW);


  uart_send(__LINE__, __func__, "Press <Enter> to draw a box with coordinates: (18, 5) - (26, 58)\r");
  uart_send(__LINE__, __func__, "or press <ESC> to return to menu: ");
  input_string(String);
  if (String[0] == 27) return;
  RGB_matrix_box(18, 5, 26, 58, RED, ACTION_DRAW);


  uart_send(__LINE__, __func__, "Press <Enter> to draw a box with coordinates: (10, 25) - (21, 35)\r");
  uart_send(__LINE__, __func__, "or press <ESC> to return to menu: ");
  input_string(String);
  if (String[0] == 27) return;
  RGB_matrix_box(10, 25, 21, 38, GREEN, ACTION_DRAW);


  uart_send(__LINE__, __func__, "Press <Enter> to draw a box with coordinates: (25, 30) - (30, 58)\r");
  uart_send(__LINE__, __func__, "or press <ESC> to return to menu: ");
  input_string(String);
  if (String[0] == 27) return;
  RGB_matrix_box(9, 3, 22, 8, YELLOW, ACTION_DRAW);


  uart_send(__LINE__, __func__, "Press <Enter> to draw a box with coordinates: (26, 32) - (15, 25)\r");
  uart_send(__LINE__, __func__, "or press <ESC> to return to menu: ");
  input_string(String);
  if (String[0] == 27) return;
  RGB_matrix_box(9, 55, 22, 60, YELLOW, ACTION_DRAW);


  uart_send(__LINE__, __func__, "Press <Enter> to erase the box with coordinates: (0, 0) - (31, 63)\r");
  uart_send(__LINE__, __func__, "or press <ESC> to return to menu: ");
  input_string(String);
  if (String[0] == 27) return;
  RGB_matrix_box(0, 0, 31, 63, 0, ACTION_ERASE);


  uart_send(__LINE__, __func__, "Press <Enter> to erase the box with coordinates: (5, 5) - (17, 58)\r");
  uart_send(__LINE__, __func__, "or press <ESC> to return to menu: ");
  input_string(String);
  if (String[0] == 27) return;
  RGB_matrix_box(5, 5, 13, 58, 0, ACTION_ERASE);



  uart_send(__LINE__, __func__, "Press <Enter> to erase LED matrix: ");
  input_string(String);
  RGB_matrix_cls(FrameBuffer);


  /* Find first unused window. */
  for (Loop1UInt8 = 0; Loop1UInt8 < MAX_WINDOWS; ++Loop1UInt8)
  {
    if (Window[Loop1UInt8].WinStatus == WINDOW_UNUSED) break;
  }

  if (Loop1UInt8 == MAX_WINDOWS)
  {
    uart_send(__LINE__, __func__, "All windows are used...\r");
    uart_send(__LINE__, __func__, "Press <Enter> to return to menu: ");
    input_string(String);
    return;
  }



  uart_send(__LINE__, __func__, " --------------------------------------------------------------------\r");
  uart_send(__LINE__, __func__, "                           WINDOW TEST 1\r");
  uart_send(__LINE__, __func__, " --------------------------------------------------------------------\r\r\r");

  /* Set specifications for first window. */
  WindowNumber = 8;
  Window[WindowNumber].StartRow     = 0;
  Window[WindowNumber].StartColumn  = 0;
  Window[WindowNumber].EndRow       = 31;
  Window[WindowNumber].EndColumn    = 63;
  Window[WindowNumber].BorderColor  = RED;
  Window[WindowNumber].InsideColor  = RED;
  Window[WindowNumber].LastBoxState = ACTION_DRAW;
  Window[WindowNumber].WinStatus    = WINDOW_ACTIVE;
  Window[WindowNumber].CountDown    = 0;


  for (Loop1UInt8 = 0; Loop1UInt8 < 32; ++Loop1UInt8)
  {
    RGB_matrix_set_color(0, 0, 31, 63, BLUE);
    RGB_matrix_set_pixel(FrameBuffer, 0, 0, 31, 63);
    win_open(WindowNumber, FLAG_OFF);

    uart_send(__LINE__, __func__, "********** WINDOW COMPLETED **********\r");
    uart_send(__LINE__, __func__, "Press <Enter> to draw next test window or <ESC> to exit test: ");
    input_string(String);
    if (String[0] == 27) return;
    RGB_matrix_cls(FrameBuffer);

    if (Window[WindowNumber].EndRow > 1)
      --Window[WindowNumber].EndRow;
    else
      break;  // get out of <for> loop when done.
  }


  uart_send(__LINE__, __func__, " --------------------------------------------------------------------\r");
  uart_send(__LINE__, __func__, "                           WINDOW TEST 2\r");
  uart_send(__LINE__, __func__, " --------------------------------------------------------------------\r\r\r");

  /* Set specifications for first window. */
  RGB_matrix_set_color(0, 0, 31, 63, BLUE);
  RGB_matrix_set_pixel(FrameBuffer, 0, 0, 31, 63);
  Window[WindowNumber].StartRow     = 0;
  Window[WindowNumber].StartColumn  = 0;
  Window[WindowNumber].EndRow       = 31;
  Window[WindowNumber].EndColumn    = 63;
  Window[WindowNumber].BorderColor  = RED;
  Window[WindowNumber].InsideColor  = RED;
  Window[WindowNumber].LastBoxState = ACTION_DRAW;
  Window[WindowNumber].WinStatus    = WINDOW_ACTIVE;
  Window[WindowNumber].CountDown    = 0;


  for (Loop1UInt8 = 0; Loop1UInt8 < 32; ++Loop1UInt8)
  {
    RGB_matrix_set_color(0, 0, 31, 63, BLUE);
    RGB_matrix_set_pixel(FrameBuffer, 0, 0, 31, 63);
    win_open(WindowNumber, FLAG_OFF);

    uart_send(__LINE__, __func__, "********** WINDOW COMPLETED **********\r");
    uart_send(__LINE__, __func__, "Press <Enter> to draw next test window or <ESC> to exit test: ");
    input_string(String);
    if (String[0] == 27) return;
    RGB_matrix_cls(FrameBuffer);

    if (Window[WindowNumber].StartRow < 31)
      ++Window[WindowNumber].StartRow;
    else
      break;  // get out of <for> loop when done.
  }


  uart_send(__LINE__, __func__, " --------------------------------------------------------------------\r");
  uart_send(__LINE__, __func__, "                           WINDOW TEST 3\r");
  uart_send(__LINE__, __func__, " --------------------------------------------------------------------\r\r\r");

  /* Set specifications for first window. */
  RGB_matrix_set_color(0, 0, 31, 63, BLUE);
  RGB_matrix_set_pixel(FrameBuffer, 0, 0, 31, 63);
  Window[WindowNumber].StartRow     = 0;
  Window[WindowNumber].StartColumn  = 0;
  Window[WindowNumber].EndRow       = 31;
  Window[WindowNumber].EndColumn    = 63;
  Window[WindowNumber].BorderColor  = RED;
  Window[WindowNumber].InsideColor  = RED;
  Window[WindowNumber].LastBoxState = ACTION_DRAW;
  Window[WindowNumber].WinStatus    = WINDOW_ACTIVE;
  Window[WindowNumber].CountDown    = 0;


  for (Loop1UInt8 = 0; Loop1UInt8 < 32; ++Loop1UInt8)
  {
    RGB_matrix_set_color(0, 0, 31, 63, BLUE);
    RGB_matrix_set_pixel(FrameBuffer, 0, 0, 31, 63);
    win_open(WindowNumber, FLAG_OFF);

    uart_send(__LINE__, __func__, "********** WINDOW COMPLETED **********\r");
    uart_send(__LINE__, __func__, "Press <Enter> to draw next test window or <ESC> to exit test: ");
    input_string(String);
    if (String[0] == 27) return;
    RGB_matrix_cls(FrameBuffer);

    if (Window[WindowNumber].StartRow < Window[WindowNumber].EndRow)
    {
      if (Loop1UInt8 % 2)
        ++Window[WindowNumber].StartRow;
      else
        --Window[WindowNumber].EndRow;
    }
    else
      break;  // get out of <for> loop when done.
  }


  uart_send(__LINE__, __func__, " --------------------------------------------------------------------\r");
  uart_send(__LINE__, __func__, "                           WINDOW TEST 4\r");
  uart_send(__LINE__, __func__, " --------------------------------------------------------------------\r\r\r");

  /* Set specifications for first window. */
  RGB_matrix_set_color(0, 0, 31, 63, BLUE);
  RGB_matrix_set_pixel(FrameBuffer, 0, 0, 31, 63);
  Window[WindowNumber].StartRow     = 0;
  Window[WindowNumber].StartColumn  = 0;
  Window[WindowNumber].EndRow       = 31;
  Window[WindowNumber].EndColumn    = 63;
  Window[WindowNumber].BorderColor  = RED;
  Window[WindowNumber].InsideColor  = RED;
  Window[WindowNumber].LastBoxState = ACTION_DRAW;
  Window[WindowNumber].WinStatus    = WINDOW_ACTIVE;
  Window[WindowNumber].CountDown    = 0;


  for (Loop1UInt8 = 0; Loop1UInt8 < 64; ++Loop1UInt8)
  {
    RGB_matrix_set_color(0, 0, 31, 63, BLUE);
    RGB_matrix_set_pixel(FrameBuffer, 0, 0, 31, 63);
    win_open(WindowNumber, FLAG_OFF);

    uart_send(__LINE__, __func__, "********** WINDOW COMPLETED **********\r");
    uart_send(__LINE__, __func__, "Press <Enter> to draw next test window or <ESC> to exit test: ");
    input_string(String);
    if (String[0] == 27) return;
    RGB_matrix_cls(FrameBuffer);

    if (Window[WindowNumber].EndColumn > 0)
      --Window[WindowNumber].EndColumn;
    else
      break;  // get out of <for> loop when done.
  }


  uart_send(__LINE__, __func__, " --------------------------------------------------------------------\r");
  uart_send(__LINE__, __func__, "                           WINDOW TEST 5\r");
  uart_send(__LINE__, __func__, " --------------------------------------------------------------------\r\r\r");

  /* Set specifications for first window. */
  RGB_matrix_set_color(0, 0, 31, 63, BLUE);
  RGB_matrix_set_pixel(FrameBuffer, 0, 0, 31, 63);
  Window[WindowNumber].StartRow     = 0;
  Window[WindowNumber].StartColumn  = 0;
  Window[WindowNumber].EndRow       = 31;
  Window[WindowNumber].EndColumn    = 63;
  Window[WindowNumber].BorderColor  = RED;
  Window[WindowNumber].InsideColor  = RED;
  Window[WindowNumber].LastBoxState = ACTION_DRAW;
  Window[WindowNumber].WinStatus    = WINDOW_ACTIVE;
  Window[WindowNumber].CountDown    = 0;


  for (Loop1UInt8 = 0; Loop1UInt8 < 64; ++Loop1UInt8)
  {
    RGB_matrix_set_color(0, 0, 31, 63, BLUE);
    RGB_matrix_set_pixel(FrameBuffer, 0, 0, 31, 63);
    win_open(WindowNumber, FLAG_OFF);

    uart_send(__LINE__, __func__, "********** WINDOW COMPLETED **********\r");
    uart_send(__LINE__, __func__, "Press <Enter> to draw next test window or <ESC> to exit test: ");
    input_string(String);
    if (String[0] == 27) return;
    RGB_matrix_cls(FrameBuffer);

    if (Window[WindowNumber].EndColumn > 1)
      --Window[WindowNumber].EndColumn;
    else
      break;  // get out of <for> loop when done.
  }


  uart_send(__LINE__, __func__, " --------------------------------------------------------------------\r");
  uart_send(__LINE__, __func__, "                           WINDOW TEST 6\r");
  uart_send(__LINE__, __func__, " --------------------------------------------------------------------\r\r\r");

  /* Set specifications for first window. */
  RGB_matrix_set_color(0, 0, 31, 63, BLUE);
  RGB_matrix_set_pixel(FrameBuffer, 0, 0, 31, 63);
  Window[WindowNumber].StartRow     = 0;
  Window[WindowNumber].StartColumn  = 0;
  Window[WindowNumber].EndRow       = 31;
  Window[WindowNumber].EndColumn    = 63;
  Window[WindowNumber].BorderColor  = RED;
  Window[WindowNumber].InsideColor  = RED;
  Window[WindowNumber].LastBoxState = ACTION_DRAW;
  Window[WindowNumber].WinStatus    = WINDOW_ACTIVE;
  Window[WindowNumber].CountDown    = 0;


  for (Loop1UInt8 = 0; Loop1UInt8 < 64; ++Loop1UInt8)
  {
    RGB_matrix_set_color(0, 0, 31, 63, BLUE);
    RGB_matrix_set_pixel(FrameBuffer, 0, 0, 31, 63);
    win_open(WindowNumber, FLAG_OFF);

    uart_send(__LINE__, __func__, "********** WINDOW COMPLETED **********\r");
    uart_send(__LINE__, __func__, "Press <Enter> to draw next test window or <ESC> to exit test: ");
    input_string(String);
    if (String[0] == 27) return;
    RGB_matrix_cls(FrameBuffer);

    if (Window[WindowNumber].StartColumn < Window[WindowNumber].EndColumn)
    {
      if (Loop1UInt8 % 2)
        ++Window[WindowNumber].StartColumn;
      else
        --Window[WindowNumber].EndColumn;
    }
    else
      break;  // get out of <for> loop when done.
  }

  return;





  /* $TITLE=Test11 */
  /* $PAGE */
  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                            Test number 8.
  \* --------------------------------------------------------------------------------------------------------------------------- */
Test8:
  // Nothing for now...
  return;





  /* $TITLE=Test9 */
  /* $PAGE */
  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                            Test number 9.
                                                   Power supply requirement tests.
  \* --------------------------------------------------------------------------------------------------------------------------- */
Test9:
  /* Power supply requirement tests. */
  printf("\r\r\r");
  uart_send(__LINE__, __func__, "Entering Test number 9\r");
  uart_send(__LINE__, __func__, "Power supply requirement.\r");
  printf("\r\r\r\r\r");
  printf("*** IMPORTANT *** This test will turn On ALL pixels on the LED matrix display\r");
  printf("                  It is recommended to proceed quickly with this test in order\r");
  printf("                  not to overload the device electronics. Even more important\r");
  printf("                  is to use an adequate power supply able to provide the current\r");
  printf("                  required by the device. Do not rely on the current provided by\r");
  printf("                  connecting the RGB-Matrix to your computer's USB port as you\r");
  printf("                  could blow up the computer's USB port it is connected to...\r");
  printf("                  Connect an Amp-meter to the RGB matrix to see the power supply\r");
  printf("                  required for the different scenarios.\r\r");
  printf("\r\r\r\r\r");


  uart_send(__LINE__, __func__, "Press <Enter> to continue or <ESC> to exit this test: ");
  input_string(String);
  if (String[0] == 27) return;
  RGB_matrix_cls(FrameBuffer);


  /* Announce test number. */
  RGB_matrix_set_color(0, 0, 31, 63, BLUE);
  win_printf(WIN_TEST, 2, 99, FONT_5x7, "Test 9");

  for (Loop1UInt8 = 0; Loop1UInt8 < 7; ++Loop1UInt8)
  {
    switch (Loop1UInt8)
    {
      case (0):
        RGB_matrix_set_color(0, 0, 31, 63, BLUE);  // blue, around 195 mA
      break;

      case (1):
        RGB_matrix_set_color(0, 0, 31, 63, GREEN);  // green, around 315 mA
      break;

      case (2):
        RGB_matrix_set_color(0, 0, 31, 63, RED);  // red, around 550 mA
      break;

      case (3):
        RGB_matrix_set_color(0, 0, 31, 63, BLUE + GREEN);  // cyan, around 480 mA
      break;

      case (4):
        RGB_matrix_set_color(0, 0, 31, 63, BLUE + RED);  // magenta, around 625 mA
      break;

      case (5):
        RGB_matrix_set_color(0, 0, 31, 63, GREEN + RED);  // yellow, round 680 mA
      break;

      case (6):
        RGB_matrix_set_color(0, 0, 31, 63, BLUE + GREEN + RED);  // white, around 730 mA
      break;
    }

    uart_send(__LINE__, __func__, "Press <Enter> to turn On all LEDs on RGB-Matrix\r");
    uart_send(__LINE__, __func__, "or <ESC> to exit test: ");
    input_string(String);
    if (String[0] == 27) return;

    uart_send(__LINE__, __func__, "NOTE: *** Current PWM level is: %u\r\r", Pwm[PWM_ID_BRIGHTNESS].Level);
    uart_send(__LINE__, __func__, "LED matrix will automatically turn Off after 5 seconds.\r");
    RGB_matrix_set_pixel(FrameBuffer, 0, 0, 31, 63);
    sleep_ms(5000);  // limit exposure time to 5 seconds to prevent overload.
    RGB_matrix_cls(FrameBuffer);
    uart_send(__LINE__, __func__, "Press <Enter> to proceed with next color\r");
    uart_send(__LINE__, __func__, "or <ESC> to exit test: ");
    input_string(String);
    if (String[0] == 27) return;
  }

  return;





  /* $TITLE=Test10 */
  /* $PAGE */
  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                            Test number 10.
                                                      Active buzzer sound queue
  \* --------------------------------------------------------------------------------------------------------------------------- */
Test10:
  /* Active buzzer sound queue. */
  printf("\r\r\r");
  uart_send(__LINE__, __func__, "Entering Test number 10\r");
  uart_send(__LINE__, __func__, "Active buzzer sound queue tests.\r");


  win_printf(WIN_TEST, 2, 99, FONT_5x7, "Test 10");


  printf("\r\r");
  printf("Active buzzer sound queue tests.\r");

  Length = 50;      // default to 50 msec.
  RepeatCount = 3;  // default to 3 repeats.

  while(1)
  {
    printf("Enter the length of the sounds (in msec, between 50 - 5000), <ESC> to exit test or <Enter> to keep [%u msec]: ", Length);
    input_string(String);
    if (String[0] == 27) return;
    if (String[0] != 0x0D) Length = atoi(String);
    if (Length < 50)   Length = 50;
    if (Length > 5000) Length = 5000;

    printf("Enter the repeat count, <ESC> to exit test or <Enter> to keep [%u times]: ", RepeatCount);
    input_string(String);
    if (String[0] == 27) return;
    if (String[0] != 0x0D) RepeatCount = atoi(String);

    /* Queue the specified sound sequence. */
    queue_add_active(Length, RepeatCount);

    /* Wait till we are done with current sound sequence. */
    while (queue_free_active() != (MAX_ACTIVE_SOUND_QUEUE - 1))
    {
      sleep_ms(50);
    };
  }

   return;





  /* $TITLE=Test11 */
  /* $PAGE */
  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                            Test number 11.
                                            Enter bootsel mode (upload-ready) by software.
  \* --------------------------------------------------------------------------------------------------------------------------- */
Test11:
  /* Enter "bootsel mode" (upload-ready) by software. */
  printf("\r\r\r");
  uart_send(__LINE__, __func__, "Entering Test number 11\r");
  uart_send(__LINE__, __func__, "Entering <bootsel mode> (upload-ready) by software.\r");


  win_printf(WIN_TEST, 2, 99, FONT_5x7, "Test 11");


  printf("\r\r");
  printf("Press <Enter> to test reset_to_usb_boot() method: \r");
  input_string(String);

  /// RGB_matrix_cls(FrameBuffer);
  /// reset_usb_boot(0l, 0l);

  printf("\r\r");
  printf("Press <Enter> to test setting 1200 baud: \r");
  input_string(String);

  RGB_matrix_cls(FrameBuffer);
  uart_init(uart0, 1200);
  /// uart_set_format(uart0, 8, 1, UART_PARITY_NONE);

  return;





  /* $TITLE=Test12 */
  /* $PAGE */
  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                            Test number 12.
  \* --------------------------------------------------------------------------------------------------------------------------- */
Test12:
  // Nothing for now...
  return;





  /* $TITLE=Test13 */
  /* $PAGE */
  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                            Test number 13.
  \* --------------------------------------------------------------------------------------------------------------------------- */
Test13:
  // Nothing for now...
  return;





  /* $TITLE=Test14 */
  /* $PAGE */
  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                            Test number 14.
  \* --------------------------------------------------------------------------------------------------------------------------- */
Test14:
  // Nothing for now...
  return;





  /* $TITLE=Test15 */
  /* $PAGE */
  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                            Test number 15.
  \* --------------------------------------------------------------------------------------------------------------------------- */
Test15:
  // Nothing for now...
  return;





  /* $TITLE=Test16 */
  /* $PAGE */
  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                            Test number 16.
  \* --------------------------------------------------------------------------------------------------------------------------- */
Test16:
  // Nothing for now...
  return;





  /* $TITLE=Test17 */
  /* $PAGE */
  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                            Test number 17.
  \* --------------------------------------------------------------------------------------------------------------------------- */
Test17:
  // Nothing for now...
  return;





  /* $TITLE=Test18 */
  /* $PAGE */
  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                            Test number 18.
  \* --------------------------------------------------------------------------------------------------------------------------- */
Test18:
  // Nothing for now...
  return;





  /* $TITLE=Test19 */
  /* $PAGE */
  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                            Test number 19.
  \* --------------------------------------------------------------------------------------------------------------------------- */
Test19:
  // Nothing for now...
  return;






  /* $TITLE=Test20 */
  /* $PAGE */
  /* --------------------------------------------------------------------------------------------------------------------------- *\
                                                            Test number 20.
  \* --------------------------------------------------------------------------------------------------------------------------- */
Test20:
  // Nothing for now...
  return;
}




/* $TITLE=uart_send() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                                 Send a string to terminal emulator.
\* ============================================================================================================================================================= */
void uart_send(UINT LineNumber, const UCHAR *FunctionName, UCHAR *Format, ...)
{
  UCHAR Dum1Str[512];
  UCHAR Dum2Str[128];
  UCHAR TimeStamp[128];

  UINT Loop1UInt;
  UINT StartChar;

  va_list argp;


  /* Transfer the text to print to variable Dum1Str. */
  va_start(argp, Format);
  vsnprintf(Dum1Str, sizeof(Dum1Str), Format, argp);
  va_end(argp);

  /* Trap special control code for <HOME>. Replace "home" by appropriate control characters for "Home" on a VT101. */
  if (strcmp(Dum1Str, "home") == 0)
  {
    Dum1Str[0] = 0x1B; // ESC code
    Dum1Str[1] = '[';
    Dum1Str[2] = 'H';
    Dum1Str[3] = 0x00;
  }

  /* Trap special control code for <CLS>. Replace "cls" by appropriate control characters for "Clear screen" on a VT101. */
  if (strcmp(Dum1Str, "cls") == 0)
  {
    Dum1Str[0] = 0x1B; // ESC code
    Dum1Str[1] = '[';
    Dum1Str[2] = '2';
    Dum1Str[3] = 'J';
    Dum1Str[4] = 0x00;
  }

  /* No line header will not be printed if first character is a '-',
     or if first character is a line feed '\r' when we simply want to do line spacing in the debug log,
     or if first character is the beginning of a control stream (for example 'Home' or "Clear screen'). */
  if ((Dum1Str[0] != '-') && (Dum1Str[0] != '\r') && (Dum1Str[0] != 0x1B) && (Dum1Str[0] != '|'))
  {
    /* Send line number through UART. */
    printf("[%7u] ", LineNumber);

    /* Retrieve current time stamp - Implement <date_stamp()> to support date and time formatting according to current locale. */
    ds3231_get_time(&CurrentTime);

    /* Send time stamp through UART. */
    printf("[%2.2d-%2.2d-%2.2d  %2.2d:%2.2d:%2.2d] ", CurrentTime.DayOfMonth, CurrentTime.Month, (CurrentTime.Year % 1000), CurrentTime.Hour, CurrentTime.Minute, CurrentTime.Second);

    /* Display function name and align all function names in log file. */
    sprintf(Dum2Str, "[%s]", FunctionName);

    /* Check if function name is too long for a clean format in the log. */
    printf("%s", Dum2Str);
    if (strlen(Dum2Str) > 27)
    {
      /// printf("Dum2Str > 27: <%s>\r", Dum2Str);
      Dum2Str[25] = ']';
      Dum2Str[26] = 0x00;
      /***
      printf("Loop %u = ");
      for (Loop1UInt = 0; Loop1UInt < 20; ++Loop1UInt)
        printf("%2u   ", Dum2Str[Loop1UInt]);
      ***/
    }

    for (Loop1UInt = strlen(FunctionName); Loop1UInt < 20; ++Loop1UInt)
    {
      printf(" ");
    }
    printf("- ");
  }

  /* Send string through UART. */
  // uart_write_blocking(uart0, (UINT8 *)LineString, strlen(LineString));
  printf(Dum1Str);

  return;
}





/* $TITLE=util_bcd2dec() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                              Utility to convert a binary-coded-decimal value to a decimal value.
\* ============================================================================================================================================================= */
UINT8 util_bcd2dec(UINT8 BCDValue)
{
  return (BCDValue >> 4) * 10 + (BCDValue & 0x0F);
}





/* $PAGE */
/* $TITLE=util_crc16() */
/* ============================================================================================================================================================= *\
                                                     Find the cyclic redundancy check of the specified data.
\* ============================================================================================================================================================= */
UINT16 util_crc16(UINT8 *Data, UINT16 DataSize)
{
  UCHAR String[256];

  UINT16 CrcValue;
  UINT8 Loop1UInt8;


  /* Validate data pointer. */
  if (Data == NULL) return 0;

  /***
  if (DebugBitMask & DEBUG_CRC16)
  {
    uart_send(__LINE__, __func__, "Calculating CRC16 of this packet (size: %u):\r", DataSize);
    util_display_data(Data, DataSize);
  }
  ***/

  CrcValue = 0;

  while (DataSize-- > 0)
  {
    CrcValue = CrcValue ^ (UINT8)*Data++ << 8;

    for (Loop1UInt8 = 0; Loop1UInt8 < 8; ++Loop1UInt8)
    {
      if (CrcValue & 0x8000)
        CrcValue = CrcValue << 1 ^ CRC16_POLYNOM;
      else
        CrcValue = CrcValue << 1;
    }
  }

  /***
  if (DebugBitMask & DEBUG_CRC16)
    uart_send(__LINE__, "CRC16 computed: %X\r\r\r", CrcValue & 0xFFFF);
  ***/

  return (CrcValue & 0xFFFF);
}





/* $TITLE=util_dec2bcd() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                              Utility to convert a decimal value to binary-coded-decimal value.
\* ============================================================================================================================================================= */
UINT8 util_dec2bcd(UINT8 DecimalValue)
{
  return ((DecimalValue / 10) << 4) + (DecimalValue % 10);
}





/* $PAGE */
/* $TITLE=util_display_data() */
/* ============================================================================================================================================================= *\
                                            Display data whose pointer is sent in argument to an external monitor.
\* ============================================================================================================================================================= */
void util_display_data(UCHAR *Data, UINT32 DataSize)
{
  UCHAR String[256];

  UINT32 Loop1UInt32;
  UINT32 Loop2UInt32;


  if (DebugBitMask & DEBUG_FLOW)
    uart_send(__LINE__, __func__, "Entering util_display_data()\r\r\r");


  uart_send(__LINE__, __func__, " -----------------------------------------------------------------------------------------\r");
  uart_send(__LINE__, __func__, "   Entering util_display_data() - Data pointer: 0x%X   DataSize: 0x%4.4lX (%u)\r", Data, DataSize, DataSize);
  uart_send(__LINE__, __func__, " -----------------------------------------------------------------------------------------\r");
  uart_send(__LINE__, __func__, "                                                                             Printable\r");
  uart_send(__LINE__, __func__, "   Address    Offset                       Hex data                          characters\r\r");


  for (Loop1UInt32 = 0; Loop1UInt32 < DataSize; Loop1UInt32 += 16)
  {
    /* First, display memory address, offset and hex part. */
    sprintf(String, "[0x%8.8X] [0x%4.4X] - ", &Data[Loop1UInt32], Loop1UInt32);

    for (Loop2UInt32 = 0; Loop2UInt32 < 16; ++Loop2UInt32)
    {
      if ((Loop1UInt32 + Loop2UInt32) >= DataSize)
        strcat(String, "   ");
      else
        sprintf(&String[strlen(String)], "%2.2X ", Data[Loop1UInt32 + Loop2UInt32]);
    }


    uart_send(__LINE__, __func__, String);


    /* Add separator. */
    sprintf(String, "| ");


    /* Then, display ASCII character if it is displayable ASCII (or <.> if it is not). */
    for (Loop2UInt32 = 0; Loop2UInt32 < 16; ++Loop2UInt32)
    {
      if ((Loop1UInt32 + Loop2UInt32) >= DataSize)
        break; // do not count up to 16 if we already reached end of data to display.

      /* Display character if it is displayable ASCII. */
      if ((Data[Loop1UInt32 + Loop2UInt32] >= 0x20) && (Data[Loop1UInt32 + Loop2UInt32] <= 0x7E) && (Data[Loop1UInt32 + Loop2UInt32] != 0x25))
        sprintf(&String[strlen(String)], "%c", Data[Loop1UInt32 + Loop2UInt32]);
      else
        strcat(String, ".");
    }
    strcat(String, "\r");
    uart_send(__LINE__, __func__, String);
    sleep_ms(10);  // prevent communication override.
  }
  printf("\r\r");

  if (DebugBitMask & DEBUG_FLOW)
    uart_send(__LINE__, __func__, "Exiting util_display_data()\r\r\r");

  return;
}





/* $PAGE */
/* $TITLE=util_reverse_8bits() */
/* ============================================================================================================================================================= *\
                                                    Reverse the bit order of the UINT8 value given in argument.
\* ============================================================================================================================================================= */
UINT8 util_reverse_8bits(UINT8 InputData)
{
  UINT8 BitMask;
  UINT8 OutputByte = 0;


  for (BitMask = 1; BitMask > 0; BitMask <<= 1)
  {
    OutputByte <<= 1;
    if (InputData & BitMask) OutputByte |= 1;
  }

  if (DebugBitMask & DEBUG_MATRIX)
  {
    uart_send(__LINE__, __func__, "Input data: 0x%2.2X     Output byte: 0x%2.2X\r", InputData, OutputByte);
  }
  /* For usage in the Waveshare Pico-Green-Clock, the 5 bits used for the display must be in lowest 5 bits. */
  /// return (OutputByte >> 3);

  return OutputByte;
}





/* $PAGE */
/* $TITLE=util_reverse_64bits() */
/* ============================================================================================================================================================= *\
                                                  Reverse the bit order of the UINT64 value given in argument.
                                                           (bit 63 becomes bit 0 and vice-versa).
\* ============================================================================================================================================================= */
UINT64 util_reverse_64bits(UINT64 InputData)
{
  UCHAR String[31];

  UINT64 BitMask;
  UINT64 OutputByte = 0x0ll;


  for (BitMask = 1ll; BitMask > 0ll; BitMask <<= 1ll)
  {
    OutputByte <<= 1ll;
    if (InputData & BitMask) OutputByte |= 1ll;
  }

  if (DebugBitMask & DEBUG_MATRIX)
  {
    uart_send(__LINE__, __func__, "Input data: 0x%16.16llX     Output byte: 0x%16.16llX\r", InputData, OutputByte);
  }

  return OutputByte;
}





/* $PAGE */
/* $TITLE=util_uint64_to_binary_string() */
/* ============================================================================================================================================================= *\
                                             Return the string representing the uint64_t value in binary.
\* ============================================================================================================================================================= */
void util_uint64_to_binary_string(UINT64 Value, UINT8 StringLength, UCHAR *BinaryString)
{
  UINT8 Loop1UInt8;


  /* Initialize binary string with all zeroes. */
  for (Loop1UInt8 = 0; Loop1UInt8 < StringLength; ++Loop1UInt8)
    BinaryString[Loop1UInt8] = '.';  // <.> is a placeholder for <0> (to make it easier visually).
  BinaryString[Loop1UInt8] = 0x00;   // end-of-string.


  /***
  uart_send(__LINE__, __func__, "Entering util_uint64_to-binary_string()\r");
  uart_send(__LINE__, __func__, "==================================================================================================================================\r");
  uart_send(__LINE__, __func__, "Converting value 0x%16.16llX to binary string of length: %u\r", Value, StringLength);
  uart_send(__LINE__, __func__, "Binary string on entry: [%s]\r", BinaryString);
  uart_send(__LINE__, __func__, "BinaryString en entry:\r");
  for (Loop1UInt8 = 0; Loop1UInt8 < StringLength; ++Loop1UInt8)
  {
    printf("0x%2.2X  ", BinaryString[Loop1UInt8]);

    // Line feed after 16 values.
    if (((Loop1UInt8 + 1) % 16) == 0)
      printf("\r");
  }
  printf("\r\r");
  ***/


  /* Replace a '0' with a '1' where it must be replaced. */
  for (Loop1UInt8 = 0; Loop1UInt8 < StringLength; ++Loop1UInt8)
  {
    if (Value & (0x01ll << Loop1UInt8)) BinaryString[StringLength - (Loop1UInt8 + 1)] = '*';  // <*> is a placeholder for <1> (to make it easier visually).
    /// uart_send(__LINE__, __func__, "Value to convert: 0x%16.16llX  Bit number: %2u   Binary string so far: [%s]\r", Value, Loop1UInt8, BinaryString);
  }
  /// uart_send(__LINE__, __func__, "==================================================================================================================================\r");

  return;
}




/* $TITLE=win_blink() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                Setup blink parameters for specific window area. Blinking itself is managed by the 1-second callback.
\* ============================================================================================================================================================= */
void win_blink(UINT8 WindowNumber, UINT8 StartRow, UINT8 StartColumn, UINT8 EndRow, UINT8 EndColumn)
{
  UINT8 ColumnNumber;
  UINT8 RowNumber;

  UINT64 TempBuffer[MAX_ROWS];


  if (DebugBitMask & DEBUG_BLINK) uart_send(__LINE__, __func__, "Entering win_blink(%u  %u  %u  %u  %u)\r", WindowNumber, StartRow, StartColumn, EndRow, EndColumn);

  /* Validate provided coordinates. */
  RGB_matrix_check_coord(&StartRow, &StartColumn, &EndRow, &EndColumn);


  /* Clear the bitmask of the current area of the CheckBuffer to be added to the blinking area. */
  if (DebugBitMask & DEBUG_BLINK)
    uart_send(__LINE__, __func__, "Clear CheckBuffer bitmask (%u  %u  %u  %u  %u)\r", WindowNumber, Window[WindowNumber].StartRow + StartRow, Window[WindowNumber].StartColumn + StartColumn, Window[WindowNumber].StartRow + EndRow, Window[WindowNumber].StartColumn + EndColumn);

  RGB_matrix_clear_pixel(CheckBuffer, Window[WindowNumber].StartRow + StartRow, Window[WindowNumber].StartColumn + StartColumn, Window[WindowNumber].StartRow + EndRow, Window[WindowNumber].StartColumn + EndColumn);



  /* Transfer the current bit pattern being blinked to the FrameBuffer buffer area. */
  for (RowNumber = Window[WindowNumber].StartRow + StartRow; RowNumber <= Window[WindowNumber].StartRow + EndRow; ++RowNumber)
  {
    for (ColumnNumber = Window[WindowNumber].StartColumn + StartColumn; ColumnNumber <= Window[WindowNumber].StartColumn + EndColumn; ++ColumnNumber)
    {
      if (FrameBuffer[RowNumber] & (0x1ll << ColumnNumber))
        BlinkBuffer[RowNumber] |= (0x1ll << ColumnNumber);
      else
        BlinkBuffer[RowNumber] &= ~(0x1ll << ColumnNumber);
    }
  }



  if (DebugBitMask & DEBUG_BLINK)
  {
    /* Optionally display CheckBuffer. */
    for (RowNumber = 0; RowNumber < MAX_ROWS; ++RowNumber)
      TempBuffer[RowNumber] = util_reverse_64bits(CheckBuffer[RowNumber]);

    uart_send(__LINE__, __func__, "CheckBuffer:\r");
    display_matrix_buffer(TempBuffer);
    printf("\r\r");


    /* Display BlinkBuffer. */
    for (RowNumber = 0; RowNumber < MAX_ROWS; ++RowNumber)
      TempBuffer[RowNumber] = util_reverse_64bits(BlinkBuffer[RowNumber]);

    uart_send(__LINE__, __func__, "BlinkBuffer:\r");
    display_matrix_buffer(TempBuffer);
    printf("\r\r");
  }


  /* Set FlagBlink on target window structure. */
  Window[WindowNumber].FlagBlink    = FLAG_ON;
  Window[WindowNumber].BlinkOnTimer = time_us_32();

  return;
}





/* $TITLE=win_blink_off() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                         Turn Off blinking on the specific window area.
\* ============================================================================================================================================================= */
void win_blink_off(UINT8 WindowNumber, UINT8 StartRow, UINT8 StartColumn, UINT8 EndRow, UINT8 EndColumn)
{
  UINT8 Loop1UInt8;


  if (DebugBitMask & DEBUG_BLINK)
    uart_send(__LINE__, __func__, "Set CheckBuffer bitmask (%u  %u  %u  %u  %u)\r", WindowNumber, Window[WindowNumber].StartRow + StartRow, Window[WindowNumber].StartColumn + StartColumn, Window[WindowNumber].StartRow + EndRow, Window[WindowNumber].StartColumn + EndColumn);

  /* Validate provided coordinates. */
  RGB_matrix_check_coord(&StartRow, &StartColumn, &EndRow, &EndColumn);

  /* Stuff the bitmask of the current area of the RGB LED matrix to be removed from the blinking area. */
  RGB_matrix_set_pixel(CheckBuffer, Window[WindowNumber].StartRow + StartRow, Window[WindowNumber].StartColumn + StartColumn, Window[WindowNumber].StartRow + EndRow, Window[WindowNumber].StartColumn + EndColumn);

  /* Clear the BlinkBuffer area containing the bitmask to blink On and Off. */
  RGB_matrix_clear_pixel(BlinkBuffer, Window[WindowNumber].StartRow + StartRow, Window[WindowNumber].StartColumn + StartColumn, Window[WindowNumber].StartRow + EndRow, Window[WindowNumber].StartColumn + EndColumn);

  /* Check if there is another area of the window that is also currently blinking. */
  for (Loop1UInt8 = Window[WindowNumber].StartRow; Loop1UInt8 <= Window[WindowNumber].EndRow; ++Loop1UInt8)
  {
    if (CheckBuffer[Loop1UInt8] != 0ll) break;  // if we find one, get out of <for> loop.
  }

  if (Loop1UInt8 > Window[WindowNumber].EndRow)
  {
    /* THere is no more blinking area in this window. */
    Window[WindowNumber].FlagBlink = FLAG_OFF;
    Window[WindowNumber].BlinkOnTimer = 0l;
  }

  return;
}





/* $TITLE=win_close() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                                     Close the specified window.
\* ============================================================================================================================================================= */
void win_close(UINT8 WindowNumber)
{
  UINT8 Loop1UInt8;


  /* Since we are closing the specified window, make it inactive. */
  Window[WindowNumber].WinStatus = WINDOW_INACTIVE;  // since we close current window, make it inactive.


  /* Make sure there is no residual scrolling structure still assigned the window we are closing. */
  for (Loop1UInt8 = 0; Loop1UInt8 < MAX_ACTIVE_SCROLL; ++Loop1UInt8)
  {
    /* Scan all active scrolls to check if some owners are the current closing window. */
    if (ActiveScroll[Loop1UInt8]->Owner == Loop1UInt8) win_scroll_off(Loop1UInt8);
  }


  /* Make sure there is no residual blinking still active for the window we are closing. */
  win_blink_off(WindowNumber, Window[WindowNumber].StartRow, Window[WindowNumber].StartColumn, Window[WindowNumber].EndRow, Window[WindowNumber].EndColumn);


  /* Restore the active Top, Middle and / or Bottom windows according to the backlinks of the window we are closing. */
  if (Window[WindowNumber].TopBackLink != MAX_WINDOWS)
  {
    if (DebugBitMask & DEBUG_WINDOW) uart_send(__LINE__, __func__, "Closing %s and setting WinTop to %s\r", Window[WindowNumber].Name, Window[Window[WindowNumber].TopBackLink].Name);
    WinTop = Window[WindowNumber].TopBackLink;

    /* Open active window as set in the backlink. */
    win_open(Window[WindowNumber].TopBackLink, FLAG_ON);  // restore the window that was the TopBackLink.
  }


  if (Window[WindowNumber].MidBackLink != MAX_WINDOWS)
  {
    if (DebugBitMask & DEBUG_WINDOW) uart_send(__LINE__, __func__, "Closing %s and setting WinMid to %s\r", Window[WindowNumber].Name, Window[Window[WindowNumber].MidBackLink].Name);
    WinMid = Window[WindowNumber].MidBackLink;
    /* We assume that both WinTop and WinMid are tied together and that restoring WinTop above has already restored WinMid simultaneously. */
  }


  if (Window[WindowNumber].BotBackLink != MAX_WINDOWS)
  {
    if (DebugBitMask & DEBUG_WINDOW) uart_send(__LINE__, __func__, "Closing %s and setting WinBot to %s\r", Window[WindowNumber].Name, Window[Window[WindowNumber].BotBackLink].Name);
    WinBot = Window[WindowNumber].BotBackLink;

    /* Open active window as set in the backlink. */
    win_open(Window[WindowNumber].BotBackLink, FLAG_ON);  // restore the window that was the BotBackLink.
  }


  /* Reset the backlink of the inactive window that we just closed. */
  Window[WindowNumber].TopBackLink = MAX_WINDOWS;    // the backlink for the closing window becomes obsolete.
  Window[WindowNumber].MidBackLink = MAX_WINDOWS;    // the backlink for the closing window becomes obsolete.
  Window[WindowNumber].BotBackLink = MAX_WINDOWS;    // the backlink for the closing window becomes obsolete.

  return;
}





/* $TITLE=win_cls() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                                     Clear the specified window.
\* ============================================================================================================================================================= */
void win_cls(UINT8 WindowNumber)
{
  /* If the last drawn box border remains turned On, clear the area inside the box. Otherwise clear including the box border. */
  if (Window[WindowNumber].LastBoxState == ACTION_DRAW)
  {
    RGB_matrix_clear_pixel(FrameBuffer, Window[WindowNumber].StartRow + 1, Window[WindowNumber].StartColumn + 1, Window[WindowNumber].EndRow - 1, Window[WindowNumber].EndColumn - 1);
    /* Redraw border in case it has been corrupted for some reason. */
    RGB_matrix_box(Window[WindowNumber].StartRow, Window[WindowNumber].StartColumn, Window[WindowNumber].EndRow, Window[WindowNumber].EndColumn, Window[WindowNumber].BorderColor, ACTION_DRAW);
  }
  else
    RGB_matrix_clear_pixel(FrameBuffer, Window[WindowNumber].StartRow, Window[WindowNumber].StartColumn, Window[WindowNumber].EndRow, Window[WindowNumber].EndColumn);

  return;
}





/* $TITLE=win_init() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                                     Initialize window structures.
\* ============================================================================================================================================================= */
void win_init()
{
  UINT16 Loop1UInt16;
  UINT16 Loop2UInt16;


  WinTop = MAX_WINDOWS;
  WinMid = MAX_WINDOWS;
  WinBot = MAX_WINDOWS;

  /* Generic windows initialization. */
  for (Loop1UInt16 = 0; Loop1UInt16 < MAX_WINDOWS; ++Loop1UInt16)
  {
    sprintf(Window[Loop1UInt16].Name, "%2.2u-Undefined", Loop1UInt16);
    Window[Loop1UInt16].StartRow      = 0;
    Window[Loop1UInt16].StartColumn   = 0;
    Window[Loop1UInt16].EndRow        = 0;
    Window[Loop1UInt16].EndColumn     = 0;
    Window[Loop1UInt16].BorderColor   = BLUE;
    Window[Loop1UInt16].InsideColor   = BLUE;
    Window[Loop1UInt16].LastBoxState  = ACTION_DRAW;
    Window[Loop1UInt16].WinStatus     = WINDOW_UNUSED;
    Window[Loop1UInt16].FlagBlink     = FLAG_OFF;
    Window[Loop1UInt16].BlinkOnTimer  = 0l;
    Window[Loop1UInt16].CountDown     = 0;
    Window[Loop1UInt16].TopBackLink   = MAX_WINDOWS;
    Window[Loop1UInt16].MidBackLink   = MAX_WINDOWS;
    Window[Loop1UInt16].BotBackLink   = MAX_WINDOWS;
    Window[Loop1UInt16].FlagTopScroll = FLAG_OFF;
    Window[Loop1UInt16].FlagMidScroll = FLAG_OFF;
    Window[Loop1UInt16].FlagBotScroll = FLAG_OFF;
  }

  /* Initialize specific parameters for WIN_DATE window. */
  sprintf(Window[WIN_DATE].Name, "WIN_DATE");
  Window[WIN_DATE].StartRow     = 0;
  Window[WIN_DATE].StartColumn  = 0;
  Window[WIN_DATE].EndRow       = 17;
  Window[WIN_DATE].EndColumn    = 63;
  Window[WIN_DATE].BorderColor  = CYAN;
  Window[WIN_DATE].InsideColor  = CYAN;
  Window[WIN_DATE].LastBoxState = ACTION_ERASE;
  Window[WIN_DATE].WinStatus    = WINDOW_INACTIVE;

  /* Initialize specific parameters for WIN_TIME window. */
  sprintf(Window[WIN_TIME].Name, "WIN_TIME");
  Window[WIN_TIME].StartRow     = 18;
  Window[WIN_TIME].StartColumn  = 0;
  Window[WIN_TIME].EndRow       = 31;
  Window[WIN_TIME].EndColumn    = 63;
  Window[WIN_TIME].BorderColor  = BLUE;
  Window[WIN_TIME].InsideColor  = GREEN;
  Window[WIN_TIME].WinStatus    = WINDOW_INACTIVE;

  /* Initialize specific parameters for WIN_TEST window. */
  sprintf(Window[WIN_TEST].Name, "WIN_TEST");
  Window[WIN_TEST].StartRow     = 0;
  Window[WIN_TEST].StartColumn  = 0;
  Window[WIN_TEST].EndRow       = 31;
  Window[WIN_TEST].EndColumn    = 63;
  Window[WIN_TEST].BorderColor  = BLUE;
  Window[WIN_TEST].InsideColor  = BLUE;
  Window[WIN_TEST].WinStatus    = WINDOW_INACTIVE;

  /* Initialize specific parameters for WIN_FUNCTION window. */
  sprintf(Window[WIN_FUNCTION].Name, "WIN_FUNCTION");
  Window[WIN_FUNCTION].StartRow     = 0;
  Window[WIN_FUNCTION].StartColumn  = 0;
  Window[WIN_FUNCTION].EndRow       = 17;
  Window[WIN_FUNCTION].EndColumn    = 63;
  Window[WIN_FUNCTION].BorderColor  = CYAN;
  Window[WIN_FUNCTION].InsideColor  = CYAN;
  Window[WIN_FUNCTION].LastBoxState = ACTION_ERASE;
  Window[WIN_FUNCTION].WinStatus    = WINDOW_INACTIVE;

  /* Initialize specific parameters for WIN_DICE window. */
  sprintf(Window[WIN_DICE].Name, "WIN_DICE");
  Window[WIN_DICE].StartRow     = 0;
  Window[WIN_DICE].StartColumn  = 0;
  Window[WIN_DICE].EndRow       = 31;
  Window[WIN_DICE].EndColumn    = 63;
  Window[WIN_DICE].BorderColor  = BLUE;
  Window[WIN_DICE].InsideColor  = BLUE;
  Window[WIN_DICE].WinStatus    = WINDOW_INACTIVE;

  /* Initialize specific parameters for WIN_MESSAGE window. */
  sprintf(Window[WIN_MESSAGE].Name, "WIN_MESSAGE");
  Window[WIN_MESSAGE].StartRow     = 0;
  Window[WIN_MESSAGE].StartColumn  = 0;
  Window[WIN_MESSAGE].EndRow       = 17;
  Window[WIN_MESSAGE].EndColumn    = 63;
  Window[WIN_MESSAGE].BorderColor  = MAGENTA;
  Window[WIN_MESSAGE].InsideColor  = MAGENTA;
  Window[WIN_MESSAGE].LastBoxState = ACTION_ERASE;
  Window[WIN_MESSAGE].WinStatus    = WINDOW_INACTIVE;

  /* Initialize specific parameters for WIN_SETUP window. */
  sprintf(Window[WIN_SETUP].Name, "WIN_SETUP");
  Window[WIN_SETUP].StartRow     = 0;
  Window[WIN_SETUP].StartColumn  = 0;
  Window[WIN_SETUP].EndRow       = 31;
  Window[WIN_SETUP].EndColumn    = 63;
  Window[WIN_SETUP].BorderColor  = MAGENTA;
  Window[WIN_SETUP].InsideColor  = MAGENTA;
  Window[WIN_SETUP].LastBoxState = ACTION_ERASE;
  Window[WIN_SETUP].WinStatus    = WINDOW_INACTIVE;

  if (DebugBitMask & DEBUG_WINDOW)
  {
    printf("\r");
    printf("main() - Displaying window names:\r");
    for (Loop1UInt16 = 0; Loop1UInt16 < MAX_WINDOWS; ++Loop1UInt16)
    {
      printf("%2u) <%s>", Loop1UInt16, Window[Loop1UInt16].Name);
      if (((Loop1UInt16 + 1) % 5) == 0)
        printf("\r");
      else
      {
        for (Loop2UInt16 = strlen(Window[Loop1UInt16].Name); Loop2UInt16 < 15; ++Loop2UInt16)
          printf(" ");
      }
    }
    printf("\r\r");
  }

  return;
}





/* $TITLE=win_open() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                                     Draw the specified window.
                                    NOTE: If we are "restoring" a previously suspended window, do not update the backlinks,
\* ============================================================================================================================================================= */
void win_open(UINT8 WindowNumber, UINT8 FlagRestore)
{
  UCHAR String[16];

  INT8 Remainder;

  UINT8 CurrentBottomRow;
  UINT8 CurrentTopRow;
  UINT8 CurrentRightColumn;
  UINT8 CurrentLeftColumn;
  UINT8 Increment;
  UINT8 Iteration;
  UINT8 Loop1UInt8;
  UINT8 Loop2UInt8;
  UINT8 NbRows;
  UINT8 NbColumns;
  UINT8 OldBR;
  UINT8 OldLC;
  UINT8 OldRC;
  UINT8 OldTR;
  UINT8 StartLength;


  /* If StartRow is greater than EndRow, reverse both values. */
  if (Window[WindowNumber].StartRow > Window[WindowNumber].EndRow)
  {
    Loop1UInt8 = Window[WindowNumber].EndRow;  // temporary buffer.
    Window[WindowNumber].EndRow   = Window[WindowNumber].StartRow;
    Window[WindowNumber].StartRow = Loop1UInt8;
  }

  /* If StartColumn is greater than EndColumn, reverse both values. */
  if (Window[WindowNumber].StartColumn > Window[WindowNumber].EndColumn)
  {
    Loop1UInt8 = Window[WindowNumber].EndColumn;  // temporary buffer.
    Window[WindowNumber].EndColumn   = Window[WindowNumber].StartColumn;
    Window[WindowNumber].StartColumn = Loop1UInt8;
  }

  /* Make sure there is at least one free row or one free column for the specified box parameters. */
  if  (Window[WindowNumber].StartRow    == Window[WindowNumber].EndRow)    return;
  if  (Window[WindowNumber].StartColumn == Window[WindowNumber].EndColumn) return;
  if ((Window[WindowNumber].EndRow    - Window[WindowNumber].StartRow)    < 2) return;
  if ((Window[WindowNumber].EndColumn - Window[WindowNumber].StartColumn) < 2) return;

  /* Parameters validation. */
  if (Window[WindowNumber].EndRow    > MAX_ROWS    - 1) return;
  if (Window[WindowNumber].EndColumn > MAX_COLUMNS - 1) return;



  Iteration = 0;  // number of "exploding" boxes already drawn.
  NbRows    = Window[WindowNumber].EndRow    - Window[WindowNumber].StartRow    + 1;
  NbColumns = Window[WindowNumber].EndColumn - Window[WindowNumber].StartColumn + 1;


  /***
  if (DebugBitMask & DEBUG_WINDOW)
  {
    uart_send(__LINE__, __func__, "Entering win_open(%u):\r\r", WindowNumber);
    uart_send(__LINE__, __func__, "StartRow:    %2u       EndRow:    %2u     NbRows:    %2u\r",   Window[WindowNumber].StartRow,    Window[WindowNumber].EndRow,    NbRows);
    uart_send(__LINE__, __func__, "StartColumn: %2u       EndColumn: %2u     NbColumns: %2u\r\r", Window[WindowNumber].StartColumn, Window[WindowNumber].EndColumn, NbColumns);
    uart_send(__LINE__, __func__, "BoxColor: %-10s EndColor: %-10s     LastBox:   %2u\r",        ColorName[Window[WindowNumber].BoxColor], ColorName[Window[WindowNumber].EndColor], Window[WindowNumber].LastBox);
    uart_send(__LINE__, __func__, "Press <Enter> to continue: ");
    input_string(String);
  }
  ***/


  if (NbColumns >= NbRows)
  {
    /* Landscape or square window. */
    /// if (DebugBitMask & DEBUG_WINDOW) uart_send(__LINE__, __func__, "Landscape or square window.\r");

    /* Compute increment and first row length. */
    if (NbColumns == ((NbRows - 1) * 2))  /// add this block to columns as well.
    {
      Increment = 2;
      Remainder = -2;
    }
    else if ((NbColumns + 1) == (NbRows * 2))
    {
      Increment = 2;
      Remainder = -1;
    }
    else
    {
      for (Loop1UInt8 = 60; Loop1UInt8 > 0; --Loop1UInt8)
      {
        if (NbColumns >= (NbRows * Loop1UInt8))
        {
          Increment = (UINT8)(NbColumns / NbRows);
          Remainder = NbColumns % NbRows;
          break;  // get out of "for" loop as soon as we found a valid multiplier.
        }
      }
    }


    if ((NbRows % 2) || (NbRows == 1))
    {
      /* Odd number of rows. */
      CurrentTopRow      = (((Window[WindowNumber].EndRow - Window[WindowNumber].StartRow) / 2) + Window[WindowNumber].StartRow);
      CurrentBottomRow   = (((Window[WindowNumber].EndRow - Window[WindowNumber].StartRow) / 2) + Window[WindowNumber].StartRow);

      /* If number of rows is odd, the length of the center row will be one Increment value + Remainder. */
      StartLength = Increment + Remainder;
      CurrentLeftColumn  = ((UINT8)((Window[WindowNumber].EndRow - Window[WindowNumber].StartRow) / 2)) * Increment;
      CurrentRightColumn = CurrentLeftColumn + StartLength - 1;

      /***
      if (DebugBitMask & DEBUG_WINDOW)
      {
        uart_send(__LINE__, __func__, "Odd  number of rows\r");
        uart_send(__LINE__, __func__, "CenterRow: %u\r", (UINT8)((Window[WindowNumber].EndRow - Window[WindowNumber].StartRow) / 2) + Window[WindowNumber].StartRow);
      }
      ***/
    }
    else
    {
      /* Even number of rows. */
      CurrentTopRow      =  (((Window[WindowNumber].EndRow - Window[WindowNumber].StartRow) / 2) + Window[WindowNumber].StartRow);
      CurrentBottomRow   = ((((Window[WindowNumber].EndRow - Window[WindowNumber].StartRow) / 2) + Window[WindowNumber].StartRow) + 1);

      /* If number of rows is even, the length of the two center rows will be two Increment values + Remainder. */
      StartLength = (2 * Increment) + Remainder;
      CurrentLeftColumn  = ((UINT8)((Window[WindowNumber].EndRow - Window[WindowNumber].StartRow) / 2) * Increment);
      CurrentRightColumn = CurrentLeftColumn + StartLength - 1;

      /***
      if (DebugBitMask & DEBUG_WINDOW)
      {
        uart_send(__LINE__, __func__, "Even number of rows\r");
        uart_send(__LINE__, __func__, "CenterRows: %2u and %2u\r", (UINT8)((Window[WindowNumber].EndRow - Window[WindowNumber].StartRow) / 2) + Window[WindowNumber].StartRow, (UINT8)((Window[WindowNumber].EndRow - Window[WindowNumber].StartRow) / 2) + Window[WindowNumber].StartRow + 1);
      }
      ***/
    }


    /***
    if (DebugBitMask & DEBUG_WINDOW)
    {
      uart_send(__LINE__, __func__, "NbRows:             %2u     NbColumns:          %2u\r",   NbRows,            NbColumns);
      uart_send(__LINE__, __func__, "Increment:          %2u     Remainder:         %3d       StartLength: %2u\r",   Increment, Remainder, StartLength);
      uart_send(__LINE__, __func__, "CurrentTopRow:      %2u     CurrentBottomRow:   %2u\r",   CurrentTopRow,     CurrentBottomRow);
      uart_send(__LINE__, __func__, "CurrentLeftColumn:  %2u     CurrentRightColumn: %2u\r\r", CurrentLeftColumn, CurrentRightColumn);
    }
    ***/


    /* Make the active Top, Middle and / or Bottom window the one we are opening to shield it from external actions, and also set its backlink. */
    if ((Window[WindowNumber].StartRow < 4) && (Window[WindowNumber].EndRow > 4))
    {
      if (DebugBitMask & DEBUG_WINDOW) uart_send(__LINE__, __func__, "Setting WinTop to %s\r", Window[WindowNumber].Name);
      if (FlagRestore == FLAG_OFF)
      {
        Window[WindowNumber].TopBackLink = WinTop;

        if (WinTop != MAX_WINDOWS)
        {
          Window[WinTop].WinStatus = WINDOW_INACTIVE;  // current top window becomes inactive.
          if (DebugBitMask & DEBUG_WINDOW) uart_send(__LINE__, __func__, "Setting window %s to INACTIVE\r", Window[WinTop].Name);
        }
      }

      WinTop = WindowNumber;
    }

    if ((Window[WindowNumber].StartRow < 12) && (Window[WindowNumber].EndRow > 12))
    {
      if (DebugBitMask & DEBUG_WINDOW) uart_send(__LINE__, __func__, "Setting WinMid to %s\r", Window[WindowNumber].Name);
      if (FlagRestore == FLAG_OFF)
      {
        Window[WindowNumber].MidBackLink = WinMid;

        if (WinMid != MAX_WINDOWS)
        {
          Window[WinMid].WinStatus = WINDOW_INACTIVE;  // current middle window becomes inactive.
          if (DebugBitMask & DEBUG_WINDOW) uart_send(__LINE__, __func__, "Setting window %s to INACTIVE\r", Window[WinMid].Name);
        }
      }

      WinMid = WindowNumber;
    }

    if ((Window[WindowNumber].StartRow < 25) && (Window[WindowNumber].EndRow > 25))
    {
      if (DebugBitMask & DEBUG_WINDOW) uart_send(__LINE__, __func__, "Setting WinBot to %s\r", Window[WindowNumber].Name);
      if (FlagRestore == FLAG_OFF)
      {
        Window[WindowNumber].BotBackLink = WinBot;

        if (WinBot != MAX_WINDOWS)
        {
          Window[WinBot].WinStatus = WINDOW_INACTIVE;
          if (DebugBitMask & DEBUG_WINDOW) uart_send(__LINE__, __func__, "Setting window %s to INACTIVE\r", Window[WinBot].Name);
        }
      }

      WinBot = WindowNumber;
    }



    /* Explode opening window. */
    while (CurrentBottomRow <= Window[WindowNumber].EndRow)
    {
      /* Draw new box. */
      RGB_matrix_box(CurrentTopRow, CurrentLeftColumn, CurrentBottomRow, CurrentRightColumn, Window[WindowNumber].BorderColor, ACTION_DRAW);
      /***
      if (DebugBitMask & DEBUG_WINDOW)
      {
        uart_send(__LINE__, __func__, "Drawing (%2u, %2u)   (%2u, %2u)\r", CurrentTopRow, CurrentLeftColumn, CurrentBottomRow, CurrentRightColumn);
        uart_send(__LINE__, __func__, "Press <Enter> to continue: ");
        input_string(String);
      }
      ***/

      /* If it is not first iteration, erase previous box. */
      if (Iteration != 0)
      {
        RGB_matrix_clear_pixel(FrameBuffer, CurrentTopRow + 1, CurrentLeftColumn + 1, CurrentBottomRow - 1, CurrentRightColumn - 1);
        /***
        if (DebugBitMask & DEBUG_WINDOW)
        {
          uart_send(__LINE__, __func__, "Erasing (%2u, %2u)   (%2u, %2u)\r", CurrentTopRow + 1, CurrentLeftColumn + 1, CurrentBottomRow - 1, CurrentRightColumn - 1);
          /// RGB_matrix_box(OldTR, OldLC, OldBR, OldRC, 0, ACTION_ERASE);
          /// uart_send(__LINE__, __func__, "Erasing (%2u, %2u)   (%2u, %2u)\r", OldTR, OldLC, OldBR, OldRC);
          uart_send(__LINE__, __func__, "Press <Enter> to continue: ");
          input_string(String);
        }
        ***/
      }

      ++Iteration;

      /* Keep track of the box just drawn to erase it on next pass. */
      OldTR = CurrentTopRow;
      OldBR = CurrentBottomRow;
      OldLC = CurrentLeftColumn;
      OldRC = CurrentRightColumn;

      /* Find coordinates of the next box to draw. */
      --CurrentTopRow;
      ++CurrentBottomRow;
      CurrentLeftColumn  -= Increment;
      CurrentRightColumn += Increment;

      sleep_ms(50);  // slow down animation a bit.
    }

    /* Check if last box must be erased. */
    if (Window[WindowNumber].LastBoxState == ACTION_ERASE)
    {
      sleep_ms(50);  // keep same animation pace than previous ones.
      RGB_matrix_box(OldTR, OldLC, OldBR, OldRC, 0, ACTION_ERASE);
      /***
      if (DebugBitMask & DEBUG_WINDOW)
      {
        uart_send(__LINE__, __func__, "Erasing last (%2u, %2u)   (%2u, %2u)\r", OldTR, OldLC, OldBR, OldRC);
        uart_send(__LINE__, __func__, "Press <Enter> to continue: ");
        input_string(String);
      }
      ***/
    }

    /* When done, set inbox final color. */
    RGB_matrix_set_color(Window[WindowNumber].StartRow + 1, Window[WindowNumber].StartColumn + 1, Window[WindowNumber].EndRow - 1, Window[WindowNumber].EndColumn - 1, Window[WindowNumber].InsideColor);
  }
  else
  {
    /* Portrait window. */
    /// if (DebugBitMask & DEBUG_WINDOW) uart_send(__LINE__, __func__, "Portrait window.\r");

    /* Compute increment and first column length. */
    for (Loop1UInt8 = 60; Loop1UInt8 > 0; --Loop1UInt8)
    {
      if (NbRows >= (NbColumns * Loop1UInt8))
      {
        Increment = (UINT8)(NbRows / NbColumns);
        Remainder = NbRows % NbColumns;
        break;  // get out of "for" loop as soon as we found a valid multiplier.
      }
    }


    if ((NbColumns % 2) || (NbColumns == 1))
    {
      /* Odd number of columns. */
      CurrentLeftColumn  = (((Window[WindowNumber].EndColumn - Window[WindowNumber].StartColumn) / 2) + Window[WindowNumber].StartColumn);
      CurrentRightColumn = (((Window[WindowNumber].EndColumn - Window[WindowNumber].StartColumn) / 2) + Window[WindowNumber].StartColumn);

      /* If number of columns is odd, the length of the center column will be one Increment value + Remainder. */
      StartLength = Increment + Remainder;
      CurrentTopRow    = ((UINT8)((Window[WindowNumber].EndColumn - Window[WindowNumber].StartColumn) / 2)) * Increment;
      CurrentBottomRow = CurrentTopRow + StartLength - 1;

      /***
      if (DebugBitMask & DEBUG_WINDOW)
      {
        uart_send(__LINE__, __func__, "Odd  number of columns\r");
        uart_send(__LINE__, __func__, "CenterColumn: %u\r", (UINT8)((Window[WindowNumber].EndColumn - Window[WindowNumber].StartColumn) / 2) + Window[WindowNumber].StartColumn);
      }
      ***/
    }
    else
    {
      /* Even number of rows. */
      CurrentLeftColumn  =  (((Window[WindowNumber].EndColumn - Window[WindowNumber].StartColumn) / 2) + Window[WindowNumber].StartColumn);
      CurrentRightColumn = ((((Window[WindowNumber].EndColumn - Window[WindowNumber].StartColumn) / 2) + Window[WindowNumber].StartColumn) + 1);

      /* If number of rows is even, the length of the two center rows will be two Increment values + Remainder. */
      StartLength = (2 * Increment) + Remainder;
      CurrentTopRow    = ((UINT8)((Window[WindowNumber].EndColumn - Window[WindowNumber].StartColumn) / 2) * Increment);
      CurrentBottomRow = CurrentTopRow + StartLength - 1;

      /***
      if (DebugBitMask & DEBUG_WINDOW)
      {
        uart_send(__LINE__, __func__, "Even number of columns\r");
        uart_send(__LINE__, __func__, "CenterColumns: %2u and %2u\r", (UINT8)((Window[WindowNumber].EndColumn - Window[WindowNumber].StartColumn) / 2) + Window[WindowNumber].StartColumn, (UINT8)((Window[WindowNumber].EndColumn - Window[WindowNumber].StartColumn) / 2) + Window[WindowNumber].StartColumn + 1);
      }
      ***/
    }



    /***
    if (DebugBitMask & DEBUG_WINDOW)
    {
      uart_send(__LINE__, __func__, "NbRows:             %2u     NbColumns:          %2u\r",   NbRows,            NbColumns);
      uart_send(__LINE__, __func__, "Increment:          %2u     Remainder:         %3d       StartLength: %2u\r",   Increment, Remainder, StartLength);
      uart_send(__LINE__, __func__, "CurrentTopRow:      %2u     CurrentBottomRow:   %2u\r",   CurrentTopRow,     CurrentBottomRow);
      uart_send(__LINE__, __func__, "CurrentLeftColumn:  %2u     CurrentRightColumn: %2u\r\r", CurrentLeftColumn, CurrentRightColumn);
    }
    ***/



    /* Explode window. */
    while (CurrentLeftColumn <= Window[WindowNumber].EndColumn)
    {
      /* Draw new box. */
      RGB_matrix_box(CurrentTopRow, CurrentLeftColumn, CurrentBottomRow, CurrentRightColumn, Window[WindowNumber].BorderColor, ACTION_DRAW);
      /***
      if (DebugBitMask & DEBUG_WINDOW)
      {
        uart_send(__LINE__, __func__, "Drawing (%2u, %2u)   (%2u, %2u)\r", CurrentTopRow, CurrentLeftColumn, CurrentBottomRow, CurrentRightColumn);
        uart_send(__LINE__, __func__, "Press <Enter> to continue: ");
        input_string(String);
      }
      ***/

      /* If it is not first iteration, erase previous box. */
      if (Iteration != 0)
      {
        RGB_matrix_clear_pixel(FrameBuffer, CurrentTopRow + 1, CurrentLeftColumn + 1, CurrentBottomRow - 1, CurrentRightColumn - 1);
        /***
        if (DebugBitMask & DEBUG_WINDOW)
        {
          uart_send(__LINE__, __func__, "Erasing (%2u, %2u)   (%2u, %2u)\r", CurrentTopRow + 1, CurrentLeftColumn + 1, CurrentBottomRow - 1, CurrentRightColumn - 1);
          uart_send(__LINE__, __func__, "Press <Enter> to continue: ");
          input_string(String);
        }
        ***/
      }

      ++Iteration;

      /* Keep track of the box just drawn to erase it on next pass. */
      OldTR = CurrentTopRow;
      OldBR = CurrentBottomRow;
      OldLC = CurrentLeftColumn;
      OldRC = CurrentRightColumn;

      /* Find coordinates of the next box to draw. */
      --CurrentLeftColumn;
      ++CurrentRightColumn;
      CurrentTopRow    -= Increment;
      CurrentBottomRow += Increment;

      sleep_ms(50);  // slow down animation a bit.
    }

    /* Check if last box must be erased. */
    if (Window[WindowNumber].LastBoxState == ACTION_ERASE)
    {
      sleep_ms(50);  // keep same animation pace than previous ones.
      RGB_matrix_box(OldTR, OldLC, OldBR, OldRC, 0, ACTION_ERASE);
      /***
      if (DebugBitMask & DEBUG_WINDOW)
      {
        uart_send(__LINE__, __func__, "Erasing last (%2u, %2u)   (%2u, %2u)\r", OldTR, OldLC, OldBR, OldRC);
        uart_send(__LINE__, __func__, "Press <Enter> to continue: ");
        input_string(String);
      }
      ***/
    }

    /* When done, set inbox final color. */
    RGB_matrix_set_color(Window[WindowNumber].StartRow + 1, Window[WindowNumber].StartColumn + 1, Window[WindowNumber].EndRow - 1, Window[WindowNumber].EndColumn - 1, Window[WindowNumber].InsideColor);
  }


  /* Make this window active. */
  Window[WindowNumber].WinStatus = WINDOW_ACTIVE;
  if (DebugBitMask & DEBUG_WINDOW)
    uart_send(__LINE__, __func__, "Just opened %s (window number %u)\r", Window[WindowNumber].Name, WindowNumber);


  if (DebugBitMask & DEBUG_WINDOW) display_windows();

  return;
}





/* $TITLE=win_part_cls()) */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                          Clear the specified part of the specified window.
     NOTE: If StartRow equals 201 (which is greater than RGB matrix number of rows - 32),  first line of target window will be cleared.
           If StartRow equals 202 (which is greater than RGB matrix number of rows - 32), second line of target window will be cleared if there are two lines.
\* ============================================================================================================================================================= */
void win_part_cls(UINT8 WindowNumber, UINT8 StartRow, UINT8 EndRow)
{
  UCHAR String[31];

  UINT8 MatrixStartRow;
  UINT8 MatrixEndRow;
  UINT8 RowNumber;
  UINT8 StartColumn;  // dummy
  UINT8 EndColumn;    // dummy


  /* Initilalize dummy variables to prevent strange behavior. */
  StartColumn = 0;
  EndColumn   = 63;

  /// printf("Entering with StartRow: %2u   EndRow: %2u   MatrixStartRow: %2u   MatrixEndRow: %2u\r", StartRow, EndRow, MatrixStartRow, MatrixEndRow);

  if (StartRow > 200)
  {
    /* Validate provided coordinates. */
    RGB_matrix_check_coord(&StartRow, &StartColumn, &EndRow, &EndColumn);
    MatrixStartRow = StartRow;
    MatrixEndRow   = EndRow;
  }
  else
  {
    /* Initialize specific, non standard values. */
    MatrixStartRow = Window[WindowNumber].StartRow + StartRow;
    MatrixEndRow   = Window[WindowNumber].StartRow + EndRow;
  }

  /// printf("Coming back with StartRow: %2u   EndRow: %2u   MatrixStartRow: %2u   MatrixEndRow: %2u\r", StartRow, EndRow, MatrixStartRow, MatrixEndRow);


  /* Clear the window's specified area. */
  for (RowNumber = MatrixStartRow; RowNumber <= MatrixEndRow; ++RowNumber)
  {
    if (Window[WindowNumber].LastBoxState == ACTION_ERASE)
      FrameBuffer[RowNumber] = 0x00ll;  // this window has not persistent border.
    else
    {
      /* This window has a persistent border, redraw it while clearing the inside. */
      if ((RowNumber == Window[WindowNumber].StartRow ) || (RowNumber == Window[WindowNumber].EndRow))
        FrameBuffer[RowNumber] = 0xFFFFFFFFFFFFFFFFll;  // Top and Bottom rows
      else
        FrameBuffer[RowNumber] = 0x8000000000000001ll;  // intermediate rows.
    }
  }

  return;
}





/* $TITLE=win_printf() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                                Display text in the specified window.
\* ============================================================================================================================================================= */
UINT8 win_printf(UINT8 WindowNumber, UINT8 StartRow, UINT8 StartColumn, UINT8 FontType, UCHAR *Format, ...)
{
  UCHAR String[256];

  UINT8 MatrixStartRow;
  UINT8 MatrixStartColumn;
  UINT8 EndRow;
  UINT8 EndColumn;
  UINT8 NextColumn;

  va_list argp;


  /* Transfer the text to print to variable <String>. */
  va_start(argp, Format);
  vsnprintf(String, sizeof(String), Format, argp);
  va_end(argp);


  // uart_send(__LINE__, __func__, "On entry - StartRow: %2u   StartColumn: %2u   MatrixStartRow: %2u   MatrixStartColumn: %u\r", StartRow, StartColumn, MatrixStartRow, MatrixStartColumn);

  /* Fill-up dummy variables. */
  EndColumn = 63;
  EndRow    = 31;

  if (StartRow > 200)
  {
    /* Validate provided coordinates. */
    RGB_matrix_check_coord(&StartRow, &StartColumn, &EndRow, &EndColumn);
    MatrixStartRow    = StartRow;
    MatrixStartColumn = StartColumn;
  }
  else
  {
    /* Initialize specific, non standard values. */
    MatrixStartRow    = Window[WindowNumber].StartRow    + StartRow;
    MatrixStartColumn = Window[WindowNumber].StartColumn + StartColumn;
  }

  // uart_send(__LINE__, __func__, "Back from check - StartRow: %2u   StartColumn: %2u   MatrixStartRow: %2u   MatrixStartColumn: %u\r", StartRow, StartColumn, MatrixStartRow, MatrixStartColumn);

  NextColumn = RGB_matrix_printf(FrameBuffer, MatrixStartRow, MatrixStartColumn, FontType, String);

  return NextColumn;
}





/* ============================================================================================================================================================ *\
                                                  Scroll the text in the specified window, on the specified line.
                                              Return the number of the ScrollNumber structure that has been assigned.
\* ============================================================================================================================================================ */
UINT8 win_scroll(UINT8 WindowNumber, UINT8 StartRow, UINT8 EndRow, UINT16 ScrollTimes, UINT8 ScrollSpeed, UINT8 FontType, UCHAR *Format, ...)
{
  UCHAR ScrollString[1024];

  UINT8 FirstFree;
  UINT8 Loop1UInt8;
  UINT8 ScrollNumber;
  UINT8 StartColumn;
  UINT8 EndColumn;

  va_list argp;


  /* Check if there is already an active scroll for target window and target line, and also keep track of first free structure in case it is required below. */
  /* NOTE: If we append more text to a currently active scrolling, font type can't be change from what it was on the first call. */
  FirstFree    = MAX_ACTIVE_SCROLL;  // assign invalid value on entry.
  ScrollNumber = MAX_ACTIVE_SCROLL;  // assign invalid value on entry.

  for (Loop1UInt8 = 0; Loop1UInt8 < MAX_ACTIVE_SCROLL; ++Loop1UInt8)
  {
    if (ActiveScroll[Loop1UInt8] != 0x00l)
    {
      /* This scroll structure has been allocated, check who's the owner. */
      if (ActiveScroll[Loop1UInt8]->Owner == WindowNumber)
      {
        /* This scroll structure is already allocated to the target window. */
        ScrollNumber = Loop1UInt8;
        if (DebugBitMask & DEBUG_SCROLL)
          uart_send(__LINE__, __func__, "Scroll structure %u is already allocated to target window: %u %s\r", Loop1UInt8, ActiveScroll[Loop1UInt8]->Owner, Window[ActiveScroll[Loop1UInt8]->Owner].Name);
        break;
      }
    }
    else
    {
      /* This active scroll structure has not already been assigned.
         Keep track of first free active scroll structure in case no active scroll has already been assigned to the target window. */
      if (FirstFree == MAX_ACTIVE_SCROLL) FirstFree = Loop1UInt8;
    }
  }



  /* If no active scroll has been already assigned to this window, use first free active scroll structure found. */
  if (ScrollNumber == MAX_ACTIVE_SCROLL)
  {
    /* This is the first scroll request for this target window. */
    ScrollNumber = FirstFree;
    ActiveScroll[ScrollNumber] = (struct active_scroll *)calloc(1, sizeof(struct active_scroll));

    if (DebugBitMask & DEBUG_SCROLL)
    {
      uart_send(__LINE__, __func__, "After scanning active scroll structures, ScrollNumber: %u has been assigned to window %u (%s) for this scroll\r", ScrollNumber, WindowNumber, Window[WindowNumber].Name);
      uart_send(__LINE__, __func__, "Memory allocation returned pointer: 0x%p   size of active_scroll structure: %u (0x%4.4X)\r", ActiveScroll[ScrollNumber], sizeof(struct active_scroll), sizeof(struct active_scroll));
    }
  }


  /* Transfer the text to print to variable <ScrollString>. */
  va_start(argp, Format);
  vsnprintf(ScrollString, sizeof(ScrollString), Format, argp);
  va_end(argp);

  if (DebugBitMask & DEBUG_SCROLL)
  {
    uart_send(__LINE__, __func__, "Length of new string being added to scroll:     %3u\r", strlen(ScrollString));
    uart_send(__LINE__, __func__, "Length of currently scrolling string:           %3u (before adding new string)\r", strlen(ActiveScroll[ScrollNumber]->Message));
  }


  /* Validate provided coordinates. */
  StartColumn =  0;  // dummy
  EndColumn   = 63;  // dummy
  RGB_matrix_check_coord(&StartRow, &StartColumn, &EndRow, &EndColumn);


  ActiveScroll[ScrollNumber]->StartRow           = StartRow;
  ActiveScroll[ScrollNumber]->EndRow             = EndRow;
  ActiveScroll[ScrollNumber]->FontType           = FontType;
  ActiveScroll[ScrollNumber]->ScrollTimes        = ScrollTimes - 1;  // first scroll is automatic and not accounted for in the total.
  ActiveScroll[ScrollNumber]->ScrollSpeed        = ScrollSpeed;
  ActiveScroll[ScrollNumber]->PixelCountCurrent  = MAX_COLUMNS;      // number of pixels remaining to scroll on LED matrix.
  ActiveScroll[ScrollNumber]->PixelCountBuffer   = 0;                // number of pixels remaining to scroll in bitmap buffer.
  ActiveScroll[ScrollNumber]->AsciiBufferPointer = 0;                // pointer to next character to be scrolled in ASCII buffer.


  /* Add current text to be scrolled at the end of any eventual message currently scrolling (in ASCII scroll buffer). */
  strcat(ActiveScroll[ScrollNumber]->Message, ScrollString);

  /* Add spaces at the end of current text in case some more text is added later,
     or in case we asked for more than one cycle (to split apart repeat cycles and make it more readable). */
  strcat(ActiveScroll[ScrollNumber]->Message, "        ");

  if (DebugBitMask & DEBUG_SCROLL)
  {
    uart_send(__LINE__, __func__, "ActiveScroll[%u]->Message: (length: %u   including 8 trailing spaces)\r\r\r", ScrollNumber, strlen(ActiveScroll[ScrollNumber]->Message));
    /// uart_send(__LINE__, __func__, "ActiveScroll[%u]->Message: <%s>\r\r\r",     ScrollNumber, ActiveScroll[ScrollNumber]->Message);
  }

  /// display_scroll();  // should not be used in a callback context.

  if ((StartRow >= 0)  && (EndRow <= 8))  Window[WindowNumber].FlagTopScroll = FLAG_ON;
  if ((StartRow >= 9)  && (EndRow <= 17)) Window[WindowNumber].FlagMidScroll = FLAG_ON;
  if ((StartRow >= 18) && (EndRow <= 31)) Window[WindowNumber].FlagBotScroll = FLAG_ON;

  /* IMPORTANT: Owner assignation must be done at the end since this is what triggers RGB_matrix_scroll() and
                we want to make sure that all other parameters have already been properly setup before doing so.*/
  ActiveScroll[ScrollNumber]->Owner = WindowNumber;  // owner of this active scroll.

  return ScrollNumber;
}





/* $TITLE=win_scroll_cancel() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                      Cancel an eventual active scrolling in the specified window and add a few character spaces in case a new scrolling is coming.
\* ============================================================================================================================================================= */
void win_scroll_cancel(UINT8 WindowNumber, UINT8 StartRow, UINT8 EndRow)
{
  UINT8 Loop1UInt8;
  UINT8 ScrollNumber;
  UINT8 StartColumn;
  UINT8 EndColumn;


  /* Validate provided coordinates. */
  StartColumn =  0;  // dummy
  EndColumn   = 63;  // dummy
  RGB_matrix_check_coord(&StartRow, &StartColumn, &EndRow, &EndColumn);


  /* Check if there is really an active scroll for target window and line number. */
  ScrollNumber = MAX_ACTIVE_SCROLL;  // assign invalid value on entry.

  /* Scan all active scroll structures, looking for one allocated to the specified window. */
  for (Loop1UInt8 = 0; Loop1UInt8 < MAX_ACTIVE_SCROLL; ++Loop1UInt8)
  {
    if (ActiveScroll[Loop1UInt8] != 0x00l)  // check for a null pointer.
    {
      /* This scroll structure has been allocated, check who's the owner. */
      if (ActiveScroll[Loop1UInt8]->Owner == WindowNumber)
      {
        if (ActiveScroll[Loop1UInt8]->StartRow == StartRow)
        {
          /* This scroll structure is allocated to the target window. */
          ScrollNumber = Loop1UInt8;
          if (DebugBitMask & DEBUG_SCROLL)
            uart_send(__LINE__, __func__, "Scroll structure %u is allocated to target window: %u %s\r", Loop1UInt8, ActiveScroll[Loop1UInt8]->Owner, Window[ActiveScroll[Loop1UInt8]->Owner].Name);
          break;
        }
        else
        {
          /* Right window number, wrong line number. */
          continue;
        }
      }
    }
  }

  if (DebugBitMask & DEBUG_SCROLL)
    uart_send(__LINE__, __func__, "After scanning all ActiveScroll structures, Loop1UInt8 is %u (MAX_ACTIVE_SCROLL: %u)   ScrollNumber: %u\r", Loop1UInt8, MAX_ACTIVE_SCROLL, ScrollNumber);

  /* If there is no active scroll for this window... */
  if (Loop1UInt8 == MAX_ACTIVE_SCROLL) return;

  /* Overwrite current remaining string in the scroll buffer by a few spaces in preparation for an eventual new message to come. */
  sprintf(ActiveScroll[ScrollNumber]->Message, "    ");

  return;
}





/* $TITLE=win_scroll_off() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                           Cleanup when a scroll has bee completed.
\* ============================================================================================================================================================= */
void win_scroll_off(UINT8 ScrollNumber)
{
  /* Turn Off scroll flag when scrolling is completed. */
  if ((ActiveScroll[ScrollNumber]->StartRow >= 0)  && (ActiveScroll[ScrollNumber]->EndRow <= 8))  Window[ActiveScroll[ScrollNumber]->Owner].FlagTopScroll = FLAG_OFF;
  if ((ActiveScroll[ScrollNumber]->StartRow >= 9)  && (ActiveScroll[ScrollNumber]->EndRow <= 17)) Window[ActiveScroll[ScrollNumber]->Owner].FlagMidScroll = FLAG_OFF;
  if ((ActiveScroll[ScrollNumber]->StartRow >= 18) && (ActiveScroll[ScrollNumber]->EndRow <= 31)) Window[ActiveScroll[ScrollNumber]->Owner].FlagBotScroll = FLAG_OFF;

  if (ActiveScroll[ScrollNumber])
  {
    /* Release memory used for specified scroll number. */
    if (DebugBitMask & DEBUG_SCROLL) uart_send(__LINE__, __func__, "Releasing memory at 0x%p used for ScrollNumber: %u (%s)\r", ActiveScroll[ScrollNumber], ScrollNumber, Window[ActiveScroll[ScrollNumber]->Owner].Name);
    free(ActiveScroll[ScrollNumber]);
    ActiveScroll[ScrollNumber] = 0x00;  // assign a NULL pointer now that memory has been released.
  }
  else
  {
    if (DebugBitMask & DEBUG_SCROLL) uart_send(__LINE__, __func__, "Invalid (uninitialized) scroll number (%u) passed to win_scroll_off()\r", ScrollNumber);
  }

  return;
}





/* $TITLE=win_set_color() */
/* $PAGE */
/* ============================================================================================================================================================= *\
                                                               Set the colors for the specified windows.
\* ============================================================================================================================================================= */
void win_set_color(UINT8 WindowNumber, UINT8 InsideColor, UINT8 BoxColor)
{
  if ((WindowNumber != WinTop) && (WindowNumber != WinBot))
  {
    /* Specified window is not currently active, do not execute. */
    uart_send(__LINE__, __func__, "Specified window (%s) is not currently active, skip command...\r", Window[WindowNumber].Name);
    return;
  }

  /* Set color for the whole window (box and inside). */
  RGB_matrix_set_color(Window[WindowNumber].StartRow, Window[WindowNumber].StartColumn, Window[WindowNumber].EndRow, Window[WindowNumber].EndColumn, InsideColor);

  /* If box color must be different, set it now to override what we did above. */
  if (InsideColor != BoxColor)
  {
    /* Top row. */
    RGB_matrix_set_color(Window[WindowNumber].StartRow, Window[WindowNumber].StartColumn, Window[WindowNumber].StartRow, Window[WindowNumber].EndColumn, BoxColor);

    /* Bottom row. */
    RGB_matrix_set_color(Window[WindowNumber].EndRow, Window[WindowNumber].StartColumn, Window[WindowNumber].EndRow, Window[WindowNumber].EndColumn, BoxColor);

    /* Left column. */
    RGB_matrix_set_color(Window[WindowNumber].StartRow, Window[WindowNumber].StartColumn, Window[WindowNumber].EndRow, Window[WindowNumber].StartColumn, BoxColor);

    /* Right column. */
    RGB_matrix_set_color(Window[WindowNumber].StartRow, Window[WindowNumber].EndColumn, Window[WindowNumber].EndRow, Window[WindowNumber].EndColumn, BoxColor);
  }

  return;
}
