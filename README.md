# ST_Anything_NodeMCU
ST_Anything on NodeMCU

Forked from https://github.com/DanielOgorchock/ST_Anything/blob/master/README.md 
Added widre support for RF433 devices using the new rc-switch fork: https://github.com/perivar/rc-switch

Note
====
This project requires a secrets.h file you have to add:
```
#define WIFI_SSID "ADD YOUR OWN HERE"
#define WIFI_PASSWORD "ADD PASSWORD HERE"
#define DEVICE_IP_ADDRESS { 192, 168, 0, 200 }
#define ROUTER_GATEWAY { 192, 168, 0, 1 }
#define LAN_SUBNET { 255, 255, 255, 0 }
#define DNS_SERVER { 192, 168, 0, 1 }
#define SERVER_PORT 8090
#define HUB_IP_ADDRESS { 192, 168, 0, 50 }
#define HUB_PORT 39500
```

