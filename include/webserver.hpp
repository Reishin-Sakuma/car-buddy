#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <WebServer.h>

// Webサーバー管理モジュール
void initWebServer();
void handleWebServerClient();
void stopWebServer();
bool isClientConnected();
int getConnectedClientCount();

#endif