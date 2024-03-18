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

#define THRESH_LOWER 10.0
#define THRESH_UPPER 10.0
#define THRESH_WALL 27.0

#define FRONT_WALL_THRESH 45.0

#define TURN_RATE 0.13
#define TURN_SPEED 1.0

#define FAST_TURN_RATE 0.1
#define SLOW_TURN_RATE 0.02

#define FAST_SPEED 7.5
#define SLOW_SPEED 1.0

int next_dir = -1;
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
  TurnAround,
  FwdExit,
};

enum TurnStage {
  ToCenter,
  Turn,
  ExitTurn,
};

TurnStage turn_stage = ToCenter;

namespace exit_mode {
enum ExitMode {
  LeftTurn,
  RightTurn,
  Fwd,
};
}

struct Window {
  Vec2 left_p;
  Vec2 right_p;

  Window(Vec2 lp, Vec2 rp) : left_p(lp), right_p(rp) {}
};

exit_mode::ExitMode exit_substate;
State s = MoveFwd;

Vec2 sensor_dx(0., 0.);
Window exit_marker = Window(Vec2(-1, -1), Vec2(-1, -1));

float turn = 0.0;
float dl = 0.0;

std::vector<int> open_sensors;
std::vector<Vec2> points;

Window prev_sensors = Window(Vec2(-1, -1), Vec2(-1, -1));
Vec2 prev_dists = Vec2(-1, -1);

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

Vec2 get_sensor_point(Vec2 s_rel_pos, double s_heading, double dist) {

  s_rel_pos.set_heading(heading + s_heading);

  auto s_pos = s_rel_pos.added(&pos);

  auto detection_dir = Vec2(heading + s_heading);
  detection_dir.set_magnitude(dist);

  auto detection_pos = s_pos.added(&detection_dir);

  return detection_pos;
}

void add_sensor_point(Vec2 s_rel_pos, double s_heading, double dist) {
  auto detection_pos = get_sensor_point(s_rel_pos, s_heading, dist);
  points.push_back(detection_pos);


  if (points.size() > 20000) {
    int pt_size = points.size();
    auto points_new = std::vector<Vec2>();

    for (int i = 9999; i < pt_size; i++) {
      points_new.push_back(points[i]);

    }
    points = points_new;
  }
  
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

void setup() { window.clear(sf::Color::Black); }

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
  case FwdExit:
    printf("FwdExit");
    break;
  case TurnAround:
    printf("TurnAround");
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

bool received_key = true;

void update_open_sensors() {

  open_sensors.clear();

  if (get_sensor_dist(RIGHT_SENSOR) > THRESH_WALL )
    open_sensors.push_back(RIGHT_SENSOR);
  if (get_sensor_dist(LEFT_SENSOR) > THRESH_WALL )
    open_sensors.push_back(LEFT_SENSOR);
  if (get_sensor_dist(FWD_SENSOR) > FRONT_WALL_THRESH)
    open_sensors.push_back(FWD_SENSOR);
}

void draw() {
  window.clear(sf::Color::Black);
  sf::Event e;
  window.pollEvent(e);

  if (e.type == sf::Event::KeyPressed) {
    switch (e.key.code) {
    case sf::Keyboard::Key::Up:
      next_dir = FWD_SENSOR;
      break;
    case sf::Keyboard::Key::Down:
      move_forward_pos(-4.0);
      break;
    case sf::Keyboard::Key::Right:
      next_dir = RIGHT_SENSOR;
      break;
    case sf::Keyboard::Key::Left:
        next_dir = LEFT_SENSOR;
      break;
    default:
      break;
    }
  }

  for (auto pt : points) {
    Vec2 pt_pos = pt.subbed(&pos);
    pt_pos.rotate(center_heading - heading);
    pt_pos.add(&screen_center);

    sf::CircleShape s(1);
    s.setPosition(pt_pos.x, pt_pos.y);
    s.setFillColor(sf::Color::Green);
    window.draw(s);
  }

  Vec2 compass_pos = Vec2(900 - 50, 50);
  sf::VertexArray compass(sf::TriangleStrip, 3);
  Vec2 compass_p1 = Vec2(center_heading - heading);
  compass_p1.set_magnitude(40.0);
  compass_p1.add(&compass_pos);

  Vec2 compass_p2 = Vec2(center_heading - heading + M_PI_2);
  compass_p2.set_magnitude(10.0);
  compass_p2.add(&compass_pos);

  Vec2 compass_p3 = Vec2(center_heading - heading - M_PI_2);
  compass_p3.set_magnitude(10.0);
  compass_p3.add(&compass_pos);

  compass[0].position = sf::Vector2f(compass_p1.x, compass_p1.y);
  compass[0].color = sf::Color::Cyan;
  compass[1].position = sf::Vector2f(compass_p2.x, compass_p2.y);
  compass[1].color = sf::Color::Cyan;
  compass[2].position = sf::Vector2f(compass_p3.x, compass_p3.y);
  compass[2].color = sf::Color::Cyan;

  window.draw(compass);

  sf::VertexArray vehicle(sf::TriangleStrip, 3);
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

  sf::CircleShape marker1(4.0);

  Vec2 m1p1 = exit_marker.left_p.subbed(&pos);
  m1p1.rotate(center_heading - heading);
  m1p1.add(&screen_center);

  Vec2 m1p2 = exit_marker.right_p.subbed(&pos);
  m1p2.rotate(center_heading - heading);
  m1p2.add(&screen_center);

  marker1.setFillColor(sf::Color::Magenta);
  marker1.setPosition(m1p1.x, m1p1.y);
  window.draw(marker1);

  marker1.setPosition(m1p2.x, m1p2.y);
  window.draw(marker1);

  window.draw(vehicle);

  window.display();
}

void set_marker() {
  Vec2 left_u = Vec2(heading - M_PI_2);
  Vec2 right_u = Vec2(heading + M_PI_2);

  double nearest_l = -1;
  double nearest_r = -1;

  Vec2 nearest_l_pt(-1, -1);
  Vec2 nearest_r_pt(-1, -1);

  for (Vec2 pt : points) {
    Vec2 dr = pt.subbed(&pos);
    Vec2 dru = dr.normalized();
    double weighted_dist_l = dr.magnitude() / (dru.dot(&left_u));
    double weighted_dist_r = dr.magnitude() / (dru.dot(&right_u));

    if ((weighted_dist_l > 0 && weighted_dist_l < nearest_l) || nearest_l < 0) {
      nearest_l = weighted_dist_l;
      nearest_l_pt = pt;
    }
    if ((weighted_dist_r > 0 && weighted_dist_r < nearest_r) || nearest_r < 0) {
      nearest_r = weighted_dist_r;
      nearest_r_pt = pt;
    }
  }

  exit_marker = Window(nearest_l_pt, nearest_r_pt);
}


bool hit_left_wall = false;
bool hit_right_wall = false;

void loop() {
  draw();
  add_sensor_points();

  print_state();
  printf("\n");

  auto lsp = prev_sensors.left_p;
  if (get_sensor_dist(LEFT_SENSOR) < THRESH_WALL) {
    auto lsp1 =
        get_sensor_point(Vec2(-10, 0), -M_PI_2, get_sensor_dist(LEFT_SENSOR));
    if (lsp1.distance(&pos) < lsp.distance(&pos)) {
      lsp = lsp1;
    }
  }
  auto rsp = prev_sensors.right_p;
  if (get_sensor_dist(RIGHT_SENSOR) < THRESH_WALL) {
    auto rsp1 =
        get_sensor_point(Vec2(10, 0), M_PI_2, get_sensor_dist(RIGHT_SENSOR));
    if (rsp1.distance(&pos) < rsp.distance(&pos)) {
      rsp = rsp1;
    }
  }
  auto window = Window(lsp, rsp);
  prev_sensors = window;
  if (prev_dists.x > -1) {
    sensor_dx = Vec2(get_sensor_dist(LEFT_SENSOR) - prev_dists.x,
                     get_sensor_dist(RIGHT_SENSOR) - prev_dists.y);
  }
  prev_dists =
      Vec2(get_sensor_dist(LEFT_SENSOR), get_sensor_dist(RIGHT_SENSOR));

  if (s == MoveFwd) {

    if (get_sensor_dist(RIGHT_SENSOR) < THRESH_WALL &&
        get_sensor_dist(LEFT_SENSOR) < THRESH_WALL &&
        get_sensor_dist(FWD_SENSOR) > THRESH_LOWER) {
      if (sensor_dx.y > 0.0 || get_sensor_dist(LEFT_SENSOR) < THRESH_LOWER) {
        turn_heading(SLOW_TURN_RATE);
      } else if (sensor_dx.y < 0.0 ||
                 get_sensor_dist(RIGHT_SENSOR) < THRESH_LOWER) {
        turn_heading(-SLOW_TURN_RATE);
      }
      move_forward_pos(FAST_SPEED);
      // } else if (get_sensor_dist(RIGHT_SENSOR) > THRESH_WALL) {
      //   // received_key = false;
      //   set_marker();
      //   // exit_marker = prev_sensors;
      //
      //   s = RightTurn;
      //   turn_stage = ToCenter;
      //   turn = heading;
      // } else if (get_sensor_dist(LEFT_SENSOR) > THRESH_WALL) {
      //
      //   // received_key = false;
      //   set_marker();
      //   s = LeftTurn;
      //   turn_stage = ToCenter;
      //   turn = heading;
      // received_key = false;
    } else {
      // turn around
      update_open_sensors();

      if (open_sensors.size() <= 0) {
        s = TurnAround;
        turn = heading;
      } else {
        int dir_pick = open_sensors[rand() % open_sensors.size()];
        set_marker();
        turn_stage = ToCenter;
        turn = heading;

        // if (dir_pick == FWD_SENSOR) TODO: Implement FwdExit properly
        // dir_pick = next_dir;
        if (dir_pick == FWD_SENSOR)
          s = FwdExit;
        if (dir_pick == LEFT_SENSOR)
          s = LeftTurn;
        else if (dir_pick == RIGHT_SENSOR)
          s = RightTurn;
        next_dir = -1;
      }
    }
  }

  if (s == TurnAround) {
    if (abs(heading - turn) < M_PI) {
      turn_heading(FAST_TURN_RATE);
    } else {
      s = MoveFwd;
    }
  }

  if (s == RightTurn && received_key) {

    if (turn_stage == ToCenter) {
      auto window_vec = exit_marker.left_p.subbed(&exit_marker.right_p);
      auto window_uvec = window_vec.normalized();
      auto delta_pos = pos.subbed(&exit_marker.right_p);
      auto fwd_vec = window_uvec.rotated(-M_PI_2);

      double fwd_dist = abs(fwd_vec.dot(&delta_pos));
      if (fwd_dist > THRESH_WALL / 1.5 || get_sensor_dist(FWD_SENSOR) < THRESH_LOWER) {
        turn = heading;
        turn_stage = Turn;
      } else {
        move_forward_pos(FAST_SPEED);
      }

    } else if (turn_stage == Turn) {
      if (abs(heading - turn) < M_PI_2) {
        turn_heading(FAST_TURN_RATE);
      } else {
        turn_stage = ExitTurn;
      }
      hit_left_wall = false;
      hit_right_wall = false;
    } else {
      auto window_vec = exit_marker.right_p.subbed(&exit_marker.left_p);
      auto window_uvec = window_vec.normalized();
      auto delta_pos = pos.subbed(&exit_marker.right_p);

      double normal_dist_travelled = window_uvec.dot(&delta_pos);

      if (get_sensor_dist(LEFT_SENSOR) < THRESH_WALL) {
        hit_left_wall = true;
      }
      if (get_sensor_dist(RIGHT_SENSOR) < THRESH_WALL) {
        hit_right_wall = true;
      }

      if (normal_dist_travelled > THRESH_WALL / 2.5 ||
          (hit_left_wall && hit_right_wall) || get_sensor_dist(FWD_SENSOR) < THRESH_LOWER) {
        s = MoveFwd;
      } else {
        move_forward_pos(FAST_SPEED);
      }
    }
  }

  if (s == FwdExit && received_key) {
    if (turn_stage == ToCenter) {
      turn_stage = ExitTurn;
      // auto window_vec = exit_marker.left_p.subbed(&exit_marker.right_p);
      // auto window_uvec = window_vec.normalized();
      // window_uvec.rotate(M_PI_2);
      // auto delta_pos = pos.subbed(&exit_marker.left_p);
      //
      //
      // double dist_straight = window_uvec.dot(&delta_pos);
      //
      // if (dist_straight > THRESH_WALL/5.0) {
      //   turn_stage = ExitTurn;
      // } else {
      //   move_forward_pos(FAST_SPEED);
      // }
    }
    if (turn_stage == ExitTurn) {
      auto window_vec = exit_marker.left_p.subbed(&exit_marker.right_p);
      auto window_uvec = window_vec.normalized();
      window_uvec.rotate(M_PI_2);
      auto delta_pos = pos.subbed(&exit_marker.left_p);

      hit_left_wall = false;
      hit_right_wall = false;

      if (get_sensor_dist(LEFT_SENSOR) < THRESH_WALL) {
        hit_left_wall = true;
      }
      if (get_sensor_dist(RIGHT_SENSOR) < THRESH_WALL) {
        hit_right_wall = true;
      }

      double dist_straight = window_uvec.dot(&delta_pos);

      if (abs(dist_straight) > FRONT_WALL_THRESH+3 || (hit_left_wall && hit_right_wall) || get_sensor_dist(FWD_SENSOR) < THRESH_LOWER) {
        s = MoveFwd;
      } else {
        move_forward_pos(FAST_SPEED);
      }
    }
  }

  if (s == LeftTurn && received_key) {

    if (turn_stage == ToCenter) {
      auto window_vec = exit_marker.right_p.subbed(&exit_marker.left_p);
      auto window_uvec = window_vec.normalized();
      auto delta_pos = pos.subbed(&exit_marker.left_p);
      auto fwd_vec = window_uvec.rotated(M_PI_2);

      double fwd_dist = abs(fwd_vec.dot(&delta_pos));
      if (fwd_dist > THRESH_WALL / 1.5 || get_sensor_dist(FWD_SENSOR) < THRESH_LOWER) {
        turn = heading;
        turn_stage = Turn;
      } else {
        move_forward_pos(FAST_SPEED);
      }

    } else if (turn_stage == Turn) {
      if (abs(heading - turn) < M_PI_2) {
        turn_heading(-FAST_TURN_RATE);
      } else {
        turn_stage = ExitTurn;
      }
      hit_left_wall = false;
      hit_right_wall = false;
    } else {
      auto window_vec = exit_marker.left_p.subbed(&exit_marker.right_p);
      auto window_uvec = window_vec.normalized();
      auto delta_pos = pos.subbed(&exit_marker.left_p);

      if (get_sensor_dist(LEFT_SENSOR) < THRESH_WALL) {
        hit_left_wall = true;
      }
      if (get_sensor_dist(RIGHT_SENSOR) < THRESH_WALL) {
        hit_right_wall = true;
      }

      double normal_dist_travelled = window_uvec.dot(&delta_pos);

      if (normal_dist_travelled > THRESH_WALL / 2.5 ||
          (hit_left_wall && hit_right_wall) || get_sensor_dist(FWD_SENSOR) < THRESH_LOWER) {
        s = MoveFwd;
      } else {
        move_forward_pos(FAST_SPEED);
      }
    }
  }
}
