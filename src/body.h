#ifndef BODY_H
#define BODY_H

#include <cmath>
#include "point.h"

struct strippedBody {
    double mass;
    point pos;
    acceleration accel = {{0, 0}, {0, 0}};
    point velocity = {0, 0};
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

    strippedBody strip() { return {mass, pos, accel, velocity}; }
    void update(strippedBody sb) {
        mass = sb.mass;
        pos = sb.pos;
        accel = sb.accel;
        velocity = sb.velocity;
    }

    void incrementCoM(point p, double m) {
        // https://www.desmos.com/calculator/4aoyrlkt7x
        pos.x += m * (p.x * mass - pos.x * mass) / (mass * (m + mass));
        pos.y += m * (p.y * mass - pos.y * mass) / (mass * (m + mass));

        mass += m;
    }
};

#endif
