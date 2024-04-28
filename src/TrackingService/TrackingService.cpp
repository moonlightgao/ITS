#include "TrackingService.h"
#include "DrawService.h"
#include "ControlService.h"
#include <signal.h>
#include <unistd.h>

using namespace std;
using namespace common;

CarTrackingService::CarTrackingService() {
  is_running = false;
  sess = CreateSession(2, 3, 0.3); // 创建追踪会话
  
}

CarTrackingService::~CarTrackingService() {
  stop();
}

void CarTrackingService::start() {
  is_running = true;
  worker = thread(&CarTrackingService::run, this);
}

void CarTrackingService::stop() {
  is_running = false;
  if (worker.joinable()) {
    worker.join();
  }
}

void CarTrackingService::run() {
  while (is_running) {
    mux_rw.lock();
    if (dec_list.size() == 0) {
      //当前存储队列无数据，不做处理，等待5ms
      mux_rw.unlock();

      usleep(5 * 1000);
      continue;
    }
    DecResultPtr dec_ptr = dec_list.front();
    dec_list.pop_front();
    mux_rw.unlock();
    std::vector<DetectionBox> dets;

    for (auto &val : dec_ptr->objs) {
      DetectionBox det;

      det.score = val.bbox_info.conf;
      det.box.height = val.bbox_info.height;
      det.box.width = val.bbox_info.width;
      det.box.x = val.bbox_info.x;
      det.box.y = val.bbox_info.y;

      dets.push_back(det);
    }

    if (dets.size() > 0) {
      std::vector<TrackingBox> trks = sess->Update(dets);
      //   for (int i = 0; i < trks.size(); i++) {
      //     cout << "i:" << i << " " << trks[i].box.x << "," << trks[i].box.y
      //          << " ";
      //   }
      //   cout << endl;
      //   for (int j = 0; j < dec_ptr->objs.size(); j++) {
      //     cout << "j:" << j << " " << dec_ptr->objs[j].bbox_info.x << ","
      //          << dec_ptr->objs[j].bbox_info.y << " ";
      //   }
      //   cout << endl;
      for (int i = 0; i < trks.size(); i++) {
        for (int j = 0; j < dec_ptr->objs.size(); j++) {
          if (fabs(dec_ptr->objs[j].bbox_info.x - trks[i].box.x) < 10 &&
              fabs(dec_ptr->objs[j].bbox_info.y - trks[i].box.y) < 10) {
            dec_ptr->objs[j].obj_id = trks[i].id;
            break;
          }
        }
      }
    }
    ControlService::instance().pushTracking(dec_ptr);
  }
}

void CarTrackingService::pushInfo(common::DecResultPtr &ret_ptr) {
  mux_rw.lock();
  dec_list.push_back(ret_ptr);
  if (dec_list.size() > 10) {
    //防止队列过长
    dec_list.pop_front();
    cout << "Error:dec_list.size()>10" << endl;
  }
  mux_rw.unlock();
}
