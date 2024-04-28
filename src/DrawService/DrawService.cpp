#include "DrawService.h"
#include "ControlService.h"
#include "ViolationDetectionService.h"
#include "opencv2/opencv.hpp"
#include <unistd.h>

using namespace std;
using namespace common;
using namespace cv;

DrawService::DrawService() {}

DrawService::~DrawService() {}

void DrawService::pushInfo(common::DecResultPtr &ret_ptr) {
  std::vector<std::vector<cv::Point>> car_contours;
  std::vector<std::vector<cv::Point>> plate_contours;
  vector<ObjInfo> &objs = ret_ptr->objs;
  Mat &img = ret_ptr->img_ptr->img;
  for (int i = 0; i < objs.size(); i++) {
    for (auto &val_point : objs[i].car_contours) {
      circle(img, val_point, 2, Scalar(0, 0, 255));
    }
    for (auto &val_point : objs[i].plate_contours) {
      circle(img, val_point, 2, Scalar(0, 255, 0));
    }
    // car_contours.push_back(objs[i].car_contours);
    // plate_contours.push_back(objs[i].plate_contours);
  }

  static int imwrite_index = 0;
  cv::Size targetSize(640, 640); // 例如 640x480 的分辨率

  //画检测框并保存第10张检测框图像
  //画检测框
  //    rectangle(all_ret.rgb_img, Point(objs, top), Point(right,
  //    bottom), Scalar(255, 178, 50), 3);
  for (int i = 0; i < objs.size(); i++) {
    //        Point(left, top), Point(right, bottom)
    int left, top, right, bottom;
    left = objs[i].bbox_info.x;
    top = objs[i].bbox_info.y;
    right = objs[i].bbox_info.x + objs[i].bbox_info.width;
    bottom = objs[i].bbox_info.y + objs[i].bbox_info.height;

    float kuoda_bili = 0.1;
    int extend_box_width = (right - left) * kuoda_bili;
    int extend_box_height = (bottom - top) * kuoda_bili;

    left -= extend_box_width;
    right += extend_box_width;
    top -= extend_box_height;
    bottom += extend_box_height;

    if (left < 0 || top < 0 || right > img.cols || bottom > img.rows) {
      cout << "roi " << Point(left, top) << "," << Point(right, bottom) << endl;
      cout << "roi " << Point(left, top) << "," << Point(right, bottom) << endl;
      cout << "roi " << Point(left, top) << "," << Point(right, bottom) << endl;
    }
    left = left > 0 ? left : 0;
    top = top > 0 ? top : 0;

    right = right > img.cols ? img.cols : right;
    bottom = bottom > img.rows ? img.rows : bottom;

    if (left < img.cols && top < img.rows && right > 0 && bottom > 0) {

      //读取待复制照片
      Mat box_img = img(Rect(Point(left, top), Point(right, bottom)));
      int img_size = box_img.rows > box_img.cols ? box_img.rows : box_img.cols;
      Mat blankImage(img_size, img_size, CV_8UC3, cv::Scalar(0, 0, 0));
      Mat roi = blankImage(Rect(0, 0, box_img.cols, box_img.rows));
      box_img.copyTo(roi);
      resize(blankImage, blankImage, Size(400, 400));
      // imshow("entire", blankImage);
      // Mat image(img_size, img_size, CV_8UC3);
      // image.setTo(Scalar(0, 0, 0));

      // if (imwrite_index % 3 == 0) {
      //   imwrite("/home/alice/data/car_output5/" + to_string(imwrite_index) +
      //               "_" + to_string(i) + "WK" + ".jpg",
      //           blankImage);
      // }
    }
    // cout << "imwrite:" << imwrite_index << endl;
    // cout << "imwrite:" << imwrite_index << endl;

    // cout << "imwrite:" << imwrite_index << endl;

    rectangle(img, Point(left, top), Point(right, bottom), Scalar(255, 178, 50),
              3);

    string label = format("id:%d", objs[i].obj_id);
    // string str_label = format("id:%s", objs[i].plate_info_ptr->plate);
    //将标签显示在检测框顶部
    int baseLine;
    Size labelSize =
        getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
    top = max(top, labelSize.height);
    rectangle(img, Point(left, top - round(1.5 * labelSize.height)),
              Point(left + round(1.5 * labelSize.width), top + baseLine),
              Scalar(255, 255, 255), FILLED);
    putText(img, label, Point(left, top), FONT_HERSHEY_SIMPLEX, 0.75,
            Scalar(0, 0, 0), 1);
    if (objs[i].plate_info_ptr != nullptr) {
      putText(img, objs[i].plate_info_ptr->plate,
              Point(objs[i].bbox_info.x - 10, objs[i].bbox_info.y - 20),
              FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 0), 1);
    }
    //得到车道线的结果
    std::vector<std::vector<common::LinePoint>> CarLines =
        ViolationDetectionService::instance().GetCarLine();
    for (int i = 0; i < CarLines.size(); i++) {
      for (int j = 0; j < CarLines[i].size(); j++) {
        // Point p  = CarLines[i][j].p;
        if (j == 0) {
          circle(img, CarLines[i][j].p, 5, Scalar(120, 120, 120), -1);
        } else {
          circle(img, CarLines[i][j].p, 5, Scalar(120, 120, 120), -1);
          line(img, CarLines[i][j - 1].p, CarLines[i][j].p, Scalar(0, 0, 255),
               2);
        }
        // circle(img, CarLines[i][-1j].p, 1, Scalar(0, 255, 0), -1);

        /* code */
      }
    }
  }
  imshow("img", img);
  waitKey(1);
  ControlService::instance().pushDrawimg(*ret_ptr->img_ptr);
  imwrite_index++;
}
