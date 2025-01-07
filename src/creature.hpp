#ifndef CREATURE_HPP
#define CREATURE_HPP

#include <glm/vec4.hpp>
#include <vector>
#include <string>
#define SLIME_SPAWN_TIME 10.0f
#define SLIME_LIMIT 1000
#define STARTING_SLIMES 100
class Creature {
public:
    Creature(float x, float y, float z, float jump_velocity, float jump_chance, float gravity);
    virtual ~Creature() = default;
    bool Update(float delta_t); //True if the slime started jumping
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
    float capture_time;
    glm::vec4 direction; // Direção do movimento

    void setPosition(glm::vec4 position);
    
    bool captured = false;

    static const float GROUND_LEVEL;
    static const float MIN_DISTANCE; // Distância mínima entre as criaturas
};

#endif // CREATURE_HPP