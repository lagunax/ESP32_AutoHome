deps_config := \
	/usr/media/Programming/esp/esp-idf/components/app_trace/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/aws_iot/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/bt/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/driver/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/esp32/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/esp_adc_cal/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/esp_event/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/esp_http_client/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/esp_http_server/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/ethernet/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/fatfs/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/freemodbus/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/freertos/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/heap/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/libsodium/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/log/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/lwip/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/mbedtls/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/mdns/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/mqtt/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/nvs_flash/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/openssl/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/pthread/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/spi_flash/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/spiffs/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/tcpip_adapter/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/vfs/Kconfig \
	/usr/media/Programming/esp/esp-idf/components/wear_levelling/Kconfig \
	/usr/media/Programming/esp/Projects/GIT/ESP32-AutoHome/components/arduino/Kconfig.projbuild \
	/usr/media/Programming/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/usr/media/Programming/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/usr/media/Programming/esp/Projects/GIT/ESP32-AutoHome/main/Kconfig.projbuild \
	/usr/media/Programming/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/usr/media/Programming/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)

ifneq "$(IDF_CMAKE)" "n"
include/config/auto.conf: FORCE
endif

$(deps_config): ;
