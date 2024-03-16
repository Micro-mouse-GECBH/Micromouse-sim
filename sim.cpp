#include "headers/sim.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#define LEFT_SENSOR 1
#define RIGHT_SENSOR 0
#define FWD_SENSOR 2

#define THRESH_LOWER 5.0
#define THRESH_UPPER 20.0
#define THRESH_WALL 25.0

#define TURN_RATE 0.03
#define TURN_SPEED 1.0

// to change heading
//  rotate(angular_velocity)

// to move forward/backward
// move_forward(velocity)

// to get sensor readings
// get_sensor_dist(sensor_index)

bool slow_move = false;
bool stop = false;

enum State {
  MoveFwd,
  Stop,
  ArrivedAtJn,
  JnExit,
  RightTurn,
  LeftTurn,
};

namespace exit_mode {
enum ExitMode {
  LeftTurn,
  RightTurn,
  Fwd,
};
}

exit_mode::ExitMode exit_substate;
State s = Stop;

float turn = 0.0;
float dl = 0.0;

std::vector<int> open_sensors;
std::vector<Vec2> points;

Vec2 pos = Vec2(300.0, 200.0);
Vec2 screen_center = Vec2(450, 450);

float center_heading = -M_PI_2;
float heading = 0.0;

sf::RenderWindow window(sf::VideoMode(900, 900), "micromouse pov");

void turn_heading(float rate) {
  heading += rate;
  rotate(rate);
}

void move_forward_pos(float rate) {
  auto v = Vec2(heading);
  v.set_magnitude(rate);
  pos.add(&v);
  move_forward(rate);
}

void add_sensor_point(Vec2 s_rel_pos, double s_heading, double dist) {
  s_rel_pos.set_heading(heading + s_heading);

  auto s_pos = s_rel_pos.added(&pos);

  auto detection_dir = Vec2(heading + s_heading);
  detection_dir.set_magnitude(dist);

  auto detection_pos = s_pos.added(&detection_dir);

  points.push_back(detection_pos);
}

void add_sensor_points() {
  add_sensor_point(Vec2(0, -35. / 2), 0., get_sensor_dist(FWD_SENSOR));
  add_sensor_point(Vec2(-10.0, 0.), -M_PI / 2.0, get_sensor_dist(LEFT_SENSOR));
  add_sensor_point(Vec2(+10.0, 0.), M_PI / 2.0, get_sensor_dist(RIGHT_SENSOR));
  // auto s_rel_pos = Vec2(0, -35. / 2);
  // s_rel_pos.set_heading(heading);
  //
  // auto s_pos = s_rel_pos.added(&pos);
  //
  // auto detection_dir = Vec2(heading);
  // detection_dir.set_magnitude(get_sensor_dist(FWD_SENSOR));
  //
  // auto detection_pos = s_pos.added(&detection_dir);
  //
  // points.push_back(detection_pos);
}

void setup() {
  window.clear(sf::Color::Black);

}

void print_state() {
  switch (s) {
  case MoveFwd:
    printf("MoveFwd");
    break;
  case LeftTurn:
    printf("LeftTurn");
    break;
  case RightTurn:
    printf("RightTurn");
    break;
  case ArrivedAtJn:
    printf("ArriveAtJn");
    break;
  case JnExit:
    printf("JnExit");
    break;
  case Stop:
    printf("Stop");
    break;
  }
}

void right_turn() {

  if (get_sensor_dist(RIGHT_SENSOR) > THRESH_UPPER) {
    rotate(TURN_RATE);
    turn += TURN_RATE;
  } else {
    move_forward(TURN_SPEED);
  }
}

void left_turn() {
  if (get_sensor_dist(LEFT_SENSOR) > THRESH_UPPER) {
    rotate(-TURN_RATE);
    turn += TURN_RATE;
  } else {
    move_forward(TURN_SPEED);
  }
}

void update_open_sensors() {

  open_sensors.clear();

  if (get_sensor_dist(RIGHT_SENSOR) > THRESH_UPPER)
    open_sensors.push_back(RIGHT_SENSOR);
  if (get_sensor_dist(LEFT_SENSOR) > THRESH_UPPER)
    open_sensors.push_back(LEFT_SENSOR);
  if (get_sensor_dist(FWD_SENSOR) > THRESH_UPPER)
    open_sensors.push_back(FWD_SENSOR);
}

void draw() {
  window.clear(sf::Color::Black);
  sf::Event e;
  window.pollEvent(e);

  if (e.type == sf::Event::KeyPressed) {
    switch (e.key.code) {
    case sf::Keyboard::Key::Up:
      move_forward_pos(4.0);
      break;
    case sf::Keyboard::Key::Down:
      move_forward_pos(-4.0);
      break;
    case sf::Keyboard::Key::Right:
      turn_heading(0.1);
      break;
    case sf::Keyboard::Key::Left:
      turn_heading(-0.1);
      break;
    default:
      break;
    }
  }


  for (auto pt : points) {
    Vec2 pt_pos = pt.subbed(&pos);
    pt_pos.rotate(center_heading-heading);
    pt_pos.add(&screen_center);

    sf::CircleShape s(1);
    s.setPosition(pt_pos.x, pt_pos.y);
    s.setFillColor(sf::Color::Green);
    window.draw(s);
  }

  sf::VertexArray vehicle( sf::TriangleStrip, 3);
  Vec2 vehicle_p1 = Vec2(center_heading);
  vehicle_p1.set_magnitude(20.0);
  vehicle_p1.add(&screen_center);

  Vec2 vehicle_p2 = Vec2(center_heading + M_PI_2);
  vehicle_p2.set_magnitude(10.0);
  vehicle_p2.add(&screen_center);

  Vec2 vehicle_p3 = Vec2(center_heading - M_PI_2);
  vehicle_p3.set_magnitude(10.0);
  vehicle_p3.add(&screen_center);


  vehicle[0].position = sf::Vector2f(vehicle_p1.x, vehicle_p1.y);
  vehicle[1].position = sf::Vector2f(vehicle_p2.x, vehicle_p2.y);
  vehicle[2].position = sf::Vector2f(vehicle_p3.x, vehicle_p3.y);

  window.draw(vehicle);


  window.display();
}

void loop() {
  draw();
  add_sensor_points();

  // if (s == MoveFwd) {
  //   turn = 0.0;
  //   if (get_sensor_dist(FWD_SENSOR) < THRESH_LOWER) {
  //     s = Stop;
  //   }
  //
  //
  //   update_open_sensors();
  //
  //   if (open_sensors.size() > 1) {
  //     //junction
  //     s = ArrivedAtJn;
  //
  //
  //
  //   } else {
  //     //turn
  //     if (get_sensor_dist(RIGHT_SENSOR) > THRESH_UPPER) {
  //       s = RightTurn;
  //     }
  //
  //     if (get_sensor_dist(LEFT_SENSOR) > THRESH_UPPER) {
  //       s = LeftTurn;
  //     }
  //   }
  // }
  //
  // printf("%f %f ", get_sensor_dist(RIGHT_SENSOR),
  // get_sensor_dist(LEFT_SENSOR)); print_state(); printf("\n");
  //
  // if ( s == RightTurn ) {
  //   right_turn();
  //   if (turn >= 3.14/2) {
  //     s = MoveFwd;
  //   }
  // }
  //
  // if ( s == LeftTurn ) {
  //   left_turn();
  //   if (turn >= 3.14/2) {
  //     s = MoveFwd;
  //   }
  // }
  //
  // if ( s == ArrivedAtJn ) {
  //     int chosen_dir = open_sensors[random() % open_sensors.size()];
  //     switch (chosen_dir) {
  //       case RIGHT_SENSOR:
  //         exit_substate = exit_mode::ExitMode::RightTurn;
  //         break;
  //     case LEFT_SENSOR:
  //         exit_substate = exit_mode::ExitMode::LeftTurn;
  //         break;
  //     case FWD_SENSOR:
  //         exit_substate = exit_mode::ExitMode::Fwd;
  //         break;
  //   }
  //   s = JnExit;
  //   dl = 0.0;
  //   turn = 0.0;
  // }
  //
  // if ( s== JnExit) {
  //   switch (exit_substate) {
  //     case exit_mode::LeftTurn:
  //       left_turn();
  //       if (turn >= 3.14/2) {
  //         exit_substate = exit_mode::Fwd;
  //       }
  //       break;
  //
  //     case exit_mode::RightTurn:
  //       right_turn();
  //       if (turn >= 3.14/2) {
  //         exit_substate = exit_mode::Fwd;
  //       }
  //       break;
  //
  //     case exit_mode::Fwd:
  //       dl += 0.4;
  //       move_forward(0.4);
  //       if (dl < 4.0) {
  //          break;
  //       }
  //       if (get_sensor_dist(LEFT_SENSOR) < THRESH_WALL ||
  //       get_sensor_dist(RIGHT_SENSOR) < THRESH_WALL ||
  //       get_sensor_dist(FWD_SENSOR) < THRESH_WALL) {
  //         s = MoveFwd;
  //       }
  //   }
  // }
  //
  // if (s==MoveFwd) {
  //   move_forward(0.4);
  // }
}
