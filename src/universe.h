#ifndef UNIVERSE_H
#define UNIVERSE_H

#include <vector>
#include <functional>
#include <cmath>
#include <ctime>
#include <queue>

#include "renderer.h"
#include "body.h"
#include "point.h"

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

struct recursionState {
    body* node;
    strippedBody star;
    bool affectCoM;
};

// TODO: implement array that holds references to all existing stars, so dont need to traverse entire tree every time
class Universe {
private:
    int width, height;
    double lengthPerPixel;
    snapshotConfig prevConfig;

    GLubyte black[3] = { 0, 0, 0 };
    GLubyte white[3] = { 255, 255, 255 };
    GLubyte red[3] = {255, 0, 0};
    GLubyte green[3] = {0, 255, 0};

    body* root = nullptr;

    void destroyStars(body* root);
    void _destroyChild(body* parent);

    //void _registerStar(body* node, strippedBody star, bool affectCoM = true, int depth = 0);
    void _registerStar(std::queue<recursionState>* states);

    void _traverse(body* node, const std::function<bool(body*, int)>& foreach, int depth = 0);

    // draw pixel with color, assuming color is a pointer to GLubyte array of at least size 3
    bool drawPixel(point p, GLubyte* color);
    bool drawPixel(int ind, GLubyte* color);

    void drawSquare(point p, int r, GLubyte* color) {
        int x = (int) (p.x / lengthPerPixel);
        int y = (int) (p.y / lengthPerPixel);
        int hr = r / 2;

        for (int _x = -hr; _x < hr; _x++) {
            for (int _y = -hr; _y < hr; _y++) {
                drawPixel(3 * ((y + _y) * width + x + _x), color);
            }
        }
    }
public:
    GLubyte* renderWindow = nullptr;
    Universe(int width, int height, double trueWidth) : width(width), height(height) {
        lengthPerPixel = trueWidth / width;

        srand(rand() ^ (uint16_t) time(NULL));

        root = new body{
            {0, 0},
            // center of viewport is (tw / 2, tw / 2)
            {{-trueWidth / 2.0, -trueWidth / 2.0}, {1.5 * trueWidth, 1.5 * trueWidth}},
            0, {nullptr}
        };

        resizeWindow(width, height);
    }

    void resizeWindow(int width, int height);
    GLubyte*& snapshot(snapshotConfig config = {});
    int toRenderGrid(point p) {
        pointi i = toRenderGridCoords(p);
        if (i.x < 0 || i.x >= width || i.y < 0 || i.y >= height) return -1;
        return 3 * (i.x + i.y * width);
    }
    pointi toRenderGridCoords(point p) { return {(int) (p.x / lengthPerPixel), (int) (p.y / lengthPerPixel)}; }

    void registerStar(strippedBody sb) {
        std::queue<recursionState>* states = new std::queue<recursionState>();
        recursionState state = {root, sb, true};
        states->push(state);

        _registerStar(states);

        delete states;
    }

    void registerStar(point pos, double mass, point vel = {0, 0}) {
        registerStar({mass, pos, {{0, 0}, {0, 0}}, vel});
    }

    void traverse(const std::function<bool(body*, int)>& foreach) { _traverse(root, foreach, 0); }
    void step();

    void registerGalaxy(point center, int amt, double coreMass, point vel = {0, 0}, point radius = {100, 20});

    Universe(Universe const&) = delete;
    Universe& operator=(const Universe&) = delete;

    ~Universe() {
        destroyStars(root);
        delete[] renderWindow;
    }
};

#endif
