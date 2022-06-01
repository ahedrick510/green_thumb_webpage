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

 char ssid[] = "GREEN THUMB";        // your network SSID (name)
 char pass[] = "fortheplants";    // your network password (use for WPA, or use as key for WEP)
 //int keyIndex = 0;                // your network key index number (needed only for WEP)
 
 int status = WL_IDLE_STATUS;
 WiFiServer server(80);
 
//Declare some variables to display on webpage
 String plant = "plant";
 int data[] = {1,1,0};
 bool water_level = 0; //from float sensor: 0 = empty, 1 = full
 int moisture_level = 0; //this number will be 0-10, sensor gives 0-3 volts output
 int daily_light = 0; //hours
 int period = 10*60*1000; //10 minutes in milliseconds

 // for finding time
 int cur_millis = 0; 
 int last_millis = 0;

 void setup() {
   //Initialize serial and wait for port to open:
   Serial.begin(9600);
   // Wifi Setup
   {
   while (!Serial) {
     ; // wait for serial port to connect. Needed for native USB port only
   }

   Serial.println("Access Point Web Server");
 

   pinMode(LEDR, OUTPUT);
   pinMode(LEDG, OUTPUT);
   pinMode(LEDB, OUTPUT);
 

   // check for the WiFi module:
   
   if (WiFi.status() == WL_NO_MODULE) {
     Serial.println("Communication with WiFi module failed!");
     // don't continue
     while (true);
   }

 
   // print the network name (SSID);
   Serial.print("Creating access point named: ");
   Serial.println(ssid);
 

   // Create open network. Change this line if you want to create an WEP network:
   status = WiFi.beginAP(ssid, pass);
   if (status != WL_AP_LISTENING) {
     Serial.println("Creating access point failed");
     // don't continue
     while (true);
   }
 

   // wait 10 seconds for connection:
   delay(10000);
 

   // start the web server on port 80
   server.begin();
 

   // you're connected now, so print out the status
   printWiFiStatus();
   }


{
// Water Level (float sensor) Setup
{}
// Moisture Level (capacitive soil sensor) Setup
{}
// Light Level Setup
{}
// Lighting Mechanism Setup
{}
// Shading Mechanism Setup
{}
// Rotation Setup
{}
}
last_millis = millis();
 }
 

 void loop() {
  // Wifi Loop
  {
   if (status != WiFi.status()) {
     // it has changed update the variable
     status = WiFi.status();
    
     if (status == WL_AP_CONNECTED) {
       // a device has connected to the AP
       Serial.println("Device connected to AP");
       for (int i = 1;i<4;i++){
       digitalWrite(LED_BUILTIN, HIGH);
       delay(500);
       digitalWrite(LED_BUILTIN, LOW);
       delay(500);
       }
     } else {
       // a device has disconnected from the AP, and we are back in listening mode
       Serial.println("Device disconnected from AP");
     }
   }
   
   WiFiClient client = server.available();   // listen for incoming clients
 

    if (client) {                             // if you get a client,
     Serial.println("new client");           // print a message out the serial port
     String currentLine = "";                // make a String to hold incoming data from the client
     while (client.connected()) {            // loop while the client's connected
       if (client.available()) {             // if there's bytes to read from the client,
         char c = client.read();             // read a byte, then
         Serial.write(c);                    // print it out the serial monitor
         if (c == '\n') {                    // if the byte is a newline character
 

           // if the current line is blank, you got two newline characters in a row.
           // that's the end of the client HTTP request, so send a response:
           if (currentLine.length() == 0) {
             // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
             // and a content-type so the client knows what's coming, then a blank line:
             client.println("HTTP/1.1 200 OK");
             client.println("Content-type:text/html");
             client.println();  


             

             client.println("<html>");
client.println("<title>Web Page Design</title>");
client.println("<body style='background-color:#9cd9a6;'>");
client.println("<h1>The Green <span style='color: #1f6b2d'>THUMB</span></h1>");
client.println("<h2>care<span style='color: #1f6b2d'>T</span>aking <span style='color: #1f6b2d'>H</span>ortic<span style='color: #1f6b2d'>U</span>lture <span style='color: #1f6b2d'>M</span>odular <span style='color: #1f6b2d'>B</span>ot</h2>");
client.println("<form action='/get'>");
  client.println("<label for='plant'>Choose a plant type:</label>");
  client.println("<select name='plant' id='plant'>");
    client.println("<option value='select'>*select a plant*</option>");
    client.println("<option value='african_spear'>African Spear</option>");
    client.println("<option value='agloenema'>Agloenema</option>");
    client.println("<option value='aloe'>Aloe</option>");
    client.println("<option value='anthurium'>Anthurium</option>");
    client.println("<option value='basil'>Basil</option>");
    client.println("<option value='begonia'>Begonia</option>");
    client.println("<option value='bromeliad'>Bromeliad</option>");
    client.println("<option value='dieffenbachia'>Dieffenbachia</option>");
    client.println("<option value='dracena'>Dracena</option>");
    client.println("<option value='fig'>Fig</option>");
    client.println("<option value='jade'>Jade</option>");
    client.println("<option value='kalanchoe'>Kalanchoe</option>");
    client.println("<option value='kentia_palm'>Kentia Palm</option>");
    client.println("<option value='lily'>Lily</option>");
    client.println("<option value='mint'>Mint</option>");
    client.println("<option value='paddle'>Paddle</option>");
    client.println("<option value='peperomia'>Peperomia</option>");
    client.println("<option value='philodendron'>Philodendron</option>");
    client.println("<option value='pilea'>Pilea</option>");
    client.println("<option value='polka_dot_plant'>Polka Dot Plant</option>");
    client.println("<option value='pothos'>Pothos</option>");
    client.println("<option value='prayer_plant'>Prayer Plant</option>");
    client.println("<option value='ranunculus'>Ranunculus</option>");
    client.println("<option value='rhipsalis'>Rhipsalis</option>");
    client.println("<option value='schefflera'>Schefflera</option>");
    client.println("<option value='snake'>Snake Plant</option>");
    client.println("<option value='spider'>Spider Plant</option>");
    client.println("<option value='string_of_dolphins'>String of Dolphins</option>");
    client.println("<option value='succulent'>Succulent</option>");
    client.println("<option value='swedish_purple_ivy'>Swedish Purple Ivy</option>");
    client.println("<option value='venus_fly_trap'>Venus Fly Trap</option>");
    client.println("<option value='violet'>Violet</option>");
    client.println("<option value='ZZ'>ZZ Plant</option>");
  client.println("</select>");
  client.println("<br><br>");
  client.println("<input type='submit' value='Submit'>");
client.println("</form>");
          
client.println("<p>You have selected " + plant);
client.println("<p>Current soil moisture: ");

client.println("</body>");
client.println("</html>");

Serial.println("hello");
           

             // The HTTP response ends with another blank line:
             client.println();
             // break out of the while loop:
             break;
           } else {    // if you got a newline, then clear currentLine:
             currentLine = "";
           }
         } else if (c != '\r') {  // if you got anything else but a carriage return character,
           currentLine += c;      // add it to the end of the currentLine
         }
 
if (currentLine.endsWith("GET /get?plant=african_spear")) {
          digitalWrite(LEDR, HIGH);
          plant = "african_spear";
          data[0] = 3; data[1] = 1; data[2] = 1;
}  if (currentLine.endsWith("plant=agloenema")) {
          digitalWrite(LEDR, HIGH);
          plant = "agloenema";
          data[0] = 2; data[1] = 3; data[2] = 1;
}  if (currentLine.endsWith("plant=aloe")) {
          digitalWrite(LEDR, HIGH);
          plant = "aloe";
          data[0] = 4; data[1] = 2; data[2] = 1;        
}  if (currentLine.endsWith("plant=anthurium")) {
          digitalWrite(LEDR, HIGH);
          plant = "anthurium";
          data[0] = 2; data[1] = 2; data[2] = 1;        
}  if (currentLine.endsWith("plant=basil")) {
          digitalWrite(LEDR, HIGH);
          plant = "basil";
          data[0] = 4; data[1] = 4; data[2] = 1; 
}  if (currentLine.endsWith("plant=begonia")) {
          digitalWrite(LEDR, HIGH);
          plant = "begonia";
          data[0] = 2; data[1] = 4; data[2] = 0;
}  if (currentLine.endsWith("plant=bromeliad")) {
          digitalWrite(LEDR, HIGH);
          plant = "bromeliad";
          data[0] = 2; data[1] = 2; data[2] = 0; 
}  if (currentLine.endsWith("plant=dieffenbachia")) {
          digitalWrite(LEDR, HIGH);
          plant = "dieffenbachia";
          data[0] = 3; data[1] = 2; data[2] = 0;
}  if (currentLine.endsWith("plant=dracena")) {
          digitalWrite(LEDR, HIGH);
          plant = "dracena";
          data[0] = 2; data[1] = 2; data[2] = 0;  
}  if (currentLine.endsWith("plant=fig")) {
          digitalWrite(LEDR, HIGH);
          plant = "fig";
          data[0] = 3; data[1] = 2; data[2] = 1;
}  if (currentLine.endsWith("plant=jade")) {
          digitalWrite(LEDR, HIGH);
          plant = "jade";
          data[0] = 3; data[1] = 1; data[2] = 1;
}  if (currentLine.endsWith("plant=kalanchoe")) {
          digitalWrite(LEDR, HIGH);
          plant = "kalanchoe";
          data[0] = 3; data[1] = 1; data[2] = 1; 
}  if (currentLine.endsWith("plant=kentia_palm")) {
          digitalWrite(LEDR, HIGH);
          plant = "kentia_palm";
          data[0] = 3; data[1] = 1; data[2] = 1;
}  if (currentLine.endsWith("plant=lily")) {
          digitalWrite(LEDR, HIGH);
          plant = "lily";
          data[0] = 3; data[1] = 4; data[2] = 0; 
}  if (currentLine.endsWith("plant=mint")) {
          digitalWrite(LEDR, HIGH);
          plant = "mint";
          data[0] = 1; data[1] = 3; data[2] = 0; 
}  if (currentLine.endsWith("plant=paddle")) {
          digitalWrite(LEDR, HIGH);
          plant = "paddle";
          data[0] = 4; data[1] = 1; data[2] = 1; 
}  if (currentLine.endsWith("plant=peperomia")) {
          digitalWrite(LEDR, HIGH);
          plant = "peperomia";
          data[0] = 3; data[1] = 1; data[2] = 1;
}  if (currentLine.endsWith("plant=philodendron")) {
          digitalWrite(LEDR, HIGH);
          plant = "philodendron";
          data[0] = 2; data[1] = 1; data[2] = 1; 
}  if (currentLine.endsWith("plant=pilea")) {
          digitalWrite(LEDR, HIGH);
          plant = "pilea";
          data[0] = 2; data[1] = 3; data[2] = 1;
}  if (currentLine.endsWith("plant=polka_dot_plant")) {
          digitalWrite(LEDR, HIGH);
          plant = "polka_dot_plant";
          data[0] = 3; data[1] = 3; data[2] = 0;
}  if (currentLine.endsWith("plant=pothos")) {
          digitalWrite(LEDR, HIGH);
          plant = "pothos";
          data[0] = 2; data[1] = 3; data[2] = 1;  
}  if (currentLine.endsWith("plant=prayer_plant")) {
          digitalWrite(LEDR, HIGH);
          plant = "prayer_plant";
          data[0] = 2; data[1] = 4; data[2] = 0; 
}  if (currentLine.endsWith("plant=ranunculus")) {
          digitalWrite(LEDR, HIGH);
          plant = "ranunculus";
          data[0] = 3; data[1] = 3; data[2] = 1; 
}  if (currentLine.endsWith("plant=rhipsalis")) {
          digitalWrite(LEDR, HIGH);
          plant = "rhipsalis";
          data[0] = 2; data[1] = 2; data[2] = 0;
}  if (currentLine.endsWith("plant=schefflera")) {
          digitalWrite(LEDR, HIGH);
          plant = "schefflera";
          data[0] = 3; data[1] = 3; data[2] = 0;
}  if (currentLine.endsWith("plant=snake")) {
          digitalWrite(LEDR, HIGH);
          plant = "snake";
          data[0] = 3; data[1] = 2; data[2] = 1;
}  if (currentLine.endsWith("plant=spider")) {
          digitalWrite(LEDR, HIGH);
          plant = "spider";
          data[0] = 2; data[1] = 2; data[2] = 0;
}  if (currentLine.endsWith("plant=string_of_dolphins")) {
          digitalWrite(LEDR, HIGH);
          plant = "string_of_dolphins";
          data[0] = 3; data[1] = 2; data[2] = 1;
}  if (currentLine.endsWith("plant=succulent")) {
          digitalWrite(LEDR, HIGH);
          plant = "succulent";
          data[0] = 4; data[1] = 1; data[2] = 1;
}  if (currentLine.endsWith("plant=swedish_purple_ivy")) {
          digitalWrite(LEDR, HIGH);
          plant = "swedish_purple_ivy";
          data[0] = 3; data[1] = 3; data[2] = 1;
}  if (currentLine.endsWith("plant=venus_fly_trap")) {
          digitalWrite(LEDR, HIGH);
          plant = "venus_fly_trap";
          data[0] = 3; data[1] = 4; data[2] = 1;
}  if (currentLine.endsWith("plant=violet")) {
          digitalWrite(LEDR, HIGH);
          plant = "violet";
          data[0] = 3; data[1] = 3; data[2] = 0;        
}  if (currentLine.endsWith("plant=ZZ")) {
          digitalWrite(LEDR, HIGH);
          plant = "ZZ";
          data[0] = 2; data[1] = 2; data[2] = 1;
// } if (currentLine.endsWith("")){
//   plant = "no plant selected";
//   int data[] = {0,0,0};
//   digitalWrite(LEDG, HIGH);
//   digitalWrite(LEDR, LOW);
}

        
       }
     }
     // close the connection:
     client.stop();
     Serial.println("client disconnected");
   }
   }
 
 /*******Arduino checks all these things after 10 minutes*********/
 cur_millis = millis();
if ((cur_millis - last_millis) > period) {
  last_millis = millis();

  // Check Water Level
{
  if ((cur_millis - last_millis) > 4*24*60*60*1000){
    water_level = 0;
  }
  else water_level = 1;

}
  // Check Moisture Level
{

}
  // Check Light Level
{}
  // Rotate Plant every 4 hours
{}

 }

delay(5000);
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

 void printWiFiStatus() {
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


  
