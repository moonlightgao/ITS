#ifndef _PLATE_DETECTOR_
#define _PLATE_DETECTOR_
#include "hyper_lpr_sdk.h"
#include "opencv2/opencv.hpp"
#include <iostream>

class PlateDetector {
public:
  PlateDetector();
  ~PlateDetector();

  void detect(cv::Mat &img, std::string &ret_type, std::string &ret_code,
              float &ret_conf);

private:
  P_HLPR_Context ctx;
};

#endif