#ifndef UNIVERSE_H
#define UNIVERSE_H

#include <vector>
#include <functional>
#include "renderer.h"

/*
* UNITS
* x, y -> parsec
* mass -> solar mass
* vel -> km/s
*/

struct snapshotConfig {
    bool debug = false;
    int depth = -1;
    bool showQuad = false;
    bool drawSameDepthOnly = false;

    bool operator==(snapshotConfig con) { return std::memcmp(this, &con, sizeof(snapshotConfig)) == 0; }
    bool operator!=(snapshotConfig con) { return !(*this == con); }
};

struct point { double x, y; };
struct pointui { uint16_t x, y; };

struct quad {
    point ll, ur;
    bool contains(point p) { return p.x >= ll.x && p.y >= ll.y && p.x <= ur.x && p.y <= ur.y; }
};

struct strippedBody {
    point pos;
    double mass;
    point acceleration = { 0, 0 };
    point velocity = { 0, 0 };
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
    struct body* children[4] = { nullptr };

    point acceleration = {0, 0};
    point velocity = {0, 0};

    body* getChild(int ind);

    bool isLeaf() { return !children[0] && !children[1] && !children[2] && !children[3]; }

    double distTo(body* b) { return std::sqrt((b->pos.x - pos.x) * (b->pos.x - pos.x) +
                                              (b->pos.y - pos.y) * (b->pos.y - pos.y)); }

    void applyForceFrom(body* b, double d);
    void applyForceFrom(body* b) { applyForceFrom(b, distTo(b)); }
};

class Universe {
private:
    int width, height;
    snapshotConfig prevConfig;

    GLubyte black[3] = { 0, 0, 0 };
    GLubyte white[3] = { 255, 255, 255 };
    GLubyte red[3] = {255, 0, 0};
    GLubyte green[3] = {0, 255, 0};

    body* root = nullptr;

    void destroyStars(body* root);
    void _destroyChild(body* parent);

    void _registerStar(body* node, strippedBody star, bool affectCoM = true);

    void _traverse(body* node, const std::function<bool(body*, int)>& foreach, int depth = 0);

    // draw pixel with color, assuming color is a pointer to GLubyte array of at least size 3
    bool drawPixel(point p, GLubyte* color);
    bool drawPixel(uint32_t ind, GLubyte* color);
public:
    GLubyte* renderWindow = nullptr;
    Universe(int width, int height) : width(width), height(height) {
        srand(rand() ^ (uint16_t) time(NULL));

        root = new body{
            {-1, -1},
            {{0, 0}, {(double) width, (double) height}},
            0, {nullptr}
        };

        resizeWindow(width, height);
    }

    // TODO: consider moving drawing functions into separate class
    // bit unclean to have both simulation and rendering code in the same class
    void resizeWindow(int width, int height);
    GLubyte*& snapshot(snapshotConfig config = {});
    // casting only at end of operation leads to incorrect result for some reason
    uint32_t toRenderGrid(point p) { return ((uint32_t) p.x + (uint32_t) p.y * width) * 3; }

    void registerStar(point pos, double mass) { _registerStar(root, {pos, mass}); }
    void traverse(const std::function<bool(body*, int)>& foreach) { _traverse(root, foreach, 0); }
    void step();

    Universe(Universe const&) = delete;
    Universe& operator=(const Universe&) = delete;

    ~Universe() {
        destroyStars(root);
        delete[] renderWindow;
    }
};

#endif
