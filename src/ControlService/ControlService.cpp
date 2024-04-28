#include "ControlService.h"
#include "DrawService.h"
#include "PlateRecogService.h"
#include "StreamServer.h"
#include "TrackingService.h"

#include <signal.h>
#include <unistd.h>

using namespace std;
using namespace common;

ControlService::ControlService() {}

ControlService::~ControlService() {}
//拿到车辆检测的检测结果
void ControlService::pushCardetector(common::DecResultPtr &ret_ptr) {
  CarTrackingService::instance().pushInfo(ret_ptr);
}
void ControlService::pushTracking(common::DecResultPtr &dec_ptr) {

  // PlateRecogService::instance().pushInfo(dec_ptr);
  DrawService::instance().pushInfo(dec_ptr);
}
void ControlService::pushPlate(common::DecResultPtr &dec_ptr) {
  // DrawService::instance().pushInfo(dec_ptr);
}
void ControlService::pushDrawimg(common::Image frame) {
  cout << "   StreamServer::instance().pushFrame(frame);" << endl;
  StreamServer::instance().pushFrame(frame);
}
/*void ControlService::pushInfo(PlateImgInfoPtr palte_info_ptr) {
  mux_rw.lock();
  data_list.push_back(palte_info_ptr);
  if (data_list.size() > 10) {
    //防止队列过长
    data_list.pop_front();
    cout << "Error:ControlService data_list.size()>10" << endl;
  }
  mux_rw.unlock();
}*/
