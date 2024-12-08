#ifndef CREATURE_HPP
#define CREATURE_HPP

#include <glm/vec4.hpp>
#include <vector>

class Creature {
public:
    Creature(float x, float y, float z);
    void Update(float delta_t);
    void Jump();
    glm::vec4 GetPosition() const;
    float GetRotationAngle() const;

    glm::vec4 position;
    float vertical_velocity;
    bool is_jumping;
    float rotation_angle; // Ângulo de rotação atual em radianos
    float target_rotation_angle; // Ângulo de rotação alvo em radianos
    glm::vec4 direction; // Direção do movimento

    static const float GRAVITY;
    static const float JUMP_VELOCITY;
    static const float GROUND_LEVEL;
    static const float MIN_DISTANCE; // Distância mínima entre as criaturas
};

std::vector<Creature> SpawnCreatures(int count, float map_width, float map_height);

#endif // CREATURE_HPP