#include "CarDectorService.h"
#include "ControlService.h"
#include "DrawService.h"
#include "TrackingService.h"

#include <signal.h>
#include <unistd.h>

using namespace std;
using namespace common;

CarDectorService::CarDectorService() {
  is_running = false;
  trt_detector_ptr = make_shared<DetTrt::TrtDetector>();
}

CarDectorService::~CarDectorService() {}

void CarDectorService::start() {
  is_running = true;
  worker = thread(&CarDectorService::run, this);
}

void CarDectorService::stop() {
  is_running = false;
  if (worker.joinable()) {
    worker.join();
  }
}

void CarDectorService::run() {
  while (is_running) {
    mux_rw.lock();
    if (image_list.size() == 0) {
      //当前存储队列无数据，不做处理，等待5ms
      mux_rw.unlock();

      usleep(5 * 1000);
      continue;
    }
    ImagePtr image_ptr = image_list.front();
    image_list.pop_front();
    mux_rw.unlock();
    std::vector<common::ObjInfo> bboxs;
    trt_detector_ptr->doInferenceFrieza(image_ptr->img, bboxs,
                                        image_ptr->timestamp);
    DecResultPtr ret_ptr = make_shared<DecResult>();
    ret_ptr->objs = bboxs;
    ret_ptr->timestamp = image_ptr->timestamp;
    ret_ptr->img_ptr = image_ptr;
    ControlService::instance().pushCardetector(ret_ptr);
    // CarTrackingService::instance().pushInfo(ret_ptr);
    // DrawService::instance().pushInfo(ret_ptr);
  }
}

void CarDectorService::pushImage(common::ImagePtr &image_ptr) {
  mux_rw.lock();
  image_list.push_back(image_ptr);
  if (image_list.size() > 10) {
    //防止队列过长
    image_list.pop_front();
    cout << "Error:image_list.size()>10" << endl;
  }
  mux_rw.unlock();
}
