#include "creature.hpp"
#include <cstdlib>
#include <ctime>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

const float Creature::GROUND_LEVEL = 0.0f;
const float Creature::MIN_DISTANCE = 5.0f; // Distância mínima entre as criaturas

Creature::Creature(float x, float y, float z, float jump_velocity = 5.0f, float jump_chance = 0.5f, float gravity = -9.81f) : position(x, y, z, 1.0f), vertical_velocity(0.0f), is_jumping(false), rotation_angle(0.0f), target_rotation_angle(0.0f), jump_velocity(jump_velocity), jump_chance(jump_chance), gravity(gravity) {
    srand(static_cast<unsigned int>(time(0)));
}

void Creature::Update(float delta_t) {
    vertical_velocity += gravity * delta_t;
    position.y += vertical_velocity * delta_t;

    if (position.y < GROUND_LEVEL) {
        position.y = GROUND_LEVEL;
        vertical_velocity = 0.0f;
        is_jumping = false;
    }

    if (!is_jumping && (rand() % 100) < this->jump_chance) { // chance de 1% de pular por frame
        Jump();
    }

    if (is_jumping) {
        position += direction * delta_t; // Move para frente na direção da rotação
    }

    // Interpolar suavemente o ângulo de rotação atual em direção ao ângulo de rotação alvo
    float rotation_speed = glm::radians(90.0f) * delta_t; // Velocidade de rotação (ajuste conforme necessário)
    if (glm::angle(glm::vec2(cos(rotation_angle), sin(rotation_angle)), glm::vec2(cos(target_rotation_angle), sin(target_rotation_angle))) > rotation_speed) {
        rotation_angle += rotation_speed * (target_rotation_angle > rotation_angle ? 1.0f : -1.0f);
    } else {
        rotation_angle = target_rotation_angle;
    }
}

void Creature::Jump() {
    if (!is_jumping) {
        is_jumping = true;
        vertical_velocity = this->jump_velocity;
        target_rotation_angle = static_cast<float>(rand()) / RAND_MAX * glm::two_pi<float>(); // Ângulo aleatório entre 0 e 2π
        direction = glm::rotate(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), target_rotation_angle, glm::vec3(0.0f, 1.0f, 0.0f)); // Direção do movimento
    }
}

glm::vec4 Creature::GetPosition() const {
    return position;
}

float Creature::GetRotationAngle() const {
    return rotation_angle;
}