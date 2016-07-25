
// Adapted from kissste's code and fix bugs

#include "ESP8266WiFi.h"
extern "C" {
#include "c_types.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "smartconfig.h"
#include "lwip/opt.h"
#include "lwip/err.h"
#include "lwip/dns.h"
//#include "user_config.h"
}

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
#define CHANNEL_HOP_INTERVAL   5000

//os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static os_timer_t channelHop_timer;

//static void loop(os_event_t *events);
static void promisc_cb(uint8 *buf, uint16 len);


struct RxControl {
    signed rssi:8;
    unsigned rate:4;
    unsigned is_group:1;
    unsigned:1;
    unsigned sig_mode:2;
    unsigned legacy_length:12;
    unsigned damatch0:1;
    unsigned damatch1:1;
    unsigned bssidmatch0:1;
    unsigned bssidmatch1:1;
    unsigned MCS:7;
    unsigned CWB:1;
    unsigned HT_length:16;
    unsigned Smoothing:1;
    unsigned Not_Sounding:1;
    unsigned:1;
    unsigned Aggregation:1;
    unsigned STBC:2;
    unsigned FEC_CODING:1;
    unsigned SGI:1;
    unsigned rxend_state:8;
    unsigned ampdu_cnt:8;
    unsigned channel:4;
    unsigned:12;
};

struct sniffer_buf {
  struct RxControl rx_ctrl; 
  uint8 buf[36 ]; // head of ieee80211 packet 
};


#define printmac(buf, i) Serial.print(buf[i+0],HEX); Serial.print(":"); Serial.print(buf[i+1],HEX); Serial.print(":"); Serial.print(buf[i+2],HEX); Serial.print(":"); Serial.print(buf[i+3],HEX); Serial.print(":"); Serial.print(":"); Serial.print(buf[i+4],HEX); Serial.print(":"); Serial.print(buf[i+5],HEX);

void channelHop(void *arg)
{
  // 1 - 13 channel hopping
  uint8 new_channel = wifi_get_channel() % 12 + 1;
  Serial.print("** hop to ");
  Serial.println(new_channel);
  wifi_set_channel(new_channel);
}

static void ICACHE_FLASH_ATTR
promisc_cb(uint8 *buf, uint16 len)
{
    struct sniffer_buf *sniffer = (struct sniffer_buf*) buf;
    Serial.print(sniffer->rx_ctrl.rssi,DEC);
    Serial.print(" :: ");
    printmac(sniffer->buf, 10); //source mac
    Serial.print(" -> ");
    printmac(sniffer->buf, 4); //destination mac
    Serial.println("");
}

//Init function 
void setup ()
{
    Serial.begin(115200);
    delayMicroseconds(100);

    Serial.println("*** Monitor mode test ***");
    
    Serial.print(" -> Promisc mode setup ... ");
    wifi_set_promiscuous_rx_cb(promisc_cb);
    wifi_promiscuous_enable(1);
    Serial.println("done.");

    Serial.print(" -> Timer setup ... ");
    os_timer_disarm(&channelHop_timer);
    os_timer_setfn(&channelHop_timer, (os_timer_func_t *) channelHop, NULL);
    os_timer_arm(&channelHop_timer, CHANNEL_HOP_INTERVAL, 1);
    Serial.println("done.\n");
    
    Serial.print(" -> Set opmode ... ");
    wifi_set_opmode( 0x1 );
    Serial.println("done.");

    //Start os task
    //system_os_task(loop, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);

    Serial.println(" -> Init finished!");
}

void loop() {
  delay(10);
}
