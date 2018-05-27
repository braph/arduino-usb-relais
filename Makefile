ARDUINO_VERSION = 116

BOARD_TAG    ?= pro
BOARD_SUB    ?= 16MHzatmega328

ARCHITECTURE = avr

AVRDUDE = /bin/avrdude
AVRDUDE_CONF = /etc/avrdude.conf

USER_LIB_PATH = $(HOME)/Arduino/libraries
ARDUINO_LIBS  = 

CFLAGS_STD   = -std=c11
CXXFLAGS_STD = -std=c++11

include /usr/share/arduino/Arduino.mk

