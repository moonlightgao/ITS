#include "PlateSegmente.h"
#include <chrono>
#include <opencv2/opencv.hpp>
#include <torch/script.h>
#include <torch/torch.h>

using namespace std;
using namespace cv;

//   torch::Device device;
std::shared_ptr<torch::Device> device_ptr;
torch::jit::script::Module model;

PlateSegmente::PlateSegmente() {

  input_width = 200;
  input_height = 200;

  cout << "input_height " << input_height << endl;

  // const std::string model_path =
  //     "/home/alice/project/Pytorch-UNet/traced_unet_model.pt";
  const std::string model_path =
      "/home/alice/project/Pytorch-UNet/traced_unet_model.pt";
  const std::string image_path = "/home/alice/data/car_10img/1377_1WK.jpg";

  // 初始化模型
  try {
    // 加载模型
    model = torch::jit::load(model_path);
  } catch (const c10::Error &e) {
    std::cerr << "Error loading model\n";
    return;
  }

  std::cout << "Model loaded successfully\n";
  // 设置设备
  // device = torch::Device(torch::kCUDA); // 或者 torch::kCPU
  device_ptr = make_shared<torch::Device>(torch::kCUDA);
  model.to(*device_ptr);
}

PlateSegmente::~PlateSegmente() {}

void PlateSegmente::doInferenceFrieza(cv::Mat &img,
                                      std::vector<common::ObjInfo> &objs) {

  static int imwrite_index = 0;
  cv::Size targetSize(640, 640); // 例如 640x480 的分辨率

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
      cout << "blankImage sized blankImage cols " << blankImage.cols
           << " input_width " << input_width << " input_height " << input_height
           << endl;

      Mat image;
      resize(blankImage, image, Size(input_width, input_height));
      float resize_bili = input_width / (float)img_size;
      cout << "blankImage sized end" << endl;

      cv::cvtColor(image, image,
                   cv::COLOR_BGR2RGB); // UNet模型通常以RGB格式输入
      image.convertTo(image, CV_32FC3, 1.0f / 255.0f); // 归一化

      // 转换图片为Tensor
      auto input_tensor =
          torch::from_blob(image.data, {1, input_height, input_width, 3});
      input_tensor = input_tensor.permute({0, 3, 1, 2}); // 转换为CHW格式
      input_tensor = input_tensor.to(torch::kFloat);
      // input_tensor = input_tensor.to(torch::kInt);
      input_tensor = input_tensor.to(*device_ptr);

      // 推理
      torch::NoGradGuard no_grad;
      auto output_tensor = model.forward({input_tensor}).toTensor();

      output_tensor = output_tensor.squeeze().detach().cpu();

      // 获取每个像素点的最大概率值及其对应的索引
      auto max_probs = torch::max(
          output_tensor, 0); // max_probs将是一个tuple，其中包含最大值和索引
      auto probs = std::get<0>(max_probs);                  // 最大概率值
      auto preds = std::get<1>(max_probs).to(torch::kByte); //
      // 最大概率值对应的索引

      // 应用概率阈值
      auto mask = (probs > 2).to(torch::kByte) *
                  preds; // 如果概率大于0.5，则保留类别索引，否则乘以0得到类别0
      for (int seg_i = 0; seg_i < input_height; seg_i++) {
        for (int seg_j = 0; seg_j < input_width; seg_j++) {
          float org_x, org_y;
          org_x = seg_j / resize_bili + left;
          org_y = seg_i / resize_bili + top;

          // 获取每个像素点的概率和类别
          float probability = probs[seg_i][seg_j].item<float>();
          int category = preds[seg_i][seg_j].item<int>();
          cout << category << " (" << org_x << "," << org_y << ")"
               << " ";
          if (category == 1) {
            objs[i].car_contours.push_back(Point(org_x, org_y));

          } else if (category == 2) {
            objs[i].plate_contours.push_back(Point(org_x, org_y));
          }
        }
        cout << endl;

        /* code */
      }

      // mask = mask.mul(100); // 转换为0-255的灰度图

      // // 将Tensor转换为OpenCV Mat

      // cv::Mat mask_mat(cv::Size(input_width, input_height), CV_8UC1,
      //                  mask.data_ptr<uint8_t>());

      // // 调整mask大小以匹配原图像
      // cv::Mat mask_resized;
      // cout << "mask_resized" << endl;
      // cv::resize(mask_mat, mask_resized, blankImage.size());
      // cout << "mask_resized end" << endl;

      // cv::Mat colored_mask;
      // cv::cvtColor(mask_resized, colored_mask, cv::COLOR_GRAY2BGR); //
      // 转为彩色

      // // 叠加mask到原图像上
      // cv::Mat result;
      // cv::addWeighted(colored_mask, 0.5, blankImage, 0.5, 0.0, result);

      // // // 显示结果
      // cv::imshow("Segmentation Result", result);
    }
  }
}
void PlateSegmente::InferSegment(cv::Mat &img) {}
