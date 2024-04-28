#ifndef VIO_DECT_SERVICE
#define VIO_DECT_SERVICE
#include "opencv2/opencv.hpp"
#include "tool.h"
#include "type.h"
#include "json/json.h"
#include <iostream>
#include <list>
#include <mutex>
#include <thread>
#include <vector>

class ViolationDetectionService {

public:
  static ViolationDetectionService &instance() {
    static ViolationDetectionService val;
    return val;
  }
  ViolationDetectionService();
  ~ViolationDetectionService();
  void start();
  void stop();

  void pushCarLine(std::vector<std::vector<common::LinePoint>> car_lines,
                   Json::Value root);
  std::vector<std::vector<common::LinePoint>>  GetCarLine();

private:
  bool is_running;
  std::thread worker;
  std::mutex mux_rw;
  std::vector<std::vector<common::LinePoint>> car_lines;
  void CreateJsonFile(Json::Value root);//写json文件
  void ReadJsonFile();//读json文件

  void run();
};

#endif