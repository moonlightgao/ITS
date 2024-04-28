#ifndef _CONTROL_SERVICE_
#define _CONTROL_SERVICE_
#include "tool.h"
#include "type.h"
#include <iostream>
#include <list>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <thread>

class ControlService {

public:
  static ControlService &instance() {
    static ControlService val;
    return val;
  }
  ControlService();
  ~ControlService();

  //把车辆检测的结果给他
  void pushCardetector(common::DecResultPtr &ret_ptr);
  void pushTracking(common::DecResultPtr &ret_ptr);
  void pushPlate(common::DecResultPtr &ret_ptr);
  void pushDrawimg(common::Image frame);
  // void pushInfo(common::PlateImgInfoPtr palte_info_ptr);

private:
  std::list<common::DecResultPtr> dec_list;
  std::mutex mux_rw;
};

#endif