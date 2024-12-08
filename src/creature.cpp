#include "creature.hpp"
#include <cstdlib>
#include <ctime>

const float Creature::GRAVITY = -9.81f;
const float Creature::JUMP_VELOCITY = 5.0f;
const float Creature::GROUND_LEVEL = 0.0f;

Creature::Creature(float x, float y, float z) : position(x, y, z, 1.0f), vertical_velocity(0.0f), is_jumping(false) {}

void Creature::Update(float delta_t) {
    vertical_velocity += GRAVITY * delta_t;
    position.y += vertical_velocity * delta_t;

    if (position.y < GROUND_LEVEL) {
        position.y = GROUND_LEVEL;
        vertical_velocity = 0.0f;
        is_jumping = false;
    }

    if (!is_jumping && (rand() % 100) < 1) { // chance de 1% de pular por frame
        Jump();
    }
}

void Creature::Jump() {
    if (!is_jumping) {
        is_jumping = true;
        vertical_velocity = JUMP_VELOCITY;
    }
}

glm::vec4 Creature::GetPosition() const {
    return position;
}

std::vector<Creature> SpawnCreatures(int count, float map_width, float map_height) {
    std::vector<Creature> Creatures;
    srand(static_cast<unsigned int>(time(0)));

    for (int i = 0; i < count; ++i) {
        float x = static_cast<float>(rand()) / RAND_MAX * map_width - map_width / 2;
        float z = static_cast<float>(rand()) / RAND_MAX * map_height - map_height / 2;
        Creatures.emplace_back(x, Creature::GROUND_LEVEL, z);
    }

    return Creatures;
}