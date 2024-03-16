#pragma once
#include <cmath>
#include <math.h>

struct Vec2 {
  double x;
  double y;

  Vec2(double x, double y) : x(x), y(y) {}

  Vec2(float dir) {
    this->x = cos(dir);
    this->y = sin(dir);
  }

  Vec2* normalize() {
    double mag = this->magnitude();
    this->x /= mag;
    this->y /= mag;
    return this;
  }

  Vec2* set_magnitude(double mag) {
    this->normalize();
    this->x *= mag;
    this->y *= mag;
    return this;
  }

  Vec2 with_magnitude(double mag) {
    Vec2 u = this->normalized();
    u.x *= mag;
    u.y *= mag;
    return u;
  }

  Vec2* add(Vec2* other) {
    this->x += other->x;
    this->y += other->y;
    return this;
  }

  Vec2* sub(Vec2* other) {
    this->x -= other->x;
    this->y -= other->y;
    return this;
  }

  Vec2* set_heading(double dir) {
    double mag = this->magnitude();
    this->x = mag * cos(dir);
    this->y = mag * sin(dir);
    return this;
  }

  Vec2 with_heading(double dir) {
    double mag = this->magnitude();
    return Vec2(mag * cos(dir), mag * sin(dir));
  }

  Vec2* rotate(float dir) {

    double cosA = cos(dir);
    double sinA = sin(dir);

    //  _               _  _  _
    //  | cos A   -sin A | | x |
    //  | sin A    cos A | | y |
    //  -               -  -  -

    this->x = cosA * this->x - sinA * this->y;
    this->y = sinA * this->x + cosA * this->y;
    return this;
  }

  Vec2 added(Vec2 *other) { return Vec2(this->x + other->x, this->y + other->y); }

  Vec2 subbed(Vec2 *other) { return Vec2(this->x - other->x, this->y - other->y); }

  Vec2 rotated(float dir) {
    double cosA = cos(dir);
    double sinA = sin(dir);

    //  _               _  _  _
    //  | cos A   -sin A | | x |
    //  | sin A    cos A | | y |
    //  -               -  -  -

    double x = cosA * this->x - sinA * this->y;
    double y = sinA * this->x + cosA * this->y;
    return Vec2(x, y);
  }

  double distance(Vec2* other) {
    return sqrt(pow(this->x - other->x, 2.0) + pow(this->y - other->y, 2.0));
  }

  double magnitude() {
    return sqrt(pow(this->x, 2.0) + pow(this->y, 2.0));
  }

  Vec2 normalized() {
    double mag = this->magnitude();
    return Vec2(this->x/mag, this->y/mag);
  }

  double direction() {
    return std::atan2(this->y, this->x);
  }
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
