#ifndef CREATURE_HPP
#define CREATURE_HPP

#include <glm/vec4.hpp>
#include <vector>

class Creature {
public:
    Creature(float x, float y, float z, float jump_velocity, float jump_chance, float gravity);
    virtual ~Creature() = default;
    void Update(float delta_t);
    void Jump();
    glm::vec4 GetPosition() const;
    float GetRotationAngle() const;
    virtual int GetType() const = 0;

    glm::vec4 position;
    float vertical_velocity;
    bool is_jumping;
    float rotation_angle; // Ângulo de rotação atual em radianos
    float target_rotation_angle; // Ângulo de rotação alvo em radianos
    float jump_velocity;
    float jump_chance;
    float gravity;
    glm::vec4 direction; // Direção do movimento

    static const float GROUND_LEVEL;
    static const float MIN_DISTANCE; // Distância mínima entre as criaturas
};

#endif // CREATURE_HPP