/*********************************.
 * WIFI server for Feather S2
 */

// WIFI libraries
// See WifiAccessPoint.ino in examples
#include <WiFi.h>
#include <NetworkClient.h>
#include <WiFiAP.h>


// Set these to your desired credentials.
const char *ssid = "emileAP";
const char *password = "strawberry";

NetworkServer server(80);

//#define SerialDebug
 
void setup() {
  // put your setup code here, to run once:

  #ifdef SerialDebug
    Serial.begin(9600);
  
    while (!Serial){
      delay(10);
    }
  #endif
  
  Serial1.begin(9600);

  // Set RX to pin 38 on board
  Serial1.setPins(38, 39);

  // You can remove the password parameter if you want the AP to be open.
  // a valid password must have more than 7 characters
  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while (1);
  }
  IPAddress myIP = WiFi.softAPIP();
  #ifdef SerialDebug
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  #endif
  server.begin();
}

struct SerialBuffer{
  char data[16];
  int len;
} buffer;

bool read(){
  if (Serial1.available()){
    int c = Serial1.read();

    if (c == 10){
      return true;
    }
    else{

      // Get all valid hex values
      if (c >= '0' && c <= '9'){
        buffer.data[buffer.len] = c;
        buffer.len = (buffer.len + 1) % 16; // quick safety precaution so that missing a new line doesn't crash
      }
      if (c >= 'A' && c <= 'F'){
        buffer.data[buffer.len] = c;
        buffer.len = (buffer.len + 1) % 16; // quick safety precaution so that missing a new line doesn't crash
      }
    }
  }
  return false;
}

void print_buffer(){
  #ifdef SerialDebug
  for (int i = 0; i < buffer.len; i++){
    Serial.print(buffer.data[i]);
  }
  Serial.print("\n");
  #endif
}

int iresult(){
  int result = 0;
  for (int i = 0; i < buffer.len; i++){
    int v = buffer.data[i] - '0';
    if (buffer.data[i] >= 'A'){
      v = (buffer.data[i] - 'A') + 10;
    }
    result += (v << ((buffer.len - i - 1) * 4));
  }
  //clip
  if (result > 4095){
    result = 4095;
  }
  if (result < 0){
    result = 0;
  }
  return result; // output range is 0-255
}

int loudness = 0;

void client_reply(NetworkClient& client){
  // Give HTTP reply to client
  //Serial.println("New Client.");  // print a message out the serial port
  String currentLine = "";        // make a String to hold incoming data from the client
  while (client.connected()) {    // loop while the client's connected
    if (client.available()) {     // if there's bytes to read from the client,
      char c = client.read();     // read a byte, then
      //Serial.write(c);            // print it out the serial monitor
      if (c == '\n') {            // if the byte is a newline character

        // if the current line is blank, you got two newline characters in a row.
        // that's the end of the client HTTP request, so send a response:
        if (currentLine.length() == 0) {
          // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
          // and a content-type so the client knows what's coming, then a blank line:
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println();

          // Reply with the loudness
          client.print("V:");
          client.print(loudness >> 2);

          // The HTTP response ends with another blank line:
          client.println();
          // break out of the while loop:
          break;
        } else {  // if you got a newline, then clear currentLine:
          currentLine = "";
        }
      } else if (c != '\r') {  // if you got anything else but a carriage return character,
        currentLine += c;      // add it to the end of the currentLine
      }
    }
  }
  client.stop();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (read()){
    // Get result of uart
    loudness = (iresult() >> 1) + (loudness >> 1);\
    buffer.len = 0;
  }
  NetworkClient client = server.accept();  // listen for incoming clients

  if (client){
    client_reply(client);
  }
}
