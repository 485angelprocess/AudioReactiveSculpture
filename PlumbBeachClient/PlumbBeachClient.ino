/*
 *  This sketch sends a message to a TCP server
 *
 */

#include <WiFi.h>
#include <WiFiMulti.h>

WiFiMulti WiFiMulti;

#include <Adafruit_NeoPixel.h>

#define NEOPIN 1
#define NUMPIXELS 25

Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIN, NEO_GRB + NEO_KHZ800);

float wavetable[255];

void setup() {

  pixels.begin();
  pixels.clear();
  pixels.setPixelColor(0, pixels.Color(0, 150, 0));

  pixels.show();

  for (int i = 0; i < 255; i++){
    wavetable[i] = (sin(2.0 * 3.14 * i / 255.0)/2.0) + 0.5;
  }


  // Start serial
  Serial.begin(115200);
  delay(10);

  // Setup wifi
  // We start by connecting to a WiFi network
  WiFiMulti.addAP("emileAP", "strawberry");

  Serial.println();
  Serial.println();
  Serial.print("Waiting for WiFi... ");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //delay(500);
}

bool parseString(String& msg, int& value){
  msg.trim(); // remove whitespace
  int l = msg.length();

  // not enough chars
  if (l <= 2){
    return false;
  }

  // Get substring
  String  sub = msg.substring(2, l);
  value = sub.toInt();
  return true; 
}

float idle_amplitude;
float active_amplitude;

void neopixel_update(){
  pixels.clear();
  float a = 10.0 * active_amplitude;

  if (a > 255.0){
    a = 255.0;
  }
  if (a < 0.0){
    a = 0.0;
  }
  
  int offset = (millis() / 50) % 255;

  for (int i = 0; i < 255; i++){
    float w = wavetable[(i + offset) % 256];
    pixels.setPixelColor(i, pixels.Color(0, 0, int(125.0 * a * w)));
  }
  pixels.show();
}

void loop() {
  //    const uint16_t port = 80;
  //    const char * host = "192.168.1.1"; // ip or dns
  const uint16_t port = 80;
  const char *host = "192.168.4.1";  // ip or dns

  //Serial.print("Connecting to ");
  //Serial.println(host);

  // Use NetworkClient class to create TCP connections
  NetworkClient client;

  if (!client.connect(host, port)) {
    Serial.println("Connection failed.");
    Serial.println("Waiting 5 seconds before retrying...");
    delay(5000);
    return;
  }

  // This will send a request to the server
  //uncomment this line to send an arbitrary string to the server
  //client.print("Send this data to the server");
  //uncomment this line to send a basic document request to the server
  client.print("GET /index.html HTTP/1.1\n\n");

  int maxloops = 0;

  //wait for the server's reply to become available
  while (!client.available() && maxloops < 50) {
    maxloops++;

    // Auto run neopixels with no input here
    // run based on ms counter
    neopixel_update();
    delay(1);  //delay 1 msec
  }
  if (client.available() > 0) {
    //read back one line from the server
    String line = client.readStringUntil('\r');
    line = client.readStringUntil('\r');
    line = client.readStringUntil('\r');
    line = client.readStringUntil('\r');

    int v = 0;
    if (parseString(line, v)){
      active_amplitude = v;
      Serial.println(v);
    }
    else{
      Serial.print("Parsing Error");
      Serial.println(line);
    }
  } else {
    Serial.println("client.available() timed out ");
  }

  //Serial.println("Closing connection.");
  client.stop();
  neopixel_update();

  //Serial.println("Waiting 5 seconds before restarting...");
  //delay(500);
}
