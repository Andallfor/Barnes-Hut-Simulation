#ifndef BODY_H
#define BODY_H

#include <cmath>
#include "point.h"

struct strippedBody {
    point pos;
    double mass;
    acceleration accel = {{0, 0}, {0, 0}};
    point velocity = {0, 0};
};

struct body {
    static constexpr double DELTA = 0;
    static constexpr double G = 4.3009172706e-03;

    point pos; // if external node, true position of body. otherwise com
    quad bounds;
    double mass; // mass of 0 means that it is empty

    /* children order
    *  ll ---+
    *    0 1 |
    *    2 3 |
    *        ur
    */
    struct body* children[4] = {nullptr};

    acceleration accel = {{0, 0}, {0, 0}};
    point velocity = {0, 0};

    body* getChild(int ind);

    bool isLeaf() { return !children[0] && !children[1] && !children[2] && !children[3]; }

    double distTo(body* b) {
        return std::sqrt((b->pos.x - pos.x) * (b->pos.x - pos.x) +
                         (b->pos.y - pos.y) * (b->pos.y - pos.y));
    }

    void applyForceFrom(body* b, double d);
    void applyForceFrom(body* b) { applyForceFrom(b, distTo(b)); }

    strippedBody strip() { return {pos, mass, accel, velocity}; }
    void update(strippedBody sb) {
        pos = sb.pos;
        mass = sb.mass;
        accel = sb.accel;
        velocity = sb.velocity;
    }
};

#endif
