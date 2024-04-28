#ifndef _STREAM_SERVER_
#define _STREAM_SERVER_
#include "common/utility/Service.h"
#include <atomic>
#include <opencv2/opencv.hpp>
#include <queue>
#include <thread>

#include "RtspServer/RtspServerTool.h"
#include "encode/encode.h"

#include "common/type.h"

class StreamServer {
private:
  int overload = 0;
  std::shared_ptr<std::thread> m_worker_;
  std::atomic_bool m_bRunning;
  //    std::shared_ptr<processor::StreamPipeline> m_stream_pipeline_;

  std::queue<common::Image> frame_que;
  std::shared_ptr<media::h264Encoder> encoder_ptr;
  std::shared_ptr<media::RtspServerTool> rtsp_server_ptr;
  //    media::h264Encoder encoder;
  //    media::RtspServerTool rtsp_server;
public:
  static StreamServer &instance() {

    static StreamServer inst;
    return inst;
  }
  StreamServer();
  ~StreamServer();
  void start();
  void stop();
  void pushFrame(common::Image frame);

private:
  void task();

  //    void pushFrame(dataio::Frame::ptr frame);
};

#endif
