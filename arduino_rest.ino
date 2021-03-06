/*
arduino_rest

With this sketch, it is possible to control Arduino via HTTP GET requisitions
using a REST-style interface. It uses Ethernet Shield based on Wiznet W5100
chip. It is based on:

- RESTDuino (https://github.com/jjg/RESTduino) to interpret the 
REST-style requisitions;

- EthernetSetup (http://bit.ly/19ncowe) to allow to change Arduino's
network configuration using a web-based interface.

See README for information on how to use.

Author: Renato Candido <renato@liria.com.br>
Copyright 2015

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

Changelog:

2015-03-11
Changed initial IP configuration to 192.168.1.150

2013-12-06
Initial commit.
 
*/

#include <SPI.h>
#include <Ethernet.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>

// Seting up the EthernetShield
// Change the defaults the match your own network
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
byte ip[] = {192,168,1,150};
byte subnet[] = {255,255,255,0};
byte gateway[] = {192,168,1,1};
byte dnsserver[] = {192,168,1,1};
EthernetServer server(80);

// Buffers through which HTML code will flow.
char buffer[100];
#define BUFSIZE 255

// Reset button Pin. Normally attached to ground.
// Attach to 5V for 3s to change IP to default value.
#define RESETBUTTON 8

// This is the HTML code all chopped up. The best way to do this is, is by typing
// your HTML code in an editor, counting your characters and divide them by 8.
// you can chop your HTML on every place, but not on the \" parts. So remember,
// you have to use \" instead of simple " within the HTML, or it will not work.

prog_char htmlx0[] PROGMEM = "<html><title>Arduino Ethernet Setup Page</title><body marginwidth=\"0\" marginheight=\"0\" ";
prog_char htmlx1[] PROGMEM = "leftmargin=\"0\" style=\"margin: 0; padding: 0;\"><table bgcolor=\"#999999\" border";
prog_char htmlx2[] PROGMEM = "=\"0\" width=\"100%\" cellpadding=\"1\" style=\"font-family:Verdana;color:#fff";
prog_char htmlx3[] PROGMEM = "fff;font-size:12px;\"><tr><td>&nbsp Arduino Ethernet Setup Page</td></tr></table><br>";
PROGMEM const char *string_table0[] = {htmlx0, htmlx1, htmlx2, htmlx3};

prog_char htmla0[] PROGMEM = "<script>function hex2num (s_hex) {eval(\"var n_num=0X\" + s_hex);return n_num;}";
prog_char htmla1[] PROGMEM = "</script><table><form><input type=\"hidden\" name=\"SBM\" value=\"1\"><tr><td>MAC:";
prog_char htmla2[] PROGMEM = "<input id=\"T1\" type=\"text\" size=\"2\" maxlength=\"2\" name=\"MA1\" value=\"";
prog_char htmla3[] PROGMEM = "\">.<input id=\"T3\" type=\"text\" size=\"2\" maxlength=\"2\" name=\"MA2\" value=\"";
prog_char htmla4[] PROGMEM = "\">.<input id=\"T5\" type=\"text\" size=\"2\" maxlength=\"2\" name=\"MA3\" value=\"";
prog_char htmla5[] PROGMEM = "\">.<input id=\"T7\" type=\"text\" size=\"2\" maxlength=\"2\" name=\"MA4\" value=\"";
prog_char htmla6[] PROGMEM = "\">.<input id=\"T9\" type=\"text\" size=\"2\" maxlength=\"2\" name=\"MA5\" value=\"";
prog_char htmla7[] PROGMEM = "\">.<input id=\"T11\" type=\"text\" size=\"2\" maxlength=\"2\" name=\"MA6\" value=\"";
PROGMEM const char *string_table1[] = {htmla0, htmla1, htmla2, htmla3, htmla4, htmla5, htmla6, htmla7};

prog_char htmlb0[] PROGMEM = "\"><input id=\"T2\" type=\"hidden\" name=\"DT1\"><input id=\"T4\" type=\"hidden\" name=\"DT2";
prog_char htmlb1[] PROGMEM = "\"><input id=\"T6\" type=\"hidden\" name=\"DT3\"><input id=\"T8\" type=\"hidden\" name=\"DT4";
prog_char htmlb2[] PROGMEM = "\"><input id=\"T10\" type=\"hidden\" name=\"DT5\"><input id=\"T12\" type=\"hidden\" name=\"D";
prog_char htmlb3[] PROGMEM = "T6\"></td></tr><tr><td>IP: <input type=\"text\" size=\"3\" maxlength=\"3\" name=\"DT7\" value=\"";
prog_char htmlb4[] PROGMEM = "\">.<input type=\"text\" size=\"3\" maxlength=\"3\" name=\"DT8\" value=\"";
prog_char htmlb5[] PROGMEM = "\">.<input type=\"text\" size=\"3\" maxlength=\"3\" name=\"DT9\" value=\"";
prog_char htmlb6[] PROGMEM = "\">.<input type=\"text\" size=\"3\" maxlength=\"3\" name=\"DT10\" value=\"";
PROGMEM const char *string_table2[] = {htmlb0, htmlb1, htmlb2, htmlb3, htmlb4, htmlb5, htmlb6};

prog_char htmlc0[] PROGMEM = "\"></td></tr><tr><td>MASK: <input type=\"text\" size=\"3\" maxlength=\"3\" name=\"DT11\" value=\"";
prog_char htmlc1[] PROGMEM = "\">.<input type=\"text\" size=\"3\" maxlength=\"3\" name=\"DT12\" value=\"";
prog_char htmlc2[] PROGMEM = "\">.<input type=\"text\" size=\"3\" maxlength=\"3\" name=\"DT13\" value=\"";
prog_char htmlc3[] PROGMEM = "\">.<input type=\"text\" size=\"3\" maxlength=\"3\" name=\"DT14\" value=\"";
PROGMEM const char *string_table3[] = {htmlc0, htmlc1, htmlc2, htmlc3};

prog_char htmld0[] PROGMEM = "\"></td></tr><tr><td>GW: <input type=\"text\" size=\"3\" maxlength=\"3\" name=\"DT15\" value=\"";
prog_char htmld1[] PROGMEM = "\">.<input type=\"text\" size=\"3\" maxlength=\"3\" name=\"DT16\" value=\"";
prog_char htmld2[] PROGMEM = "\">.<input type=\"text\" size=\"3\" maxlength=\"3\" name=\"DT17\" value=\"";
prog_char htmld3[] PROGMEM = "\">.<input type=\"text\" size=\"3\" maxlength=\"3\" name=\"DT18\" value=\"";
prog_char htmld4[] PROGMEM = "\"></td></tr><tr><td><br></td></tr><tr><td><input id=\"button1\"type=\"submit\" value=\"SUBMIT\" ";
prog_char htmld5[] PROGMEM = "></td></tr></form></table></body></html>";
PROGMEM const char *string_table4[] = {htmld0, htmld1, htmld2, htmld3, htmld4, htmld5};

prog_char htmle0[] PROGMEM = "Onclick=\"document.getElementById('T2').value ";
prog_char htmle1[] PROGMEM = "= hex2num(document.getElementById('T1').value);";
prog_char htmle2[] PROGMEM = "document.getElementById('T4').value = hex2num(document.getElementById('T3').value);";
prog_char htmle3[] PROGMEM = "document.getElementById('T6').value = hex2num(document.getElementById('T5').value);";
prog_char htmle4[] PROGMEM = "document.getElementById('T8').value = hex2num(document.getElementById('T7').value);";
prog_char htmle5[] PROGMEM = "document.getElementById('T10').value = hex2num(document.getElementById('T9').value);";
prog_char htmle6[] PROGMEM = "document.getElementById('T12').value = hex2num(document.getElementById('T11').value);\"";
PROGMEM const char *string_table5[] = {htmle0, htmle1, htmle2, htmle3, htmle4, htmle5, htmle6};

const byte ID = 0x92; //used to identify if valid data in EEPROM the "know" bit, 
// if this is written in EEPROM the sketch has ran before
// We use this, so that the very first time you'll run this sketch it will use
// the values written above.
// defining which EEPROM address we are using for what data

void setup() {
  ShieldSetup(); //Setup the Ethernet shield
  server.begin(); //starting the server
}

void ShieldSetup() {
  int idcheck = EEPROM.read(0);
  if (idcheck != ID){
    //ifcheck id is not the value as const byte ID,
    //it means this sketch has NOT been used to setup the shield before
    //just use the values written in the beginning of the sketch
  }
  if (idcheck == ID){
    //if id is the same value as const byte ID,
    //it means this sketch has been used to setup the shield.
    //So we will read the values out of EERPOM ans use them
    //to setup the shield.
    for (int i = 0; i < 6; i++){
      mac[i] = EEPROM.read(i+1);
    }
    for (int i = 0; i < 4; i++){
      ip[i] = EEPROM.read(i+7);
    }
    for (int i = 0; i < 4; i++){
      subnet[i] = EEPROM.read(i+11);
    }
    for (int i = 0; i < 4; i++){
      gateway[i] = EEPROM.read(i+15);
    }
  }
  Ethernet.begin(mac, ip, dnsserver, gateway, subnet);
}

void loop() {
  if (digitalRead(RESETBUTTON)) {
    delay(3000);
    if (digitalRead(RESETBUTTON)) {
      // If RESETBUTTON pin is set to HIGH for more than 3s,
      // change const byte ID forcing the system to use the
      // ip address set in the beginning of the sketch
      EEPROM.write(0, 0x00);
    }
  }

  char clientline[BUFSIZE];
  int index = 0;
  // listen for incoming clients
  EthernetClient client = server.available();

  if (client) {
    //  reset input buffer
    index = 0;
    int nchars = 0;

    while (client.connected()) {
      if (client.available()) {
        if (nchars < 10) {
          // Reads 10 characters to check if the client is accessing /SETUP to change
          // network configuration
          char c = client.read();
          // fill url the buffer
          if (c != '\n' && c != '\r' && index < BUFSIZE) {
            // Reads until either an eol character is reached or the buffer is full
            clientline[index++] = c;
            nchars++;
            continue;
          }
        }
        else {
          String urlString = String(clientline);
          urlString = urlString.substring(urlString.indexOf('/'), urlString.indexOf(' ', urlString.indexOf('/')));
          urlString.toUpperCase();
          if (urlString.equals("/SETUP")) {
            // if you find the word "SBM" continue looking for more
            // if you don't find that word, stop looking and go further
            // it means the SUBMIT button hasn't been pressed an nothing has
            // been submitted. Just go to the place where the setup page is
            // been build and show it in the client's browser.
            if (client.findUntil("SBM", "\n\r")) {
              byte SET = client.parseInt();
              // Now while you are looking for the letters "DT", you'll have to remember
              // every number behind "DT" and put them in "val" and while doing so, check
              // for the according values and put those in mac, ip, subnet and gateway.
              while(client.findUntil("DT", "\n\r")) {
                int val = client.parseInt();
                // if val from "DT" is between 1 and 6 the according value must be a MAC value.
                if(val >= 1 && val <= 6) {
                  mac[val - 1] = client.parseInt();
                }
                // if val from "DT" is between 7 and 10 the according value must be a IP value.
                if(val >= 7 && val <= 10) {
                  ip[val - 7] = client.parseInt();
                }
                // if val from "DT" is between 11 and 14 the according value must be a MASK value.
                if(val >= 11 && val <= 14) {
                  subnet[val - 11] = client.parseInt();
                }
                // if val from "DT" is between 15 and 18 the according value must be a GW value.
                if(val >= 15 && val <= 18) {
                  gateway[val - 15] = client.parseInt();
                }
              }
              // Now that we got all the data, we can save it to EEPROM
              for (int i = 0 ; i < 6; i++){
                EEPROM.write(i + 1,mac[i]);
              }
              for (int i = 0 ; i < 4; i++){
                EEPROM.write(i + 7, ip[i]);
              }
              for (int i = 0 ; i < 4; i++){
                EEPROM.write(i + 11, subnet[i]);
              }
              for (int i = 0 ; i < 4; i++){
                EEPROM.write(i + 15, gateway[i]);
              }
              // set ID to the known bit, so when you reset the Arduino is will use the EEPROM values
              EEPROM.write(0, 0x92); 
              // if al the data has been written to EEPROM we should reset the arduino
              // for now you'll have to use the hardware reset button
            }
            // and from this point on, we can start building our setup page
            // and show it in the client's browser.
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println();
            for (int i = 0; i < 4; i++) {
              strcpy_P(buffer, (char*)pgm_read_word(&(string_table0[i])));
              client.print( buffer );
            }
            for (int i = 0; i < 3; i++) {
              strcpy_P(buffer, (char*)pgm_read_word(&(string_table1[i])));
              client.print( buffer );
            }
            client.print(mac[0],HEX);
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table1[3])));
            client.print( buffer );
            client.print(mac[1],HEX);
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table1[4])));
            client.print( buffer );
            client.print(mac[2],HEX);
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table1[5])));
            client.print( buffer );
            client.print(mac[3],HEX);
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table1[6])));
            client.print( buffer );
            client.print(mac[4],HEX);
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table1[7])));
            client.print( buffer );
            client.print(mac[5],HEX);
            for (int i = 0; i < 4; i++) {
              strcpy_P(buffer, (char*)pgm_read_word(&(string_table2[i])));
              client.print( buffer );
            }
            client.print(ip[0],DEC);
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table2[4])));
            client.print( buffer );
            client.print(ip[1],DEC);
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table2[5])));
            client.print( buffer );
            client.print(ip[2],DEC);
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table2[6])));
            client.print( buffer );
            client.print(ip[3],DEC);
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table3[0])));
            client.print( buffer );
            client.print(subnet[0],DEC);
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table3[1])));
            client.print( buffer );
            client.print(subnet[1],DEC);
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table3[2])));
            client.print( buffer );
            client.print(subnet[2],DEC);
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table3[3])));
            client.print( buffer );
            client.print(subnet[3],DEC);
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table4[0])));
            client.print( buffer );
            client.print(gateway[0],DEC);
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table4[1])));
            client.print( buffer );
            client.print(gateway[1],DEC);
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table4[2])));
            client.print( buffer );
            client.print(gateway[2],DEC);
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table4[3])));
            client.print( buffer );
            client.print(gateway[3],DEC);
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table4[4])));
            client.print( buffer );
            for (int i = 0; i < 7; i++) {
              strcpy_P(buffer, (char*)pgm_read_word(&(string_table5[i])));
              client.print( buffer );
            }
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table4[5])));
            client.print( buffer );
            break;
          } // Fim do setup
          else {
            char c = client.read();
            // fill url the buffer
            if (c != '\n' && c != '\r' && index < BUFSIZE) { // Reads until either an eol character is reached or the buffer is full
              clientline[index++] = c;
              nchars++;
              continue;
            }
        
            // Flush any remaining bytes from the client buffer
            client.flush();
            
            // convert clientline into a proper
          
            // string for further processing
            urlString = String(clientline);
            
            // we're only interested in the first part...
            urlString = urlString.substring(urlString.indexOf('/'), urlString.indexOf(' ', urlString.indexOf('/')));
            
            // put what's left of the URL back in client line
            urlString.toUpperCase();
            
            urlString.toCharArray(clientline, BUFSIZE);

            //  get the first two parameters
            char *pin = strtok(clientline,"/");
            char *value = strtok(NULL,"/");
            
            //  this is where we actually *do something*!
            String outValue = String();
            String jsonOut = String();
            
            if(pin != NULL) {
              if(value != NULL) {
                //  select the pin
                int selectedPin = atoi(pin);
                
                //  set the pin for output
                pinMode(selectedPin, OUTPUT);
                
                //  determine digital or analog (PWM)
                if(strncmp(value, "HIGH", 4) == 0 || strncmp(value, "LOW", 3) == 0) {
                  if(strncmp(value, "HIGH", 4) == 0) {
                    digitalWrite(selectedPin, HIGH);
                  }
                  
                  if(strncmp(value, "LOW", 3) == 0) {
                    digitalWrite(selectedPin, LOW);
                  }
                }
                else {
                  //  get numeric value
                  int selectedValue = atoi(value);
                  analogWrite(selectedPin, selectedValue);
                }
                //  return status
                client.println("HTTP/1.1 200 OK");
                client.println("Content-Type: text/html");
                client.println();
              }
              else {
                //  determine analog or digital
                if(pin[0] == 'a' || pin[0] == 'A') {
                  //  analog
                  int selectedPin = pin[1] - '0';
                  outValue = String(analogRead(selectedPin));
                }
                else if(pin[0] != NULL) {
                  //  digital
                  int selectedPin = pin[0] - '0';
                  pinMode(selectedPin, INPUT);
                  
                  int inValue = digitalRead(selectedPin);
                  
                  if(inValue == 0) {
                    outValue = "LOW";
                  }
                  
                  if(inValue == 1) {
                    outValue = "HIGH";
                  }
                }
                //  assemble the json output
                jsonOut += "{\"";
                jsonOut += pin;
                jsonOut += "\":\"";
                jsonOut += outValue;
                jsonOut += "\"}";
                //  return value with wildcarded Cross-origin policy
                client.println("HTTP/1.1 200 OK");
                client.println("Content-Type: text/html");
                client.println("Access-Control-Allow-Origin: *");
                client.println();
                client.println(jsonOut);
              }
            }
            else {
              //  error
              client.println("HTTP/1.1 404 Not Found");
              client.println("Content-Type: text/html");
              client.println();
            }
          }
        }
        break;
      }
    }
    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    //client.stop();
    client.stop();
    while(client.status() != 0) {
      delay(5);
    }
  }
}