# UNMAINTAINED: gpio2mqttd

This project was quickly thrown together to publish MQTT messages on GPIO events.
Having MQTT code around from vallumd made it rather easy to create.

Unfortunately libgpiod dropped the contextless interfaces in 
9b6e6d268671 ("ctxless: drop all context-less interfaces"), and due to that,
this program is no longer functional. Fixing gpio2mqttd to work with newer
libgpiod releases seems non-trivial, so I will retire my Raspberry Pi Zero W
motion sensor and replace it with an ESP32-C3 running esphome.
