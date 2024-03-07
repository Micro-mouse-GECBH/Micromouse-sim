#pragma once
#include <math.h>

struct Vec2 {
  double x;
  double y;
};

struct Line {
  Vec2 p1;
  Vec2 p2;
};

static double mag(Vec2 *v) { return sqrt(v->x * v->x + v->y * v->y); }

static double dir(Vec2 *v) { return atan2(v->y, v->x); }

static Vec2 add(Vec2 *v1, Vec2 *v2) {
  return Vec2{v2->x + v1->x, v2->y + v1->y};
}

static Vec2 normalized(Vec2 *v) {
  double d = dir(v);
  return Vec2{cos(d), sin(d)};
}

static Vec2 vecWithHeading(Vec2 *v, double heading) {
  double m = mag(v);
  return Vec2{m * cos(heading), m * sin(heading)};
}

static Vec2 vecRotated(Vec2 *v, double angle) {
  double cosA = cos(angle);
  double sinA = sin(angle);

  //  _               _  _  _
  //  | cos A   -sin A | | x |
  //  | sin A    cos A | | y |
  //  -               -  -  -

  double x = cosA * v->x - sinA * v->y;
  double y = sinA * v->x + cosA * v->y;

  return Vec2{x, y};
};

static double distance(Vec2 *v1, Vec2 *v2) {
  return sqrt(pow(v2->x - v1->x, 2.0) + pow(v2->y - v1->y, 2.0));
}
