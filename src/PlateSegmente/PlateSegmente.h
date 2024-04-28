#ifndef _PLATE_SEGMENT_
#define _PLATE_SEGMENT_
#include "common/type.h"
#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>

#include <vector>

class PlateSegmente {
private:
  int input_width;
  int input_height;

//   //   torch::Device device;
//   std::shared_ptr<torch::Device> device_ptr;
//   torch::jit::script::Module model;

  /* data */
public:
  PlateSegmente(/* args */);
  ~PlateSegmente();

  void InferSegment(cv::Mat &img);
  void doInferenceFrieza(cv::Mat &img, std::vector<common::ObjInfo> &objs);
};

#endif
