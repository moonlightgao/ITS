#include "TrtDetector.h"

using namespace std;
using namespace nvinfer1;
using namespace nvonnxparser;
using namespace cv;
using namespace sample;
using namespace DetTrt;

TrtDetector::TrtDetector() {
  // engineFile =
  //     "/home/alice/project/tensorrt_demos-master/yolo/yolov3-int8-416.trt";
  engineFile =
      "/home/alice/project/tensorrt_demos-master/yolo/yolov3-int8-608.trt";

  obj_threshold = 0.4;
  nms_threshold = 0.4;
  CATEGORY = 1;
  BATCH_SIZE = 1;
  INPUT_CHANNEL = 3;
  DETECT_WIDTH = 608;
  DETECT_HEIGHT = 608;

  buffers[0] = nullptr;
  buffers[1] = nullptr;
  buffers[2] = nullptr;

  stream = nullptr;
  context = nullptr;
  engine = nullptr;
  context = nullptr;

  readTrtFile();
}

TrtDetector::~TrtDetector() {
  if (stream != nullptr) {
    cudaStreamDestroy(stream);
  }
  if (buffers[0] != nullptr) {
    CHECK(cudaFree(buffers[0]));
  }
  if (buffers[1] != nullptr) {
    CHECK(cudaFree(buffers[1]));
  }
  if (buffers[2] != nullptr) {
    CHECK(cudaFree(buffers[2]));
  }

  //    CHECK(cudaFree(buffers[1]));
  //    CHECK(cudaFree(buffers[2]));

  // destroy the engine
  if (context != nullptr) {
    context->destroy();
  }
  if (engine != nullptr) {
    engine->destroy();
  }
  if (context != nullptr) {
    runtime->destroy();
  }
}

/**
 *
 *
 */
bool TrtDetector::readTrtFile() // output buffer for the TensorRT model
{
  fstream file;
  cout << "loading filename from:" << engineFile << endl;
  file.open(engineFile, ios::binary | ios::in);
  file.seekg(0, ios::end);
  int length = file.tellg();
  // cout << "length:" << length << endl;
  file.seekg(0, ios::beg);
  std::unique_ptr<char[]> data(new char[length]);
  file.read(data.get(), length);
  file.close();
  cout << "load engine done" << endl;
  std::cout << "deserializing" << endl;
  runtime = createInferRuntime(gLogger.getTRTLogger());
  engine = runtime->deserializeCudaEngine(data.get(), length);
  cout << "deserialize done" << endl;
  trtModelStream = engine->serialize();
  context = engine->createExecutionContext();
  // get engine
  assert(runtime != nullptr);
  if (gArgs.useDLACore >= 0) {
    runtime->setDLACore(gArgs.useDLACore);
  }
  // 创建推理引擎
  assert(engine != nullptr);
  assert(context != nullptr);

  //读取输入数据到缓冲区管理对象中
  cout << "engine->getNbBindings():" << engine->getNbBindings() << endl;
  assert(engine->getNbBindings() == 2);

  int nbBindings = engine->getNbBindings();
  bufferSize.resize(nbBindings);

  for (int i = 0; i < nbBindings; ++i) {
    nvinfer1::Dims dims = engine->getBindingDimensions(i);
    nvinfer1::DataType dtype = engine->getBindingDataType(i);
    int64_t totalSize = volume(dims) * 1 * getElementSize(dtype);
    bufferSize[i] = totalSize;
    CHECK(cudaMalloc(&buffers[i], totalSize));
  }

  // 创建CUDA流以执行此推断
  //    cudaStream_t stream;
  CHECK(cudaStreamCreate(&stream));

  outSize1 = bufferSize[0] / sizeof(float);
  outSize2 = bufferSize[1] / sizeof(float);
  out2 = new float[outSize2];

  return true;
}

/**
 * @brief 执行图像推理
 * @param src 输入彩色单目原图
 * @param bboxs 返回推理结果集合
 * @param time_stamp 当前传入的时间戳
 */
void TrtDetector::doInferenceFrieza(Mat &src, vector<common::ObjInfo> &bboxs,
                                    const unsigned long long &time_stamp) {
  //图像前处理
  vector<float> curInput = prepareImage(src, DETECT_WIDTH, DETECT_HEIGHT);
  // 将数据从主机输入缓冲区异步复制到设备输入缓冲区
  CHECK(cudaMemcpyAsync(buffers[0], curInput.data(), bufferSize[0],
                        cudaMemcpyHostToDevice, stream));
  // 执行推理
  auto t_start = std::chrono::high_resolution_clock::now();
  context->execute(BATCH_SIZE, buffers);
  auto t_end = std::chrono::high_resolution_clock::now();
  float total =
      std::chrono::duration<float, std::milli>(t_end - t_start).count();
  std::cout << "Inference take: " << total << " ms." << endl;
  //推理结果从gpu获取
  CHECK(cudaMemcpyAsync(out2, buffers[1], bufferSize[1], cudaMemcpyDeviceToHost,
                        stream));
  cudaStreamSynchronize(stream);
  vector<DetectionRes> detections;
  //后处理
  postProcess(out2, outSize2, src.cols, src.rows, detections);
  //数据格式对外部调用的转换
  bboxs.clear();
  for (DetectionRes &ret : detections) {
    common::ObjInfo obj_info;
    obj_info.timestamp = time_stamp;
    obj_info.bbox_info.conf = ret.prob;
    obj_info.bbox_info.x = ret.x;
    obj_info.bbox_info.y = ret.y;
    obj_info.bbox_info.width = ret.w;
    obj_info.bbox_info.height = ret.h;
    bboxs.push_back(obj_info);
  }
}
/**
 * @brief 图像预处理
 * @param img 输入彩色单目原图
 * @param DETECT_WIDTH 处理后目标图像宽
 * @param DETECT_HEIGHT 处理后目标图像高
 * @return vector<float> 存放处理后数据的容器
 */
vector<float> TrtDetector::prepareImage(cv::Mat &img, int DETECT_WIDTH,
                                        int DETECT_HEIGHT) {
  int c = 3;             //网络输入图像通道数
  int h = DETECT_HEIGHT; // net h
  int w = DETECT_WIDTH;  // net w
  //缩放因子计算
  float scale = min(float(w) / img.cols, float(h) / img.rows);
  auto scaleSize = cv::Size(img.cols * scale, img.rows * scale);
  cv::Mat rgb;
  // bgr->rgb
  cv::cvtColor(img, rgb, COLOR_BGR2RGB);
  cv::Mat resized;
  //分辨率转换与黑边填充
  cv::resize(rgb, resized, scaleSize, 0, 0, INTER_CUBIC);
  cv::Mat cropped(h, w, CV_8UC3, 127);
  Rect rect((w - scaleSize.width) / 2, (h - scaleSize.height) / 2,
            scaleSize.width, scaleSize.height);
  resized.copyTo(cropped(rect));
  //图像数据由int8->float
  cv::Mat img_float;
  cropped.convertTo(img_float, CV_32FC3, 1.f / 255.0);
  //数据格式转换HWC->CHW
  vector<Mat> input_channels(c);
  //通道分离
  cv::split(img_float, input_channels);
  vector<float> result(h * w * c);
  auto data = result.data();
  int channelLength = h * w;
  //拷贝数据到vector容器
  for (int i = 0; i < c; ++i) {
    memcpy(data, input_channels[i].data, channelLength * sizeof(float));
    data += channelLength;
  }
  return result;
}
/**
 * @brief 图像后处理
 * @param output 输入推理结果
 * @param outsize 输出层数据长度
 * @param width、height 原图像的宽高
 * @param detections 返回检测结果
 */
void TrtDetector::postProcess(float *output, int outsize, int width, int height,
                              vector<DetectionRes> &detections) {
  int h = DETECT_HEIGHT; // net h
  int w = DETECT_WIDTH;  // net w
  //重新计算缩放因子
  float scale = min(float(w) / width, float(h) / height);
  float scaleSize[] = {width * scale, height * scale};
  //数据解析
  for (int i = 0; i < outsize; i += 7) {
    if (output[i + 4] * output[i + 6] > obj_threshold && output[i + 4] > 0.1) {
      //执信度大于阈值
      vector<float> alone;
      for (int j = 0; j < 7; j++) {
        alone.push_back(output[i + j]);
      }
      //数据转换
      DetectionRes bbox;
      bbox.x = (output[i] * w - (w - scaleSize[0]) / 2.f) / scale;
      bbox.y = (output[i + 1] * h - (h - scaleSize[1]) / 2.f) / scale;
      bbox.w = output[i + 2] * w;
      bbox.h = output[i + 3] * h;
      bbox.w /= scale;
      bbox.h /= scale;
      bbox.prob = output[i + 4] * output[i + 6];
      detections.push_back(bbox);
    }
  }
  // nms算法二次筛选，去掉重叠过大区域
  float nmsThresh = nms_threshold;
  if (nmsThresh > 0)
    DoNms(detections, nmsThresh);
}
/**
 * @brief nms算法二次处理
 * @param detections 未筛选检测结果
 * @param nmsThresh nms阈值
 */
void TrtDetector::DoNms(std::vector<DetectionRes> &detections,
                        float nmsThresh) {
  //对两个检测框筛选的匿名函数
  auto iouCompute = [](float *lbox, float *rbox) {
    float interBox[] = {
        std::max(lbox[0], rbox[0]),                     // left
        std::min(lbox[0] + lbox[2], rbox[0] + rbox[2]), // right
        std::max(lbox[1], rbox[1]),                     // top
        std::min(lbox[1] + lbox[3], rbox[1] + rbox[3]), // bottom
    };
    if (interBox[2] >= interBox[3] || interBox[0] >= interBox[1])
      return 0.0f;
    float interBoxS =
        (interBox[1] - interBox[0] + 1) * (interBox[3] - interBox[2] + 1);
    return interBoxS / (lbox[2] * lbox[3] + rbox[2] * rbox[3] - interBoxS);
  };
  //按prob值进行排序
  sort(detections.begin(), detections.end(),
       [=](const DetectionRes &left, const DetectionRes &right) {
         return left.prob > right.prob;
       });
  vector<DetectionRes> result;
  //两两匹配
  for (unsigned int m = 0; m < detections.size(); ++m) {
    result.push_back(detections[m]);
    for (unsigned int n = m + 1; n < detections.size(); ++n) {
      if (iouCompute((float *)(&detections[m]), (float *)(&detections[n])) >
          nmsThresh) {
        detections.erase(detections.begin() + n);
        --n;
      }
    }
  }
  detections = move(result);
}

// void TrtDetector::postProcess(float *output, int outsize, int width, int
// height,
//                               vector<DetectionRes> &detections) {
//   //    detections.clear();
//   //    vector<DetectionRes> detections;

//   int h = DETECT_HEIGHT; // net h
//   int w = DETECT_WIDTH;  // net w

//   float scale = min(float(w) / width, float(h) / height); // scale bbox to
//   img

//   float scaleSize[] = {width * scale, height * scale};

//   //    cout<<"postProcess2 out:";
//   //    vector<vector<float>> obj_detections;
//   for (int i = 0; i < outsize; i += 7) {
//     if (output[i + 4] * output[i + 6] > obj_threshold && output[i + 4] > 0.1)
//     {
//       //            cout<<"find";
//       vector<float> alone;
//       for (int j = 0; j < 7; j++) {
//         //                cout<<output[i+j]<<" ";
//         alone.push_back(output[i + j]);
//       }
//       //            cout<<endl;
//       //            obj_detections.push_back(alone);

//       DetectionRes bbox;
//       bbox.x = (output[i] * w - (w - scaleSize[0]) / 2.f) / scale;
//       bbox.y = (output[i + 1] * h - (h - scaleSize[1]) / 2.f) / scale;
//       bbox.w = output[i + 2] * w;
//       bbox.h = output[i + 3] * h;
//       bbox.w /= scale;
//       bbox.h /= scale;
//       bbox.prob = output[i + 4] * output[i + 6];

//       detections.push_back(bbox);
//     }
//   }

//   // nms
//   float nmsThresh = nms_threshold;
//   if (nmsThresh > 0)
//     DoNms(detections, nmsThresh);

//   //    cout<<"detections.size:"<<detections.size()<<endl;
// }
