#include "PlateRecogService.h"
#include "ControlService.h"
#include "DrawService.h"
#include "opencv2/opencv.hpp"
#include <signal.h>
#include <unistd.h>

using namespace std;
using namespace common;
using namespace cv;

PlateRecogService::PlateRecogService() {
  is_running = false;

  plate_detector_ptr = make_shared<PlateDetector>();
}

PlateRecogService::~PlateRecogService() { stop(); }

void PlateRecogService::start() {
  is_running = true;
  worker = thread(&PlateRecogService::run, this);
  work = thread(&PlateRecogService::runAll, this);
}

void PlateRecogService::stop() {
  is_running = false;
  if (worker.joinable()) {
    worker.join();
  }
  if (work.joinable()) {
    work.join();
  }
}

void PlateRecogService::run() {
  while (is_running) {
    mux_rw.lock();
    if (data_list.size() == 0) {
      //当前存储队列无数据，不做处理，等待5ms
      mux_rw.unlock();

      usleep(5 * 1000);
      continue;
    }
    PlateImgInfoPtr img_info = data_list.front();
    data_list.pop_front();
    mux_rw.unlock();
    plate_detector_ptr->detect(*img_info->img, img_info->type, img_info->plate,
                               img_info->conf);
  }
}
void PlateRecogService::runAll() {
  int index = 0;
  while (is_running) {
    mux_rw.lock();
    if (dec_list.size() == 0) {
      //当前存储队列无数据，不做处理，等待5ms
      mux_rw.unlock();

      usleep(5 * 1000);
      continue;
    }
    DecResultPtr img_info = dec_list.front();
    dec_list.pop_front();
    mux_rw.unlock();
    for (int i = 0; i < img_info->objs.size(); i++) {
      // img_info->img_ptr->img;
      if (img_info->objs[i].bbox_info.x + img_info->objs[i].bbox_info.width >
          img_info->img_ptr->img.cols) {
        img_info->objs[i].bbox_info.width =
            img_info->img_ptr->img.cols - img_info->objs[i].bbox_info.x;
      }
      if (img_info->objs[i].bbox_info.y + img_info->objs[i].bbox_info.height >
          img_info->img_ptr->img.rows) {
        img_info->objs[i].bbox_info.height =
            img_info->img_ptr->img.rows - img_info->objs[i].bbox_info.y;
      }

      img_info->objs[i].bbox_info.x =
          img_info->objs[i].bbox_info.x < 0 ? 0 : img_info->objs[i].bbox_info.x;
      img_info->objs[i].bbox_info.y =
          img_info->objs[i].bbox_info.y < 0 ? 0 : img_info->objs[i].bbox_info.y;

      // cv::Mat image = img_info->img_ptr->img;
      cv::Rect rect(img_info->objs[i].bbox_info.x,
                    img_info->objs[i].bbox_info.y,
                    img_info->objs[i].bbox_info.width,
                    img_info->objs[i].bbox_info.height);

      shared_ptr<Mat> roi_ptr = make_shared<Mat>(img_info->img_ptr->img(rect));

      img_info->objs[i].plate_info_ptr = make_shared<PlateImgInfo>(roi_ptr);
      plate_detector_ptr->detect(*img_info->objs[i].plate_info_ptr->img,
                                 img_info->objs[i].plate_info_ptr->type,
                                 img_info->objs[i].plate_info_ptr->plate,
                                 img_info->objs[i].plate_info_ptr->conf);

      // if (img_info->objs[i].plate_info_ptr->conf > 0) {
      //   imwrite(img_info->objs[i].plate_info_ptr->plate + ".jpg",
      //           *img_info->objs[i].plate_info_ptr->img);
      // }

      // imwrite("plates/" + to_string(index++) + ".jpg",
      //         *img_info->objs[i].plate_info_ptr->img);

      /* code */
    }
    ControlService::instance().pushPlate(img_info);
  }
}

void PlateRecogService::pushInfo(PlateImgInfoPtr palte_info_ptr) {
  mux_rw.lock();
  data_list.push_back(palte_info_ptr);
  if (data_list.size() > 10) {
    //防止队列过长
    data_list.pop_front();
    cout << "Error:PlateRecogService data_list.size()>10" << endl;
  }
  mux_rw.unlock();
}
void PlateRecogService::pushInfo(DecResultPtr dec_result_ptr) {
  mux_rw.lock();
  dec_list.push_back(dec_result_ptr);
  if (dec_list.size() > 10) {
    //防止队列过长
    dec_list.pop_front();
    cout << "Error:PlateRecogService data_list.size()>10" << endl;
  }
  mux_rw.unlock();
}