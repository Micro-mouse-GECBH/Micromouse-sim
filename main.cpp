#include "headers/maze.h"
#include "headers/vector.h"
#include "headers/vehicle.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window.hpp>
#include <SFML/Window/Keyboard.hpp>
#include "headers/sim.h"
#include "headers/sim_fns.h"
#include <cmath>
#include <iostream>

Vehicle sim_v;
std::vector<Line> walls;

void rotate(float aspeed) {
  sim_v.rotate(aspeed);
}

void move_forward(float speed) {
  sim_v.move_forward(speed);
}

float get_sensor_dist(int idx) {
  return sim_v.distance_readings(&walls, 3)[idx];
}

int main() {
  sf::RenderWindow window(sf::VideoMode(900, 900), "Micromouse sim");

  window.setVerticalSyncEnabled(true);

  window.setFramerateLimit(60);


  sim_v.dir = 0;
  sim_v.pos = Vec2{300.0, 200.0};
  sim_v.sensors = {Sensor{Vec2{10.0, 0.0/2.0}, 0.}};
  sim_v.sensors.push_back(Sensor{Vec2{-10.0, 0.0/2.0}, M_PI});
  sim_v.sensors.push_back(Sensor{Vec2{0.0, -35.0 / 2.0}, -M_PI_2});


  Maze maze(20, 20, 45);

  maze.generate_maze(Vec2{0, 0});

  for (int x = 0; x < maze.ncols; x++) {
    for (int y = 0; y < maze.nrows; y++) {
      Vec2 pos = Vec2{(double)x * maze.cell_size, (double)y * maze.cell_size};
      auto cell_walls = maze.cell_at(x, y)->get_walls_as_lines(&pos, maze.cell_size);

      for (auto wall: cell_walls) {
        walls.push_back(wall);
      }
    }
  }


  setup();

  while (window.isOpen()) {
    sf::Event event;
    if (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        window.close();
        break;
      }

      if (event.type == sf::Event::KeyPressed) {
        // if (event.key.code == sf::Keyboard::Key::Up) {
        //   sim_v.move_forward(5.0);
        // } else if (event.key.code == sf::Keyboard::Key::Down) {
        //   sim_v.move_forward(-5.0);
        // }
        //
        // else if (event.key.code == sf::Keyboard::Key::Left) {
        //   sim_v.rotate(-0.05);
        // } else if (event.key.code == sf::Keyboard::Key::Right) {
        //   sim_v.rotate(0.05);
        // }
      }
    }

    loop();

    window.clear(sf::Color::Black);

    // for (auto l : walls) {
    //   sf::VertexArray line(sf::LineStrip, 2);
    //   line[0].position = sf::Vector2f(l.p1.x, l.p1.y);
    //   line[1].position = sf::Vector2f(l.p2.x, l.p2.y);
    //   window.draw(line);
    // }

    sf::RectangleShape vehicleShape(sf::Vector2f(20, 35));
    vehicleShape.setOrigin(10, 35.0 / 2.0);
    vehicleShape.setPosition(sim_v.pos.x, sim_v.pos.y);
    vehicleShape.setFillColor(sf::Color::Magenta);

    vehicleShape.rotate(sim_v.dir * 180 / M_PI);

    sf::RectangleShape vehicleHead(sf::Vector2f(20, 5));
    vehicleHead.setOrigin(10, 35.0 / 2.0);
    vehicleHead.setPosition(sim_v.pos.x, sim_v.pos.y);
    vehicleHead.setFillColor(sf::Color::Yellow);

    vehicleHead.rotate(sim_v.dir * 180 / M_PI);

    auto sensorVals = sim_v.sensor_values(&walls);
    

    for (auto val : sensorVals) {
      sf::CircleShape c(5);
      c.setFillColor(sf::Color::Green);
      c.setPosition(val.x - 5, val.y - 5);
      window.draw(c);
    }

    for (auto s : sim_v.sensors) {

      auto basePos = vecRotated(&s.pos, sim_v.dir);
      basePos = add(&basePos, &sim_v.pos);

      auto nextPos = Vec2{0., 200.0};
      nextPos = vecWithHeading(&nextPos, sim_v.dir + s.dir);
      nextPos = add(&basePos, &nextPos);

      sf::VertexArray line(sf::LineStrip, 2);
      line[0].position = sf::Vector2f(basePos.x, basePos.y);
      line[1].position = sf::Vector2f(nextPos.x, nextPos.y);

      line[0].color = sf::Color::Blue;
      line[1].color = sf::Color(0, 0, 20);
      window.draw(line);
    }

    window.draw(vehicleShape);
    window.draw(vehicleHead);

    maze.render(&window);

    auto svals = sim_v.distance_readings(&walls, 3);

    window.display();
  }

  return 0;
}
