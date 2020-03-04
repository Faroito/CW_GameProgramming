//
// Created by Timothée Couble on 03/03/2020.
//

#include "Player.hpp"

scene::Player::Player(const MazeDisplay &maze) {
    reset(maze);
}

std::ostream &operator<<(std::ostream &stream, const glm::vec3 &vec) {
    stream << vec.x << " " << vec.y << " " << vec.z;
    return stream;
}

void scene::Player::draw(const scene::Models_t &models, const gl_wrapper::Shaders_t &shaders) {
    _ball.draw(models, shaders);
}

void scene::Player::reset(const MazeDisplay &maze) {
    auto &start = maze.getStart();
    _speed = glm::vec3(0.0f);
    _ball.setPosition(glm::vec3(start.x, 1.5f, start.y));
}

void scene::Player::update(scene::Camera_ptr_t &camera, const scene::MazeDisplay &maze) {
    double currentTime = glfwGetTime();
    double delta = currentTime - _lastTime;
    _lastTime = currentTime;

    // Apply gravity
    _speed += glm::vec3(0.0f, -9.81f, 0.0f) * (float) delta * 0.5f;
    auto pos = _ball.getPosition();
    glm::vec3 newPos = pos + _speed * (float) delta;
    normal_collision_t collisions[2];

    try {
        collisions[0] = maze.intersectSphereWalls(newPos, 0.2f);
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        reset(maze);
        return;
    }
    collisions[1] = maze.intersectSphereFloor(newPos, 0.2f);

    bool isCollide = false;
    for (auto &collision : collisions) {
        if (std::get<0>(collision)) {
            pos += doCollision(collision, pos, newPos) * (float) delta;
            isCollide = true;
        }
    }
    if (!isCollide)
        pos = newPos;
    _ball.setPosition(pos);

    if (_lock) {
        const glm::vec3 dir(0.0f, -12.0f, -6.0f);
        camera->setCameraPosition(pos - dir);
        camera->setCameraFront(dir);
    }
}

glm::vec3 scene::Player::doCollision(normal_collision_t &collision, glm::vec3 &pos, glm::vec3 &newPos) {
    const glm::vec3 &normal = std::get<1>(collision);
    const glm::vec3 speedThreshold = normal * _speed;

    if (glm::length(glm::abs(speedThreshold)) < 0.5f) {
        _speed -= speedThreshold;
        return _speed;
    } else {
        const glm::vec3 offset = normal * 0.2f;
        const glm::vec3 adj_distance = -std::get<2>(collision) - glm::abs(normal) * (pos - newPos) + offset;
        // Hypotenuse = adj / cos(a)
        // Note: cos(acos(a)) == a
        const glm::vec3 hypotenuse = adj_distance / glm::dot(glm::normalize(_speed), glm::normalize(adj_distance));
        const float ratio = glm::length(hypotenuse) / glm::distance(pos, newPos + offset);

        const glm::vec3 oldSpeed = _speed;
        _speed = _speed - 2 * glm::dot(_speed, normal) * normal;
        _speed *= 0.85f; // collision speed cost

        return _speed * (1 - ratio) + oldSpeed * ratio;
    }
}

void scene::Player::goUp() {
    _speed += glm::vec3(0.0f, 0.0f, -_velocity);
    if (_speed.z < -_maxSpeed)
        _speed.z = -_maxSpeed;
}

void scene::Player::goRight() {
    _speed += glm::vec3(_velocity, 0.0f, 0.0f);
    if (_speed.x > _maxSpeed)
        _speed.x = _maxSpeed;

}

void scene::Player::goDown() {
    _speed += glm::vec3(0.0f, 0.0f, _velocity);
    if (_speed.z > _maxSpeed)
        _speed.z = _maxSpeed;
}

void scene::Player::goLeft() {
    _speed += glm::vec3(-_velocity, 0.0f, 0.0f);
    if (_speed.x < -_maxSpeed)
        _speed.x = -_maxSpeed;

}

void scene::Player::doJump() {
    if (_speed.y < 0.1f)
        _speed += glm::vec3(0.0f, 2.5f, 0.0f);
}

void scene::Player::unLockCamera() {
    static double oldTime = 0;

    if (_lastTime - oldTime > 0.5f) {
        oldTime = glfwGetTime();
        _lock = !_lock;
    }
}

const glm::vec3 scene::Player::getPosition() const {
    return _ball.getPosition();
}