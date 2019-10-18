#/bin/bash

ln -sd $1 ./components/arduino/libraries/ESP32_SPIDisplay
cd ./components/arduino
./libraries/ESP32_SPIDisplay/install.sh
