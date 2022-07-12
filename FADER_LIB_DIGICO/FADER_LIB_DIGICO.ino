// FADER TRIM SETTINGS
#define TOP 960
#define BOT 70
int faderTrimTop[8] = {TOP, TOP, TOP, TOP, TOP, TOP, TOP, TOP}; // ADJUST THIS IF A SINGLE FADER ISN'T READING 255 AT THE TOP OF ITS TRAVEL
int faderTrimBottom[8] = {BOT, BOT, BOT, BOT, BOT, BOT, BOT, BOT}; // ADJUST THIS IF A SINGLE FADER ISN'T READING 0 AT THE BOTTOM OF ITS TRAVEL

// MOTOR SETTINGS
#define MOTOR_MAX_SPEED 210

// Default MIN_SPEED for Main Board version 1.0-1.2 = 170
// Default MIN_SPEED for Main Board version 1.3 = 190
// Default MIN_SPEED for Main Board version 1.4 = 145
#define MOTOR_MIN_SPEED 145

// Default MOTOR_FREQUENCY for 1.0-1.3 = 18000
// Default MOTOR_FREQUENCY for 1.4+ = 256
#define MOTOR_FREQUENCY 256

#define TOUCH_THRESHOLD 20

// ETHERNET SETTINGS
byte MAC_ADDRESS[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
int DIGICO_ADDRESS[] = {192, 168, 6, 159};
int FADER_8_ADDRESS[] = {192, 168, 6, 10};
int DIGICO_SEND_PORT = 8000;
int DIGICO_RECEIVE_PORT = 9000;
int FADERS[] = {1, 2, 3, 4, 5, 6, 7, 8};

#define FADER_COUNT 8
#define DEBUG false

#include <NativeEthernetUdp.h>
#include <OSCMessage.h>

#define HEARTBEAT_INTERVAL 10


EthernetUDP Udp;
uint8_t packetBuffer[UDP_TX_PACKET_MAX_SIZE];
int packetSize = 0;
IPAddress DESTINATION_IP(DIGICO_ADDRESS[0], DIGICO_ADDRESS[1], DIGICO_ADDRESS[2], DIGICO_ADDRESS[3]);


void loop() {
  if (getEthernetStatus() != 0) {

    packetSize = Udp.parsePacket();
    OSCMessage oscMsg;
    if (packetSize) {
      for (int j = 0; j < packetSize; j += UDP_TX_PACKET_MAX_SIZE - 1) {
        Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE - 1);
        oscMsg.fill(packetBuffer, UDP_TX_PACKET_MAX_SIZE - 1);
      }
      onOSCMessage(oscMsg);
    }

  }
  faderLoop();

}

void setup() {
  Serial.begin(9600);
  delay(1500);
  faderSetup();

}

void ethernetSetup() {
  Udp.begin(DIGICO_SEND_PORT);

}

void faderHasMoved(byte i) {

  if (getEthernetStatus() != 0) {
    String address = "/Input_Channels/xx/fader";
    address.replace("xx", FADERS[i]);
    char addressBuf[address.length()+1];
    address.toCharArray(addressBuf, address.length()+1);
    OSCMessage msg(addressBuf);

    float mappedVal = customLogMap(getFaderValue(i));
    msg.add(mappedVal);

    Udp.beginPacket(DESTINATION_IP, DIGICO_RECEIVE_PORT);
    msg.send(Udp);
    Udp.endPacket();
  }
}


void onOSCMessage(OSCMessage &msg) {
  msg.dispatch("/Input_Channels/*/fader", OSCFaderValue);
}
void OSCFaderValue(OSCMessage &msg) {
  char f[2];
  msg.getAddress(f, 16, 2);

  int faderNumber = String(f).toInt();

  for(byte i=0; i<FADER_COUNT; i++){
    if(FADERS[i] == faderNumber){
      setFaderTarget(i, customLinearMap(msg.getFloat(0)));
    }
  }


}

int customLogMap(int val){
  int mappedVal = 0;
  if (val >= 0 && val < 50) {
    mappedVal = map(val, 0, 50, -150, -60);
  } else if (val >= 50 && val < 256) {
    mappedVal = map(val, 50, 256, -60, -20);
  } else if (val >= 256) {
    mappedVal = map(val, 256, 512, -20, 10);
  }
  return mappedVal;
}

int customLinearMap(int val){
  int mappedVal = 0;
  if (val >= -150 && val < -60) {
    mappedVal = map(val, -150, -60, 0, 50);
  } else if (val >= -60 && val < -20) {
    mappedVal = map(val, -60, -20, 50, 256);
  } else if (val >= -20) {
    mappedVal = map(val, -20, 10, 256, 512);
  }
  return mappedVal;
}
