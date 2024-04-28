#include "ViolationDetectionService.h"
#include "ControlService.h"
#include "DrawService.h"
#include "TrackingService.h"

#include "http_service.h"

#include "http_client.h"
#include "type.h"

#include <stdio.h>
#include <string.h>
#include <sys/vfs.h>

#include <iostream>

#include <signal.h>
#include <unistd.h>

using namespace std;
using namespace common;

ViolationDetectionService::ViolationDetectionService() { 
    is_running = false; 
    ReadJsonFile();
}
ViolationDetectionService::~ViolationDetectionService() {}
void ViolationDetectionService::start() {
  is_running = true;
  worker = thread(&ViolationDetectionService::run, this);
}
void ViolationDetectionService::stop() {
  is_running = false;
  if (worker.joinable()) {
    worker.join();
  }
}
void ViolationDetectionService::run() {}
void ViolationDetectionService::pushCarLine(
    vector<vector<common::LinePoint>> car_lines_, Json::Value root) {
  CreateJsonFile(root);
  car_lines = car_lines_;
}
void ViolationDetectionService::CreateJsonFile(Json::Value root) {

  std::string strFilePath = "/home/alice/data/json/CarLine.json";
  Json::StyledStreamWriter streamWriter;
  ofstream outFile(strFilePath);
  streamWriter.write(outFile, root);
  outFile.close();

  std::cout << "json文件生成成功！" << endl;
}
void ViolationDetectionService::ReadJsonFile() {
  std::string strFilePath = "/home/alice/data/json/CarLine.json";
  Json::Reader json_reader;
  Json::Value root;

  ifstream infile(strFilePath.c_str(), ios::binary);
  if (!infile.is_open()) {
    cout << "Open config json file failed!" << endl;
    return;
  }

  if (json_reader.parse(infile, root)) {
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
  } else {
    cout << "Can not parse Json file!";
  }

  infile.close();
}
//接口：把车道线传给绘制，在绘制里调用这个结果
std::vector<std::vector<common::LinePoint>> ViolationDetectionService::GetCarLine()
{
     return car_lines;

}
