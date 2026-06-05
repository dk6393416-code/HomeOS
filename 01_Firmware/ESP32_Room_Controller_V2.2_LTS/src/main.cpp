#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <Preferences.h>
#include "esp_system.h"


// ================= HOMEOS =================

const char* DEVICE_ID = "HOMEOS_ROOM_001";
const char* DEVICE_NAME = "Bedroom Controller";
const char* VERSION = "2.2 LTS";


// ================= WIFI =================

const char* ssid = "ESPTEST";
const char* password = "12345678";


// ================= OBJECTS =================

WebServer server(80);
WiFiUDP udp;
Preferences prefs;


// ================= CONFIG =================

#define STATUS_LED 2

const int UDP_PORT = 4210;

char incomingPacket[255];

unsigned long lastWifiTry = 0;


// ================= HARDWARE =================

int relayPin[6] =
{
23,22,21,19,18,5
};


int switchPin[6] =
{
32,33,27,14,12,13
};


bool relayState[6];

unsigned long lastSwitchTime[6];

const int debounceDelay = 150;


// ================= RELAY =================


void applyRelayState(int i)
{
  digitalWrite(
    relayPin[i],
    relayState[i] ? HIGH : LOW
  );
}


// ================= MEMORY =================


void saveState()
{

for(int i=0;i<6;i++)
{
String key="r"+String(i);

prefs.putBool(
key.c_str(),
relayState[i]
);

}

}



void loadState()
{

for(int i=0;i<6;i++)
{

String key="r"+String(i);

relayState[i] =
prefs.getBool(
key.c_str(),
false
);

}

}


// ================= CONTROL =================


void relayON(int i)
{

relayState[i]=true;

applyRelayState(i);

saveState();

server.send(
200,
"text/plain",
"ON"
);

}



void relayOFF(int i)
{

relayState[i]=false;

applyRelayState(i);

saveState();

server.send(
200,
"text/plain",
"OFF"
);

}



// ================= API =================


void statusAPI()
{

String json="{";


for(int i=0;i<6;i++)
{

json+="\"relay";
json+=String(i+1);
json+="\":";

json+=relayState[i]
?
"true"
:
"false";


if(i<5)
json+=",";

}


json+="}";


server.send(
200,
"application/json",
json
);

}



void infoAPI()
{

String json="{";

json+="\"device\":\"HomeOS\",";

json+="\"version\":\"";
json+=VERSION;
json+="\",";


json+="\"id\":\"";
json+=DEVICE_ID;
json+="\",";


json+="\"name\":\"";
json+=DEVICE_NAME;
json+="\",";


json+="\"ip\":\"";
json+=WiFi.localIP().toString();
json+="\",";


json+="\"rssi\":";
json+=String(WiFi.RSSI());
json+=",";


json+="\"uptime_sec\":";
json+=String(millis()/1000);
json+=",";


json+="\"free_ram\":";
json+=String(ESP.getFreeHeap());


json+="}";


server.send(
200,
"application/json",
json
);

}



// ================= WIFI =================


void startWiFi()
{

Serial.println("Starting WiFi");


WiFi.persistent(false);

WiFi.mode(WIFI_STA);

WiFi.setSleep(false);


WiFi.begin(
ssid,
password
);

}



// ================= SETUP =================


void setup()
{

Serial.begin(115200);


Serial.println();
Serial.println("================");
Serial.println("HomeOS V2.2 LTS");
Serial.println("================");


Serial.print("Reset reason: ");
Serial.println(
esp_reset_reason()
);


pinMode(
STATUS_LED,
OUTPUT
);


// HARDWARE FIRST

prefs.begin(
"HomeOS",
false
);


loadState();


for(int i=0;i<6;i++)
{

pinMode(
relayPin[i],
OUTPUT
);


pinMode(
switchPin[i],
INPUT_PULLUP
);


applyRelayState(i);

}


// WIFI

startWiFi();



// HTTP


server.on(
"/",
[]()
{
server.send(
200,
"text/plain",
"HomeOS V2.2 Online"
);
}
);


server.on(
"/status",
statusAPI
);


server.on(
"/info",
infoAPI
);



server.on("/r1on",[]{relayON(0);});
server.on("/r1off",[]{relayOFF(0);});

server.on("/r2on",[]{relayON(1);});
server.on("/r2off",[]{relayOFF(1);});

server.on("/r3on",[]{relayON(2);});
server.on("/r3off",[]{relayOFF(2);});

server.on("/r4on",[]{relayON(3);});
server.on("/r4off",[]{relayOFF(3);});

server.on("/r5on",[]{relayON(4);});
server.on("/r5off",[]{relayOFF(4);});

server.on("/r6on",[]{relayON(5);});
server.on("/r6off",[]{relayOFF(5);});


server.begin();


udp.begin(
UDP_PORT
);


Serial.println("System Ready");

}



// ================= LOOP =================


void loop()
{


server.handleClient();


// MANUAL SWITCH


for(int i=0;i<6;i++)
{

if(
digitalRead(switchPin[i])
==
LOW
)
{

if(
millis()-lastSwitchTime[i]
>
debounceDelay
)
{

relayState[i]=!relayState[i];

applyRelayState(i);

saveState();

lastSwitchTime[i]=millis();

}

}

}


// WIFI MONITOR


if(
WiFi.status()
==
WL_CONNECTED
)
{

digitalWrite(
STATUS_LED,
HIGH
);


static bool shown=false;


if(!shown)
{

Serial.print("IP: ");

Serial.println(
WiFi.localIP()
);


MDNS.begin("homeos");


shown=true;

}

}


else
{


digitalWrite(
STATUS_LED,
millis()/500%2
);


if(
millis()-lastWifiTry
>
10000
)
{

Serial.println(
"WiFi reconnect..."
);


WiFi.reconnect();


lastWifiTry=millis();

}

}



// UDP DISCOVERY


int packetSize =
udp.parsePacket();


if(packetSize)
{

int len =
udp.read(
incomingPacket,
255
);


if(len>0)
incomingPacket[len]=0;



if(
String(incomingPacket)
==
"DISCOVER_HOMEOS"
)
{


String reply="{";

reply+="\"id\":\"";
reply+=DEVICE_ID;

reply+="\",\"ip\":\"";

reply+=
WiFi.localIP()
.toString();

reply+="\"}";


udp.beginPacket(
udp.remoteIP(),
udp.remotePort()
);

udp.print(reply);

udp.endPacket();

}

}

}
