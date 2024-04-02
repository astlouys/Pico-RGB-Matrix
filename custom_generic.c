/* ============================================================================================================================================================= *\
                                                                   General configuration.
\* ============================================================================================================================================================= */
FlashConfig1.AutoScroll[0].Period            = 15;  // 15-minute auto scroll.
FlashConfig1.AutoScroll[0].FunctionNumber[0] =  9;  // Firmware Version.
FlashConfig1.AutoScroll[0].FunctionNumber[1] = 12;  // brightness info.
FlashConfig1.AutoScroll[0].FunctionNumber[2] = 11;  // temperature.
FlashConfig1.AutoScroll[0].FunctionNumber[3] = 10;  // microcontroller.



/* ============================================================================================================================================================= *\
                                                                   Customize calendar events.
\* ============================================================================================================================================================= */

Loop1UInt16 = 0;

FlashConfig1.Event[Loop1UInt16].Day    = 1;
FlashConfig1.Event[Loop1UInt16].Month  = 1;
FlashConfig1.Event[Loop1UInt16].Jingle = 0;
sprintf(FlashConfig1.Event[Loop1UInt16].Message, "   Wish you a Happy New Year !!!");

++Loop1UInt16;

FlashConfig1.Event[Loop1UInt16].Day    = 1;
FlashConfig1.Event[Loop1UInt16].Month  = 2;
FlashConfig1.Event[Loop1UInt16].Jingle = 0;
sprintf(FlashConfig1.Event[Loop1UInt16].Message, "   Today is February 1st !");

++Loop1UInt16;

FlashConfig1.Event[Loop1UInt16].Day    = 14;
FlashConfig1.Event[Loop1UInt16].Month  = 2;
FlashConfig1.Event[Loop1UInt16].Jingle = 0;
sprintf(FlashConfig1.Event[Loop1UInt16].Message, "   Wish you a Happy Valentine Day !");

++Loop1UInt16;

FlashConfig1.Event[Loop1UInt16].Day    = 1;
FlashConfig1.Event[Loop1UInt16].Month  = 3;
FlashConfig1.Event[Loop1UInt16].Jingle = 0;
sprintf(FlashConfig1.Event[Loop1UInt16].Message, "   Today is March 1st !");

++Loop1UInt16;

FlashConfig1.Event[Loop1UInt16].Day    = 1;
FlashConfig1.Event[Loop1UInt16].Month  = 4;
FlashConfig1.Event[Loop1UInt16].Jingle = 0;
sprintf(FlashConfig1.Event[Loop1UInt16].Message, "   Today is April 1st !");

++Loop1UInt16;

FlashConfig1.Event[Loop1UInt16].Day    = 1;
FlashConfig1.Event[Loop1UInt16].Month  = 5;
FlashConfig1.Event[Loop1UInt16].Jingle = 0;
sprintf(FlashConfig1.Event[Loop1UInt16].Message, "   Today is May 1st !");

++Loop1UInt16;

FlashConfig1.Event[Loop1UInt16].Day    = 1;
FlashConfig1.Event[Loop1UInt16].Month  = 6;
FlashConfig1.Event[Loop1UInt16].Jingle = 0;
sprintf(FlashConfig1.Event[Loop1UInt16].Message, "   Today is June 1st !");

++Loop1UInt16;

FlashConfig1.Event[Loop1UInt16].Day    = 1;
FlashConfig1.Event[Loop1UInt16].Month  = 7;
FlashConfig1.Event[Loop1UInt16].Jingle = 0;
sprintf(FlashConfig1.Event[Loop1UInt16].Message, "   Today is July 1st !");

++Loop1UInt16;

FlashConfig1.Event[Loop1UInt16].Day    = 1;
FlashConfig1.Event[Loop1UInt16].Month  = 8;
FlashConfig1.Event[Loop1UInt16].Jingle = 0;
sprintf(FlashConfig1.Event[Loop1UInt16].Message, "   Today is August 1st !");

++Loop1UInt16;

FlashConfig1.Event[Loop1UInt16].Day    = 1;
FlashConfig1.Event[Loop1UInt16].Month  = 9;
FlashConfig1.Event[Loop1UInt16].Jingle = 0;
sprintf(FlashConfig1.Event[Loop1UInt16].Message, "   Today is September 1st !");

++Loop1UInt16;

FlashConfig1.Event[Loop1UInt16].Day    = 1;
FlashConfig1.Event[Loop1UInt16].Month  = 10;
FlashConfig1.Event[Loop1UInt16].Jingle = 0;
sprintf(FlashConfig1.Event[Loop1UInt16].Message, "   Today is October 1st !");

++Loop1UInt16;

FlashConfig1.Event[Loop1UInt16].Day    = 1;
FlashConfig1.Event[Loop1UInt16].Month  = 11;
FlashConfig1.Event[Loop1UInt16].Jingle = 0;
sprintf(FlashConfig1.Event[Loop1UInt16].Message, "   Today is November 1st !");

++Loop1UInt16;

FlashConfig1.Event[Loop1UInt16].Day    = 1;
FlashConfig1.Event[Loop1UInt16].Month  = 12;
FlashConfig1.Event[Loop1UInt16].Jingle = 0;
sprintf(FlashConfig1.Event[Loop1UInt16].Message, "   Today is December 1st !");

++Loop1UInt16;

FlashConfig1.Event[Loop1UInt16].Day    = 25;
FlashConfig1.Event[Loop1UInt16].Month  = 12;
FlashConfig1.Event[Loop1UInt16].Jingle = 0;
sprintf(FlashConfig1.Event[Loop1UInt16].Message, "   Wish you a Merry Christmas !!!");
