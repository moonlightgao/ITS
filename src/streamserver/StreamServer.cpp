#include "StreamServer.h"
#include "DrawService.h"

using namespace cv;
using namespace std;

void StreamServer::start() {
  m_worker_ = std::make_shared<std::thread>([this] { task(); });
}

void StreamServer::stop() { m_bRunning = false; }

StreamServer::StreamServer() {
  encoder_ptr = make_shared<media::h264Encoder>();
  rtsp_server_ptr = make_shared<media::RtspServerTool>();
}

StreamServer::~StreamServer() {}

void StreamServer::task() {
  m_bRunning = true;
  while (m_bRunning) {
    if (!frame_que.empty()) {
      common::Image &frame = frame_que.front();
      rtsp_server_ptr->pushH264Frame(encoder_ptr->pushFrame(frame));
      frame_que.pop();
    }
    xop::Timer::Sleep(5);
  }
}

void StreamServer::pushFrame(common::Image frame) {
  cout << "StreamServer::pushFrame" << endl;
  frame_que.push(frame);
  if (frame_que.size() > 10) {
    frame_que.pop();
  }
}
