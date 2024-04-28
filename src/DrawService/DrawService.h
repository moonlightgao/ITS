#ifndef DRAW_SERVICE
#define DRAW_SERVICE
#include "type.h"
#include <iostream>
#include <list>
#include <thread>

class DrawService {
public:
  DrawService();
  ~DrawService();

  static DrawService &instance() {
    static DrawService val;
    return val;
  }

  void pushInfo(common::DecResultPtr &ret_ptr);

private:
};

#endif