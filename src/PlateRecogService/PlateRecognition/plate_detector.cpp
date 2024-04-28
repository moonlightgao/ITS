//
// Created by Tunm-Air13 on 2023/2/23.
//
#include "plate_detector.h"
#include "hyper_lpr_sdk.h"
#include "opencv2/opencv.hpp"
#include <iostream>

using namespace std;
using namespace cv;

static const std::vector<std::string> TYPES = {
    "蓝牌",     "黄牌单层", "白牌单层", "绿牌新能源", "黑牌港澳",
    "香港单层", "香港双层", "澳门单层", "澳门双层",   "黄牌双层"};

PlateDetector::PlateDetector() {
  char model_path[] = "/home/alice/project/HyperLPR/Prj-Linux/hyperlpr3/"
                      "resource/models/r2_mobile";
  // 配置车牌识别参数
  HLPR_ContextConfiguration configuration = {0};
  configuration.models_path = model_path;     // 模型文件夹路径
  configuration.max_num = 5;                  // 最大识别车牌数量
  configuration.det_level = DETECT_LEVEL_LOW; // 检测器等级
  configuration.use_half = false;
  configuration.nms_threshold = 0.5f; // 非极大值抑制置信度阈值
  configuration.rec_confidence_threshold = 0.5f; // 车牌号文本阈值
  configuration.box_conf_threshold = 0.30f;      // 检测器阈值
  configuration.threads = 1;
  // 实例化车牌识别算法Context
  ctx = HLPR_CreateContext(&configuration);
  // 查询实例化状态
  HREESULT ret = HLPR_ContextQueryStatus(ctx);
  if (ret != HResultCode::Ok) {
    printf("HLPR_ContextQueryStatus create error.\n");
    // return;
    exit(0);
  }
  HLPR_PlateResultList results = {0};
}

PlateDetector::~PlateDetector() {

  // 销毁Context
  HLPR_ReleaseContext(ctx);
}

// void PlateDetector::detect(cv::Mat &img, std::string &ret_type,
//                            std::string &ret_code, float &ret_conf) {
//   // 读取图像
//   // cv::Mat image = cv::imread(image_path);
//   cv::Mat image;
//   img.copyTo(image);
//   // 创建ImageData
//   HLPR_ImageData data = {0};
//   data.data = image.ptr<uint8_t>(0); // 设置图像数据流
//   data.width = image.cols;           // 设置图像宽
//   data.height = image.rows;          // 设置图像高
//   data.format = STREAM_BGR;          // 设置当前图像编码格式
//   data.rotation = CAMERA_ROTATION_0; // 设置当前图像转角
//   // 创建数据Buffer
//   P_HLPR_DataBuffer buffer = HLPR_CreateDataBuffer(&data);

//   // // 配置车牌识别参数
//   // HLPR_ContextConfiguration configuration = {0};
//   // configuration.models_path = model_path;     // 模型文件夹路径
//   // configuration.max_num = 5;                  // 最大识别车牌数量
//   // configuration.det_level = DETECT_LEVEL_LOW; // 检测器等级
//   // configuration.use_half = false;
//   // configuration.nms_threshold = 0.5f; // 非极大值抑制置信度阈值
//   // configuration.rec_confidence_threshold = 0.5f; // 车牌号文本阈值
//   // configuration.box_conf_threshold = 0.30f;      // 检测器阈值
//   // configuration.threads = 1;
//   // // 实例化车牌识别算法Context
//   // P_HLPR_Context ctx = HLPR_CreateContext(&configuration);
//   // // 查询实例化状态
//   // HREESULT ret = HLPR_ContextQueryStatus(ctx);
//   // if (ret != HResultCode::Ok) {
//   //   printf("create error.\n");
//   //   return;
//   // }
//   HLPR_PlateResultList results = {0};
//   // 执行车牌识别算法
//   auto start = std::chrono::high_resolution_clock::now();

//   HLPR_ContextUpdateStream(ctx, buffer, &results);
//   auto end_step1 = std::chrono::high_resolution_clock::now();
//   std::chrono::duration<double> elapsed_step1 = end_step1 - start;
//   std::cout << "步骤1耗时: " << elapsed_step1.count() << "秒" << std::endl;

//   for (int i = 0; i < results.plate_size; ++i) {
//     // 解析识别后的数据
//     std::string type;
//     if (results.plates[i].type == HLPR_PlateType::PLATE_TYPE_UNKNOWN) {
//       type = "未知";
//     } else {
//       type = TYPES[results.plates[i].type];
//     }

//     printf("<%d> %s, %s, %f\n", i + 1, type.c_str(), results.plates[i].code,
//            results.plates[i].text_confidence);
//   }

//   // 销毁Buffer
//   HLPR_ReleaseDataBuffer(buffer);
//   // 销毁Context
//   // HLPR_ReleaseContext(ctx);
// }

void PlateDetector::detect(cv::Mat &img, std::string &ret_type,
                           std::string &ret_code, float &ret_conf) {
  ret_code = "";
  ret_conf = 0;
  // 创建ImageData
  HLPR_ImageData data = {0};
  Mat image;
  img.copyTo(image);
  data.data = image.ptr<uint8_t>(0); // 设置图像数据流
  data.width = image.cols;           // 设置图像宽
  data.height = image.rows;          // 设置图像高
  data.format = STREAM_BGR;          // 设置当前图像编码格式
  data.rotation = CAMERA_ROTATION_0; // 设置当前图像转角
  // 创建数据Buffer
  P_HLPR_DataBuffer buffer = HLPR_CreateDataBuffer(&data);
  // 执行车牌识别算法
  HLPR_PlateResultList results = {0};

  auto start = std::chrono::high_resolution_clock::now();

  HLPR_ContextUpdateStream(ctx, buffer, &results);
  auto end_step1 = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed_step1 = end_step1 - start;
  std::cout << "步骤1耗时: " << elapsed_step1.count() << "秒" << std::endl;

  if (results.plate_size > 1) {
    cout << "Error:plate_size > 1, plate_size " << results.plate_size << endl;
  }

  for (int i = 0; i < results.plate_size; ++i) {
    // 解析识别后的数据
    std::string type;
    if (results.plates[i].type == HLPR_PlateType::PLATE_TYPE_UNKNOWN) {
      type = "未知";
    } else {
      type = TYPES[results.plates[i].type];
    }

    if (type != "未知") {
      ret_code = results.plates[i].code;
      ret_conf = results.plates[i].text_confidence;
    }

    printf("<%d> %s, %s, %f\n", i + 1, type.c_str(), results.plates[i].code,
           results.plates[i].text_confidence);
  }
  // 销毁Buffer
  HLPR_ReleaseDataBuffer(buffer);
}

// int main(int argc, char **argv) {
//   char *model_path = argv[1];
//   char *image_path = argv[2];
//   // 读取图像
//   cv::Mat image = cv::imread(image_path);
//   // 创建ImageData
//   HLPR_ImageData data = {0};
//   data.data = image.ptr<uint8_t>(0); // 设置图像数据流
//   data.width = image.cols;           // 设置图像宽
//   data.height = image.rows;          // 设置图像高
//   data.format = STREAM_BGR;          // 设置当前图像编码格式
//   data.rotation = CAMERA_ROTATION_0; // 设置当前图像转角
//   // 创建数据Buffer
//   P_HLPR_DataBuffer buffer = HLPR_CreateDataBuffer(&data);

//   // 配置车牌识别参数
//   HLPR_ContextConfiguration configuration = {0};
//   configuration.models_path = model_path;     // 模型文件夹路径
//   configuration.max_num = 5;                  // 最大识别车牌数量
//   configuration.det_level = DETECT_LEVEL_LOW; // 检测器等级
//   configuration.use_half = false;
//   configuration.nms_threshold = 0.5f; // 非极大值抑制置信度阈值
//   configuration.rec_confidence_threshold = 0.5f; // 车牌号文本阈值
//   configuration.box_conf_threshold = 0.30f;      // 检测器阈值
//   configuration.threads = 1;
//   // 实例化车牌识别算法Context
//   P_HLPR_Context ctx = HLPR_CreateContext(&configuration);
//   // 查询实例化状态
//   HREESULT ret = HLPR_ContextQueryStatus(ctx);
//   if (ret != HResultCode::Ok) {
//     printf("create error.\n");
//     return -1;
//   }
//   HLPR_PlateResultList results = {0};
//   // 执行车牌识别算法
//   auto start = std::chrono::high_resolution_clock::now();

//   HLPR_ContextUpdateStream(ctx, buffer, &results);
//   auto end_step1 = std::chrono::high_resolution_clock::now();
//   std::chrono::duration<double> elapsed_step1 = end_step1 - start;
//   std::cout << "步骤1耗时: " << elapsed_step1.count() << "秒" << std::endl;

//   for (int i = 0; i < results.plate_size; ++i) {
//     // 解析识别后的数据
//     std::string type;
//     if (results.plates[i].type == HLPR_PlateType::PLATE_TYPE_UNKNOWN) {
//       type = "未知";
//     } else {
//       type = TYPES[results.plates[i].type];
//     }

//     printf("<%d> %s, %s, %f\n", i + 1, type.c_str(), results.plates[i].code,
//            results.plates[i].text_confidence);
//   }

//   // 销毁Buffer
//   HLPR_ReleaseDataBuffer(buffer);
//   // 销毁Context
//   HLPR_ReleaseContext(ctx);

//   return 0;
// }