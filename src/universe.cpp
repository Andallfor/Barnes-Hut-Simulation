#include <iostream>
#include <stdlib.h>
#include <vector>
#include <functional>
#include <random>
#include <queue>
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

void Universe::_registerStar(std::queue<recursionState>* states) {
    // https://stackoverflow.com/questions/8970500/visit-a-tree-or-graph-structure-using-tail-recursion
    // in retrospect uneeded as the solution to stack size was preventing bodies from being too close to each other
    // but something something sunk cost fallacy
    if (states->empty()) return;

    recursionState state = states->front();
    states->pop(); // why doesnt it return the top :(

    for (int i = 0; i < 4; i++) {
        body* child = state.node->getChild(i);
        if (child->bounds.contains(state.star.pos)) {
            if (state.affectCoM) {
                // edge case for root node, which starts with no mass
                if (state.node->mass == 0) state.node->update(state.star);
                else state.node->incrementCoM(state.star.pos, state.star.mass);
            }

            // child is empty, replace it with the star
            if (child->mass == 0) {
                child->update(state.star);
                registerToBodyIndex(child);
            } else {
                if (child->isLeaf()) {
                    // something is here and is leaf node -> therefore must be a singular body
                    // which means the child node then needs to become an internal node
                    // and have the new star and itself as children (not necessarily direct children)
                    states->push({child, child->strip(), false});
                    // remove this node from registeredBodies (to be readded in if statement above)
                    child->index = -1;
                }
                // dont need to reinit everything
                state.node = child;
                states->push(state);
            }

            break;
        }
    }

    _registerStar(states);
}

void Universe::step() {
    if (registeredBodies.size() == 0) return;

    // TODO: reset all colored pixels from snapshot()
    resizeWindow(width, height);

    for (int ind = 0; ind < bodyIndex; ind++) {
        body* b = registeredBodies[ind];
        if (!b) continue;

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
    }

    // apply the acceleration (and velocity)
    // this can be collapsed into the above traversal but is kept out for readability and cleanliness
    // (this does not need to be super optimized)
    struct changePropogation {
        body* parent;
        double mass;
        point pos;
    };

    std::vector<strippedBody> nodeChange;
    std::vector<changePropogation> change;
    for (int ind = 0; ind < bodyIndex; ind++) {
        body* b = registeredBodies[ind];
        if (!b) continue;

        // leapfrog finite diff approx for t + 1
        double dt = 0.01; // if inner ring starts pulsating in and out, decrease t
        b->pos.x += b->velocity.x * dt + 0.5 * b->accel.past.x * dt * dt;
        b->pos.y += b->velocity.y * dt + 0.5 * b->accel.past.y * dt * dt;

        b->velocity.x += 0.5 * (b->accel.future.x + b->accel.past.x) * dt;
        b->velocity.y += 0.5 * (b->accel.future.y + b->accel.past.y) * dt;

        b->accel.past = b->accel.future;
        b->accel.future = {0, 0};

        // out of bounds, remove
        if (!(root->bounds.contains(b->pos))) {
            registeredBodies[b->index] = nullptr;
            for (int i = 0; i < 4; i++) {
                if (b->parent->children[i] == b) {
                    b->parent->children[i] = nullptr;
                    break;
                }
            }

            // remove influence of this node on parent
            b->parent->notifyChildRemoval(b->pos, b->mass, [] (body* parent) -> bool {return parent == nullptr; });

            delete b;
            continue;
        }

        // TODO: need to replace current CoM calculation system as it doesnt update com when the body moves within a quad

        // if star moves out of current quad bounds
        if (!b->bounds.contains(b->pos)) {
            // all bodies are leaf nodes which do not have children
            // therefore these bodies can easily be removed and reintroduced
            // it would be fastest to search for position to insert into upwards but that functionality
            // is not currently supported
            strippedBody sb = b->strip();

            // remove self from parents CoM as well
            b->parent->notifyChildRemoval(b->pos, b->mass, [] (body* parent) -> bool {return parent == nullptr;});

            b->mass = 0; // mass 0 denotes that this is not a star, irrespective of pos/vel/accel
            registeredBodies[b->index] = nullptr;
            b->index = -1;
            nodeChange.push_back(sb);
        }
    }

    // propogate change
    for (std::vector<changePropogation>::const_iterator it = change.cbegin(); it != change.cend(); it++) {
        it->parent->mass -= it->mass;
        it->parent->pos.x += it->pos.x;
        it->parent->pos.y += it->pos.y;
        if (it->parent->mass <= 1e-6) it->parent->mass = 0;
    }

    for (std::vector<strippedBody>::const_iterator it = nodeChange.cbegin(); it != nodeChange.cend(); it++) {
        registerStar(*it);
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
    // unoptimized but should be fine since this is just for debugging and should not be changing without user input
    if (config != prevConfig) resizeWindow(width, height);
    prevConfig = config;

    _traverse(root, [this, config] (body* b, int depth) -> bool {
        pointi _p = toRenderGridCoords(b->pos);
        //if (_p.x < 0 || _p.y < 0 || _p.x > width || _p.y > height) return true;

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

            if (b->mass == 0) return true;

            // draw point
            GLubyte* color = (b->isLeaf()) ? green : red;
            if (!config.drawSameDepthOnly || (config.depth == -1 || config.depth == depth)) drawSquare(b->pos, 10, color);
        } else {
            if (b->mass == 0 || !b->isLeaf()) return true;
            drawPixel(b->pos, white);
        }

        return true;
    });

    return renderWindow;
}

void Universe::registerGalaxy(point center, int amt, double coreMass, point coreVel, point radius) {
    // ngl im basically guessing on all of these values idk anything about galactic properties

    point massRange = {2.0, 150};
    registerStar(center, coreMass, coreVel);

    // each star should take up at most 0.5 units^2
    double area = (3.14159 * radius.y * radius.y) - (3.14159 * radius.x * radius.x);
    double maxCapacity = area / 0.5;
    if (amt > maxCapacity) {
        amt = (int) maxCapacity;
        std::cout << "WARNING: Trying to add more points than radius area can sustain (" << amt << ")\n";
    }

    double areaVariance = 0.2 * (area / (double) amt);

    // https://stackoverflow.com/questions/28567166/uniformly-distribute-x-points-inside-a-circle
    double b = std::round(2.0 * std::sqrt(amt));
    double phi = 0.5 * (std::sqrt(5) + 1.0);
    for (double k = 4; k < amt + 1; k++) {
        double r = (k > amt - b) ? 1.0 : (std::sqrt(k - 0.5) / (std::sqrt(amt - (b + 1.0) / 2.0)));
        r = r * radius.y + radius.x;
        // stars near the core should be closer together and further away more spread out
        r = std::pow(r, 1.0 + (rand() % 1000) / 20000.0);
        double theta = 2.0 * 3.14159 * k / (phi * phi);

        point pos = {
            center.x + std::cos(theta) * r + (rand() % 1000) / 1000.0 * areaVariance,
            center.y + std::sin(theta) * r + (rand() % 1000) / 1000.0 * areaVariance
        };

        double v = std::sqrt(body::G * coreMass / r);
        point vel = {
            v * std::sin(theta) + coreVel.x,
            -v * std::cos(theta) + coreVel.y
        };

        double mass = (rand() % 1000) / 1000.0 * (massRange.y - massRange.x) + massRange.x;
        registerStar(pos, mass, vel);
    }
}

void Universe::_traverse(body* node, const std::function<bool(body*, int)>& foreach, int _depth) {
    if (!node || node->mass == 0) return;

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
    if (ind < 0 || ind >= width * height * 3) return false;

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
