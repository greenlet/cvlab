#pragma once

#include "logger.h"


class CameraAndroid : Logger {
public:
    CameraAndroid();
    void start();

private:
    bool _started = false;
};

