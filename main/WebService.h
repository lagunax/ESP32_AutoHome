/*
 * WebServer.h
 *
 *  Created on: 29 сент. 2019 г.
 *      Author: producer
 */

#ifndef MAIN_WEBSERVICE_H_
	#define MAIN_WEBSERVICE_H_
	#include <Arduino.h>
	#include <WebServer.h>
	#define FILESYSTEM SPIFFS
		// You only need to format the filesystem once
		//#define FORMAT_FILESYSTEM true

	#if FILESYSTEM == FFat
		#include <FFat.h>
	#endif
	#if FILESYSTEM == SPIFFS
		#include <SPIFFS.h>
	#endif

namespace Service {
	namespace HTTP {
		void Handle(); // Run it in loop()
		bool WEB_is_authentified();
		void WEB_handleLogin();
		void WEB_handleRoot();
		void WEB_handleNotFound();
		void handleFileList();
		void handleFileCreate();
		void handleFileDelete();
		void handleFileUpload();
		bool handleFileRead(String);
		bool exists(String);
		String getContentType(String);
		String formatBytes(size_t);
		void Start(int port=80); // Run it in setup(). if parameter (port) not used then port=80 by default
	}
}
#endif /* MAIN_WEBSERVICE_H_ */
