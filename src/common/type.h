#ifndef _COMMON_
#define _COMMON_
#include <iostream>
#include <vector>
// #include <eigen3/Eigen/Dense>
#include <memory>
#include <opencv2/opencv.hpp>

namespace common {

typedef unsigned int TASTime;
typedef unsigned int TASInt;
typedef unsigned char TASByte;
typedef float TASFloat;
typedef double TASDouble;
typedef long long TASLong;
typedef unsigned short TASShort;

//车牌信息
struct PlateImgInfo {
  std::shared_ptr<cv::Mat> img;
  std::string plate;
  std::string type;
  float conf;
  PlateImgInfo(std::shared_ptr<cv::Mat> img_) {
    img = img_;
    plate = "";
  }
};
typedef std::shared_ptr<PlateImgInfo> PlateImgInfoPtr;

struct bbox {
  int x;
  int y;
  int width;
  int height;
  float conf;
  bbox() {
    x = 0;
    y = 0;
    width = 0;
    height = 0;
    conf = 0;
  }
};

struct ObjInfo {
  std::vector<cv::Point> car_contours;
  std::vector<cv::Point> plate_contours;
  PlateImgInfoPtr plate_info_ptr;

  long obj_id;
  bbox bbox_info;
  float dis;
  unsigned long long timestamp;
  //   Eigen::Vector3f pos;
  //   Eigen::Vector3f speed;
  ObjInfo() {
    obj_id = 0;
    dis = 0;
  }
};
typedef std::shared_ptr<ObjInfo> ObjInfoPtr;

struct Image {
  cv::Mat img;
  long frame_id;
  unsigned long long timestamp;
};
typedef std::shared_ptr<Image> ImagePtr;

struct DecResult {
  long frame_id;
  unsigned long long timestamp;
  std::vector<ObjInfo> objs;
  ImagePtr img_ptr;
  DecResult() { img_ptr = nullptr; }
};
typedef std::shared_ptr<DecResult> DecResultPtr;

// extern int printCode;

//一组
#pragma pack(1)
typedef struct _CarDataHead CarDataHead;
struct _CarDataHead {
  TASShort flag;      // 数据帧头, 取值为 0x7E7E 2
  TASLong devid;      // 设备 ID                  8
  TASLong timestamp;  // timestamp    8
  TASInt frame;       // 传输帧号 4
  TASShort datalen;   // 数据区长度    2
  TASShort targetnum; // 目标数量 2
  // TASFloat rdir;			// 雷达朝向，与正北的夹角，0.0 - 360.0 4
  //	....... TARGETS DATA ........
  //	TASShort crc;			// 校验位, 采用 CRC16-x25 校验方式
  //  TASShort eflag;			// 8 数据帧尾, 取值为 0x7E7D
};

//单个
typedef struct _CarDataTarget CarDataTarget;
struct _CarDataTarget {
  TASInt id;         // ID
  TASByte obj_class; // object class
  TASFloat speed;    // 速度, 单位 km/h
  TASFloat y;        // 相对距离，单位 m
  TASFloat x;        // 相对横，单位 度
  TASFloat stable;   // 稳定度
};
#pragma pack()

struct LinePoint {
  cv::Point p;
  float distance;
};

} // namespace common

#endif
