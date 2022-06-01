/*
 #include <SPI.h>
 */
#include <Arduino.h>
#include <WiFiNINA.h>
/*
PinDescription g_APinDescription[] = {
  // D0 - D7
  { p1,         NULL, NULL, NULL },    // D0
  { p0,         NULL, NULL, NULL },    // D1
  { p25,        NULL, NULL, NULL },    // D2
  { p15,        NULL, NULL, NULL },    // D3
  { p16,        NULL, NULL, NULL },    // D4
  { p17,        NULL, NULL, NULL },    // D5
  { p18,        NULL, NULL, NULL },    // D6
  { p19,        NULL, NULL, NULL },    // D7

  // D8 - D13
  { p20,        NULL, NULL, NULL },    // D8
  { p21,        NULL, NULL, NULL },    // D9
  { p5,         NULL, NULL, NULL },    // D10
  { p7,         NULL, NULL, NULL },    // D11 / SPITX
  { p4,         NULL, NULL, NULL },    // D12 / SPIRX
  { p6,         NULL, NULL, NULL },    // D13 / SPICLK / LEDB 

  // Analog as digital
  // A4 to A7 are controlled by Nina module and exposed via different APIs
  { p26,        NULL, NULL, NULL },    // A0 -> D14
  { p27,        NULL, NULL, NULL },    // A1 -> D15
  { p28,        NULL, NULL, NULL },    // A2 -> D16
  { p29,        NULL, NULL, NULL },    // A3 -> D17

  // I2C
  { p12,        NULL, NULL, NULL },    // A4 / SDA  -> D18
  { p13,        NULL, NULL, NULL },    // A5 / SCL  -> D19

  // Internal pins - D20 - D23
  { p2,         NULL, NULL, NULL },    // GPIO0
  { p24,        NULL, NULL, NULL },    // IMU IRQ
  { p22,        NULL, NULL, NULL },    // PDM DATA IN
  { p23,        NULL, NULL, NULL },    // PDM CLOCK

  // Internal pins Nina - D24 - D29
  { p3,         NULL, NULL, NULL },    // RESET_NINA
  { p8,         NULL, NULL, NULL },    // SPI1_CIPO / UART1_TX
  { p9,         NULL, NULL, NULL },    // SPI1_CS / UART1_RX
  { p10,        NULL, NULL, NULL },    // SPI1_ACK / UART1_CTS
  { p11,        NULL, NULL, NULL },    // SPI1_COPI / UART1_RTS
  { p14,        NULL, NULL, NULL },    // SPI1_SCK
};
*/


/*
D2 -> SW1
D3 -> MTR_ENBL
D4 -> MTR_LATCH
D5 -> MTR_SCLK
D6 -> MTR_SDATIN
D7 -> SERVO1
D8 -> SERVO2
D9 -> SERVO3
D10 -> SERVO4
D11 -> MOTOR_FOR
D12 -> MOTOR_BACK

A4 -> LIGHT
A2 -> SW2
A1 -> BAT_SENSE
A0 -> SOIL_SIG
D13 -> SSR1

Motor Phases connected to shift registers:
A1 -> Bit 0
B1 -> Bit 1
C1 -> Bit 2
D1 -> But 3
A2 -> Bit 4
B2 -> Bit 5
C2 -> Bit 6
D2 -> Bit 7
A3 -> Bit 8
B3 -> Bit 9
C3 -> Bit 10
D3 -> Bit 11
NC -> Bit 12
NC -> Bit 13
NC -> Bit 14
NC -> Bit 15
 */

uint8_t pEnable = p15;
uint8_t pData = p18;
uint8_t pCLK = p17;
uint8_t pLatch = p16;
uint8_t MOTOR_FOR = 11;
uint8_t MOTOR_BACK = 12;
//uint8_t LIGHT = A4;
uint8_t SSR1 = 13;
uint8_t test = p6;
uint8_t SOIL = A0;
uint8_t SERVO1 = 7;
uint8_t SERVO2 = 8;
uint8_t SERVO3 = 9;
uint8_t SERVO4 = 10;

uint16_t pA1 = 0b1000000000000000;
uint16_t pB1 = 0b0100000000000000;
uint16_t pC1 = 0b0010000000000000;
uint16_t pD1 = 0b0001000000000000;
uint16_t pA2 = 0b0000100000000000;
uint16_t pB2 = 0b0000010000000000;
uint16_t pC2 = 0b0000001000000000;
uint16_t pD2 = 0b0000000100000000;
uint16_t pA3 = 0b0000000010000000;
uint16_t pB3 = 0b0000000001000000;
uint16_t pC3 = 0b0000000000100000;
uint16_t pD3 = 0b0000000000010000;

uint16_t pOff = 0b0000000000000000;
uint16_t pOn = 0b1111111111111111;

void printWiFiStatus();
void phaseOut(uint16_t phases);

char ssid[] = "GREEN THUMB";  // your network SSID (name)
char pass[] = "fortheplants"; // your network password (use for WPA, or use as key for WEP)
// int keyIndex = 0;                // your network key index number (needed only for WEP)

int status = WL_IDLE_STATUS;
WiFiServer server(80);

// Declare some variables to display on webpage
String plant = "plant";
int data[] = {1, 1, 0};
bool water_level = 0;        // from float sensor: 0 = empty, 1 = full
int moisture_level = 0;      // this number will be 0-10, sensor gives 0-3 volts output
int daily_light = 0;         // hours
int period = 10 * 60 * 1000; // 10 minutes in milliseconds

// for finding time
int cur_millis = 0;
int last_millis = 0;

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(pEnable, OUTPUT);
  pinMode(pData, OUTPUT);
  pinMode(pCLK, OUTPUT);
  pinMode(pLatch, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(pEnable, HIGH);
}

void loop()
{
  digitalWrite(LEDR,HIGH);
  delay(1000);
  digitalWrite(LEDR,LOW);
  delay(1000);
  for (int i = 0; i <= 50; i++)
  {
    phaseOut(pA3);
    delay(20);
    phaseOut(pB3);
    delay(20);
    phaseOut(pC3);
    delay(20);
    phaseOut(pD3);
    delay(20);
  }
  digitalWrite(LEDR,HIGH);
  delay(1000);
  digitalWrite(LEDR,LOW);
  delay(1000);
}

// Wifi stuff reference: Karl Söderby

// HELPER FUNCTIONS!!
void phaseOut(uint16_t phases)
{
  digitalWrite(pLatch, LOW);
  delayMicroseconds(20);
  shiftOut(pData, pCLK, LSBFIRST, lowByte(phases));
  shiftOut(pData, pCLK, LSBFIRST, highByte(phases));
  delayMicroseconds(20);
  digitalWrite(pLatch, HIGH);
  delayMicroseconds(20);
  digitalWrite(pLatch, LOW);
  delayMicroseconds(20);
}

void printWiFiStatus()
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

/**********************************************
Plant vector rules:
int plant_name[] = {light level, water level, light type};

Light Levels:
1 = 2 hours
2 = 5 hours
3 = 8 hours
4 = 11 hours
5 = 14 hours

Water Levels:
1 = super dry plant i.e. succ - water once per week (don't care about sensor)
2 = dry plant i.e. ZZ Plant - soil should be COMPLETELY DRY before watering (and had been for a day)
3 = average plant i.e. violet - soil dry before watering, then water "well"
4 = moist plant i.e. basil - soil slightly moist before watering
5 = water plant i.e. kelp - plant in water

Light Types:
0 = indirect (lamp never turns on)
1 = direct (lamp can turn on)
2 = light type doesn't matter

**************************************************/
