#ifndef CREATURE_H
#define CREATURE_H

#include <glm/vec4.hpp>
#include <vector>

class Creature {
public:
    Creature(float x, float y, float z);
    void Update(float delta_t);
    void Jump();
    glm::vec4 GetPosition() const;

    glm::vec4 position;
    float vertical_velocity;
    bool is_jumping;
    static const float GRAVITY;
    static const float JUMP_VELOCITY;
    static const float GROUND_LEVEL;
};

std::vector<Creature> SpawnCreatures(int count, float map_width, float map_height);

#endif // CREATURE_H