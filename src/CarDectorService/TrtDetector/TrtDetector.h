#ifndef _TRT_DETECTOR_
#define _TRT_DETECTOR_

#include <algorithm>
#include <opencv2/opencv.hpp>
#include <assert.h>
#include <cmath>
#include <cuda_runtime_api.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <time.h>

#include "NvInfer.h"
#include "NvOnnxParser.h"
//#include "NvOnnxParserRuntime.h"
#include "TrtDetector/common/argsParser.h"
#include "TrtDetector/common/logger.h"
#include "TrtDetector/common/common.h"

#include "TrtDetector/common/yolo_layer.h"
#include "common/type.h"
//using namespace std;
//using namespace nvinfer1;
//using namespace nvonnxparser;
//using namespace cv;
//using namespace sample;


namespace DetTrt {

typedef struct DetectionRes {
    float x, y, w, h, prob;
} DetectionRes;

inline float exponential(float in) {
    return exp(in);
}

inline float* merge(float* out1, float* out2, int bsize_out1, int bsize_out2)
{
    float* out_total = new float[bsize_out1 + bsize_out2];

    for (int j = 0; j < bsize_out1; ++j)
    {
        int index = j;
        out_total[index] = out1[j];
    }

    for (int j = 0; j < bsize_out2; ++j)
    {
        int index = j + bsize_out1;
        out_total[index] = out2[j];
    }
    return out_total;
}


inline int64_t volume(const nvinfer1::Dims& d)
{
    return std::accumulate(d.d, d.d + d.nbDims, 1, std::multiplies<int64_t>());
}

inline unsigned int getElementSize(nvinfer1::DataType t)
{
    switch (t)
    {
    case nvinfer1::DataType::kINT32: return 4;
    case nvinfer1::DataType::kFLOAT: return 4;
    case nvinfer1::DataType::kHALF: return 2;
    case nvinfer1::DataType::kINT8: return 1;
    }
    throw std::runtime_error("Invalid DataType.");
    return 0;
}

class TrtDetector{
public:

    TrtDetector();
    ~TrtDetector();

    void doInferenceFrieza(cv::Mat& src,std::vector<common::ObjInfo> & bboxs,const unsigned long long & time_stamp);

private:
    std::string engineFile;

    float obj_threshold;
    float nms_threshold;
    int CATEGORY;
    int BATCH_SIZE;
    int INPUT_CHANNEL;
    int DETECT_WIDTH;
    int DETECT_HEIGHT;


    samplesCommon::Args gArgs;

    IHostMemory* trtModelStream;
    IRuntime* runtime;
    ICudaEngine* engine;
    IExecutionContext* context;
    cudaStream_t stream;

    void* buffers[3];
    std::vector<int64_t> bufferSize;
    int outSize1;
    int outSize2;
    float* out2;//save inference result need delete

    bool readTrtFile();
    std::vector<float> prepareImage(cv::Mat& img,int DETECT_WIDTH,int DETECT_HEIGHT);
    void DoNms(std::vector<DetectionRes>& detections, float nmsThresh);
    void postProcess(float * output,int outsize,int width,int height,std::vector<DetectionRes> & objs);
};


}


#endif
