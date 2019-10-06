/*
 * System.cpp
 *
 *  Created on: 13 сент. 2019 г.
 *      Author: producer
 */

#include "System.h"

System::System() {
	// TODO Auto-generated constructor stub
	#ifdef FTP_SERVERESP_H
		FTP_Server = new Service::FTP::FtpServer();
	#endif
		tft = new TFT_eSPI();

}

System::~System() {
	// TODO Auto-generated destructor stub
    delete(tft);
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


#endif


	void System::DrawPng(){
		unsigned width, height;
		void *buffer;
		void *pixel;
		upng = upng_new_from_file("/t.png");
		if (upng_get_error(upng) == UPNG_ENOTFOUND)
		{
			Serial.print("file not found");
		}
		upng_decode(upng);
		//Serial.printf("size = %i", (int) upng_get_size(upng));

		buffer = malloc((unsigned long int)upng_get_size(upng));
		buffer=(void *)upng_get_buffer(upng);

/*		char ttt;

		for(char bbb=0; bbb<7; bbb++){
			ttt=*((char *)(buffer+bbb));
			Serial.printf("buffer[0...7]=%i",(int)ttt);
		}
*/
		Serial.printf("buffer size: %i",(int) upng_get_size(upng));
		if (upng_get_error(upng) != UPNG_EOK) {
			//printf("error: %u %u\n", upng_get_error(upng), upng_get_error_line(upng));
			Serial.printf("error: %u %u\n", upng_get_error(upng), upng_get_error_line(upng));
			//return 0;
		}else{

		width=upng_get_width(upng);
		height=upng_get_height(upng);
		//uint bpp=upng_get_bitdepth(upng);
		uint bpp = upng_get_bpp(upng);

		pixel = malloc((unsigned long int) ((bpp+7)/8));
		//buffer = malloc((unsigned long int)upng_get_size(upng));
		//buffer=(void *)upng_get_buffer(upng);
		tft->printf("bpp=%n", (int*) &bpp);


		switch (upng_get_components(upng)) {
		case 1: //UPNG_LUM
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, upng_get_width(upng), upng_get_height(upng), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, upng_get_buffer(upng));
			break;
		case 2://UPNG_LUMA
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, upng_get_width(upng), upng_get_height(upng), 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, upng_get_buffer(upng));
			break;
		case 3://UPNG_RGB
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, upng_get_width(upng), upng_get_height(upng), 0, GL_RGB, GL_UNSIGNED_BYTE, upng_get_buffer(upng));
			break;
		case 4://UPNG_RGBA
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, upng_get_width(upng), upng_get_height(upng), 0, GL_RGBA, GL_UNSIGNED_BYTE, upng_get_buffer(upng));
			break;
		default: //0 or any else
			break;
			//return 1;
		}

		free(pixel);
		free(buffer);

		upng_free(upng);
		}
		for (char xx=0; xx<0xF; xx++){for (char yy=0; yy<0xF; yy++){

			tft->drawPixel(xx,yy,(uint)((xx<<12)*0+(yy<<24)));
		}}

	}



