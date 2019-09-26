#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "Arduino.h"
//#include "tftprint.cpp"
//#include "Free_Fonts.cpp"
#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip
#include <SPI.h>
#include <ESPmDNS.h>

#define TFT_GREY 0x5AEB // New colour

TFT_eSPI tft = TFT_eSPI();  // Invoke library


#include "System.h"

TaskHandle_t loopTaskHandle = NULL;
System *sys = new System();


#ifdef WEBSERVER_H
//#include "../modules/HTTP_Server.cpp"
	bool WEB_is_authentified() {
	  Serial.println("Enter is_authentified");
	  if (sys->Web_Server->hasHeader("Cookie")) {
	    Serial.print("Found cookie: ");
	    String cookie = sys->Web_Server->header("Cookie");
	    Serial.println(cookie);
	    if (cookie.indexOf("ESPSESSIONID=1") != -1) {
	      Serial.println("Authentification Successful");
	      return true;
	    }
	  }
	  Serial.println("Authentification Failed");
	  return false;
	}

	//login page, also called for disconnect
	void WEB_handleLogin() {
	  tft.println("http: login");
	  String msg;
	  if (sys->Web_Server->hasHeader("Cookie")) {
	    Serial.print("Found cookie: ");
	    String cookie = sys->Web_Server->header("Cookie");
	    Serial.println(cookie);
	  }
	  if (sys->Web_Server->hasArg("DISCONNECT")) {
	    Serial.println("Disconnection");
	    sys->Web_Server->sendHeader("Location", "/login");
	    sys->Web_Server->sendHeader("Cache-Control", "no-cache");
	    sys->Web_Server->sendHeader("Set-Cookie", "ESPSESSIONID=0");
	    sys->Web_Server->send(301);
	    return;
	  }
	  if (sys->Web_Server->hasArg("USERNAME") && sys->Web_Server->hasArg("PASSWORD")) {
	    if (sys->Web_Server->arg("USERNAME") == "admin" &&  sys->Web_Server->arg("PASSWORD") == "admin") {
	    	sys->Web_Server->sendHeader("Location", "/");
	    	sys->Web_Server->sendHeader("Cache-Control", "no-cache");
	    	sys->Web_Server->sendHeader("Set-Cookie", "ESPSESSIONID=1");
	    	sys->Web_Server->send(301);
	      Serial.println("Log in Successful");
	      return;
	    }
	    msg = "Wrong username/password! try again.";
	    Serial.println("Log in Failed");
	  }
	  String content = "<html><body><form action='/login' method='POST'>To log in, please use : admin/admin<br>";
	  content += "User:<input type='text' name='USERNAME' placeholder='user name'><br>";
	  content += "Password:<input type='password' name='PASSWORD' placeholder='password'><br>";
	  content += "<input type='submit' name='SUBMIT' value='Submit'></form>" + msg + "<br>";
	  content += "You also can go <a href='/inline'>here</a></body></html>";
	  sys->Web_Server->send(200, "text/html", content);
	}

	//root page can be accessed only if authentification is ok
	void WEB_handleRoot() {
	  tft.println("http: root");
	  Serial.println("Enter handleRoot");
	  String header;
	  if (!WEB_is_authentified()) {
		  sys->Web_Server->sendHeader("Location", "/login");
		  sys->Web_Server->sendHeader("Cache-Control", "no-cache");
		  sys->Web_Server->send(301);
	    return;
	  }
	  String content = "<html><body><H2>hello, you successfully connected to esp8266!</H2><br>";
	  if (sys->Web_Server->hasHeader("User-Agent")) {
	    content += "the user agent used is : " + sys->Web_Server->header("User-Agent") + "<br><br>";
	  }
	  content += "You can access this page until you <a href=\"/login?DISCONNECT=YES\">disconnect</a></body></html>";
	  sys->Web_Server->send(200, "text/html", content);
	}

	//no need authentification
	void WEB_handleNotFound() {
	  tft.println("http: not found");
	  String message = "File Not Found\n\n";
	  message += "URI: ";
	  message += sys->Web_Server->uri();
	  message += "\nMethod: ";
	  message += (sys->Web_Server->method() == HTTP_GET) ? "GET" : "POST";
	  message += "\nArguments: ";
	  message += sys->Web_Server->args();
	  message += "\n";
	  for (uint8_t i = 0; i < sys->Web_Server->args(); i++) {
	    message += " " + sys->Web_Server->argName(i) + ": " + sys->Web_Server->arg(i) + "\n";
	  }
	  sys->Web_Server->send(404, "text/plain", message);
	}

	void System::WEB_Start(){
		  Serial.println(WiFi.localIP());
		  //this->BTwrite(WiFi.localIP(), 13);
		  tft.println("web starting...");


		  Web_Server->on("/", WEB_handleRoot);
		  Web_Server->on("/login", WEB_handleLogin);
		  /*Web_Server->on("/inline", []() {
			  Web_Server->send(200, "text/plain", "this works without need of authentification");
		  });*/

		  Web_Server->onNotFound(WEB_handleNotFound);

		  tft.println("web configuring...");
		  //here the list of headers to be recorded
		  const char * headerkeys[] = {"User-Agent", "Cookie"} ;
		  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
		  //ask server to track these headers
		  Web_Server->collectHeaders(headerkeys, headerkeyssize);
		  Web_Server->begin();
		  Serial.println("HTTP server started");
	}
#endif



#if CONFIG_AUTOSTART_ARDUINO

bool loopTaskWDTEnabled;

//void setup();
//extern "C" void loop();


void setup() {
	  tft.init();
	  tft.setRotation(2);
	  tft.fillScreen(TFT_BLACK);
	  tft.setCursor(0, 0, 2);
	  tft.setTextColor(TFT_WHITE,TFT_BLACK);
	  tft.setTextSize(1);

	  tft.println("starting...");
  Serial.begin(115200);
  tft.println("serial");

  sys->BTInit();
  tft.println("BT");

  tft.println("WiFi");
  sys->WFAPInit();
  sys->WEB_Start();
  tft.println("Running");
  for (int y=10; y<100; y++){
	  tft.drawPixel(20,y,0xFF00FF);
  };

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

	delay(20);
	sys->Web_Server->handleClient();
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


