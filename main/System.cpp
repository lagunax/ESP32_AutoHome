/*
 * System.cpp
 *
 *  Created on: 13 сент. 2019 г.
 *      Author: producer
 */

#include "System.h"
//using Device::Display::Graphics::Graph;
System::System() {
	// TODO Auto-generated constructor stub
#ifdef FTP_SERVERESP_H
	//FTP_Server = new Service::FTP::FtpServer();
	upng = 0;
#endif
	Device::Display::init();
	Graph=new Device::Display::Graphics::Graphics();
	Graph->fillScreen(TFT_BLUE);

}

System::~System() {
	// TODO Auto-generated destructor stub
	//delete (tft);
//	Device::Display::remove();
}

#ifdef _BLUETOOTH_SERIAL_H_
//#include "../modules/Bluetooth.cpp"
void System::BTInit() {
	this->BTbegin("ESP32-Autohome"); //Bluetooth device name
	Serial.println("[Bluetooth] started!");
}

bool System::BTbegin(String localName = String()) { //String localName){//
	return SerialBT.begin(localName);
}

int System::BTavailable(void) {
	return SerialBT.available();
}

int System::BTpeek(void) {
	return SerialBT.peek();
}

bool System::BThasClient(void) {
	return SerialBT.hasClient();
}

int System::BTread(void) {
	return SerialBT.read();
}

size_t System::BTwrite(uint8_t c) {
	return SerialBT.write(c);
}

size_t System::BTwrite(const uint8_t *buffer, size_t size) {
	return SerialBT.write(buffer, size);
}

void System::BTflush() {
	SerialBT.flush();
}

void System::BTend(void) {
	SerialBT.end();
}

esp_err_t System::BTregister_callback(esp_spp_cb_t *callback) {
	return SerialBT.register_callback(callback);
}

#endif

#ifdef WiFiClientSecure_h
//#include "../modules/WiFiClient.cpp"

void System::WFAPInit() {
#include "../../wf.pass"

	//contains initialization for ssid and password for wifi
	WiFi.begin(ssid, password);

	// attempt to connect to Wifi network:
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
#ifdef _BLUETOOTH_SERIAL_H_
		this->BTwrite((const unsigned char*) ".", 1);
#endif
		// wait 1 second for re-trying
		delay(1000);
	}
#ifdef _BLUETOOTH_SERIAL_H_
	this->BTwrite((const unsigned char*) "WiFiConnected", 13);
#endif

}

IPAddress System::WFlocalIP() {
	return WiFi.localIP();
}

#endif

void System::DrawPng() {
	Serial.printf("\nstart draw\n");
	unsigned width, height;
	uint16_t* tmpbuffer;
	uint8_t* tmpalpha;
	int alphas;
	uint32_t tmp;
	const char *filenames[2] = { "/24wifi.png", "/t.png"};
	int maxfiles = 2, last_y = 0;
	char xx = 0, yy = 0;
	for (int numfiles = 0; numfiles < maxfiles; numfiles++) {

		upng = upng_new_from_file(filenames[numfiles]);
		if (upng_get_error(upng) == UPNG_ENOTFOUND) {
			Serial.print("file not found");
		}
		upng_decode(upng);

		if (upng_get_error(upng) != UPNG_EOK) {
			Serial.printf("error: %u %u\n", upng_get_error(upng),
					upng_get_error_line(upng));
			//return 0;pixel=%i
		} else {

			width = upng_get_width(upng);
			height = upng_get_height(upng);

			tmpbuffer=colorBuffer_R8G8B8toR5G6B5(upng);
			Graph->drawImageBuffer(0,yy,tmpbuffer,width, height); yy+=height+5;//ToDo: make buffer converting

			tmpalpha=colorBuffer_A8R8G8B8toA8(upng);
			//alphas=upng_get_alpha( upng,  &tmpalpha);
			//Serial.printf("\nAlphas=%i",alphas);
			Graph->drawImageBufferAlpha(100,100,tmpbuffer,tmpalpha,width, height);// yy+=height+5;//ToDo: make buffer converting
			free(tmpalpha);

			free(tmpbuffer);

			switch (upng_get_components(upng)) {
			case 1: //UPNG_LUM
				//glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, upng_get_width(upng), upng_get_height(upng), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, upng_get_buffer(upng));
				break;
			case 2:			//UPNG_LUMA
							//glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, upng_get_width(upng), upng_get_height(upng), 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, upng_get_buffer(upng));
				break;
			case 3:			//UPNG_RGB
							//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, upng_get_width(upng), upng_get_height(upng), 0, GL_RGB, GL_UNSIGNED_BYTE, upng_get_buffer(upng));
				break;
			case 4:			//UPNG_RGBA
							//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, upng_get_width(upng), upng_get_height(upng), 0, GL_RGBA, GL_UNSIGNED_BYTE, upng_get_buffer(upng));
				break;
			case 5: //UPNG_PAL
				break;
			default: //0 or any else
				break;
				//return 1;
			}

			//free(buffer); do not need to free here, upng_free removes buffer

			upng_free(upng);
		}

	}
}

