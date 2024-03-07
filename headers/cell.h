#pragma once
#include "vector.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window.hpp>

enum CellWall { Up, Right, Down, Left };

struct Cell {
  int i, j;

  bool walls[4]; // U, R, D, L

  Cell(int pos_i, int pos_j)
      : i(pos_i), j(pos_j), walls{true, true, true, true} {}

  void break_wall(CellWall wall) {
    switch (wall) {
    case CellWall::Up:
      walls[0] = false;
      break;
    case CellWall::Right:
      walls[1] = false;
      break;
    case CellWall::Down:
      walls[2] = false;
      break;
    case CellWall::Left:
      walls[3] = false;
      break;
    }
  }

  std::vector<Line> get_walls_as_lines(Vec2 *pos, int size) {
    std::vector<Line> lines;

    if (walls[0]) {
      lines.push_back(Line{Vec2{pos->x, pos->y}, Vec2{pos->x + size, pos->y}});
    }

    if (walls[1]) {
      lines.push_back(Line{Vec2{pos->x + size, pos->y},
                           Vec2{pos->x + size, pos->y + size}});
    }

    if (walls[2]) {
      lines.push_back(Line{
          Vec2{pos->x + size, pos->y + size},
          Vec2{pos->x, pos->y + size},
      });
    }
    if (walls[3]) {
      lines.push_back(Line{
          Vec2{pos->x, pos->y + size},
          Vec2{pos->x, pos->y},
      });
    }

    return lines;
  };

  void render(sf::RenderWindow *window, sf::Vector2f *pos, int size) {
    sf::VertexArray line = sf::VertexArray(sf::LineStrip, 2);

    line[0].color = sf::Color::White;
    line[1].color = sf::Color::White;

    if (walls[0]) {
      line[0].position = sf::Vector2f(pos->x, pos->y);
      line[1].position = sf::Vector2f(pos->x + size, pos->y);
      window->draw(line);
    }

    if (walls[1]) {
      line[0].position = sf::Vector2f(pos->x + size, pos->y);
      line[1].position = sf::Vector2f(pos->x + size, pos->y + size);
      window->draw(line);
    }

    if (walls[2]) {
      line[0].position = sf::Vector2f(pos->x + size, pos->y + size);
      line[1].position = sf::Vector2f(pos->x, pos->y + size);
      window->draw(line);
    }

    if (walls[3]) {
      line[0].position = sf::Vector2f(pos->x, pos->y + size);
      line[1].position = sf::Vector2f(pos->x, pos->y);
      window->draw(line);
    }
  }
};
