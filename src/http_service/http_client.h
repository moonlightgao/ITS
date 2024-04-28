#ifndef __HTTP_CLIENT_H__
#define __HTTP_CLIENT_H__
#include "mongoose.h"

#include <iostream>
#include <mutex>
#include <string>

struct dg_response
{
    std::string url;
    std::string head;
    std::string body;
};

class HttpClient {
public:
    static HttpClient &service()
    {
        static HttpClient instance;
        return instance;
    }

public:
    HttpClient();
    ~HttpClient();
    dg_response request(const std::string &url, const std::string &post_data, std::string token = "");

private:
    // static std::string s_url;
    // static std::string s_post_data;      // POST data
    static dg_response rep;
    // static uint64_t s_timeout_ms;  // Connect timeout in milliseconds
    static std::mutex mtx_connext_;
    struct mg_mgr     mgr;
    static void       fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data);
};

#endif //__HTTP_CLIENT_H__