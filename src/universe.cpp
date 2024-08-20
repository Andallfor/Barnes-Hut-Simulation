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
                if (node->mass == 0) {
                    node->mass = star.mass;
                    node->pos = star.pos;
                } else {
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
            if (child->mass == 0) {
                child->mass = star.mass;
                child->pos = star.pos;
            } else { // recurse!
                if (child->isLeaf()) { // something is here and is leaf node -> therefore must be a singular body
                    // new star should either be the same or a level below the child node
                    // which means the child node then needs to become an internal node
                    _registerStar(child, { child->pos, child->mass }, false);
                }
                _registerStar(child, star);
            }

            return;
        }
    }

    std::cerr << "Error: Star traversed to end of tree but could not find a place to insert itself. The star may not be within the root node bounds." << std::endl;
}

void Universe::step() {
    // reset all colored pixels from snapshot()
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

    _traverse(root, [this, config] (body* b, int depth) -> void {
        if (config.debug) {
            // quad bound drawing
            if (config.showQuad && (config.depth == -1 || config.depth == depth)) {
                GLubyte* outline = (b->mass == 0) ? red : green;

                for (double x = b->bounds.ll.x; x < b->bounds.ur.x; x++) {
                    drawPixel({x, b->bounds.ll.y}, outline);
                    drawPixel({x, b->bounds.ur.y - 1}, outline);
                }

                for (double y = b->bounds.ll.y; y < b->bounds.ur.y; y++) {
                    drawPixel({b->bounds.ll.x, y}, outline);
                    drawPixel({b->bounds.ur.x - 1, y}, outline);
                }
            }

            // draw point
            if (config.drawSameDepthOnly) {
                if (config.depth == -1 || config.depth == depth) {
                    GLubyte* color = (b->isLeaf()) ? green : red;
                    for (int i = -2; i < 3; i++) {
                        for (int j = -2; j < 3; j++) {
                            drawPixel({b->pos.x + i, b->pos.y + j}, color);
                        }
                    }
                }
            } else {
                GLubyte* color = (b->isLeaf()) ? green : red;
                for (int i = -2; i < 3; i++) {
                    for (int j = -2; j < 3; j++) {
                        drawPixel({b->pos.x + i, b->pos.y + j}, color);
                    }
                }
            }
        } else {
            if (!b->isLeaf()) return;
            drawPixel(b->pos, white);
        }
    });

    return renderWindow;
}

void Universe::_traverse(body* node, const std::function<void(body*, int)>& foreach, int _depth) {
    if (!node) return;

    // directly access children as we dont want to evaluate any children (no operation is being performed here)
    _traverse(node->children[0], foreach, _depth + 1);
    _traverse(node->children[1], foreach, _depth + 1);
    _traverse(node->children[2], foreach, _depth + 1);
    _traverse(node->children[3], foreach, _depth + 1);

    foreach(node, _depth);
}

bool Universe::drawPixel(point p, GLubyte* c) {
    uint32_t ind = toRenderGrid(p);
    if (ind >= (uint32_t) (width * height * 3)) return false;

    renderWindow[ind] = c[0];
    renderWindow[ind + 1] = c[1];
    renderWindow[ind + 2] = c[2];

    return true;
}

bool Universe::drawPixel(uint32_t ind, GLubyte* c) {
    if (ind >= (uint32_t) (width * height * 3)) return false;

    renderWindow[ind] = c[0];
    renderWindow[ind + 1] = c[1];
    renderWindow[ind + 2] = c[2];

    return true;
}
