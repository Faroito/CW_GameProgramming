/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the MIT License.
 * If a copy of the ML was not distributed with this
 * file, You can obtain one at https://opensource.org/licenses/MIT */

/* author: jackred@tuta.io */

#include "Maze.hpp"

backstage::Maze::Maze(size_t width, size_t length, size_t minDist) {
  _width = width;
  _length = length;
  srand(time(NULL));
  init(minDist);  
}

// regenerate a maze
void backstage::Maze::init(size_t minDist) {
  do {
    _maze = generateEmptyMaze();
    generateSeed();
    generateCorridor();
    eraseCorridor();
    makeBorder();
    assignStart();
    toWalls();
    assignEndUntilPath(minDist);
  } while(_aStar.empty());
}

// create empty maze
backstage::maze_t backstage::Maze::generateEmptyMaze() {
  maze_t newMaze;
  for (size_t i=0 ; i<_width ; i++) {
    std::vector<unsigned int> arr(_length);
    newMaze.push_back(arr);
    for (size_t j=0; j<_length; j++) {
      newMaze[i][j]=0;
    }
  }
  return newMaze;
}

// create a random 0/1 maze
void backstage::Maze::generateSeed() {
  for (size_t i=0 ; i<_width ; i++) {
    for (size_t j=0 ; j<_length ; j++) {
      _maze[i][j] = (rand() % 2);
    }
  }
}

// count the number of neighbor being cell around a particular cell
unsigned int backstage::Maze::countNeighbors(backstage::maze_t maze, size_t x, size_t y) {
  unsigned int sum=0;
  size_t minx = (x == 0) ? 0 : x-1;
  size_t miny = (y == 0) ? 0 : y-1;
  for (size_t i=minx ; i<std::min(maze.size(), x+2) ; i++) {
    for (size_t j=miny ; j<std::min(maze[0].size(), y+2) ; j++) {
      if ((i != x) || (j != y)) {
        sum += maze[i][j];
      }
    }
  }
  return sum;
}

// apply rule rule B3_1234 on a cell at time t
// if there is 3 neighbor, the cell become a wall
// if there is 1,2,3 or 4 neighbor, the cell stay alive
// the cell is dead (empty) if not
unsigned int backstage::Maze::ruleB3_1234Cell(backstage::maze_t maze, size_t x, size_t y) {
  unsigned int nbNeighbors = countNeighbors(maze, x, y);
  return (((maze[x][y] == 1) && ((nbNeighbors >= 1) && (nbNeighbors <= 4)))|| nbNeighbors == 3);
}

// apply rule B3_1234 on all cel at time t
void backstage::Maze::ruleB3_1234Iteration() {
  backstage::maze_t newMaze = generateEmptyMaze();
  for (size_t i=0 ; i<_width ; i++) {
    for (size_t j=0 ; j<_length ; j++) {
      newMaze[i][j] = ruleB3_1234Cell(_maze, i, j);
    }
  }
  _maze = newMaze;
}

// apply an Iteration of the rule B3_1234 20 times to generate a maze
void backstage::Maze::generateCorridor() {
  for (int i = 0 ; i < 20 ; i++) {
    // std::cout << *this << std::endl;
    ruleB3_1234Iteration();
  }
}

// erase some corridor randomly to make more path
void backstage::Maze::eraseCorridor() {
  for (size_t i=0 ; i<_width ; i++) {
    for (size_t j=0 ; j<_length ; j++) {
      size_t tmpCount = countNeighbors(_maze, i, j);
      if ((_maze[i][j] == 1) && ( tmpCount>= 1) && (tmpCount <= 5) && ((rand() % 9) == 0)){
        _maze[i][j] = 0;
      }
    }
  }
}

// add a border
void backstage::Maze::makeBorder() {
  for (size_t i=0 ; i<_width ; i++) {
    for (size_t j=0 ; j<_length ; j++) {
      if (( i == 0) || (j == 0) || ((i+1) == _width) || ((j+1) == _length)){
        _maze[i][j] = 1;
      }
    }
  }
}

// does this cell has neightbor horizontally?
bool backstage::Maze::hasNeighbor(maze_t maze, size_t x, size_t y) {
  bool res = false;
  if (y != 0) {
    res = res || (maze[x][y-1] == 1);
  }
  if ((y+1) != maze[x].size()) {
    res = res || (maze[x][y+1] == 1);
  }
  return res;
}

// append the vector of wall horizontal of size > 1
void backstage::Maze::toWallsHorizontal() {
  size_t i = 0;
  size_t j = 0;
  size_t start = SIZE_MAX;
  while (i < _width) {
    j = 0;
    start = SIZE_MAX;
    while (j < _length) {
      if (start == SIZE_MAX) {
        if ((_maze[i][j] == 1)  && (hasNeighbor(_maze, i, j))) {
          start = j;
        }
      } else {
        if (_maze[i][j] == 0) {
          if (j > (start+1)) {
            _walls.push_back({glm::vec2(i, start), glm::vec2(i, j-1)});
          }
          start = SIZE_MAX;
        }
      }
      j++;
    }
    if (start != SIZE_MAX) {
      _walls.push_back({glm::vec2(i, start), glm::vec2(i, j-1)});
    }          
    i++;
  }
}

// append all the wall of size vertical >1 without horizontal neighbor
void backstage::Maze::toWallsVertical() {
  size_t i = 0;
  size_t j = 0;
  size_t start = SIZE_MAX;
  while (j < _length) {
    i = 0;
    start = SIZE_MAX;
    while (i < _width) {
      if (start == SIZE_MAX) {
        if ((_maze[i][j] == 1) && !(hasNeighbor(_maze, i, j))) {
          start = i;
        }
      } else {
        if ((_maze[i][j] == 0) || (hasNeighbor(_maze, i, j))) {
          _walls.push_back({glm::vec2(start, j), glm::vec2(i-1, j)});
          start = SIZE_MAX;
        }
      }
      i++;
    }
    if (start != SIZE_MAX) {
      _walls.push_back({glm::vec2(start, j), glm::vec2(i-1, j)});
    }
    j++;
  }
}

// generate the vectors of walls, to optimize wall drawing
void backstage::Maze::toWalls() {
  _walls.clear();
  toWallsHorizontal();
  toWallsVertical();
}

/* overloaded */
// overload maze operator to have debug
std::ostream& backstage::operator<<(std::ostream &os, const backstage::Maze &maze) {
  backstage::maze_t arr = maze.getMaze();
  // os << "\033c";
  for (size_t i=0; i<maze.getWidth(); i++) {
    for (size_t j=0; j<maze.getLength(); j++) {
      if ((i == maze.getStart()[0]) && (j == maze.getStart()[1])) {
        os << "\033[42m "; // start
      } else  if ((i == maze.getEnd()[0]) && (j == maze.getEnd()[1])) {
        os << "\033[41m "; // end
      } else if (maze._aStar2.find(glm::vec2(i, j)) != maze._aStar2.end()) {
        os << "\033[45m-"; // path
      }
      else {
        if (arr[i][j] == 1) {
          os << "\033[46m "; // 1
        } else {
          os << "\033[49m "; // 0
        }
      }
    }
    os << "\033[49m " << std::endl;
  }
  return os;
}

/* getters */
backstage::walls_t backstage::Maze::getWalls() const {
  return _walls;
}

glm::vec2 backstage::Maze::getStart() const {
  return _start;
}

glm::vec2 backstage::Maze::getEnd() const {
  return _end;
}

size_t backstage::Maze::getWidth() const {
  return _width;
}

size_t backstage::Maze::getLength() const {
  return _length;
}

std::vector<std::vector<unsigned int>> backstage::Maze::getMaze() const {
  return _maze;
}

std::stack<glm::vec2> backstage::Maze::getAStar() const {
  return _aStar;
}



/* path */
// create a new start-end path with a reachable end. Start is given in argument
void backstage::Maze::newPath(glm::vec2 start, size_t minDist) {
  _start = start;
  do {
    assignEndUntilPath(minDist);
  } while (_aStar.empty());
}

// create a new start-end path with a reachable end. Start is the old path end
void backstage::Maze::resetPathFromEnd(size_t minDist) {
  newPath(_end, minDist);
}

// set the start of the maze and redo A*
void backstage::Maze::setStart(glm::vec2 start) {
  _start = start;
  aStar();
}

// assign a random start, which is not a wall
void backstage::Maze::assignStart() {
  size_t x;
  size_t y;
  do {
    x = rand() % _width;
    y = rand() % _length;
  } while(_maze[x][y] == 1);
  _start = glm::vec2(x, y);
}


// assign a random end, not a wall.
// Manhattan distance between end and start must me < minDist
void backstage::Maze::assignEnd(size_t minDist) {
  size_t x;
  size_t y;
  do {
    x = rand() % _width;
    y = rand() % _length;
  } while((_maze[x][y] == 1) || (manhattanDistance(_start, glm::vec2(x, y)) <= minDist));
  _end = glm::vec2(x, y);
}

// assign an end until it's reachable or we are over 200 iterations
// if there's more than 200 iterations, the other functions will regenerate the maze
void backstage::Maze::assignEndUntilPath(size_t minDist) {
  int i = 0;
  do {
    assignEnd(minDist);
    aStar();
    i++;
  } while(_aStar.empty() && (i < 200));
}


/* aStar */
size_t backstage::Maze::manhattanDistance(glm::vec2 x, glm::vec2 y) {
  return abs(x[0] - y[0]) + abs(x[1] - y[1]);
}

// insert the next cell to visit in the list, at a sorted position
void backstage::Maze::insertInPosition(glm::vec2 toInsert, size_t heuristicDist, std::list<glm::vec2> &toVisit, std::map<size_t, std::tuple<glm::vec2, size_t, size_t>> mapPoint) {
  std::list<glm::vec2>::iterator it=toVisit.begin();
  while ((it != toVisit.end()) && (std::get<1>(mapPoint[_length * (*it)[0] + (*it)[1]]) + std::get<2>(mapPoint[_length*(*it)[0] + (*it)[1]]) < heuristicDist)) {
    it++;
  }
  toVisit.insert(it, toInsert);
}

// expand a cell
// if it was not visited, add it to the visit list
// if it was visited, update the distance from the start if it's better
void backstage::Maze::expand(glm::vec2 toExpand, size_t x, size_t y, std::list<glm::vec2> &toVisit, std::map<size_t, std::tuple<glm::vec2, size_t, size_t>> &mapPoint, std::set<glm::vec2> visited) {
  if ((x > 0) && (x < _maze.size()) && (y > 0) && (y < _maze.size()) && (_maze[x][y] == 0)) { // if in maze and a cell
    glm::vec2 tmp = glm::vec2(x, y);
    size_t distance = manhattanDistance(_end, tmp);
    size_t distToStart = std::get<2>(mapPoint[_length * toExpand[0] + toExpand[1]])+1;
    std::map<size_t, std::tuple<glm::vec2, size_t, size_t>>::iterator it = mapPoint.find(_length*x + y);
    if (it == mapPoint.end()){
      mapPoint.insert(std::pair<size_t, std::tuple<glm::vec2, size_t, size_t>>(_length*x+y, std::tuple<glm::vec2, size_t, size_t>(toExpand, distance, distToStart)));
      insertInPosition(tmp, distToStart + distance, toVisit, mapPoint);
    }  else if ((std::get<1>(mapPoint[_length*x + y]) + std::get<2>(mapPoint[_length*x + y])) > (distance+distToStart)) {
      it->second = std::tuple<glm::vec2, size_t, size_t>(toExpand, distance, distToStart);
    }
  }
}

// expand all cell around the visited cell
void backstage::Maze::expandAll(glm::vec2 toExpand, std::list<glm::vec2> &toVisit, std::map<size_t, std::tuple<glm::vec2, size_t, size_t>> &mapPoint, std::set<glm::vec2> visited) {
  expand(toExpand, toExpand[0], toExpand[1]+1, toVisit, mapPoint, visited); // south
  expand(toExpand, toExpand[0], toExpand[1]-1, toVisit, mapPoint, visited); // north
  expand(toExpand, toExpand[0]+1, toExpand[1], toVisit, mapPoint, visited); // east
  expand(toExpand, toExpand[0]-1, toExpand[1], toVisit, mapPoint, visited); // west
}


// recursively rebuild the path of A* from the end, and stack it
// also add it in a set for debug (terminal print)
std::stack<glm::vec2> backstage::Maze::rebuildPath(std::map<size_t, std::tuple<glm::vec2, size_t, size_t>> mapPoint, std::stack<glm::vec2> acc, std::set<glm::vec2> acc2) {
  glm::vec2 tmp = std::get<0>(mapPoint[_length * acc.top()[0] + acc.top()[1]]);
  if (tmp == acc.top()) {
    _aStar2 = acc2;
    return acc;
  } else {
    acc.push(tmp);
    acc2.insert(tmp);
    return rebuildPath(mapPoint, acc, acc2);
  }
}

// a star
// visited: set of visited cell
// toVisit: sorted list of cell to Visit
// mapPoint: map of cell, the cell we come from to arrive at this cell, the heuristic and the disctance from the start
void backstage::Maze::aStar() {
  std::set<glm::vec2> visited;
  std::list<glm::vec2> toVisit;
  // X : from Y, h, g
  std::map<size_t, std::tuple<glm::vec2, size_t, size_t>> mapPoint;
  toVisit.push_front(_start);
  mapPoint.insert(std::pair<size_t, std::tuple<glm::vec2, size_t, size_t>>(_length * _start[0] + _start[1], std::tuple<glm::vec2, size_t, size_t>(_start, manhattanDistance(_start, _end), 0)));
  while ((visited.find(_end) == visited.end()) && (toVisit.size() != 0)) {
    glm::vec2 tmp = toVisit.front();
    toVisit.pop_front();
    if (visited.find(tmp) == visited.end()) {
      visited.insert(tmp);
      expandAll(tmp, toVisit, mapPoint, visited);
    }
  }
  if (visited.find(_end) == visited.end()) {
    _aStar = std::stack<glm::vec2>();
  } else {
    _aStar = rebuildPath(mapPoint, std::stack<glm::vec2>({_end}), std::set<glm::vec2>({_end}));
  }
}
