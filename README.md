# Conde_Home_Automation

Home Automation Engine for my house. Hosted on a Raspberry PI 3.

## Inputs (IoT devices)

(existing, but not yes implemented in the unified code)
- House Power Meter real time statistics (Meter is outdor. Datas are grabbed serialy through a PIC microcontroller, and sent OTA using Xbee to a Raspberry)
- Comming : Indoor&Outdoor Temperature / Humidity / Pressure

## Inputs (for presence detection)

- Wifi registration of my iPhone on my home Cisco WLAN Controller (regular check)
- Wifi events on my controller (Real Time SNMP events)
- iOS Geofencing app (Locative)

## Outputs

- Light Control (433Mhz RF power socket - currently using homemade 433Mhz RF trasmitter, moving to a TI CC1101 based transceiver)
- Electric Heaters (868Mhz control of my Delta Dore enabled heaters allowing Confort/Eco/HG/Off commands, using CC1101 transceiver)
