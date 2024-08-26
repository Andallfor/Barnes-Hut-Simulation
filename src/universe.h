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

class Universe {
private:
    int width, height;
    double lengthPerPixel;
    snapshotConfig prevConfig;

    GLubyte black[3] = { 0, 0, 0 };
    GLubyte white[3] = { 255, 255, 255 };
    GLubyte red[3] = {255, 0, 0};
    GLubyte green[3] = {0, 255, 0};
    GLubyte orange[3] = {255, 165, 0};

    GLubyte stellar[7][3] = {
        {255, 181, 108},
        {255, 218, 181},
        {255, 237, 227},
        {249, 245, 255},
        {213, 224, 255},
        {162, 192, 255},
        {146, 181, 255}
    };

    body* root = nullptr;
    std::vector<body*> registeredBodies;
    int bodyIndex = 0;

    void destroyStars(body* root);
    void _destroyChild(body* parent);

    void _registerStar(std::queue<recursionState>* states);

    void _traverse(body* node, const std::function<bool(body*, int)>& foreach, int depth = 0);

    // TODO move into seperate file
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
    void drawBlackHole(point p, GLubyte* color) {
        pointi c = toRenderGridCoords(p);
        if (c.x < 0 || c.x >= width || c.y < 0 || c.y >= height) return;

        // ez
        drawPixel(3 * ((c.y + 2) * width + c.x), color);
        drawPixel(3 * ((c.y - 2) * width + c.x), color);
        drawPixel(3 * (c.y * width + c.x + 2), color);
        drawPixel(3 * (c.y * width + c.x - 2), color);

        drawPixel(3 * ((c.y + 1) * width + c.x - 1), color);
        drawPixel(3 * ((c.y + 1) * width + c.x + 1), color);
        drawPixel(3 * ((c.y - 1) * width + c.x - 1), color);
        drawPixel(3 * ((c.y - 1) * width + c.x + 1), color);
    }

    void drawCross(point p, GLubyte* color) {
        pointi c = toRenderGridCoords(p);
        if (c.x < 0 || c.x >= width || c.y < 0 || c.y >= height) return;

        drawPixel(3 * (c.y * width + c.x), color);
        drawPixel(3 * ((c.y + 1) * width + c.x), color);
        drawPixel(3 * ((c.y - 1) * width + c.x), color);
        drawPixel(3 * (c.y * width + c.x + 1), color);
        drawPixel(3 * (c.y * width + c.x - 1), color);
    }

    void drawBody(body* b) {
        // these dont match reality at all but /shrug
        if (b->mass >= 10e3) drawBlackHole(b->pos, red);
        else if (b->mass > 149.8) drawCross(b->pos, stellar[6]);
        else if (b->mass > 149.5) drawCross(b->pos, stellar[5]);
        else if (b->mass > 140) drawPixel(b->pos, stellar[4]);
        else if (b->mass > 100) drawPixel(b->pos, stellar[3]);
        else if (b->mass > 70) drawPixel(b->pos, stellar[2]);
        else if (b->mass > 20) drawPixel(b->pos, stellar[1]);
        else drawPixel(b->pos, stellar[0]);
    }

    void hideBody(body* b) {
        if (b->mass >= 10e3) drawBlackHole(b->pos, black);
        else if (b->mass >= 145) drawCross(b->pos, black);
        else drawPixel(b->pos, black);
    }

    void registerToBodyIndex(body* b, bool verify = true) {
        if (verify) {
            size_t s = registeredBodies.capacity();
            // resize because we use operator[] to access - reserve does not immediantely increase size of array
            if (b->index >= s) registeredBodies.resize((size_t) b->index * 2);
        }

        registeredBodies[b->index] = b;
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

        registeredBodies.resize(100);

        resizeWindow(width, height);
    }

    void resizeWindow(int width, int height, bool redraw = true);
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
        registerStar({mass, pos, {{0, 0}, {0, 0}}, vel, bodyIndex++});
    }

    void traverse(const std::function<bool(body*, int)>& foreach) { _traverse(root, foreach, 0); }
    void step();

    void registerGalaxy(point center, int amt, double coreMass, point coreVel, point radius);

    Universe(Universe const&) = delete;
    Universe& operator=(const Universe&) = delete;

    ~Universe() {
        destroyStars(root);
        delete[] renderWindow;
    }
};

#endif
