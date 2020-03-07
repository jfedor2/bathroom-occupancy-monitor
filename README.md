# Bathroom occupancy monitor

This is code for a system that shows the current free/occupied status of bathroom stalls on a website.

`arduino` directory contains the code running on the ESP8266 that listens to the 433 MHz door sensors.

In the `firebase` directory there's code for the cloud function that the ESP8266 talks to and the website showing the current state of the bathroom.

More details here:

https://blog.jfedor.org/2020/03/bathroom-occupancy-monitor.html
