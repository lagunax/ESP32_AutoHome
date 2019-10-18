/*
 * System.h
 *
 *  Created on: 13 сент. 2019 г.
 *      Author: producer
 */


#define SERVICE_FTP
#define SERVICE_WEB

#ifndef MAIN_SYSTEM_H_
	#define MAIN_SYSTEM_H_

	//#define FTP_DEBUG

	#include "freertos/FreeRTOS.h"
	#include "freertos/task.h"
	#include "esp_task_wdt.h"
	#include <Arduino.h>
	#include <FS.h>
	#include <BluetoothSerial.h>
	//#include <WiFi.h>
	//#include <WiFiClient.h>
	#include <WiFiClientSecure.h>
//	#include <ESPmDNS.h>
	//#include "tftprint.cpp"
	//#include "Free_Fonts.cpp"
//	#include "../components/arduino/libraries/TFT_eSPI/ESP32-SPIDisplay.h" // Graphics and font library
#include <ESP32_SPIDisplay.h>
#include <Extensions/Graphics.h>
#include <Extensions/Touch.h>
//	#include <SPI.h>
//	#include <ESPmDNS.h>

	#ifdef SERVICE_FTP
		#include "FtpServer.h"
	#endif

	#ifdef SERVICE_WEB
		#include "WebService.h"
	#endif

	#define TFT_GREY 0x5AEB // New colour

#include "upng.h"

	class System {


		private:
			#ifdef _BLUETOOTH_SERIAL_H_
				#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
					#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
				#endif
				BluetoothSerial SerialBT;
			#endif

			#ifdef WiFiClientSecure_h
				WiFiClientSecure WiFi_client;
			#endif
				upng_t* upng;

		public:
			#ifdef _BLUETOOTH_SERIAL_H_
				#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
					#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
				#endif

				void BTInit();

				//Device::Display::Screen *tft;  // Invoke library

				bool BTbegin(String);
				int BTavailable(void);
				int BTpeek(void);
				bool BThasClient(void);
				int BTread(void);
				size_t BTwrite(uint8_t c);
				size_t BTwrite(const uint8_t *buffer, size_t size);
				void BTflush();
				void BTend(void);
				esp_err_t BTregister_callback(esp_spp_cb_t * callback);

				//#include "../modules/Bluetooth.h"
			#endif

			#ifdef WiFiClientSecure_h
				void WFAPInit();
				//bool WFScan();
				//bool WFConnect(const char*, const char *, int32_t , const uint8_t*, bool );//const char* ssid, const char *passphrase, int32_t channel, const uint8_t* bssid, bool connect
				//wl_status_t WFStatus();
				IPAddress WFlocalIP();
			//	#include "../modules/WiFiClient.h"
			#endif


			#ifdef WEBSERVER_H
				//#include "../modules/HTTP_Server.h

			#endif

			#ifdef FTP_SERVERESP_H
				//Service::FTP::FtpServer *FTP_Server;
			#endif
				void DrawPng();

		System();
		virtual ~System();

		//void InitFS();



	};

#endif /* MAIN_SYSTEM_H_ */
