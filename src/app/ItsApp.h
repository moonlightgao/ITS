#ifndef ITS_APP
#define ITS_APP
#include "type.h"
#include <iostream>
#include <thread>

class ItsApp {
private:
  /* data */
public:
  static ItsApp &instance() {
    static ItsApp val;
    return val;
  }
  ItsApp(/* args */);
  ~ItsApp();

  void start();
  void stop();

private:
  std::thread worker;
  bool is_running;
  cv::VideoCapture cap;
  std::string video_path;

  void run();
};

#endif