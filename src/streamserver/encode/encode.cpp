#include "streamserver/encode/encode.h"
#include "common/tool.h"

using namespace std;
using namespace cv;

namespace media {

h264Encoder::h264Encoder() {
  string codec_name = "h264_nvenc";

  int ret;

  codec = avcodec_find_encoder_by_name(codec_name.data());

  if (!codec) {
    fprintf(stderr, "Codec '%s' not found\n", codec_name.data());
    exit(1);
  }

  c = avcodec_alloc_context3(codec);
  if (!c) {
    fprintf(stderr, "Could not allocate video codec context\n");
    exit(1);
  }

  pkt = av_packet_alloc();
  if (!pkt)
    exit(1);

  /* put sample parameters */
  c->bit_rate = 819200;
  /* resolution must be a multiple of two */
  //    c->width = common::getPixWidth()/2;
  //    c->height = common::getPixHeight()/2;
  //    c->width = 640;
  //    c->height = 360;
  c->width = 1280;
  c->height = 960;
  /* frames per second */
  c->time_base = (AVRational){1, 25};
  c->framerate = (AVRational){25, 1};

  /* emit one intra frame every ten frames
   * check frame pict_type before passing frame
   * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
   * then gop_size is ignored and the output of encoder
   * will always be I frame irrespective to gop_size
   */
  c->gop_size = 30;
  c->max_b_frames = 0;
  c->pix_fmt = AV_PIX_FMT_YUV420P;

  if (codec->id == AV_CODEC_ID_H264)
    av_opt_set(c->priv_data, "preset", "slow", 0);

  /* open it */
  ret = avcodec_open2(c, codec, NULL);
  if (ret < 0) {
    //        fprintf(stderr, "Could not open codec: %s\n", av_err2str(ret));
    exit(1);
  }

  frame = av_frame_alloc();
  if (!frame) {
    fprintf(stderr, "Could not allocate video frame\n");
    exit(1);
  }
  frame->format = c->pix_fmt;
  frame->width = c->width;
  frame->height = c->height;

  ret = av_frame_get_buffer(frame, 0);
  if (ret < 0) {
    fprintf(stderr, "Could not allocate the video frame data\n");
    exit(1);
  }
}

h264Encoder::~h264Encoder() {
  if (c != nullptr) {
    avcodec_free_context(&c);
  }

  if (frame != nullptr) {
    av_frame_free(&frame);
  }

  if (pkt != nullptr) {
    av_packet_free(&pkt);
  }
}

void h264Encoder::encode(AVCodecContext *enc_ctx, AVFrame *frame) {
  int ret;

  /* send the frame to the encoder */
  //    if (frame)
  //        printf("Send frame %3"PRId64"\n", frame->pts);

  ret = avcodec_send_frame(enc_ctx, frame);
  if (ret < 0) {
    fprintf(stderr, "Error sending a frame for encoding\n");
    exit(1);
  }

  //    while (ret >= 0) {
  ret = avcodec_receive_packet(enc_ctx, pkt);
  if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
    return;
  else if (ret < 0) {
    fprintf(stderr, "Error during encoding\n");
    exit(1);
  }

  //        printf("Write packet %3"PRId64" (size=%5d)\n", pkt->pts, pkt->size);
  //        fwrite(pkt->data, 1, pkt->size, outfile);
  //        av_packet_unref(pkt);
  //        cout<<"encode pkt->size:"<<pkt->size<<endl;
  //    }
}

AVPacket *h264Encoder::pushFrame(common::Image &src) {
  //    static int frame_id = 0;
  /* Make sure the frame data is writable.
     On the first round, the frame is fresh from av_frame_get_buffer()
     and therefore we know it is writable.
     But on the next rounds, encode() will have called
     avcodec_send_frame(), and the codec may have kept a reference to
     the frame in its internal structures, that makes the frame
     unwritable.
     av_frame_make_writable() checks that and allocates a new buffer
     for the frame only if necessary.
   */
  int ret = av_frame_make_writable(frame);
  if (ret < 0)
    exit(1);

  /* Prepare a dummy image.
     In real code, this is where you would have your own logic for
     filling the frame. FFmpeg does not care what you put in the
     frame.
   */

  Mat dst;
  //    cvtColor(src,dst,CV_RGB2YCrCb);
  //    cout<<"YCrCb:"<<dst.cols<<","<<dst.rows<<endl;
  cvtColor(src.img, dst, COLOR_RGB2YUV_I420);
  cout << "I420:" << dst.cols << "," << dst.rows << endl;

  for (int i = 0; i < c->height; i++) {
    memcpy(&frame->data[0][i * frame->linesize[0]], (void *)dst.ptr(i),
           dst.cols);
    //        memcpy((void*)(&frame->data[0][i *
    //        frame->linesize[0]]),(void*)dst.data,dst.cols);
  }
  int uv_lines = c->height / 4;
  for (int i = 0; i < uv_lines; i++) {
    memcpy(&frame->data[2][(2 * i) * frame->linesize[2]],
           (void *)dst.ptr(i + c->height), dst.cols / 2);
    memcpy(&frame->data[2][(2 * i + 1) * frame->linesize[2]],
           ((uchar *)(dst.ptr(i + c->height)) + dst.cols / 2 - 1),
           dst.cols / 2);
  }
  int lines = c->height + c->height / 4;
  for (int i = 0; i < uv_lines; i++) {
    memcpy(&frame->data[1][(2 * i) * frame->linesize[1]],
           (void *)dst.ptr(i + lines), dst.cols / 2);
    memcpy(&frame->data[1][(2 * i + 1) * frame->linesize[1]],
           ((uchar *)(dst.ptr(i + lines)) + dst.cols / 2 - 1), dst.cols / 2);
  }

  /* Y */
  //    for (y = 0; y < c->height; y++) {
  //        for (x = 0; x < c->width; x++) {
  //            frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
  //        }
  //    }

  /* Cb and Cr */
  //    for (y = 0; y < c->height/2; y++) {
  //        for (x = 0; x < c->width/2; x++) {
  //            frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
  //            frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
  //        }
  //    }

  frame->pts = src.frame_id;
  /* encode the image */
  encode(c, frame);
  //    cout<<"encode2 pkt->size:"<<pkt->size<<endl;
  return pkt;
}

} // namespace media
