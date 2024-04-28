#ifndef PLATE_RECOG_SERVICE
#define PLATE_RECOG_SERVICE
#include "PlateRecognition/plate_detector.h"
#include "tool.h"
#include "type.h"
#include <iostream>
#include <list>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <thread>

class PlateRecogService {

public:
  static PlateRecogService &instance() {
    static PlateRecogService val;
    return val;
  }
  PlateRecogService();
  ~PlateRecogService();
  void start();
  void stop();
  //拿到车辆检测的结果
  void pushInfo(common::PlateImgInfoPtr palte_info_ptr);
  void pushInfo(common::DecResultPtr dec_result_ptr);

private:
  std::list<common::PlateImgInfoPtr> data_list;
  std::list<common::DecResultPtr> dec_list;
  std::thread worker;
  std::thread work;
  bool is_running;
  std::mutex mux_rw;

  std::shared_ptr<PlateDetector> plate_detector_ptr;

  // TrackingSession *sess; // 创建追踪会话
  void run();
  void runAll();
};

#endif