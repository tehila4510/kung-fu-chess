#ifndef MOTION_VIEW_H
#define MOTION_VIEW_H

#include "model/Position.h"

#include <string>

struct MotionView {
    std::string piece;
    Position from;
    Position to;
    double progress = 0.0;
};

#endif
