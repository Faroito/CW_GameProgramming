//
// Created by Timothée Couble on 28/02/2020.
//

#include "Particles.hpp"

scene::Particles::Particles() : _instances(MAX_PARTICLES) {}

// Draw and update particles
void scene::Particles::draw(const gl_wrapper::Shader_ptr_t &shader, const MazeDisplay &maze) {
    double currentTime = glfwGetTime();
    double delta = currentTime - _lastTime;
    _lastTime = currentTime;

    if (_activated) {
        this->refreshParticles(delta);
        this->updateModel(maze, delta);

        _instances.draw(shader);
    }
}

// Reset particles
void scene::Particles::reset(const MazeDisplay &maze) {
    for (auto &particle : _particlesContainer)
        particle.life = -1.0f;
    _position = glm::vec3(maze.getEnd().x, 0.01f, maze.getEnd().y);
}

// Create new particles if possible
void scene::Particles::refreshParticles(double &delta) {
    int nb_particles = (int) (delta * 800);
    if (nb_particles > (int) (0.016f * 800))
        nb_particles = (int) (0.016f * 800);

    for (int i = 0; i < nb_particles; i++) {
        int index = this->findUnusedParticle();
        _particlesContainer[index].life = 5.0f;
        _particlesContainer[index].pos = _position;

        float spread = 1.5f;
        glm::vec3 maindir = glm::vec3(0.0f, 6.0f, 0.0f);
        double R = 0.8f;

        double phi = (rand() % (int) (2 * M_PI * 1000)) / 1000.0f;
        double costheta = (rand() % 2000 - 1000) / 1000.0f;
        double u = (rand() % 1000) / 1000.0f;

        double theta = std::acos(costheta);
        double r = R * std::cbrt(u);

        glm::vec3 randDir;
        randDir.x = r * std::sin(theta) * std::cos(phi);
        randDir.y = r * std::sin(theta) * std::sin(phi);
        randDir.z = r * std::cos(theta);

        _particlesContainer[index].speed = maindir + randDir * spread;

        _particlesContainer[index].rgb.r = (rand() % 160 + 50) / 255.0f;
        _particlesContainer[index].rgb.g = (rand() % 160 + 50) / 255.0f;
        _particlesContainer[index].rgb.b = (rand() % 160 + 50) / 255.0f;

        _particlesContainer[index].size = (rand() % 1000) / 2000.0f + 0.1f;
    }
}

// Update existing particles
void scene::Particles::updateModel(const MazeDisplay &maze, double &delta) {
    unsigned int actual_amount = 0;
    for (auto &p : _particlesContainer) {
        if (p.life > 0.0f) {
            p.life -= (float) delta;
            if (p.life > 0.0f){
                p.speed += glm::vec3(0.0f, -9.81f, 0.0f) * (float) delta * 0.5f;
                if (!maze.intersectSquare(p.pos + p.speed * (float) delta, 0.1f * p.size))
                    p.pos += p.speed * (float) delta;

                glm::mat4 translate = glm::translate(glm::mat4(1.0f), p.pos);
                glm::mat4 rotate = glm::mat4(1.0f);
                glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f * p.size));

                _instances[actual_amount].color = p.rgb;
                _instances[actual_amount].matrix = translate * rotate * scale;
            }
            actual_amount++;
        }
    }
    _instances.updateModel(actual_amount);
}

// Find unused particles
int scene::Particles::findUnusedParticle() {
    for (int i = _lastUsedParticle; i < MAX_PARTICLES; i++){
        if (_particlesContainer[i].life < 0){
            _lastUsedParticle = i;
            return i;
        }
    }

    for (int i = 0; i < _lastUsedParticle; i++){
        if (_particlesContainer[i].life < 0){
            _lastUsedParticle = i;
            return i;
        }
    }
    return 0;
}

void scene::Particles::toggleActivation() {
    _activated = !_activated;
}