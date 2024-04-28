#ifndef CAR_DECTOR_SERVICE
#define CAR_DECTOR_SERVICE
#include "TrtDetector/TrtDetector.h"
#include "opencv2/opencv.hpp"
#include "tool.h"
#include "type.h"
#include <iostream>
#include <list>
#include <mutex>
#include <thread>

class CarDectorService {
public:
  static CarDectorService &instance() {
    static CarDectorService val;
    return val;
  }
  CarDectorService();
  ~CarDectorService();
  void start();
  void stop();

  void pushImage(common::ImagePtr &image_ptr);

private:
  std::list<common::ImagePtr> image_list;
  std::thread worker;
  bool is_running;
  std::shared_ptr<DetTrt::TrtDetector> trt_detector_ptr;
  std::mutex mux_rw;

  void run();
};

#endif