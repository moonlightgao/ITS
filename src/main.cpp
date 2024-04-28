#include "CarDectorService.h"
#include "ItsApp.h"
#include "PlateRecogService.h"
#include "StreamServer.h"
#include "TrackingService.h"
#include "http_service.h"
#include <iostream>

using namespace std;

int main() {
  StreamServer::instance().start();
  PlateRecogService::instance().start();
  CarDectorService::instance().start();
  CarTrackingService::instance().start();
  HttpService::instance().start();

  ItsApp::instance().start();
  //等待收到ctrl-c信号，打开应用ItsApp start函数阻塞
  ItsApp::instance().stop();
  CarDectorService::instance().stop();
  CarTrackingService::instance().stop();
  PlateRecogService::instance().stop();
  StreamServer::instance().stop();
  HttpService::instance().stop();
}
