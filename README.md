# I'm Still Here
A generic Esp8266 telling the world through a web service ping that it is still alive and sleeps between pings. This is also the first Esp8266 project for me so it includes a get started tutorial so I can remember the next time.

## Get started with WiFiWebServerBlink
1. Hook up according to `WiFiBlink_*.jpg`. 
1. Use e.g. a "FTDI Friend" and not some cheap FTDI clones, I haven't got it to work with the ones I had. 
1. My FTDI Friend has 5V VCC but 3.3V signal levels so power the Esp8266 from a 3.3V power supply.
1. Make a copy of `WiFiConfig_template.h` called `WiFiConfig.h` with your credentials.
1. Enable Esp8266 board definitions from https://github.com/esp8266/Arduino in a 1.6.9+ Arduino IDE.
1. Choose Generic Esp8266 and connect your FTDI Friend and select it's port.
1. Make sure the Esp's GPIO0 is pulled down to ground and CH_PD is pulled up to VCC.
1. Sometimes I need to pull out and put back the Esps VCC to restart it at this point, sometimes it is fine.
1. Now I can upload a sketch from Arduino IDE.
1. Open the serial monitor to see connection status and IP, go to `http://192.168.1.6/gpio/1` to turn on and `http://192.168.1.6/gpio/0` to turn off.
1. An internet of thing thing!
