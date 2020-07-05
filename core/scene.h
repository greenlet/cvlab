#pragma once

#include "common.h"
#include "scene.h"
#include "view.h"

class Scene {
 public:
  Scene();

 private:
  std::vector<View> views_;
};
