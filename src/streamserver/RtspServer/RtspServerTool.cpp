#include "streamserver/RtspServer/RtspServerTool.h"
using namespace std;

namespace media {

RtspServerTool::RtspServerTool() {
  is_start = false;
  m_worker_ = std::make_shared<std::thread>([this] { listen(); });
}

RtspServerTool::~RtspServerTool() {}

void RtspServerTool::listen() {
  std::string suffix = "live";
  std::string ip = "127.0.0.1";
  // std::string ip = "192.168.96.239";
  std::string port = "5554";
  //    port = "631";
  //    std::string port2 = "650";
  std::string rtsp_url = "rtsp://" + ip + ":" + port + "/" + suffix;

  std::shared_ptr<xop::EventLoop> event_loop(new xop::EventLoop());
  std::shared_ptr<xop::RtspServer> server =
      xop::RtspServer::Create(event_loop.get());

  if (!server->Start("0.0.0.0", atoi(port.c_str()))) {
    printf("RTSP Server listen on %s failed.\n", port.c_str());
    return;
  }

  xop::MediaSession *session = xop::MediaSession::CreateNew("live");
  session->AddSource(xop::channel_0, xop::H264Source::CreateNew());
  // session->StartMulticast();
  session->AddNotifyConnectedCallback([](xop::MediaSessionId sessionId,
                                         std::string peer_ip,
                                         uint16_t peer_port) {
    printf("RTSP client connect, ip=%s, port=%hu \n", peer_ip.c_str(),
           peer_port);
  });

  session->AddNotifyDisconnectedCallback([](xop::MediaSessionId sessionId,
                                            std::string peer_ip,
                                            uint16_t peer_port) {
    printf("RTSP client disconnect, ip=%s, port=%hu \n", peer_ip.c_str(),
           peer_port);
  });

  session_id = server->AddSession(session);
  rtsp_server = server.get();

  is_start = true;

  while (1) {
    xop::Timer::Sleep(100);
  }
}

void RtspServerTool::pushH264Frame(AVPacket *pkt) {
  if (!is_start)
    return;
  static int buf_size = 2000000;
  static std::unique_ptr<uint8_t> frame_buf(new uint8_t[buf_size]);

  //    cout<<"pkt->size:"<<pkt->size<<endl;
  if (pkt->size > 0) {
    memcpy(frame_buf.get(), pkt->data, pkt->size);
    xop::AVFrame videoFrame = {0};
    videoFrame.type = 0;
    videoFrame.size = pkt->size;
    videoFrame.timestamp = xop::H264Source::GetTimestamp();
    videoFrame.buffer.reset(new uint8_t[videoFrame.size]);
    memcpy(videoFrame.buffer.get(), frame_buf.get(), videoFrame.size);
    //        cout<<"videoFrame.size:"<<videoFrame.size<<endl;
    rtsp_server->PushFrame(session_id, xop::channel_0, videoFrame);
  }
  //    else {
  //        break;
  //    }
}

} // namespace media
