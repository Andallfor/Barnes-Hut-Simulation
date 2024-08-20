#include <iostream>
#include <stdlib.h>
#include <vector>
#include <functional>
#include "Universe.h"

void Universe::destroyStars(body* node) {
    if (!node) {
        std::cout << "WARN: Attempting to destroy stars but none exist." << std::endl;
        return;
    }

    _destroyChild(node);
}

void Universe::_destroyChild(body* parent) {
    for (int i = 0; i < 4; i++) {
        if (parent->children[i]) _destroyChild(parent->children[i]);
    }

    delete parent;
}

void Universe::_registerStar(body* node, strippedBody star, bool affectCoM) {
    for (int i = 0; i < 4; i++) {
        body* child = node->getChild(i);
        if (child->bounds.contains(star.pos)) {
            if (affectCoM) {
                // edge case for root node, which starts with no mass
                if (node->mass == 0) node->update(star);
                else {
                    double m = node->mass;
                    double mn = m + star.mass;
                    double cx = node->pos.x * m;
                    double cy = node->pos.y * m;

                    // https://www.desmos.com/calculator/4aoyrlkt7x (im bad at math)
                    node->pos.x += (star.mass * (star.pos.x * m - cx)) / (m * mn);
                    node->pos.y += (star.mass * (star.pos.y * m - cy)) / (m * mn);
                    node->mass = mn;
                }
            }

            // child is empty, replace it with the star
            if (child->mass == 0) child->update(star);    
            else { // recurse!
                if (child->isLeaf()) { // something is here and is leaf node -> therefore must be a singular body
                    // new star should either be the same or a level below the child node
                    // which means the child node then needs to become an internal node
                    _registerStar(child, child->strip(), false);
                }
                _registerStar(child, star);
            }

            return;
        }
    }

    std::cerr << "Error: Star traversed to end of tree but could not find a place to insert self. The star may not be within the root node bounds." << std::endl;
}

void Universe::step() {
    // TODO: optimize!
    // TODO: reset all colored pixels from snapshot()
    resizeWindow(width, height);

    _traverse(root, [this] (body* b, int) -> bool {
        // this loop looks for all bodies
        if (b->mass == 0 || !b->isLeaf()) return true;

        // calc forces
        _traverse(root, [this, b] (body* actor, int) -> bool {
            // cannot be self and must have mass
            if (actor->mass == 0 || (actor->pos.x == b->pos.x && actor->pos.y == b->pos.y)) return true;

            // if leaf node, manually calc force
            if (actor->isLeaf()) {
                b->applyForceFrom(actor);
                return false;
            } else {
                double s = actor->bounds.ur.x - actor->bounds.ll.x;
                double d = std::sqrt(
                    (b->pos.x - actor->pos.x) * (b->pos.x - actor->pos.x) +
                    (b->pos.y - actor->pos.y) * (b->pos.y - actor->pos.y));

                double delta = s / d;

                // node is sufficiently far away, treat as singular
                if (delta < body::DELTA) {
                    b->applyForceFrom(actor, d);
                    return false;
                }
            }

            return true;
        });

        return true;
    }, 0);

    // apply the acceleration (and velocity)
    std::vector<strippedBody> copy;
    _traverse(root, [&copy] (body* b, int) -> bool {
        if (b->mass == 0 || !b->isLeaf()) return true;

        // leapfrog finite diff aprox for t + 1
        double dt = 0.01;
        b->pos.x += b->velocity.x * dt + 0.5 * b->accel.past.x * dt * dt;
        b->pos.y += b->velocity.y * dt + 0.5 * b->accel.past.y * dt * dt;

        b->velocity.x += 0.5 * (b->accel.future.x + b->accel.past.x) * dt;
        b->velocity.y += 0.5 * (b->accel.future.y + b->accel.past.y) * dt;
        
        copy.push_back(b->strip());

        return true;
    });

    // restructure tree (i will optimize this later i swear)
    destroyStars(root);
    root = new body {
        {-1, -1},
        {{0, 0}, {(double) width, (double) height}},
        0, {nullptr}
    };

    for (std::vector<strippedBody>::iterator it = copy.begin(); it != copy.end(); it++) {
        _registerStar(root, *it);
    }
}

void Universe::resizeWindow(int w, int h) {
    if (renderWindow) delete[] renderWindow;

    this->width = w;
    this->height = h;

    renderWindow = new GLubyte[(int) (width * height * 3)] {0};
}

GLubyte* & Universe::snapshot(snapshotConfig config) {
    // config changes, redraw everything
    // unoptimized but should be fine since this is just for debugging and should not be changing without
    // user input
    if (config != prevConfig) resizeWindow(width, height);
    prevConfig = config;

    _traverse(root, [this, config] (body* b, int depth) -> bool {
        if (config.debug) {
            // quad bound drawing
            if (config.showQuad && (config.depth == -1 || config.depth == depth)) {
                GLubyte* outline = (b->mass == 0) ? red : green;

                pointi ll = toRenderGridCoords(b->bounds.ll);
                pointi ur = toRenderGridCoords(b->bounds.ur);

                for (int x = ll.x; x < ur.x; x++) {
                    drawPixel(3 * (ll.y * width + x), outline);
                    drawPixel(3 * (ur.y * width + x), outline);
                }

                for (int y = ll.y; y < ur.y; y++) {
                    drawPixel(3 * (y * width + ll.x), outline);
                    drawPixel(3 * (y * width + ur.x), outline);
                }
            }

            // draw point
            GLubyte* color = (b->isLeaf()) ? green : red;
            if (b->mass == 5000) color = white;
            if (!config.drawSameDepthOnly || (config.depth == -1 || config.depth == depth)) drawSquare(b->pos, 10, color);
        } else {
            if (!b->isLeaf()) return true;
            drawPixel(b->pos, white);
        }

        return true;
    });

    return renderWindow;
}

void Universe::_traverse(body* node, const std::function<bool(body*, int)>& foreach, int _depth) {
    if (!node) return;

    // returning false means to end execution of current branch
    if (!foreach(node, _depth)) return;

    // directly access children as we dont want to evaluate any children (no operation is being performed here)
    _traverse(node->children[0], foreach, _depth + 1);
    _traverse(node->children[1], foreach, _depth + 1);
    _traverse(node->children[2], foreach, _depth + 1);
    _traverse(node->children[3], foreach, _depth + 1);
}

bool Universe::drawPixel(point p, GLubyte* c) {
    int ind = toRenderGrid(p);
    if (ind >= width * height * 3 || ind < 0) return false;

    renderWindow[ind] = c[0];
    renderWindow[ind + 1] = c[1];
    renderWindow[ind + 2] = c[2];

    return true;
}

bool Universe::drawPixel(int ind, GLubyte* c) {
    if (ind >= (width * height * 3) || ind < 0) return false;

    renderWindow[ind] = c[0];
    renderWindow[ind + 1] = c[1];
    renderWindow[ind + 2] = c[2];

    return true;
}
