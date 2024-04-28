#ifndef CAR_TRACKING_SERVICE
#define CAR_TRACKING_SERVICE
#include "TrtTracking/sor.h"
#include "opencv2/opencv.hpp"
#include "tool.h"
#include "type.h"
#include <iostream>
#include <list>
#include <mutex>
#include <thread>

class CarTrackingService {
public:
  static CarTrackingService &instance() {
    static CarTrackingService val;
    return val;
  }
  CarTrackingService();
  ~CarTrackingService();
  void start();
  void stop();

  void pushInfo(common::DecResultPtr &ret_ptr);

private:
  std::list<common::DecResultPtr> dec_list;
  std::thread worker;
  bool is_running;
  std::mutex mux_rw;
  TrackingSession *sess; // 创建追踪会话
  void run();
};
#endif