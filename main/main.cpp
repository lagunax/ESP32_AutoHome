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


// Need to move it into class System:
TFT_eSPI tft = TFT_eSPI();  // Invoke library
FtpServer ftpSrv;   //set #define FTP_DEBUG in ESP8266FtpServer.h to see ftp verbose on serial

// #########################################3 webupload FS  ####################################3
#define FILESYSTEM SPIFFS
// You only need to format the filesystem once
//#define FORMAT_FILESYSTEM true
#define DBG_OUTPUT_PORT Serial

//const char* host = "esp32fs";

#if FILESYSTEM == FFat
#include <FFat.h>
#endif
#if FILESYSTEM == SPIFFS
#include <SPIFFS.h>
#endif

fs::File fsUploadFile;
//format bytes
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}


String getContentType(String filename) {
  if (sys->Web_Server->hasArg("download")) {
    return "application/octet-stream";
  } else if (filename.endsWith(".htm")) {
    return "text/html";
  } else if (filename.endsWith(".html")) {
    return "text/html";
  } else if (filename.endsWith(".css")) {
    return "text/css";
  } else if (filename.endsWith(".js")) {
    return "application/javascript";
  } else if (filename.endsWith(".png")) {
    return "image/png";
  } else if (filename.endsWith(".gif")) {
    return "image/gif";
  } else if (filename.endsWith(".jpg")) {
    return "image/jpeg";
  } else if (filename.endsWith(".ico")) {
    return "image/x-icon";
  } else if (filename.endsWith(".xml")) {
    return "text/xml";
  } else if (filename.endsWith(".pdf")) {
    return "application/x-pdf";
  } else if (filename.endsWith(".zip")) {
    return "application/x-zip";
  } else if (filename.endsWith(".gz")) {
    return "application/x-gzip";
  }
  return "text/plain";
}

bool exists(String path){
  bool yes = false;
  fs::File file = FILESYSTEM.open(path, "r");
  if(!file.isDirectory()){
    yes = true;
  }
  file.close();
  return yes;
}

bool handleFileRead(String path) {
  DBG_OUTPUT_PORT.println("handleFileRead: " + path);
  if (path.endsWith("/")) {
    path += "index.htm";
  }
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (exists(pathWithGz) || exists(path)) {
    if (exists(pathWithGz)) {
      path += ".gz";
    }
    fs::File file = FILESYSTEM.open(path, "r");
    sys->Web_Server->streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload() {
  if (sys->Web_Server->uri() != "/edit") {
    return;
  }
  HTTPUpload& upload = sys->Web_Server->upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    DBG_OUTPUT_PORT.print("handleFileUpload Name: "); DBG_OUTPUT_PORT.println(filename);
    fsUploadFile = FILESYSTEM.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    //DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
    if (fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      fsUploadFile.close();
    }
    DBG_OUTPUT_PORT.print("handleFileUpload Size: "); DBG_OUTPUT_PORT.println(upload.totalSize);
  }
}

void handleFileDelete() {
  if (sys->Web_Server->args() == 0) {
    return sys->Web_Server->send(500, "text/plain", "BAD ARGS");
  }
  String path = sys->Web_Server->arg(0);
  DBG_OUTPUT_PORT.println("handleFileDelete: " + path);
  if (path == "/") {
    return sys->Web_Server->send(500, "text/plain", "BAD PATH");
  }
  if (!exists(path)) {
    return sys->Web_Server->send(404, "text/plain", "FileNotFound");
  }
  FILESYSTEM.remove(path);
  sys->Web_Server->send(200, "text/plain", "");
  path = String();
}

void handleFileCreate() {
  if (sys->Web_Server->args() == 0) {
    return sys->Web_Server->send(500, "text/plain", "BAD ARGS");
  }
  String path = sys->Web_Server->arg(0);
  DBG_OUTPUT_PORT.println("handleFileCreate: " + path);
  if (path == "/") {
    return sys->Web_Server->send(500, "text/plain", "BAD PATH");
  }
  if (exists(path)) {
    return sys->Web_Server->send(500, "text/plain", "FILE EXISTS");
  }
  fs::File file = FILESYSTEM.open(path, "w");
  if (file) {
    file.close();
  } else {
    return sys->Web_Server->send(500, "text/plain", "CREATE FAILED");
  }
  sys->Web_Server->send(200, "text/plain", "");
  path = String();
}

void handleFileList() {
  if (!sys->Web_Server->hasArg("dir")) {
    sys->Web_Server->send(500, "text/plain", "BAD ARGS");
    return;
  }

  String path = sys->Web_Server->arg("dir");
  DBG_OUTPUT_PORT.println("handleFileList: " + path);


  fs::File root = FILESYSTEM.open(path);
  path = String();

  String output = "[";
  if(root.isDirectory()){
	  fs::File file = root.openNextFile();
      while(file){
          if (output != "[") {
            output += ',';
          }
          output += "{\"type\":\"";
          output += (file.isDirectory()) ? "dir" : "file";
          output += "\",\"name\":\"";
          output += String(file.name()).substring(1);
          output += "\"}";
          file = root.openNextFile();
      }
  }
  output += "]";
  sys->Web_Server->send(200, "text/json", output);
}

// ########################################## end of WEBUPLOAD fs ###################################


#ifdef WEBSERVER_H
//#include "../modules/HTTP_Web_Server->cpp"
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


		  sys->Web_Server->on("/", WEB_handleRoot);
		  sys->Web_Server->on("/login", WEB_handleLogin);
		  /*Web_Server->on("/inline", []() {
			  Web_Server->send(200, "text/plain", "this works without need of authentification");
		  });*/


		  //list directory
		  sys->Web_Server->on("/list", HTTP_GET, handleFileList);
		  //load editor
		  sys->Web_Server->on("/edit", HTTP_GET, []() {
		    if (!handleFileRead("/edit.htm")) {
		      sys->Web_Server->send(404, "text/plain", "FileNotFound");
		    }
		  });
		  //create file
		  sys->Web_Server->on("/edit", HTTP_PUT, handleFileCreate);
		  //delete file
		  sys->Web_Server->on("/edit", HTTP_DELETE, handleFileDelete);
		  //first callback is called after the request has ended with all parsed arguments
		  //second callback handles file uploads at that location
		  sys->Web_Server->on("/edit", HTTP_POST, []() {
		    sys->Web_Server->send(200, "text/plain", "");
		  }, handleFileUpload);

		  //get heap status, analog input value and all GPIO statuses in one json call
		  sys->Web_Server->on("/all", HTTP_GET, []() {
		    String json = "{";
		    json += "\"heap\":" + String(ESP.getFreeHeap());
		    json += ", \"analog\":" + String(analogRead(A0));
		    json += ", \"gpio\":" + String((uint32_t)(0));
		    json += "}";
		    sys->Web_Server->send(200, "text/json", json);
		    json = String();
		  });






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

  /*DBG_OUTPUT_PORT.begin(115200);
  DBG_OUTPUT_PORT.print("\n");
  DBG_OUTPUT_PORT.setDebugOutput(true);
  if (FORMAT_FILESYSTEM) FILESYSTEM.format();
  FILESYSTEM.begin();
  {
	  fs::File root = FILESYSTEM.open("/");
	  fs::File file = root.openNextFile();
      while(file){
          String fileName = file.name();
          size_t fileSize = file.size();
          DBG_OUTPUT_PORT.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
          file = root.openNextFile();
      }
      DBG_OUTPUT_PORT.printf("\n");
  }*/

  /*
  MDNS.begin(host);
  DBG_OUTPUT_PORT.print("Open http://");
  DBG_OUTPUT_PORT.print(host);
  DBG_OUTPUT_PORT.println(".local/edit to see the file browser");
*/
  SPIFFS.begin(true);
  tft.println("SPIFFS");

  sys->BTInit();
  tft.println("BT");

  sys->WFAPInit();
  tft.println("WiFi");

  sys->WEB_Start();
  tft.println("HTTP");

  ftpSrv.begin("user","pass");
  tft.println("FTP");
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

	ftpSrv.handleFTP();
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


