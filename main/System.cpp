/*
 * System.cpp
 *
 *  Created on: 13 сент. 2019 г.
 *      Author: producer
 */

#include "System.h"

System::System() {
	// TODO Auto-generated constructor stub
	#ifdef WEBSERVER_H
		Web_Server = new WebServer(80);
	#endif

	#ifdef FTP_SERVERESP_H
		FTP_Server = new FtpServer();
		ftpServer = new WiFiServer( FTP_CTRL_PORT );
		dataServer = new WiFiServer( FTP_DATA_PORT_PASV );
	#endif
}

System::~System() {
	// TODO Auto-generated destructor stub
}

#ifdef _BLUETOOTH_SERIAL_H_
//#include "../modules/Bluetooth.cpp"
	void System::BTInit(){
		this->BTbegin("ESP32test"); //Bluetooth device name
		Serial.println("[Bluetooth] started!");
	}

	bool System::BTbegin(String localName=String()){//String localName){//
		return SerialBT.begin(localName);
	}

	int System::BTavailable(void){
		return SerialBT.available();
	}

	int System::BTpeek(void){
		return SerialBT.peek();
	}

	bool System::BThasClient(void){
		return SerialBT.hasClient();
	}

	int System::BTread(void){
		return SerialBT.read();
	}

	size_t System::BTwrite(uint8_t c){
		return SerialBT.write(c);
	}

	size_t System::BTwrite(const uint8_t *buffer, size_t size){
		return SerialBT.write(buffer, size);
	}

	void System::BTflush(){
		SerialBT.flush();
	}

	void System::BTend(void){
		SerialBT.end();
	}

	esp_err_t System::BTregister_callback(esp_spp_cb_t * callback){
		return SerialBT.register_callback(callback);
	}

#endif




#ifdef WiFiClientSecure_h
//#include "../modules/WiFiClient.cpp"

	void System::WFAPInit(){
		const char* ssid     = "****";     // your network SSID (name of wifi network)
		const char* password = "****"; // your network password

		//const char*  pskIdent = "Client_identity"; // PSK identity (sometimes called key hint)
		//const char*  psKey = "ad2019cd"; // PSK Key (must be hex string without 0x)

		//int connection_tries = 10; // Number of tries to connect (if 0 then infinity)

		WiFi.begin(ssid, password);

		// attempt to connect to Wifi network:
		while (WiFi.status() != WL_CONNECTED) {
			Serial.print(".");
			#ifdef _BLUETOOTH_SERIAL_H_
			this->BTwrite((const unsigned char *) ".",1);
			#endif
			// wait 1 second for re-trying
			delay(1000);
		}
		#ifdef _BLUETOOTH_SERIAL_H_
    			this->BTwrite((const unsigned char *) "WiFiConnected", 13);
		#endif

	}

	IPAddress System::WFlocalIP(){
		return WiFi.localIP();
	}

	//void System::FTPhandleClient(){
		//this->FTP->handleFTP();
	//}
#endif





