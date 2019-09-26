/*
 * System.h
 *
 *  Created on: 13 сент. 2019 г.
 *      Author: producer
 */

#ifndef MAIN_SYSTEM_H_
#define MAIN_SYSTEM_H_

#include <Arduino.h>
#include <FS.h>
#include <BluetoothSerial.h>
//#include <WiFi.h>
//#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <ESPmDNS.h>

//WebServer Web_Server(80);
//Main Variables

			bool WEB_is_authentified();
			void WEB_handleLogin();
			void WEB_handleRoot();
			void WEB_handleNotFound();

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

		#ifdef WEBSERVER_H
			//WebServer Web_Server(80);
		#endif


	public:
		#ifdef _BLUETOOTH_SERIAL_H_
			#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
				#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
			#endif

			void BTInit();

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
/*
			bool WEB_is_authentified();
			void WEB_handleLogin();
			void WEB_handleRoot();
			void WEB_handleNotFound();*/
			WebServer *Web_Server;

			void WEB_Start();

		#endif


	System();
	virtual ~System();

	//void InitFS();



};

#endif /* MAIN_SYSTEM_H_ */
