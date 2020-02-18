//
// Created by Timothée Couble on 16/02/2020.
//

#include <MazeDisplay.hpp>

void scene::MazeDisplay::init() {
  backstage::walls_t _test = _maze.getWalls();
  const glm::vec2 size = glm::vec2(_maze.getWidth(), _maze.getLength());

  for (const auto &it : _test) {
    auto *cube = new scene::Wall();
    const glm::vec2 &start = it[0];
    const glm::vec2 &end = it[1];
    cube->setPosition(glm::vec3(start.x - size.x / 2, 1.0f, start.y - size.y / 2));
    cube->setShape(glm::vec3(1.0f + end.x - start.x, 1.0f, 1.0f + end.y - start.y));
    _walls.push_back(cube);
  }
  _floor = new scene::Wall();
  _floor->setPosition(glm::vec3(-size.x / 2, 0, -size.y / 2));
  _floor->setShape(glm::vec3(size.x, 0.2f, size.y));
}

void scene::MazeDisplay::draw(const scene::Models_t &models, const gl_wrapper::Shaders_t &shaders) {
  for (const auto &it : _walls)
    it->draw(models, shaders);
  _floor->draw(models, shaders);
}

void scene::MazeDisplay::checkDepth(const scene::Models_t &models, const gl_wrapper::Shader_ptr_t &depth) {
  for (const auto &it : _walls)
    it->checkDepth(models, depth);
  _floor->checkDepth(models, depth);
}

void scene::MazeDisplay::clear() {
  for (const auto &it : _walls)
    delete it;
  delete _floor;
}
