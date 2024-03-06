![image](https://github.com/astlouys/Pico-RGB-Matrix/assets/102733655/7ccc4ed1-cc92-4a48-820c-ed7c5804f344)

# Coming soon...
# Firmware for the Waveshare's Pico-RGB-Matrix device

PLEASE NOTE: This is a work-in-progress. I encourage you to report bugs, but be aware that there is no "quality-control team" behind the code development. So, be indulgent...

Firmware hilights:

1)	Add support for NTP (“Network Time Protocol”) to periodically resynchronize the RGB Matrix with an Internet time reference source if user installed a PicoW.
2)	Handling of the RGB Matrix framebuffer ported from 8-bits to 64-bits for easier, faster and optimized support.
3)	Add horizontal scrolling feature for text on the LED display.
4)	Provide customizable “auto-scroll” option to scroll information on RGB Matrix at adjustable time intervals. Information to be scrolled can be selected by user among a predefined choice (see details the User Guide). Five (5) independant auto-scrolls are available.
5)	Rename many variables and function names so that they are more representative.
6)	Keep only the Raspberry Pi Pico (or PicoW) source code (discard EPS32 microcontroller support).
7)	Discard Chinese language support.
8)	Add a 5x7 character set bitmap with variable width.
9)	Add a 8x10 character set bitmap. Many thanks to Pasquale D’Antini (“oldmaker” on GitHub) for his collaboration on this.
10)	Provide 9 different and independent alarms.
11)	Alarms can be configured for different (selectable) days-of-week, different number of “rings” at different intervals, different message scrolling on LED matrix when the alarm is ringing.
12)	RGB Matrix provides indicators to show what alarms are On and what alarms are Off.
13)	RGB Matrix provides indicators to show those days-of-week that currently have active alarms and those that don’t have any.
14)	Sounds for the active buzzer integrated in the Pico-RGB-Matrix are now handled by a callback for a much easier support, mostly independent from Firmware.
15)	Add a circular buffer to improve and optimize sound handling with the active buzzer integrated in the device.
16)	LED display brightness is now controlled by a PWM signal from the Pico to free up the microcontroller from this task. This allows for a very large range of brightness levels.
17)	Allow adjustment of the range of brightness desired by user (Low level / Hi level)
18)	Add a hysteresis to the automatic brightness control to prevent quick changes in the LED brightness because of a quick interference with the ambient light.
19)	Add support for the remote control unit provided with the Pico-RGB-Matrix.
20)	Pico's core 1 is now used to relief core 0 of some tasks.
21)	Add an elementary windowing system with “exploding windows” animation..
22)	Add a debugging engine and algorithm to easily turn On debugging for specific sections of code through an external terminal emulator program.
23)	Add many “RGB_matrix_xxx()” family of hi-level functions with an easier interface.
24)	Add “win_xxx()” family of hi-level windowing functions with an easier interface.
25)	Add a complete interface with an optional external terminal emulator. This allows to go deeper and in more details for some device options / features.
26)	Change the layout of the clock display.
27)	Currently support English and French languages.
28)	Add the infrastructure to easily add translation in other languages.
29)	Add support for 64 different “Calendar events” allowing the user to program birthdays and / or other specific dates. An alarm and a message will periodically ring / scroll during the target day when the date is reached.
30)	Add an option for an hourly chime.
31)	Add an option for a half-hour light chime.
32)	Provide a way to shut off the hourly chime (and half-hour light chime) during a configurable period of time so that there is no sound during the night.
33)	RGB Matrix can be configured to become a “night light” when the ambient light is dark.
34)	Provide ambient temperature display.
35)	Temperature unit adjustable in Celsius or Fahrenheit.
36)	Add a mechanism to allow for device customization for different usages / users / rooms.
37)	Allow the device to be set in “bootsel” mode by firmware to compensate for a problem due to a physical difference between Pico and PicoW (see section about uploading a new Firmware version in User Guide).
38)	Add a delay during the power-up sequence so that developers can put the Pico in “bootsel mode” while the LED matrix is blank (to prevent LED over-bright).
39)	Add a delay during the power-up sequence so that users have time to start a terminal emulator software to interact with the Pico-RGB-Matrix.
40)	Add a “device integrity check” function to test every LED of the RGB Matrix.
41)	Add a “Test zone” with many chunks of test code (for developers).
42)	Invert the logic of ambient light reading so that a higher value means “more ambient light”, to make it more intuitive.
43)	Add a compile time option to completely shut Off sound from the device.
44)	Allow time display in 12-hour or 24-hour format (to be completed).
45)	Allow automatic support of daylight saving time for most – if not all – countries in the world, along with different timezones (to be completed).
46)	Implement a watchdog mechanism to recover from a Firmware crash for users who want to modify / work on the Firmware.
47)	LED display provides indicators to show that the main system "forever loop" is up and running (still active).
48)	Functions have been implemented to support “human time”, “tm time” and "Unix time”, to better support DS3231, Network Time Protocol and other inherent functions.
49)	Optionally scroll the cumulative device “Up time” since last power-up.
50)	Provide a specific indicator on the RGB Matrix to let the user know if there is a problem with Wi-Fi and / or NTP support.
51)	Provide a “Golden Age” option to help senior people having problems in their day-to-day lifes.
52)	Allows for automatic detection of microcontroller used (Pico or PicoW), along with microcontroller “Unique ID” (“serial number”).
54)	Many other cleanup and optimizations from the original code by Waveshare.
