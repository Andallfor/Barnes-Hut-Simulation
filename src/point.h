#ifndef POINT_H
#define POINT_H

struct point { double x, y; };
struct pointi { int x, y; };

struct acceleration {
    point past = {0, 0};
    point future = {0, 0};
};

struct quad {
    point ll, ur;
    bool contains(point p) { return p.x >= ll.x && p.y >= ll.y && p.x <= ur.x && p.y <= ur.y; }
};

#endif
