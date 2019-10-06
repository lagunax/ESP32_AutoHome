/*
 * WebServer.cpp
 *
 *  Created on: 29 сент. 2019 г.
 *      Author: producer
 */

#include "WebService.h"

#ifdef MAIN_WEBSERVICE_H_
namespace Service {
	namespace HTTP {

		//WebServer Web_Server(80);
		WebServer* Web_Server;
		void Handle(){
			Web_Server->handleClient();
		}

		bool WEB_is_authentified() {
		  Serial.println("Enter is_authentified");
		  if (Web_Server->hasHeader("Cookie")) {
			Serial.print("Found cookie: ");
			String cookie = Web_Server->header("Cookie");
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
		  String msg;
		  if (Web_Server->hasHeader("Cookie")) {
			Serial.print("Found cookie: ");
			String cookie = Web_Server->header("Cookie");
			Serial.println(cookie);
		  }
		  if (Web_Server->hasArg("DISCONNECT")) {
			Serial.println("Disconnection");
			Web_Server->sendHeader("Location", "/login");
			Web_Server->sendHeader("Cache-Control", "no-cache");
			Web_Server->sendHeader("Set-Cookie", "ESPSESSIONID=0");
			Web_Server->send(301);
			return;
		  }
		  if (Web_Server->hasArg("USERNAME") && Web_Server->hasArg("PASSWORD")) {
			if (Web_Server->arg("USERNAME") == "admin" &&  Web_Server->arg("PASSWORD") == "admin") {
				Web_Server->sendHeader("Location", "/");
				Web_Server->sendHeader("Cache-Control", "no-cache");
				Web_Server->sendHeader("Set-Cookie", "ESPSESSIONID=1");
				Web_Server->send(301);
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
		  Web_Server->send(200, "text/html", content);
		}

		//root page can be accessed only if authentification is ok
		void WEB_handleRoot() {
		  Serial.println("Enter handleRoot");
		  String header;
		  if (!WEB_is_authentified()) {
			  Web_Server->sendHeader("Location", "/login");
			  Web_Server->sendHeader("Cache-Control", "no-cache");
			  Web_Server->send(301);
			return;
		  }
		  String content = "<html><body><H2>hello, you successfully connected to esp8266!</H2><br>";
		  if (Web_Server->hasHeader("User-Agent")) {
			content += "the user agent used is : " + Web_Server->header("User-Agent") + "<br><br>";
		  }
		  content += "You can access this page until you <a href=\"/login?DISCONNECT=YES\">disconnect</a></body></html>";
		  Web_Server->send(200, "text/html", content);
		}

		//no need authentification
		void WEB_handleNotFound() {
		  String message = "File Not Found\n\n";
		  message += "URI: ";
		  message += Web_Server->uri();
		  message += "\nMethod: ";
		  message += (Web_Server->method() == HTTP_GET) ? "GET" : "POST";
		  message += "\nArguments: ";
		  message += Web_Server->args();
		  message += "\n";
		  for (uint8_t i = 0; i < Web_Server->args(); i++) {
			message += " " + Web_Server->argName(i) + ": " + Web_Server->arg(i) + "\n";
		  }
		  Web_Server->send(404, "text/plain", message);
		}

		// #########################################3 webupload FS  ####################################3

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
		  if (Web_Server->hasArg("download")) {
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
			Web_Server->streamFile(file, contentType);
			file.close();
			return true;
		  }
		  return false;
		}

		void handleFileUpload() {
		  if (Web_Server->uri() != "/edit") {
			return;
		  }
		  HTTPUpload& upload = Web_Server->upload();
		  if (upload.status == UPLOAD_FILE_START) {
			String filename = upload.filename;
			if (!filename.startsWith("/")) {
			  filename = "/" + filename;
			}
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
		  }
		}

		void handleFileDelete() {
		  if (Web_Server->args() == 0) {
			return Web_Server->send(500, "text/plain", "BAD ARGS");
		  }
		  String path = Web_Server->arg(0);
		  if (path == "/") {
			return Web_Server->send(500, "text/plain", "BAD PATH");
		  }
		  if (!exists(path)) {
			return Web_Server->send(404, "text/plain", "FileNotFound");
		  }
		  FILESYSTEM.remove(path);
		  Web_Server->send(200, "text/plain", "");
		  path = String();
		}

		void handleFileCreate() {
		  if (Web_Server->args() == 0) {
			return Web_Server->send(500, "text/plain", "BAD ARGS");
		  }
		  String path = Web_Server->arg(0);
		  if (path == "/") {
			return Web_Server->send(500, "text/plain", "BAD PATH");
		  }
		  if (exists(path)) {
			return Web_Server->send(500, "text/plain", "FILE EXISTS");
		  }
		  fs::File file = FILESYSTEM.open(path, "w");
		  if (file) {
			file.close();
		  } else {
			return Web_Server->send(500, "text/plain", "CREATE FAILED");
		  }
		  Web_Server->send(200, "text/plain", "");
		  path = String();
		}

		void handleFileList() {
		  if (!Web_Server->hasArg("dir")) {
			Web_Server->send(500, "text/plain", "BAD ARGS");
			return;
		  }

		  String path = Web_Server->arg("dir");


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
		  Web_Server->send(200, "text/json", output);
		}

		// ########################################## end of WEBUPLOAD fs ###################################
		void Start(int port){
			  Serial.println(WiFi.localIP());

			  Web_Server = new WebServer(port);


			  Web_Server->on("/", WEB_handleRoot);
			  Web_Server->on("/login", WEB_handleLogin);
			  Web_Server->on("/inline", []() {
				  Web_Server->send(200, "text/plain", "this works without need of authentification");
			  });


			  //list directory
			  Web_Server->on("/list", HTTP_GET, handleFileList);
			  //load editor
			  Web_Server->on("/edit", HTTP_GET, []() {
				if (!handleFileRead("/edit.htm")) {
				  Web_Server->send(404, "text/plain", "FileNotFound");
				}
			  });
			  //create file
			  Web_Server->on("/edit", HTTP_PUT, handleFileCreate);
			  //delete file
			  Web_Server->on("/edit", HTTP_DELETE, handleFileDelete);
			  //first callback is called after the request has ended with all parsed arguments
			  //second callback handles file uploads at that location
			  Web_Server->on("/edit", HTTP_POST, []() {
				Web_Server->send(200, "text/plain", "");
			  }, handleFileUpload);

			  //get heap status, analog input value and all GPIO statuses in one json call
			  Web_Server->on("/all", HTTP_GET, []() {
				String json = "{";
				json += "\"heap\":" + String(ESP.getFreeHeap());
				json += ", \"analog\":" + String(analogRead(A0));
				json += ", \"gpio\":" + String((uint32_t)(0));
				json += "}";
				Web_Server->send(200, "text/json", json);
				json = String();
			  });

			  Web_Server->onNotFound(WEB_handleNotFound);

			  //here the list of headers to be recorded
			  const char * headerkeys[] = {"User-Agent", "Cookie"} ;
			  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
			  //ask server to track these headers
			  Web_Server->collectHeaders(headerkeys, headerkeyssize);
			  Web_Server->begin();
			  Serial.println("HTTP server started");
		}
	}
}
#endif

