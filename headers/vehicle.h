#pragma once
#include "utils.h"
#include "vector.h"
#include <math.h>
#include <vector>
#include <stdio.h>

struct Sensor {
  Vec2 pos;
  double dir;
};

struct Vehicle {
  Vec2 pos;
  double dir;
  std::vector<Sensor> sensors;

  std::vector<float> distance_readings(std::vector<Line> *walls) {
    auto vals = sensor_values(walls);

    std::vector<float> dists;

    for (int i = 0; i < vals.size(); i++) {
      Vec2 sensor_pos = vecRotated(&sensors[i].pos, dir);
      sensor_pos = add(&sensor_pos, &pos);
      dists.push_back(distance(&sensor_pos, &vals[i]));

    }

    return dists;
  }

  std::vector<Vec2> sensor_values(std::vector<Line> *walls) {
    std::vector<Vec2> sensorVals;

    for (auto sensor : sensors) {
      auto sensorFix = vecRotated(&sensor.pos, dir);
      auto sensorPt1 = add(&pos, &sensorFix);

      auto unitVec = Vec2{0., 1.0};
      auto sensorPt2 = vecWithHeading(&unitVec, sensor.dir + dir);
      sensorPt2 = add(&sensorPt1, &sensorPt2);

      auto sensorLine = Line{sensorPt1, sensorPt2};

      Vec2 sensorVal;
      double shortestDist = -1;

      for (auto wall : *walls) {
        CollissionInfo collission = checkCollission(&sensorLine, &wall);

        if (collission.onSecondLine && collission.infrontFirst) {
          if (distance(&pos, &collission.point) < shortestDist ||
              shortestDist < 0.0) {
            sensorVal = collission.point;
            shortestDist = distance(&pos, &collission.point);
          }
        }
      }

      if (shortestDist > -1.0) {
        sensorVals.push_back(sensorVal);
      }
    }

    return sensorVals;
  }
};
