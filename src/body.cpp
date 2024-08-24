#include "body.h"
#include "point.h"
#include <iostream>
#include <cmath>

body* body::getChild(int ind) {
    if (ind < 0 || ind >= 4) {
        std::cerr << "-_-" << std::endl;
        return nullptr;
    }

    // child are lazily created (only when accessed), so may need to init here
    if (children[ind]) return children[ind];

    point step = {(bounds.ur.x - bounds.ll.x) / 2.0, (bounds.ur.y - bounds.ll.y) / 2.0};
    point anchor = {(double) (ind % 2), (double) (ind / 2)};
    children[ind] = new body {
        {-1, -1},
        {{step.x * anchor.x + bounds.ll.x, step.y * anchor.y + bounds.ll.y},
        {step.x + step.x * anchor.x + bounds.ll.x, step.y + step.y * anchor.y + bounds.ll.y}},
        0, {nullptr}
    };

    children[ind]->parent = this;

    return children[ind];
}

void body::applyForceFrom(body* b, double r) {
    double force = G * (b->mass * mass) / (r * r);
    double theta = std::atan2(b->pos.y - pos.y, b->pos.x - pos.x);

    double fx = force * std::cos(theta);
    double fy = force * std::sin(theta);

    accel.future.x += fx / mass;
    accel.future.y += fy / mass;
}
