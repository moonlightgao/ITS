#include "http_service.h"

#include "ViolationDetectionService.h"
#include "http_client.h"
#include "type.h"
#include <stdio.h>
#include <string.h>
#include <sys/vfs.h>

#include <iostream>

using namespace std;

// CommandListener *HttpService::listener_ = nullptr;

HttpService::HttpService() {
  blistening_ = false;
  mg_mgr_init(&mgr_);
  std::string s_http_addr_;
  mg_connection *listen_ptr = NULL;
  int port = 8080;
  s_http_addr_ = s_http_ip_ + to_string(port);
  listen_ptr =
      mg_http_listen(&mgr_, s_http_addr_.c_str(), this->ev_handler, NULL);
  if (listen_ptr == NULL) {
    cout << "http service failed to initialize" << endl;
  } else {
    cout << "The http service port is " << port - 1;
  }
}

HttpService::~HttpService() {}

void HttpService::work() {
  cout << "HttpService running...";
  while (blistening_) {
    mg_mgr_poll(&mgr_, 1000);
  }
}

void HttpService::start() {
  if (!this->blistening_) {
    this->blistening_ = true;

    worker_ = std::thread(&HttpService::work, this);
    worker_.detach();
  }
}

void HttpService::stop() {
  if (this->blistening_) {
    this->blistening_ = false;
    mg_mgr_free(&mgr_);
  }
}

bool HttpService::httpBodyToJson(struct mg_http_message *hm,
                                 Json::Value &root) {
  bool res;
  JSONCPP_STRING errs;
  Json::CharReaderBuilder readerBuilder;

  struct mg_str params = mg_str_n(hm->body.ptr, hm->body.len);
  std::unique_ptr<Json::CharReader> const jsonReader(
      readerBuilder.newCharReader());
  res = jsonReader->parse(params.ptr, params.ptr + params.len, &root, &errs);
  if (!res || !errs.empty()) {
    cout << "parseJson err. " << errs;
    return false;
  }
  return true;
}

bool HttpService::httpBodyToJson(const string &str, Json::Value &root) {
  bool res;
  JSONCPP_STRING errs;
  Json::CharReaderBuilder readerBuilder;
  std::unique_ptr<Json::CharReader> const jsonReader(
      readerBuilder.newCharReader());

  res = jsonReader->parse(str.data(), str.data() + str.length(), &root, &errs);
  if (!res || !errs.empty()) {
    cout << "parseJson err. " << errs;
    return false;
  }
  return true;
}

#include <random>
static std::string strRand(int length) {
  char tmp;
  string buf;
  std::random_device rd;
  std::default_random_engine random(rd());

  for (size_t i = 0; i < length; i++) {
    tmp = random() % 36;
    if (tmp < 10) {
      tmp += '0';
    } else {
      tmp -= 10;
      tmp += 'a';
    }
    buf += tmp;
  }
  return buf;
}

void HttpService::ev_handler(struct mg_connection *c, int ev, void *ev_data,
                             void *fn_data) {

  if (ev == MG_EV_ACCEPT && fn_data != NULL) {
    // todo init https params
  } else if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *)ev_data;
    Json::Value response;
    response["code"] = 200;
    response["message"] = "OK";
    // Json::Value root;

    // cout << string(hm->uri.ptr, hm->uri.len) << " Called";
    // deepedge::util::InfoL << string(hm->uri.ptr, hm->uri.len) << " Called";
    if (mg_http_match_uri(hm, "/api/updateCarLines")) {

      Json::Value response;
      response["code"] = 200;
      response["message"] = "OK";
      if (!mg_vcmp(&hm->method, "POST")) {
        cout << " /api/updateCarLines POST Called" << endl;
        Json::Value root;
        if (!httpBodyToJson(hm, root)) {
          response["code"] = 400;
          response["message"] = "invalid params";
          mg_http_reply(c, 400, "Content-Type: application/json\r\n",
                        response.toStyledString().c_str(), (int)hm->uri.len,
                        hm->uri.ptr);
        } else {
          cout << "updateCarLines: " << root.toStyledString() << endl;
          vector<vector<common::LinePoint>> car_lines;
          if (root.isArray()) {
            for (int i = 0; i < root.size(); i++) {
              if (root[i].isArray()) {
                vector<common::LinePoint> car_line;
                for (int j = 0; j < root[i].size(); j++) {
                  if (root[i][j].isMember("point_x") &&
                      root[i][j].isMember("point_y") &&
                      root[i][j].isMember("distance")) {
                    common::LinePoint line_point;
                    line_point.p = cv::Point(root[i][j]["point_x"].asInt(),
                                             root[i][j]["point_y"].asInt());
                    line_point.distance = root[i][j]["distance"].asInt();
                    car_line.push_back(line_point);
                  }
                }
                if (car_line.size() >= 2) {
                  car_lines.push_back(car_line);
                }
              }
            }
          }
          //   if (root.isMember("roi")) {
          //     Json::Value roi_info = root["roi"];
          //     // updateRoiInfo(roi_info);
          //     ConnectController::service().roi_controller()->setRoi(roi_info);
          //   }
          ViolationDetectionService::instance().pushCarLine(car_lines, root);
        }
      }
      // Json::Value roi       = MetaServerConfig::instance().cams()->roi();
      // Json::Value masterroi =
      // MetaServerConfig::instance().cams()->master_roi();

      // if (masterroi != Json::nullValue) {
      //     response["data"]["roi"] = masterroi;
      // } else if (roi != Json::nullValue) {
      //     response["data"]["roi"] = roi;
      // }

      mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                    response.toStyledString().c_str(), (int)hm->uri.len,
                    hm->uri.ptr);
    }
  }
  (void)fn_data;
}
