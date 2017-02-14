# clock
clock with ntp synk

# libs
 1 "Rtc by Makuna"
 https://github.com/Makuna/Rtc.git
 2.1
 https://github.com/adafruit/Adafruit-GFX-Library.git
 2.2
 https://github.com/markruys/arduino-Max72xxPanel.git
 
# referenses
schematic: https://circuits.io/circuits/3798057-net-clock-led

# esp8266 reference
http://www.kloppenborg.net/blog/microcontrollers/2016/08/02/getting-started-with-the-esp8266

Pin | | | | | comment
---:| --- | --- | --- | ---| ---
1|	RST|
2|ADC|TOUT
3|CHIP_EN|CH_PD
4|GPIO16|XPD_DCDC
5|GPIO14|MTMS|HSPI_CLK
6|GPIO12|MTDI|HSPI_MISO
7|GPIO13|MTCK|HSPI_MOSI|U0CTS
8|VCC
15|GND
16|GPIO15|MTDO|HSPI_CS|U0RTS
17|GPIO2|U1TXD
18|GPIO0|SPI_CS2
19|GPIO4|SDA
20|GPIO5|SCL
21|GPIO3|U0RXD
22|GPIO1|U0TXD|SPI_CS1



https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet#lists
