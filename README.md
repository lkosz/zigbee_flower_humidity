# About the project

The goal was to check if summer flowers can be grown in the middle of a winter with artificial light. It was hard to determine how much of water they need, it was approx. 2.5 litres for all 3 flowerpots daily, so decided to make a battery powere zigbee soil humidity sensor.


# Devices

- ESP32 H2 devkit (C6 version was more power consuming)
- DFRobot Gravity capacitive soil moisture sensor
- 2x NiMH AA batteries
- Pololu 3.3V boost voltage converter
- 3d printed case
- 1P button to reset the device


# Already existing services used

- Zigbee network managed by zigbee2mqtt1
- python service doing automation of zigbee devices, receiving data from sensors and storing it into logs
- logs from sensors stored in ELK stack, visualized in Grafana

- and I used chatgpt v4 to help in writing of C++ code for zigbee sensor


# Flowers:

- Centaurea cyanus L.
- Tagetes patula L.
- Matthiola longipetala


# Configuration

- put together all the hardware, solder, remember about decoupling capacitors
- flash ESP32 board
- connect it to Zigbee network
- configure reporting in zigbee2mqtt

# Results and conclusions

- moisture sensor improved care over plants. The main problem was that Centaurea cyanus were using a lot of water, when humidity dropped below exact level, plants were getting stressed and lower leaves were starting to be softer
- using graphs in grafana made it easy and straightforward

- usage of battery power was not a good idea - moisture sensor had big drift in measurements (even ~10 percent points) which you can see below:

OBRAZEK

- flowers had too few light and that's why they were so long (I used 2x professional Verticana lightning 35W LED, 2x Ikea Ledare 1800lm LED light bulbs and 3x Ikea Trådfri 800lm RGB Zigbee light bulbs)
- I had to make gentle "sunrise" and "sunset" using zigbee dimmer on Verticana lights because Centaurea cyanus were getting heavy stress. Matthiola longipetala needed long "evening" using orange-red light from Trådfri light bulbs to smell strong like in the summer
- all in all - flowers were nice. I seeded them in early November 2024 and they last until June 2025

OBRAZEK

- usage of chatgpt was a disaster, I wasted a lot of time on debugging. It was a first time when I was dealing with zigbee library and low-level zigbee stack in general, chatgpt probably as well haha. As a result all of the suggestions were horrible and if I didn't use it, I would write the code faster.
- next flowers I seeded in June and I used more Verticana lamps, window shades were also opening automatically during sunny days (the window in on south-east side), sensors were rewriten to python code on raspberry pi pico, data transmission over wifi, power supply is not battery anymore. WiFi transmission (using REST protocol to python service) is not as stable as Zigbee, but orders of magnitude simpler.
