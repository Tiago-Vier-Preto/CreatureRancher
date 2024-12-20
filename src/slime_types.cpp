#include <cstdlib>
#include <ctime>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "slime_types.hpp"

Anemo_Slime::Anemo_Slime(float x, float y, float z): Creature(x, y, z, 2.0f, 5.0f, -9.81f){
};

int Anemo_Slime::GetType() const {
    return ANEMO;
}

Cryo_Slime::Cryo_Slime(float x, float y, float z) : Creature(x, y, z, 8.0f, 0.2f, -7.81f){
};

int Cryo_Slime::GetType() const {
    return CRYO;
}

Dendro_Slime::Dendro_Slime(float x, float y, float z) : Creature(x, y, z, 5.0f, 0.5f, -9.81f){
};

int Dendro_Slime::GetType() const {
    return DENDRO;
}

Electro_Slime::Electro_Slime(float x, float y, float z) : Creature(x, y, z, 4.0f, 1.0f, -9.81f){
};

int Electro_Slime::GetType() const {
    return ELECTRO;
}

Fire_Slime::Fire_Slime(float x, float y, float z) : Creature(x, y, z, 6.0f, 0.75f, -8.81f){
};

int Fire_Slime::GetType() const {
    return FIRE;
}

Geo_Slime::Geo_Slime(float x, float y, float z) : Creature(x, y, z, 4.0f, 0.3f, -14.81f){
};

int Geo_Slime::GetType() const {
    return GEO;
}

Mutated_Electro_Slime::Mutated_Electro_Slime(float x, float y, float z) : Creature(x, y, z, 3.0f, 1.5f, -9.81f){
};

int Mutated_Electro_Slime::GetType() const {
    return MUTATED_ELECTRO;
}
Water_Slime::Water_Slime(float x, float y, float z) : Creature(x, y, z, 4.5f, 0.75f, -9.81f){
};

int Water_Slime::GetType() const {
    return WATER;
}

std::vector<Creature*> SpawnCreatures(int count, float map_width, float map_height) {
    std::vector<Creature*> creatures;
    srand(static_cast<unsigned int>(time(0)));

    for (int i = 0; i < count; ++i) {
        bool valid_position = false;
        float x, z;

        while (!valid_position) {
            x = static_cast<float>(rand()) / RAND_MAX * map_width - map_width / 2;
            z = static_cast<float>(rand()) / RAND_MAX * map_height - map_height / 2;
            valid_position = true;

            for (const auto& creature : creatures) {
                float distance = glm::distance(glm::vec2(x, z), glm::vec2(creature->GetPosition().x, creature->GetPosition().z));
                if (distance < Creature::MIN_DISTANCE) {
                    valid_position = false;
                    break;
                }
            }
        }

        Slime_Type type = static_cast<Slime_Type>(rand() % (WATER + 1));

        Creature* new_creature = nullptr;
        
        switch (type) {
            case ANEMO:
                new_creature = new Anemo_Slime(x, Creature::GROUND_LEVEL, z);
                break;
            case CRYO:
                new_creature = new Cryo_Slime(x, Creature::GROUND_LEVEL, z);
                break;
            case DENDRO:
                new_creature = new Dendro_Slime(x, Creature::GROUND_LEVEL, z);
                break;
            case ELECTRO:
                new_creature = new Electro_Slime(x, Creature::GROUND_LEVEL, z);
                break;
            case FIRE:
                new_creature = new Fire_Slime(x, Creature::GROUND_LEVEL, z);
                break;
            case GEO:
                new_creature = new Geo_Slime(x, Creature::GROUND_LEVEL, z);
                break;
            case MUTATED_ELECTRO:
                new_creature = new Mutated_Electro_Slime(x, Creature::GROUND_LEVEL, z);
                break;
            case WATER:
                new_creature = new Water_Slime(x, Creature::GROUND_LEVEL, z);
                break;
        }
        if (new_creature) {
            creatures.push_back(new_creature);
        }
    }

    return creatures;
}