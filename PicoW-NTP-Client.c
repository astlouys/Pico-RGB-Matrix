/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* ============================================================================================================================================================= *\
   Adapted for Pico-RGB_Matrix
   St-Louys, Andre - January 2024
   astlouys@gmail.com
   Revision 09-MAR-2024
   Compiler: arm-none-eabi-gcc 7.3.1
   Version 2.00

   REVISION HISTORY:
   =================
               1.00 - Initial version from Raspberry Pi (Trading) Ltd.
               cmake -DPICO_BOARD=pico_w -DPICO_STDIO_USB=1 -DWIFI_SSID=<NetworkName> -DWIFI_PASSWORD=<Password>
   18-FEB-2024 2.00 - Adapted for Pico-RGB-Matrix.
\* ============================================================================================================================================================= */

/// #define RELEASE_VERSION
#ifdef RELEASE_VERSION
#warning ===============> NTP client built as RELEASE_VERSION.
#else   // RELEASE_VERSION
#define DEVELOPER_VERSION
#warning ===============> NTP client built as DEVELOPER_VERSION.
#endif  // RELEASE_VERSION



#include "debug.h"
#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "pico/stdlib.h"
#include "PicoW-NTP-Client.h"
#include <string.h>
#include <time.h>



/* ============================================================================================================================================================= *\
                                                                             Function prototypes.
\* ============================================================================================================================================================= */
/* Initialize the cyw43 on PicoW. */
void init_cyw43(UINT CountryCode);

/* Call back with a DNS result. */
static void ntp_dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg);

/* NTP request failed. */
static int64_t ntp_failed_handler(alarm_id_t id, void *user_data);

/* NTP data received. */
static void ntp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);

/* Make an NTP request. */
static void ntp_request(void);

/* Called with results of operation. */
static void ntp_result(int status, time_t *result);





/* ============================================================================================================================================================= *\
                                                                            Global variables.
\* ============================================================================================================================================================= */
struct ntp_data   NTPData;
struct ntp_struct NTPStruct;





/* $PAGE */
/* $TITLE=display_ntp_info() */
/* ============================================================================================================================================================= *\
                                                                    Display handy NTP-related information.
\* ============================================================================================================================================================= */
void display_ntp_info(void)
{
  INT64 AbsoluteTimeDiff;
  INT64 TimeValue;

  absolute_time_t AbsoluteTime;


  AbsoluteTime = get_absolute_time();

  uart_send(__LINE__, __func__, "=========================================================\r");

  uart_send(__LINE__, __func__, " NTPData:\r");
  uart_send(__LINE__, __func__, " --------\r");
  uart_send(__LINE__, __func__, "FlagNTPInit:                   0x%2.2X\r", NTPData.FlagNTPInit);
  uart_send(__LINE__, __func__, "FlagNTPResync:                 0x%2.2X\r", NTPData.FlagNTPResync);
  uart_send(__LINE__, __func__, "FlagNTPSuccess:                0x%2.2X\r", NTPData.FlagNTPSuccess);
  uart_send(__LINE__, __func__, "FlagNTPHistory:                0x%2.2X\r", NTPData.FlagNTPHistory);
  sleep_ms(80);  // prevent communication override.
  uart_send(__LINE__, __func__, "NTPErrors:             %12lu\r",           NTPData.NTPErrors);
  uart_send(__LINE__, __func__, "NTPPollCycles:         %12lu\r",           NTPData.NTPPollCycles);
  uart_send(__LINE__, __func__, "NTPReadCycles:         %12lu\r",           NTPData.NTPReadCycles);
  uart_send(__LINE__, __func__, "NTPLatency (usec):     %12lld\r",          NTPData.NTPLatency);
  uart_send(__LINE__, __func__, "NTPUpdateTime:         %12.12llu\r",       NTPData.NTPUpdateTime);
  uart_send(__LINE__, __func__, "NTPLag:                %12.12llu\r",       NTPData.NTPLag);
  uart_send(__LINE__, __func__, "UnixTime:              %12.12llu\r\r",     NTPData.UnixTime);
  sleep_ms(80);  // prevent communication override.


  uart_send(__LINE__, __func__, " NTPStruct:\r");
  uart_send(__LINE__, __func__, " ----------\r");
  uart_send(__LINE__, __func__, "NTPServerAddress:   %15s\r",               ip4addr_ntoa(&NTPStruct.NTPServerAddress));
  uart_send(__LINE__, __func__, "DNSRequestSent:                0x%2.2X\r", NTPStruct.DNSRequestSent);
  uart_send(__LINE__, __func__, "NTPResendAlarm:              %6u\r",       NTPStruct.NTPResendAlarm);
  uart_send(__LINE__, __func__, "\r");
  sleep_ms(80);  // prevent communication override.


  uart_send(__LINE__, __func__, " Miscellaneous:\r");
  uart_send(__LINE__, __func__, " --------------\r");
  uart_send(__LINE__, __func__, "AbsoluteTime:          %12llu\r\r", time_us_64() / 1000000ll);
  sleep_ms(80);  // prevent communication override.


  TimeValue = (absolute_time_diff_us(AbsoluteTime, NTPData.NTPUpdateTime) / 1000000ll);
  uart_send(__LINE__, __func__, "NTPUpdateTime (sec):   %12llu\r",   to_us_since_boot(NTPData.NTPUpdateTime) / 1000000ll);
  if (TimeValue < 0)
    uart_send(__LINE__, __func__, "Time over by (sec):    %12lld\r", TimeValue);
  else
    uart_send(__LINE__, __func__, "Time remaining (sec):  %12lld\r", TimeValue);
  uart_send(__LINE__, __func__, "NTPRefresh:            %12u\r\r",   NTPData.NTPRefresh);
  sleep_ms(80);  // prevent communication override.


  TimeValue = (absolute_time_diff_us(AbsoluteTime, NTPData.NTPLag) / 1000000ll);
  uart_send(__LINE__, __func__, "NTPLag (sec):          %12llu\r",   to_us_since_boot(NTPData.NTPLag) / 1000000ll);
  if (TimeValue < 0)
    uart_send(__LINE__, __func__, "Time over by (sec):    %12lld\r", TimeValue);
  else
    uart_send(__LINE__, __func__, "Time remaining (sec):  %12lld\r", TimeValue);
  uart_send(__LINE__, __func__, "NTPLagTime:            %12lu\r",    NTPData.NTPLagTime);
  uart_send(__LINE__, __func__, "=========================================================\r\r");
  sleep_ms(80);  // prevent communication override.

  return;
}



/* $PAGE */
/* $TITLE=init_cyw43() */
/* ============================================================================================================================================================= *\
                                                                  Initialize the cyw43 on PicoW.
\* ============================================================================================================================================================= */
void init_cyw43(UINT CountryCode)
{
#ifdef RELEASE_VERSION
  UINT8 FlagLocalDebug = FLAG_OFF;  // must remain OFF all time.
#else   // RELEASE_VERSION
  UINT8 FlagLocalDebug = FLAG_OFF;  // may be modify for debug purposes.
#endif  // RELEASE_VERSION


  if (FlagLocalDebug) uart_send(__LINE__, __func__, "Entering init_cyw43()\r");

#ifdef RELEASE_VERSION
  /// if (cyw43_arch_init() == 0)
  if (cyw43_arch_init_with_country(CYW43_COUNTRY_WORLDWIDE) == 0)
#else  // RELEASE_VERSION
  if (cyw43_arch_init_with_country(CYW43_COUNTRY_CANADA) == 0)
#endif  // RELEASE_VERSION
  {
    if (FlagLocalDebug) uart_send(__LINE__, __func__, "cyw43 initialized without error.\r");
  }
  else
  {
    if (FlagLocalDebug) uart_send(__LINE__, __func__, "Error while trying to initialize cyw43.\r");
  }
}





/* $PAGE */
/* $TITLE=ntp_dns_found() */
/* ============================================================================================================================================================= *\
                                                                   Call back with a DNS result.
\* ============================================================================================================================================================= */
static void ntp_dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg)
{
#ifdef RELEASE_VERSION
  UINT8 FlagLocalDebug = FLAG_OFF;  // must remain OFF all time.
#else   // RELEASE_VERSION
  UINT8 FlagLocalDebug = FLAG_OFF;  // may be modify for debug purposes.
#endif  // RELEASE_VERSION

  /// UCHAR IpAddress[INET_ADDRSTRLEN];


  if (FlagLocalDebug) uart_send(__LINE__, __func__, "Entering ntp_dns_found()\r");

  if (ipaddr)
  {
    NTPStruct.NTPServerAddress = *ipaddr;
    if (FlagLocalDebug) uart_send(__LINE__, __func__, "NTP server address:    %15s\r", ip4addr_ntoa(ipaddr));
    if (FlagLocalDebug) uart_send(__LINE__, __func__, "Printing IP address from pointer: %lX\r", ipaddr);
    ntp_request();
  }
  else
  {
    if (FlagLocalDebug) uart_send(__LINE__, __func__, "NTP DNS request failed.\r");
    ntp_result(-1, NULL);
  }

  return;
}





/* $PAGE */
/* $TITLE=ntp_failed_handler() */
/* ============================================================================================================================================================= *\
                                                                           NTP request failed.
\* ============================================================================================================================================================= */
static int64_t ntp_failed_handler(alarm_id_t id, void *user_data)
{
#ifdef RELEASE_VERSION
  UINT8 FlagLocalDebug = FLAG_OFF;  // must remain OFF all time.
#else   // RELEASE_VERSION
  UINT8 FlagLocalDebug = FLAG_OFF;  // may be modify for debug purposes.
#endif  // RELEASE_VERSION


  if (FlagLocalDebug)
  {
    uart_send(__LINE__, __func__, "Entering ntp_failed_handler()\r");
    uart_send(__LINE__, __func__, "NTP request failed.\n");
  }

  ntp_result(-1, NULL);

  return 0;
}





/* $PAGE */
/* $TITLE=ntp_get_time() */
/* ============================================================================================================================================================= *\
                                                               Retrieve current UTC time from NTP server.
\* ============================================================================================================================================================= */
void ntp_get_time(void)
{
#ifdef RELEASE_VERSION
  UINT8 FlagLocalDebug = FLAG_OFF;  // must remain OFF all time.
#else   // RELEASE_VERSION
  UINT8 FlagLocalDebug = FLAG_OFF;  // may be modified for debug purposes.
#endif  // RELEASE_VERSION

  INT ReturnCode;

  INT64 AbsoluteTimeDiff;
  INT64 DeltaTime;

  absolute_time_t AbsoluteTime;


  AbsoluteTime = get_absolute_time();

  if (FlagLocalDebug)
  {
    uart_send(__LINE__, __func__, "=========================================================\r");
    uart_send(__LINE__, __func__, "                 Entering ntp_get_time()\r");
    display_ntp_info();
  }


  if ((!NTPData.FlagNTPResync) && (is_nil_time(NTPData.NTPLag) == false) && ((absolute_time_diff_us(AbsoluteTime, NTPData.NTPLag) / 1000000ll) > 0ll))
  {
    if (FlagLocalDebug)
    {
      uart_send(__LINE__, __func__, "=========================================================\r");
      uart_send(__LINE__, __func__, "                       Poll cycle\r");
      uart_send(__LINE__, __func__, "=========================================================\r");
      /// display_ntp_info();
    }

    NTPData.FlagNTPSuccess = FLAG_POLL;
    NTPData.NTPPollCycles++;
    NTPData.NTPUpdateTime = make_timeout_time_ms(NTPData.NTPRefresh * 1000);

    return;
  }


  if (FlagLocalDebug)
  {
    uart_send(__LINE__, __func__, "=========================================================\r");
    uart_send(__LINE__, __func__, "                      Read cycle\r");
    uart_send(__LINE__, __func__, "=========================================================\r");
    /// display_ntp_info();
  }

  NTPData.NTPUpdateTime = make_timeout_time_ms(NTPData.NTPRefresh * 1000);
  NTPData.NTPLag        = make_timeout_time_ms(NTPData.NTPLagTime * 1000);
  NTPData.NTPReadCycles++;

  /* Set alarm in case udp requests are lost (10 seconds). */
  NTPStruct.NTPResendAlarm = add_alarm_in_ms(NTP_RESEND_TIME, ntp_failed_handler, &NTPStruct, true);

  /* NOTE: cyw43_arch_lwip_begin() / cyw43_arch_lwip_end() should be used around calls into LwIP to ensure correct locking.
           You can omit them if you are in a callback from LwIP. Note that when using pico_cyw_arch_poll library these calls
           are a no-op and can be omitted, but it is a good practice to use them in case you switch the cyw43_arch type later. */
  cyw43_arch_lwip_begin();
  {
    ReturnCode = dns_gethostbyname(NTP_SERVER, &NTPStruct.NTPServerAddress, ntp_dns_found, &NTPStruct);
  }
  cyw43_arch_lwip_end();


  NTPStruct.DNSRequestSent = true;
  if (FlagLocalDebug) uart_send(__LINE__, __func__, "Sent a request to DNS server to get a NTP server IP address (return code: %d)\r", ReturnCode);


  if (ReturnCode == 0)
  {
    if (FlagLocalDebug) uart_send(__LINE__, __func__, "Cache DNS response.\r");
    ntp_request();  // cached result.
  }
  else if (ReturnCode != ERR_INPROGRESS)
  {
    /* ERR_INPROGRESS means expect a callback. */
    if (FlagLocalDebug) uart_send(__LINE__, __func__, "DNS request failed.\r");
    ntp_result(-1, NULL);
  }
  else
  {
    /* Other error code. */
    if (FlagLocalDebug) uart_send(__LINE__, __func__, "DNS request return code: %d  (-5 is Ok)\r", ReturnCode);
  }

  return;
}





/* $PAGE */
/* $TITLE=ntp_init() */
/* ============================================================================================================================================================= *\
                                                                   Initialize Wi-Fi connection.
\* ============================================================================================================================================================= */
int ntp_init(UCHAR *SSID, UCHAR *Password)
{
#ifdef RELEASE_VERSION
  UINT8 FlagLocalDebug = FLAG_OFF;  // must remain OFF all time.
#else   // RELEASE_VERSION
  UINT8 FlagLocalDebug = FLAG_OFF;  // may be modify for debug purposes.
#endif  // RELEASE_VERSION

  UINT8 Loop1UInt8;
  UINT8 RetryCount;

  int ReturnCode;


  /* Initializations. */
  RetryCount = 0;
  ReturnCode = 0;  // assume no error on entry.
  NTPData.NTPLagTime = NTP_LAG;


  /* Enable Wi-Fi Station mode. */
  if (FlagLocalDebug)
  {
    uart_send(__LINE__, __func__, "Trying to establish Wi-Fi connection with these credentials:\r");
    uart_send(__LINE__, __func__, "SSID:     [%s]\r", SSID);
    uart_send(__LINE__, __func__, "Password: [%s]\r", Password);
    /// uart_send(__LINE__, __func__, "Data below is displayed by CYW43:\r");
  }
  printf("===================================================================================================================\r");
  cyw43_arch_enable_sta_mode();  // initialize Wi-Fi as a client.
  printf("===================================================================================================================\r\r\r");
  sleep_ms(250);

  /* The time-out below may be increased or reduced, depending on your Wi-Fi answering speed. */
  ReturnCode = cyw43_arch_wifi_connect_timeout_ms(SSID, Password, CYW43_AUTH_WPA2_AES_PSK, 5000);
  if (ReturnCode != 0)
  {
    /* While connection is not successful, blink PicoW's LED a number of times corresponding to the current retry count and wait until time-out. */
    while (cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) != CYW43_LINK_UP)
    {
      ++RetryCount;

      if (RetryCount < MAX_NETWORK_RETRIES)
        uart_send(__LINE__, __func__, "Wi-Fi connection failure    Retry count: %2u / %u   (retrying - return code: %d)\r", RetryCount, MAX_NETWORK_RETRIES, ReturnCode);
      else
        uart_send(__LINE__, __func__, "Wi-Fi connection failure    Retry count: %2u / %u   (aborting)\r", RetryCount, MAX_NETWORK_RETRIES);

      if (RetryCount >= MAX_NETWORK_RETRIES) break;  // time-out.


      /* Blink PicoW's LED "current retry count". */
      for (Loop1UInt8 = 0; Loop1UInt8 < RetryCount; ++Loop1UInt8)
      {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(200);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(300);
      }

      /* No connection yet, wait and try again. */
      sleep_ms(1000);
    }

    /* If we went out of "while" loop after MAX_NETWORK_RETRIES connection failures, fast blink PicoW's LED many times to indicate Wi-Fi connection error. */
    if (RetryCount >= MAX_NETWORK_RETRIES)
    {
      /* In case of error, fast-blink Pico's LED many times to indicate Wi-Fi connection error. */
      if (FlagLocalDebug) uart_send(__LINE__, __func__, "Failed to establish a Wi-Fi connection.\r\r");
      for (Loop1UInt8 = 0; Loop1UInt8 < 30; ++Loop1UInt8)
      {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(100);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(100);
      }
      NTPData.FlagNTPInit   = FLAG_OFF;
      NTPData.NTPUpdateTime = make_timeout_time_ms(NTPData.NTPLagTime * 1000);
      return false;
    }
  }


  /* Wi-Fi connection successful. */
  uart_send(__LINE__, __func__, "Wi-Fi connection succeeded (after %u retries).\r", RetryCount);


  /* Fast blink Pico's LED 5 times to indicate Wi-Fi successful connection. */
  for (Loop1UInt8 = 0; Loop1UInt8 < 5; ++Loop1UInt8)
  {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(100);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    sleep_ms(100);
  }


  NTPStruct.NTPPcb = udp_new_ip_type(IPADDR_TYPE_ANY);
  if (!NTPStruct.NTPPcb)
  {
    if (FlagLocalDebug) uart_send(__LINE__, __func__, "Failed to create pcb.\r");
    NTPData.FlagNTPInit = FLAG_OFF;
    return false;
  }


  NTPData.FlagNTPInit = FLAG_ON;  // ntp_init() was successful.
  udp_recv(NTPStruct.NTPPcb, ntp_recv, &NTPStruct);

  return true;
}





/* $PAGE */
/* $TITLE=ntp_recv() */
/* ============================================================================================================================================================= *\
                                                                         NTP data received.
\* ============================================================================================================================================================= */
static void ntp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
#ifdef RELEASE_VERSION
  UINT8 FlagLocalDebug = FLAG_OFF;  // must remain OFF all time.
#else   // RELEASE_VERSION
  UINT8 FlagLocalDebug = FLAG_OFF;  // may be modify for debug purposes.
#endif  // RELEASE_VERSION

  UINT8 SecondBuffer[4] = {0};

  UINT64 SecondsSince1900;
  UINT64 SecondsSince1970;

  time_t UnixTime;


  NTPData.NTPReceive = get_absolute_time();

  if (FlagLocalDebug) uart_send(__LINE__, __func__, "Entering ntp_recv()\r");

  /* Initializations. */
  SecondsSince1900 = 0ll;
  SecondsSince1970 = 0ll;


  UINT8 mode     = pbuf_get_at(p, 0) & 0x7;
  UINT8 stratum  = pbuf_get_at(p, 1);


  /* Check the result. */
  if (ip_addr_cmp(addr, &NTPStruct.NTPServerAddress) && (port == NTP_PORT) && (p->tot_len == NTP_MSG_LEN) && (mode == 0x4) && (stratum != 0))
  {
    pbuf_copy_partial(p, SecondBuffer, sizeof(SecondBuffer), 40);
    NTPData.NTPLatency = (absolute_time_diff_us(NTPData.NTPSend, NTPData.NTPReceive) / 2);
    SecondsSince1900   = ((UINT64)SecondBuffer[0] << 24) | ((UINT64)SecondBuffer[1] << 16) | ((UINT64)SecondBuffer[2] << 8) | ((UINT64)SecondBuffer[3]);
    SecondsSince1970   = SecondsSince1900 - NTP_DELTA;
    UnixTime           = SecondsSince1970;

    if (FlagLocalDebug)
    {
      uart_send(__LINE__, __func__, "Stratum:                             %u\r", stratum);
      uart_send(__LINE__, __func__, "NTPSend:                    %10llu\r", NTPData.NTPSend);
      uart_send(__LINE__, __func__, "NTPReceive:                 %10llu\r", NTPData.NTPReceive);
      uart_send(__LINE__, __func__, "NTPLatency (usec):          %10lld\r", NTPData.NTPLatency);
      uart_send(__LINE__, __func__, "NTP_DELTA:                  %10lu\r",  (UINT32)NTP_DELTA);
      uart_send(__LINE__, __func__, "SecondsSince1970:         %12llu\r\r", SecondsSince1970);
      /// uart_send(__LINE__, __func__, "SecondsSince1900:         %12llu\r", SecondsSince1900);
    }

    ntp_result(0, &UnixTime);
  }
  else
  {
    if (FlagLocalDebug) uart_send(__LINE__, __func__, "Invalid ntp response\r");
    ntp_result(-1, NULL);
  }

  pbuf_free(p);

  return;
}




/* $PAGE */
/* $TITLE=ntp_request() */
/* ============================================================================================================================================================= *\
                                                                         Make an NTP request.
\* ============================================================================================================================================================= */
/// static void ntp_request(NTP_T *NTPStruct)
static void ntp_request(void)
{
#ifdef RELEASE_VERSION
  UINT8 FlagLocalDebug = FLAG_OFF;  // must remain OFF all time.
#else   // RELEASE_VERSION
  UINT8 FlagLocalDebug = FLAG_OFF;  // may be modify for debug purposes.
#endif  // RELEASE_VERSION


  if (FlagLocalDebug) uart_send(__LINE__, __func__, "Entering ntp_request()\r");


  /* NOTE: cyw43_arch_lwip_begin() / cyw43_arch_lwip_end() should be used around calls into LwIP to ensure correct locking.
           You can omit them if you are in a callback from LwIP. Note that when using pico_cyw_arch_poll library these calls
           are a no-op and can be omitted, but it is a good practice to use them in case you switch the cyw43_arch type later. */
  cyw43_arch_lwip_begin();
  {
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, NTP_MSG_LEN, PBUF_RAM);
    uint8_t *req = (uint8_t *)p->payload;
    memset(req, 0, NTP_MSG_LEN);
    req[0] = 0x1B;
    udp_sendto(NTPStruct.NTPPcb, p, &NTPStruct.NTPServerAddress, NTP_PORT);
    NTPData.NTPSend = get_absolute_time();
    pbuf_free(p);
  }
  cyw43_arch_lwip_end();

  return;
}





/* $PAGE */
/* $TITLE=ntp_result() */
/* ============================================================================================================================================================= *\
                                                                  Called with results of operation.
\* ============================================================================================================================================================= */
/// static void ntp_result(NTP_T* NTPStruct, int status, time_t *UnixTime)
static void ntp_result(int status, time_t *UnixTime)
{
#ifdef RELEASE_VERSION
  UINT8 FlagLocalDebug = FLAG_OFF;  // must remain OFF all time.
#else   // RELEASE_VERSION
  UINT8 FlagLocalDebug = FLAG_OFF;  // may be modify for debug purposes.
#endif  // RELEASE_VERSION


  if (FlagLocalDebug) uart_send(__LINE__, __func__, "Entering ntp_result()\r");

  if ((status == 0) && UnixTime)
  {
    if (FlagLocalDebug) uart_send(__LINE__, __func__, "UnixTime:                 %12llu\r", *UnixTime);
    NTPData.UnixTime       = *UnixTime;
    NTPData.FlagNTPSuccess = FLAG_ON;
  }
  else
  {
    NTPData.FlagNTPSuccess = FLAG_OFF;
    NTPData.FlagNTPHistory = FLAG_OFF;
  }

  if (NTPStruct.NTPResendAlarm > 0)
  {
    if (FlagLocalDebug) uart_send(__LINE__, __func__, "Cancelling alarm\r");
    cancel_alarm(NTPStruct.NTPResendAlarm);
    NTPStruct.NTPResendAlarm = 0;
  }

  if (FlagLocalDebug) uart_send(__LINE__, __func__, "Resetting DNSRequestSent\r");
  NTPStruct.DNSRequestSent = false;

  return;
}
