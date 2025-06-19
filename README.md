# Audio Reactive Sculpture

Collaboration with sculptural artist. This project runs on three boards

- An RP2040 which takes I2S data from a microphone. This is placed close to speaker/main event. The RP2040 processes the audio into a loudness number. This is essentially the linear loudness above an average baseline.
- A FeatherS2 which acts as a wireless server. This server reads the loudness from the RP2040 and await client requests to send the loudness over.
- A remote FeatherS2 is located in the sculpture which requests data from the server. This lights a strand of neopixels accordingly.

While possible to do with the server running both audio processing and client requests, I had limited time and hardware to complete this, so it's going to stay separate.

