/*
   I2S stereo microphone (input) example
   Run using the Arduino Serial Plotter to see waveform.
   Released to the Public Domain by Earle F. Philhower, III

   For the Google AIY Voice Hat Microphone daughterboard, part
   of the Raspberry Pi AIY cardboard box, the I2S stereo pinout
   looking at the board top with the RPI logo on the left hand
   side:
            +--   ------------------------------------  --+
   left RPI | (1) GND (2) DIN (3) BCLK (4) LRCLK (5) 3.3V | AIY right
       logo +---------------------------------------------+ logo

   For an Adfruit I2S MEMS microphone (https://www.adafruit.com/product/3421),
   connect the pins as follows:

      DOUT -> GPIO0
      BCLK <- GPIO1
      LRCL <- GPIO2  # LRCLK = BCLK + 1
      GND <-> GND
      3V  <-> 3V3OUT

   The other idiosyncrasy of most modern MEMS microphones is that they
   require a minimum clock rate to wake up. For example, the SPH0645
   microphone needs at least 1MHz.
*/

#include <I2S.h>

I2S i2s(INPUT);

const float max_value = 1024.0;

void setup() {

  // TX pin on the feather
  Serial1.setTX(0);

  // Serial seems to crash
  // if above a certain baud rate
  // Must be something with how the I2S library
  // is using some resource
  Serial.begin(9600); //USB
  Serial1.begin(9600); //UART on gpio

  pinMode(LED_BUILTIN, OUTPUT);

  // DOUT - D5
  // BCLK - D6
  // LRCLK- D9
  i2s.setDATA(7);
  i2s.setBCLK(8); // Note: LRCLK = BCLK + 1

  // Use these values to get the SPH0645 up
  // NOTE: The following values are known to work with the Adafruit microphone:
  i2s.setBitsPerSample(32);
  i2s.setFrequency(16000);
  i2s.begin();

  int i = 0;

  float baseline = 0.0;
  float intensity = 0.0;
  float minimum = 0.0;
  float maximum = 4000.0;

  float value = 0.0;
  
  while (1) {
    int32_t l, r;

    // Read in sample
    i2s.read32(&l, &r);

    // Add sample / 2 with very basic low pass filter
    intensity = (intensity * 0.5) + (float(abs(l) >> 17));

    if (i == 100){
      // After n samples, give intensity

      // Keep track of average baseline
      baseline = (baseline * 0.99) + (intensity * 0.01);

      // This is the intensity above the average baseline
      float result = abs(intensity - baseline);

      value = map(result, minimum, maximum, 0.0, max_value);


      // Auto scale large values
      if (result > 2000.0){
        if (result > maximum){
          maximum += 100.0;
          value = max_value;
        }
        else{
          maximum -= 1.0;
        }
      }
        

      if (result < 0.0){
        value = 0.0;
      }

      // Print to USB Serial
      Serial.println(int(value));

      // Print to TX port
      Serial1.println(int(value), HEX);

      // Show intensity on LED
      analogWrite(LED_BUILTIN, value / 4.0);
      
      i = 0; 
    }
    else{
      i++;
    }
    
  }
}

void loop() {
  /* Nothing here */
}
