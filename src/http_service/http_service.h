#ifndef __HTTP_SERVICE_H__
#define __HTTP_SERVICE_H__

#include "common/utility/Service.h"
#include "mongoose.h"
#include "json/json.h"

#include <sys/types.h>

#include <functional>
#include <iostream>
#include <thread>

class HttpService {
public:
  static HttpService &instance() {
    static HttpService instance;
    return instance;
  }

public:
  HttpService();
  ~HttpService();

  void start();
  void stop();

private:
  void work();
  static void ev_handler(struct mg_connection *c, int ev, void *ev_data,
                         void *fn_data);

  static bool httpBodyToJson(struct mg_http_message *hm, Json::Value &root);
  static bool httpBodyToJson(const std::string &str, Json::Value &root);

private:
  bool blistening_;

  struct mg_mgr mgr_;
  std::thread worker_;
  std::string s_http_ip_ = "http://0.0.0.0:";

  std::string tmp_uri_ = "";
};

#endif //__HTTP_SERVICE_H__