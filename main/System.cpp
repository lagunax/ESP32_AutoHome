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
		//FTP_Server = new Service::FTP::FtpServer();
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
		this->BTbegin("ESP32-Autohome"); //Bluetooth device name
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
#include "../../wf.pass"; //contains initialization for ssid and password for wifi
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
		upng_s_rgba32b *pixel;
		upng_s_rgb24b *pixel_src = InitColorR8G8B8();
		upng_s_rgb16b *pixel_dst = InitColorR5G6B5();
		uint32_t tmp;

		upng = upng_new_from_file("/t.png");
		if (upng_get_error(upng) == UPNG_ENOTFOUND)
		{
			Serial.print("file not found");
		}
		upng_decode(upng);
		//Serial.printf("size = %i", (int) upng_get_size(upng));

		buffer = malloc((unsigned long int)upng_get_size(upng));
		buffer=(void *)upng_get_buffer(upng);

//		char ttt;

//		for(char bbb=0; bbb<7; bbb++){
//			ttt=*((char *)(buffer+bbb));
//			Serial.printf("buffer[0...7]=%i",(int)ttt);
//		}
//
//		Serial.printf("buffer size: %i",(int) upng_get_size(upng));
		if (upng_get_error(upng) != UPNG_EOK) {
			//printf("error: %u %u\n", upng_get_error(upng), upng_get_error_line(upng));
			Serial.printf("error: %u %u\n", upng_get_error(upng), upng_get_error_line(upng));
			//return 0;
		}else{

		width=upng_get_width(upng);
		height=upng_get_height(upng);
		//uint bpp=upng_get_bitdepth(upng);
		uint bpp = upng_get_bpp(upng);

		pixel = (upng_s_rgba32b *)malloc((unsigned long int) ((bpp+7)/8));
		//buffer = malloc((unsigned long int)upng_get_size(upng));
		//buffer=(void *)upng_get_buffer(upng);
		//tft->printf("bpp=%n", (int*) &bpp);

for (char xx=0; xx<width; xx++){for (char yy=0; yy<height; yy++){
//	Serial.printf("\n--- x = %i, y = %i --- \n",(int)xx,(int)yy);
	upng_GetPixel((void *)pixel, upng, (int)xx, (int)yy);
//	Serial.printf("\n r=%i, g=%i, b=%i, a=%i", int (pixel->rgb.r), int (pixel->rgb.g), int (pixel->rgb.b), int(pixel->alpha));
	*pixel_src=pixel->rgb;
	upng_rgb24bto16b(pixel_dst, pixel_src);
//	Serial.printf("\n r=%i, g=%i, b=%i", int (pixel_dst->r), int (pixel_dst->g), int (pixel_dst->b));
	upng_rgb16btouint32(&tmp, pixel_dst);
	tft->drawPixel(xx,yy,tmp);
}}


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
		case 5: //UPNG_PAL
			break;
		default: //0 or any else
			break;
			//return 1;
		}

		free(pixel);
		//free(buffer); do not need to free here, upng_free removes buffer

		upng_free(upng);
		buffer=0;
		}

	}



