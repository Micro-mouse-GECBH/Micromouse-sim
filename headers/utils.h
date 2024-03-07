#pragma once
#include "vector.h"

struct CollissionInfo {
  Vec2 point;
  bool onFirstLine;
  bool onSecondLine;
  bool infrontFirst;
};

// https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection
static void __collissionCheckInner(double x1, double y1, double x2, double y2,
                                   double x3, double y3, double x4, double y4,
                                   double *t, double *u) {

  *t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) /
       ((x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4));
  *u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) /
       ((x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4));
}

static Vec2 __collissionPointInner(double x1, double y1, double x2, double y2,
                                   double t) {
  return Vec2{x1 + t * (x2 - x1), y1 + t * (y2 - y1)};
}

static CollissionInfo checkCollission(Line *l1, Line *l2) {
  double t, u;
  __collissionCheckInner(l1->p1.x, l1->p1.y, l1->p2.x, l1->p2.y, l2->p1.x,
                         l2->p1.y, l2->p2.x, l2->p2.y, &t, &u);

  Vec2 point =
      __collissionPointInner(l1->p1.x, l1->p1.y, l1->p2.x, l1->p2.y, t);

  return CollissionInfo{point, t >= 0 && t <= 1, u >= 0 && u <= 1, t >= 0};
}
