#pragma once
#include "cell.h"
#pragma once
#include "utils.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <cstdlib>

struct Maze {
  int nrows, ncols;
  int cell_size;

  std::vector<Cell> cells;
  std::vector<bool> visit_map;

  Maze(int num_rows, int num_cols, int cell_size_v)
      : nrows(num_rows), ncols(num_cols), cell_size(cell_size_v) {
    for (int i = 0; i < nrows * ncols; i++) {
      cells.push_back(Cell(i % ncols, i / ncols));
      visit_map.push_back(false);
    }
  }

  Vec2 xy_from_cell_idx(int idx) {
    return Vec2{(float)(idx % ncols), (float)(idx / ncols)};
  }

  Cell *cell_at(int x, int y) { return &cells[y * ncols + x]; }

  void visit_cell(int x, int y) { visit_map[y * ncols + x] = true; }

  bool has_visited(int x, int y) { return visit_map[y* ncols + x]; }

  void render(sf::RenderWindow* window) {
    for (int x = 0; x < ncols; x++) {
      for (int y = 0; y < nrows; y++) {
        Cell* c = cell_at(x, y);
        sf::Vector2f pos(x*cell_size + 1, y*cell_size + 1);
        c->render(window, &pos, cell_size);
      }
    }

  }

  void generate_maze(Vec2 initial_cell) {

    srand(10435);
    std::vector<Cell *> stack;
    std::vector<Cell *> route;

    stack.push_back(cell_at(initial_cell.x, initial_cell.y));
    visit_cell(initial_cell.x, initial_cell.y);
    route.push_back(cell_at(initial_cell.x, initial_cell.y));

    while (stack.size() > 0) {
      Cell *c = stack[stack.size() - 1];
      stack.pop_back();


      std::vector<Cell *> possible_visits;
      for (int x = c->i - 1; x <= c->i + 1; x++) {
        for (int y = c->j - 1; y <= c->j + 1; y++) {
          if (x < 0 || y < 0 || x >= ncols || y >= nrows || (x == c->i &&
              y == c->j) || (x != c->i && y != c->j))
            continue;


          if (!has_visited(x, y))
          possible_visits.push_back(cell_at(x, y));
        }
      }

      if (possible_visits.size() > 0) {
        route.push_back(c);
        int pick = rand() % possible_visits.size();
        Cell *nc = possible_visits[pick];

        if (nc->i > c->i) {
          nc->break_wall(CellWall::Left);
          c->break_wall(CellWall::Right);
        }
        else if (nc->i < c->i) {
          c->break_wall(CellWall::Left);
          nc->break_wall(CellWall::Right);
        }
        else if (nc->j > c->j) {
          c->break_wall(CellWall::Down);
          nc->break_wall(CellWall::Up);
        }
        else if (nc->j < c->j) {
          nc->break_wall(CellWall::Down);
          c->break_wall(CellWall::Up);
        } else {
          printf("\nREACHED UNREACHABLE BRANCH. No walls to break\n");
        }

        visit_cell(nc->i, nc->j);
        stack.push_back(nc);
      } else {
        if (route.size() > 0) {
        auto back = route[route.size() - 1];
        stack.push_back(back);
        route.pop_back();
        }
      }
    }
  }
};
