ARDUINO_DIR            = /usr/share/arduino
ARDUINO_LIBS           = DHTlib SdFat Wire Ethernet SPI
MCU                    = atmega328p
F_CPU                  = 16000000
ARDUINO_PORT           = /dev/ttyACM0
AVRDUDE_ARD_BAUDRATE   = 115200
AVRDUDE_ARD_PROGRAMMER = arduino
PLOT_FILE              = "TEMPHUMI.TXT"

include /usr/share/arduino/Arduino.mk

plot_graph:
	cat plot.gplt | gnuplot

