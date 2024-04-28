#ifndef _ENCODE_
#define _ENCODE_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "common/type.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>

}
#include <opencv2/opencv.hpp>

namespace media {

class h264Encoder{
public:
    h264Encoder();
    ~h264Encoder();
    AVPacket* pushFrame(common::Image& src);

private:
    AVPacket *pkt;
    const AVCodec *codec;
    AVCodecContext *c;
    AVFrame *frame;

    void encode(AVCodecContext *enc_ctx, AVFrame *frame);
};

}


#endif
