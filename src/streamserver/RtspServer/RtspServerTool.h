#ifndef _RTSP_SERVER_TOOL_
#define _RTSP_SERVER_TOOL_
extern "C"
{
#include <libavcodec/avcodec.h>
}
#include "xop/RtspServer.h"
#include "net/Timer.h"
#include <thread>
#include <memory>
#include <iostream>
#include <string>

namespace media {

class RtspServerTool{
public:
    RtspServerTool();
    ~RtspServerTool();
    void pushH264Frame(AVPacket* pkt);

private:
    xop::MediaSessionId session_id;
    xop::RtspServer* rtsp_server;

    std::shared_ptr<std::thread> m_worker_;
    void listen();


    bool is_start;
};

}



#endif
