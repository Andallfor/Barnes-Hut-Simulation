#include "body.h"
#include "point.h"
#include <iostream>
#include <cmath>

#define USE_ATAN2_APPROX

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
#ifndef USE_ATAN2_APPROX
    double theta = std::atan2(b->pos.y - pos.y, b->pos.x - pos.x);
#else
    // atan2 approx
    double y = b->pos.y - pos.y;
    double x = b->pos.x - pos.x;
    // https://gist.github.com/volkansalma/2972237
    double abs_y = std::fabs(y) + 1e-10;      // kludge to prevent 0/0 condition
    double _r = (x - std::copysign(abs_y, x)) / (abs_y + std::fabs(x));
    double angle = 3.14159 / 2. - std::copysign(3.14159 / 4., x);

    angle += (0.1963 * _r * _r - 0.9817) * _r;
    double theta = std::copysign(angle, y);
#endif

    double fx = force * std::cos(theta);
    double fy = force * std::sin(theta);

    accel.future.x += fx / mass;
    accel.future.y += fy / mass;
}

    // call from parent, give child position
void body::incrementCoM(point p, double m) {
    // https://www.desmos.com/calculator/4aoyrlkt7x
    pos.x += m * (p.x * mass - pos.x * mass) / (mass * (m + mass));
    pos.y += m * (p.y * mass - pos.y * mass) / (mass * (m + mass));

    mass += m;
}

// call from parent, give child position
void body::decrementCoM(point p, double m) {
    // https://www.desmos.com/calculator/kpurqi6hmd
    point u = {
        pos.x * mass - (m * p.x),
        pos.y * mass - (m * p.y)
    };

    pos.x += m * (p.x * (mass - m) - u.x) / ((mass - m) * mass);
    pos.y += m * (p.y * (mass - m) - u.y) / ((mass - m) * mass);
    mass -= m;

    if (mass < 1e-6) mass = 0;
}

// call from parent, give child position
void body::moveCoM(point delta, double m) {
    pos.x += delta.x * m / mass;
    pos.y += delta.y * m / mass;
}

void body::notifyChildRemoval(point p, double m, const std::function<bool(body*)>& condition) {
    if (condition(this)) return;

    decrementCoM(p, m);
    parent->notifyChildRemoval(p, m, condition);
}

void body::notifyChildMovement(point delta, double m, const std::function<bool(body*)>& condition) {
    if (condition(this)) return;

    moveCoM(delta, m);
    parent->notifyChildMovement(delta, m, condition);
}
