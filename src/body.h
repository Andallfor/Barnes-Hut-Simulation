#ifndef BODY_H
#define BODY_H

#include <cmath>
#include <functional>
#include "point.h"

struct strippedBody {
    double mass;
    point pos;
    acceleration accel = {{0, 0}, {0, 0}};
    point velocity = {0, 0};
    int index;
};

struct body {
    // TODO: consider using delta to inform whether or not updating the com matters
    static constexpr double DELTA = 0.5;
    static constexpr double G = 4.3009172706e-03; // parsec / solar mass * (km/s) ^ 2
    static constexpr double C = 299792.0; // km/s

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
    struct body* parent = nullptr;

    int index = -1;

    // shush
    bool isLeaf() {
        return !((children[0] && children[0]->mass > 1e-6) ||
                 (children[1] && children[1]->mass > 1e-6) ||
                 (children[2] && children[2]->mass > 1e-6) ||
                 (children[3] && children[3]->mass > 1e-6));
    }
    //bool isLeaf() {return !children[0] && !children[1] && !children[2] && !children[3]; }

    acceleration accel = {{0, 0}, {0, 0}};
    point velocity = {0, 0};

    body* getChild(int ind);

    double distTo(body* b) {
        return std::sqrt((b->pos.x - pos.x) * (b->pos.x - pos.x) +
                         (b->pos.y - pos.y) * (b->pos.y - pos.y));
    }

    void applyForceFrom(body* b, double d);
    void applyForceFrom(body* b) { applyForceFrom(b, distTo(b)); }

    strippedBody strip() { return {mass, pos, accel, velocity, index}; }
    void update(strippedBody sb) {
        mass = sb.mass;
        pos = sb.pos;
        accel = sb.accel;
        velocity = sb.velocity;
        index = sb.index;
    }

    void incrementCoM(point p, double m);
    void decrementCoM(point p, double m);
    void moveCoM(point delta, double m);

    void notifyChildRemoval(point p, double m, const std::function<bool(body*)>& condition);
    //void notifyChildMovement(body* node, point p, double m, std::function<bool(body*)>& condition);
};

#endif
