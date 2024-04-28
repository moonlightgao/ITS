#include "ItsApp.h"
#include "CarDectorService.h"
#include "DrawService.h"

#include "tool.h"
#include <csignal>
#include <semaphore.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;
using namespace common;

sem_t sem_signal;

void signal_handler(int sig) {
  cout << "signal_handler " << sig << endl;
  sem_post(&sem_signal);
}

ItsApp::ItsApp() {
  is_running = false;
  video_path = "/home/alice/data/video_240309/output5.mp4";
  cap.open(video_path);
}

ItsApp::~ItsApp() {}

void ItsApp::start() {
  is_running = true;
  worker = thread(&ItsApp::run, this);

  signal(SIGINT, signal_handler);
  signal(SIGQUIT, signal_handler);
  signal(SIGTERM, signal_handler);
  signal(SIGHUP, signal_handler);

  sem_wait(&sem_signal);
}

void ItsApp::stop() {
  is_running = false;
  if (worker.joinable()) {
    worker.join();
  }
}

void ItsApp::run() {
  static uint64_t frame_id = 0;
  while (is_running) {
    ImagePtr img = make_shared<Image>();
    bool ret = cap.read(img->img);
    if (!ret) {
      std::cerr << "No more frames to read" << std::endl;
      cap.open(video_path);
      continue;
    }
    img->timestamp = getLoaclTime();
    img->frame_id = frame_id++;
    CarDectorService::instance().pushImage(img);
    usleep(10 * 1000);
  }
}
