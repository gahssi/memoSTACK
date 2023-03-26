/*
  BMEG 257 Adherence Bottle Project

 The code of a simple webserver to be located on the device that allows a use to select a reminder time.
 This sketch will print the IP address of your WiFi module (once connected)
 to the Serial monitor. From there, you can open that address in a web browser.
 
 If the IP address of your board is yourAddress:
 http://yourAddress/H turns the LED on
 http://yourAddress/L turns it off

 This example is written for a network using WPA encryption. For
 WEP or WPA, change the Wifi.begin() call accordingly.

 Circuit:
 * Board with NINA module (Arduino MKR WiFi 1010, MKR VIDOR 4000 and UNO WiFi Rev.2)
 * LED attached to pin 9
 */
#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <TimeAlarms.h>
#include <TimeLib.h>

#include "arduino_secrets.h" 

// ---------------------------------------------------------------------------------------------------
// PIN variables

int reminderLight = 13;
int wifiStatusLight = 8;


// ---------------------------------------------------------------------------------------------------
// Time variables

int utcHour = 0;
int utcMin = 0;
int utcSec = 0;

int localHour = 0;
int localMin = 0;
int localSec = 0;

unsigned int localPort = 2390;      // local port to listen for UDP packets

IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;


// ---------------------------------------------------------------------------------------------------
// WiFi setup

// PLEASE enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;    // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;             // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
WiFiServer server(80);

// ---------------------------------------------------------------------------------------------------
// Alarm variables

int alarmHour = 8;
int alarmMinute = 00;
int HOD = 0; // 0 = am 1 = pm
int alarmSet = 0;


// ---------------------------------------------------------------------------------------------------

void setup() {
  Serial.begin(9600);
  while (!Serial) ; // wait for Arduino Serial Monitor
  
  pinMode(reminderLight, OUTPUT);       // set the LED pin mode
  pinMode(wifiStatusLight, OUTPUT);     // set the LED pin mode

  wifiSetup();
  timeSetup();
  time();

  Alarm.delay(1000);
  
  setTime(localHour,localMin,localSec,1,1,11); // set time to Saturday 8:29:00am Jan 1 2011
  digitalClockDisplay();
}

void loop() {
  digitalClockDisplay();
  WiFiClient client = server.available();   // listen for incoming clients
  clientActions(client);

   if(WiFi.status() != WL_CONNECTED){
    digitalWrite(wifiStatusLight, LOW);
    wifiSetup();
  }
  
  Alarm.delay(1000); // wait one second between clock display
}

// ---------------------------------------------------------------------------------------------------

// functions to be called when an alarm triggers:
void alarmRing() {
  digitalWrite(reminderLight, HIGH);
  Serial.print("Bzzzzz!\n");
  String TOD = (String)hour() + ":" + (String)minute() + ":" + (String)second();
}

void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.println();
}

void printDigits(int digits) {
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

// ---------------------------------------------------------------------------------------------------


void clientActions(WiFiClient client){
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

            // the content of the HTTP response follows the header:
            client.print("<h1>memoSTACK - Alarm App</h1>");
            
           if(alarmSet == 0){
            client.print("<br>Confirm your medication regimen has been inputted correctly in to the memoSTACK pill device: <a href=\"\">Yes</a> / <a href=\"\">No</a><br>");

            client.print("<p>Set your next reminder below:");
            client.print("<table onload border=\"1\" cellpadding=\"5\" cellspacing=\"3\">");
            client.print("<tr><td colspan=\"2\" align=\"center\"><b>Today</b></td>");
            client.print("<td colspan=\"2\" align=\"center\"><b>Tomorrow</b></td>");
            client.print("<tr><td>");
            client.print("<a href=\"/H00\">12:00 AM</a><br>");
            client.print("<a href=\"/H01\">1:00 AM</a><br>"); 
            client.print("<a href=\"/H02\">2:00 AM</a><br>"); 
            client.print("<a href=\"/H03\">3:00 AM</a><br>"); 
            client.print("<a href=\"/H04\">4:00 AM</a><br>"); 
            client.print("<a href=\"/H05\">5:00 AM</a><br>"); 
            client.print("<a href=\"/H06\">6:00 AM</a><br>"); 
            client.print("<a href=\"/H07\">7:00 AM</a><br>"); 
            client.print("<a href=\"/H08\">8:00 AM</a><br>"); 
            client.print("<a href=\"/H09\">9:00 AM</a><br>"); 
            client.print("<a href=\"/H10\">10:00 AM</a><br>");
            client.print("<a href=\"/H11\">11:00 AM</a><br>");
            client.print("</td>");
            client.print("<td>");
            client.print("<a href=\"/H12\">12:00 PM</a><br>");
            client.print("<a href=\"/H13\">1:00 PM</a><br>");
            client.print("<a href=\"/H14\">2:00 PM</a><br>");
            client.print("<a href=\"/H15\">3:00 PM</a><br>");
            client.print("<a href=\"/H16\">4:00 PM</a><br>");
            client.print("<a href=\"/H17\">5:00 PM</a><br>");
            client.print("<a href=\"/H18\">6:00 PM</a><br>");
            client.print("<a href=\"/H19\">7:00 PM</a><br>");
            client.print("<a href=\"/H20\">8:00 PM</a><br>");
            client.print("<a href=\"/H21\">9:00 PM</a><br>");
            client.print("<a href=\"/H22\">10:00 PM</a><br>");
            client.print("<a href=\"/H23\">11:00 PM</a><br>");
            client.print("</td>");
            
            client.print("<td>");
            client.print("<a href=\"/H00\">12:00 AM</a><br>");
            client.print("<a href=\"/H01\">1:00 AM</a><br>"); 
            client.print("<a href=\"/H02\">2:00 AM</a><br>"); 
            client.print("<a href=\"/H03\">3:00 AM</a><br>"); 
            client.print("<a href=\"/H04\">4:00 AM</a><br>"); 
            client.print("<a href=\"/H05\">5:00 AM</a><br>"); 
            client.print("<a href=\"/H06\">6:00 AM</a><br>"); 
            client.print("<a href=\"/H07\">7:00 AM</a><br>"); 
            client.print("<a href=\"/H08\">8:00 AM</a><br>"); 
            client.print("<a href=\"/H09\">9:00 AM</a><br>"); 
            client.print("<a href=\"/H10\">10:00 AM</a><br>");
            client.print("<a href=\"/H11\">11:00 AM</a><br>");
            client.print("</td>");
            client.print("<td>");
            client.print("<a href=\"/H12\">12:00 PM</a><br>");
            client.print("<a href=\"/H13\">1:00 PM</a><br>");
            client.print("<a href=\"/H14\">2:00 PM</a><br>");
            client.print("<a href=\"/H15\">3:00 PM</a><br>");
            client.print("<a href=\"/H16\">4:00 PM</a><br>");
            client.print("<a href=\"/H17\">5:00 PM</a><br>");
            client.print("<a href=\"/H18\">6:00 PM</a><br>");
            client.print("<a href=\"/H19\">7:00 PM</a><br>");
            client.print("<a href=\"/H20\">8:00 PM</a><br>");
            client.print("<a href=\"/H21\">9:00 PM</a><br>");
            client.print("<a href=\"/H22\">10:00 PM</a><br>");
            client.print("<a href=\"/H23\">11:00 PM</a><br>");
            client.print("</td></tr></table><br>");
            
            //client.print("<a href=\"/HDEMO\">Demo time</a>");
           }

            if(alarmSet == 1){
              client.print("<br><br>You set your next alarm to <b>");
              client.print(alarmHour);
              client.print(":");
              if (alarmMinute < 10) {              // In the first 10 seconds of each alarmMinute, we'll want a leading '0'
                client.print("0");
              }
              client.print(alarmMinute);
              client.print("   ");
              if(HOD == 1){
                client.print("PM");
              }
              else{
                client.print("AM");
              }
              client.println("</b><br><br>Want to revert your last alarm? To do so, press the reset button on the device.");
            }
            
            client.print("<br><br>Click <a href=\"/L\">here</a> to confirm you took your medication dose.<br>");

            client.print("<br><br><br>View your recent medication use history with memoSTACK's <a href=\"\">Google Integration</a>.");
            
            
  
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
  
        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(reminderLight, LOW);                // GET /L turns the LED off
          String TOD = (String)hour() + ":" + (String)minute() + ":" + (String)second();
        }

        if (currentLine.endsWith("GET /HDEMO")) {
          alarmHour = 11;
          alarmMinute = 31;
          HOD = 0;

          if(HOD == 1){
            Alarm.alarmRepeat(alarmHour+12,alarmMinute,0, alarmRing);  // 8:3pam every day
            Serial.println("alarm set");
          }
          else{
            Alarm.alarmRepeat(alarmHour,alarmMinute,0, alarmRing);  // 8:30am every day
            Serial.println("alarm set");
          }

          alarmSet = 1;
        }

        // Sets alarmHour to recieved alarmHour time
        if (currentLine.endsWith("GET /H00") || currentLine.endsWith("GET /H01") || currentLine.endsWith("GET /H02") || currentLine.endsWith("GET /H03") || currentLine.endsWith("GET /H04") || currentLine.endsWith("GET /H05") || currentLine.endsWith("GET /H06") || currentLine.endsWith("GET /H07") || currentLine.endsWith("GET /H08") || currentLine.endsWith("GET /H09") || currentLine.endsWith("GET /H10") || currentLine.endsWith("GET /H11") || currentLine.endsWith("GET /H12") || currentLine.endsWith("GET /H13") || currentLine.endsWith("GET /H14") || currentLine.endsWith("GET /H15") || currentLine.endsWith("GET /H16") || currentLine.endsWith("GET /H17") || currentLine.endsWith("GET /H18") || currentLine.endsWith("GET /H19") || currentLine.endsWith("GET /H20") || currentLine.endsWith("GET /H21") || currentLine.endsWith("GET /H22") || currentLine.endsWith("GET /H23")) {
        if (currentLine.endsWith("GET /H00") || currentLine.endsWith("GET /H12")) {
          alarmHour = 12;
          if (currentLine.endsWith("GET /H00")) {
            HOD = 0;
          } else {
            HOD = 1;
          }
        }
        if (currentLine.endsWith("GET /H01") || currentLine.endsWith("GET /H13")) {
          alarmHour = 1;
          if (currentLine.endsWith("GET /H01")) {
            HOD = 0;
          } else {
            HOD = 1;
          }
        }
        if (currentLine.endsWith("GET /H02") || currentLine.endsWith("GET /H14")) {
          alarmHour = 2;
          if (currentLine.endsWith("GET /H02")) {
            HOD = 0;
          } else {
            HOD = 1;
          }
        }
        if (currentLine.endsWith("GET /H03") || currentLine.endsWith("GET /H15")) {
          alarmHour = 3;
          if (currentLine.endsWith("GET /H03")) {
            HOD = 0;
          } else {
            HOD = 1;
          }
        }
        if (currentLine.endsWith("GET /H04") || currentLine.endsWith("GET /H16")) {
          alarmHour = 4;
          if (currentLine.endsWith("GET /H04")) {
            HOD = 0;
          } else {
            HOD = 1;
          }
        }
        if (currentLine.endsWith("GET /H05") || currentLine.endsWith("GET /H17")) {
          alarmHour = 5;
          if (currentLine.endsWith("GET /H05")) {
            HOD = 0;
          } else {
            HOD = 1;
          }
        }
        if (currentLine.endsWith("GET /H06") || currentLine.endsWith("GET /H18")) {
          alarmHour = 6;
          if (currentLine.endsWith("GET /H06")) {
            HOD = 0;
          } else {
            HOD = 1;
          }
        }
        if (currentLine.endsWith("GET /H07") || currentLine.endsWith("GET /H19")) {
          alarmHour = 7;
          if (currentLine.endsWith("GET /H07")) {
            HOD = 0;
          } else {
            HOD = 1;
          }
        }
        if (currentLine.endsWith("GET /H08") || currentLine.endsWith("GET /H20")) {
          alarmHour = 8;
          if (currentLine.endsWith("GET /H08")) {
            HOD = 0;
          } else {
            HOD = 1;
          }
        }
        if (currentLine.endsWith("GET /H09") || currentLine.endsWith("GET /H21")) {
          alarmHour = 9;
          if (currentLine.endsWith("GET /H09")) {
            HOD = 0;
          } else {
            HOD = 1;
          }
        }
        if (currentLine.endsWith("GET /H10") || currentLine.endsWith("GET /H22")) {
          alarmHour = 10;
          if (currentLine.endsWith("GET /H10")) {
            HOD = 0;
          } else {
            HOD = 1;
          }
        }
        if (currentLine.endsWith("GET /H11") || currentLine.endsWith("GET /H23")) {
          alarmHour = 11;
          if (currentLine.endsWith("GET /H11")) {
            HOD = 0;
          } else {
            HOD = 1;
          }
        }

        if(HOD == 1){
            Alarm.alarmRepeat(alarmHour+12,alarmMinute,0, alarmRing);  // 8:3pam every day
            Serial.println("alarm set");
          }
          else{
            Alarm.alarmRepeat(alarmHour,alarmMinute,0, alarmRing);  // 8:30am every day
            Serial.println("alarm set");
          }

          alarmSet = 1;

        }
         
      }
    }
  // close the connection:
  client.stop();
  Serial.println("client disonnected");
  Serial.println(alarmHour);
  Serial.println(alarmMinute);
  Serial.println(HOD);
  }
}

void wifiSetup(){
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();                           // start the web server on port 80
  digitalWrite(wifiStatusLight, HIGH);
  printWifiStatus();                        // you're connected now, so print out the status
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}


// ---------------------------------------------------------------------------------------------------

void printTime(){
  Serial.print("The local time is ");
  Serial.print(localHour); // print the alarmHour (86400 equals secs per day)
  Serial.print(':');
  if (localMin < 10) {
    // In the first 10 alarmMinutes of each alarmHour, we'll want a leading '0'
    Serial.print('0');
  }
  Serial.print(localMin); // print the alarmMinute (3600 equals secs per alarmMinute)
  Serial.print(':');
  if (localSec < 10) {
    // In the first 10 seconds of each alarmMinute, we'll want a leading '0'
    Serial.print('0');
  }
  Serial.println(localSec); // print the second
}


void timeSetup(){
  Serial.println("\nStarting connection to server...");
  Udp.begin(localPort);
}

void convertToLocal(){
  if(utcHour <= 7){
    localHour = 24-7+utcHour;
    localMin = utcMin;
    localSec = utcSec;
  }
  else{
    localHour = utcHour-7;
    localMin = utcMin;
    localSec = utcSec;
  }
}

void time(){
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
  if (Udp.parsePacket()) {
    Serial.println("packet received");
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    
    // now convert NTP time into everyday time:
    const unsigned long seventyYears = 2208988800UL;
    unsigned long epoch = secsSince1900 - seventyYears;
    
    utcHour = (epoch  % 86400L) / 3600;
    utcMin = (epoch  % 3600) / 60;
    utcSec = epoch % 60;

    //Serial.print("UTC alarmHour");
    //Serial.print(utcHour);

    convertToLocal();
  }
  // wait ten seconds before asking for the time again
  //delay(10000);
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address) {
  //Serial.println("1");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  //Serial.println("2");
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  //Serial.println("3");

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  //Serial.println("4");
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  //Serial.println("5");
  Udp.endPacket();
  //Serial.println("6");
}
