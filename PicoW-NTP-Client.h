/*
	picow_ntp_client.h

	Copyright (c) 2022 olav andrade; all rights reserved.

 */

/* ============================================================================================================================================================= *\
   Adapted for Pico-RGB_Matrix
   St-Louys, Andre - January 2024
   astlouys@gmail.com
   Revision 18-FEB-2024
   Compiler: arm-none-eabi-gcc 7.3.1
   Version 2.00

   REVISION HISTORY:
   =================
               1.00 - Initial version from Raspberry Pi (Trading) Ltd.
               cmake -DPICO_BOARD=pico_w -DPICO_STDIO_USB=1 -DWIFI_SSID=<NetworkName> -DWIFI_PASSWORD=<Password>
   18-FEB-2024 2.00 - Adapted for Pico-RGB-Matrix.
\* ============================================================================================================================================================= */

#ifndef _NTP_CLIENT_
#define _NTP_CLIENT_

#include "pico/cyw43_arch.h"
#include "time.h"


typedef int           INT;
typedef int64_t       INT64;
typedef unsigned int  UINT;   // processor-optimized.
typedef uint8_t       UINT8;
typedef uint16_t      UINT16;
typedef uint32_t      UINT32;
typedef uint64_t      UINT64;
typedef unsigned char UCHAR;



#define FLAG_OFF                0x00
#define FLAG_ON                 0x01
#define FLAG_POLL               0x02

#define MAX_NETWORK_RETRIES       20
#define MAX_NTP_CHECKS            20

#define NTP_DELTA         2208988800   // number of seconds between 01-JAN-1900 and 01-JAN-1970.
#define NTP_LAG                86400   // 86400
#define NTP_MSG_LEN               48
#define NTP_PORT                 123
#define NTP_REFRESH              240
#define NTP_RESEND_TIME   (10 * 1000)
#define NTP_SERVER     "pool.ntp.org"
#define NTP_TEST_TIME     (60 * 1000)



struct ntp_data
{
  UINT8  FlagNTPInit;     // flag indicating if NTP initialization has been done with success.
  UINT8  FlagNTPResync;   // flag set to On if there is a specific reason to request an NTP update without delay.
  UINT8  FlagNTPSuccess;  // flag indicating that NTP date and time request has succeeded.
  UINT8  FlagNTPHistory;
  UINT16 NTPRefresh;
  UINT32 NTPLagTime;
	UINT32 NTPErrors;       // cumulative number of errors while trying to re-sync with NTP.
  UINT32 NTPPollCycles;
	UINT32 NTPReadCycles;
  INT64  NTPLatency;
  absolute_time_t NTPUpdateTime;
  absolute_time_t NTPLag;
  absolute_time_t NTPSend;
  absolute_time_t NTPReceive;
  time_t UnixTime;
};



struct ntp_struct
{
  ip_addr_t        NTPServerAddress;
  bool             DNSRequestSent;
  struct udp_pcb  *NTPPcb;
  alarm_id_t       NTPResendAlarm;
};





/* Display NTP-related information. */
void display_ntp_info(void);

/* Initialize the cyw43 on Pico W. */
void init_cyw43(unsigned int CountryCode);

/* Retrieve current utc time from NTP server. */
void ntp_get_time(void);

/* Initialize NTP connection. */
int ntp_init(UCHAR *SSID, UCHAR *Password);

/* Send a string to external monitor through Pico UART (or USB CDC). */
extern void uart_send(UINT LineNumber, const UCHAR *FunctionName, UCHAR *Format, ...);

#endif  // _NTP_CLIENT_