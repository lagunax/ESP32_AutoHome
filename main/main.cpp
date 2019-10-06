/* ############################################################################
*
* ESP32-AutoHome project
* 	Made for automation process for plants, flowers, etc.
*
* Creator: Novoselov Vitaliy
* Author URI: https://github.com/lagunax
* Project URI: https://github.com/lagunax/ESP32-AutoHome
*
* Info:
* 	Project created for chipsets ESP32
* 	Official Web site (URI): https://www.espressif.com/en/products/hardware/esp32/overview
* 	Official Github URI: https://github.com/espressif/esp-idf
* 	Arduino add-on Github URI: https://github.com/espressif/arduino-esp32
*
* 	Hardware tested:
* 		TTGO-T-Display (got from  Aliexpress.com)
*
*/// ############################################################################

#include "System.h"

TaskHandle_t loopTaskHandle = NULL;
System *sys = new System();


//Service::FTP::FtpServer ftpSrv;   //set #define FTP_DEBUG in ESP8266FtpServer.h to see ftp verbose on serial


#if CONFIG_AUTOSTART_ARDUINO

bool loopTaskWDTEnabled;

//void setup();
//extern "C" void loop();


void setup() {
	  sys->tft->init();
	  sys->tft->setRotation(2);
	  sys->tft->fillScreen(TFT_BLACK);
	  sys->tft->setCursor(0, 0, 2);
	  sys->tft->setTextColor(TFT_WHITE,TFT_BLACK);
	  sys->tft->setTextSize(1);

	  sys->tft->println("starting...");
  Serial.begin(115200);
  sys->tft->println("serial");

  SPIFFS.begin(true);
  sys->tft->println("SPIFFS");

  sys->BTInit();
  sys->tft->println("BT");

  sys->WFAPInit();
  sys->tft->println("WiFi");

#ifdef SERVICE_WEB
  Service::HTTP::Start();
  sys->tft->println("HTTP");
#endif
#ifdef SERVICE_FTP
  Service::FTP::Start("user","pass");
  sys->tft->println("FTP");
#endif

  sys->tft->fillScreen(TFT_BLACK);

  for (int y=10; y<100; y++){
	  sys->tft->drawPixel(20,y,0xFF);
  };

  sys->DrawPng();

}

void loop() {

	#ifdef _BLUETOOTH_SERIAL_H_
int testi;
	//Bluetooth module

		if (sys->BTavailable()) {
			sys->BTwrite(Serial.read());
		}
		if (sys->BTavailable()) {
			testi=sys->BTread();
			Serial.write(testi);
			sys->BTwrite((const unsigned char *)"test",4);
			sys->BTwrite(testi);
		}
	#endif

	Service::FTP::Handle();
	Service::HTTP::Handle();
}



//Starting Programm

void loopTask(void *pvParameters)
{
    setup();
    for(;;) {
        if(loopTaskWDTEnabled){
            esp_task_wdt_reset();
        }
        loop();
    }
}

extern "C" void app_main()
{
    loopTaskWDTEnabled = false;
    initArduino();
    xTaskCreateUniversal(loopTask, "loopTask", 8192, NULL, 1, &loopTaskHandle, CONFIG_ARDUINO_RUNNING_CORE);
    //if (sys) delete sys;
}

#endif


