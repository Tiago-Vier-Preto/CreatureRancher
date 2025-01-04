#include <cstdlib>
#include <ctime>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "slime_types.hpp"

Anemo_Slime::Anemo_Slime(float x, float y, float z): Creature(x, y, z, 8.0f, 0.2f, -7.81f){
};

int Anemo_Slime::GetType() const {
    return ANEMO;
}

Cryo_Slime::Cryo_Slime(float x, float y, float z) : Creature(x, y, z, 2.0f, 5.0f, -9.81f){
};

int Cryo_Slime::GetType() const {
    return CRYO;
}

Dendro_Slime::Dendro_Slime(float x, float y, float z) : Creature(x, y, z, 5.0f, 0.5f, -9.81f){
};

int Dendro_Slime::GetType() const {
    return DENDRO;
}

Plasma_Slime::Plasma_Slime(float x, float y, float z) : Creature(x, y, z, 4.0f, 1.0f, -9.81f){
};

int Plasma_Slime::GetType() const {
    return PLASMA;
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

Electro_Slime::Electro_Slime(float x, float y, float z) : Creature(x, y, z, 3.0f, 1.5f, -9.81f){
};

int Electro_Slime::GetType() const {
    return ELECTRO;
}
Water_Slime::Water_Slime(float x, float y, float z) : Creature(x, y, z, 4.5f, 0.75f, -9.81f){
};

int Water_Slime::GetType() const {
    return WATER;
}

std::vector<Creature*> InitialCreatureSpawn(int count, float map_width, float map_length) 
{
    std::vector<Creature*> creatures;
    Creature* new_creature;
    Slime_Type type;
    bool valid_position;
    int tile;
    float x, z;
    float distance;
    constexpr int TILE_COUNT = 9;

    for (int i = 0; i < count; i++) 
    {
        type = Slime_Type(rand() % (TILE_COUNT - 1));
        valid_position = false;
        switch (type) 
        {
            case ANEMO:
                tile = 0;
                break;
            case CRYO:
                tile = 1;
                break;
            case DENDRO:
                tile = 2;
                break;
            case PLASMA:
                tile = 3;
                break;
            case FIRE:
                tile = 5;
                break;
            case GEO:
                tile = 6;
                break;
            case ELECTRO:
                tile = 7;
                break;
            case WATER:
                tile = 8;
                break;
        }
        while (!valid_position) 
        {
            x = -280 + (200 * (tile % 3)) + (static_cast<float>(rand()) / RAND_MAX) * 180;
            z = -280 + (200 * (tile / 3)) + (static_cast<float>(rand()) / RAND_MAX) * 180;
            for (const auto& creature : creatures) 
            {
                distance = glm::distance(glm::vec2(x, z), glm::vec2(creature->GetPosition().x, creature->GetPosition().z));
                if (distance < Creature::MIN_DISTANCE) 
                {
                    valid_position = false;
                    break;
                }
            } 
           valid_position = true;
        }

        new_creature = nullptr;
        switch (type) 
        {
            case ANEMO:
                new_creature = new Anemo_Slime(x, Creature::GROUND_LEVEL, z);
                break;
            case CRYO:
                new_creature = new Cryo_Slime(x, Creature::GROUND_LEVEL, z);
                break;
            case DENDRO:
                new_creature = new Dendro_Slime(x, Creature::GROUND_LEVEL, z);
                break;
            case PLASMA:
                new_creature = new Plasma_Slime(x, Creature::GROUND_LEVEL, z);
                break;
            case FIRE:
                new_creature = new Fire_Slime(x, Creature::GROUND_LEVEL, z);
                break;
            case GEO:
                new_creature = new Geo_Slime(x, Creature::GROUND_LEVEL, z);
                break;
            case ELECTRO:
                new_creature = new Electro_Slime(x, Creature::GROUND_LEVEL, z);
                break;
            case WATER:
                new_creature = new Water_Slime(x, Creature::GROUND_LEVEL, z);
                break;
        }
        if (new_creature) 
        {
            creatures.push_back(new_creature);
        }
    }

    return creatures;
}

Creature* SpawnCreature(float map_width, float map_length, std::vector<Creature*> creatures) 
{
    Creature* creature;
    Slime_Type type;
    bool valid_position;
    int tile;
    float x, z;
    float distance;
    constexpr int TILE_COUNT = 9;

    type = Slime_Type(rand() % (TILE_COUNT - 1));
    valid_position = false;
    switch (type) 
    {
        case ANEMO:
            tile = 0;
            break;
        case CRYO:
            tile = 1;
            break;
        case DENDRO:
            tile = 2;
            break;
        case PLASMA:
            tile = 3;
            break;
        case FIRE:
            tile = 5;
            break;
        case GEO:
            tile = 6;
            break;
        case ELECTRO:
            tile = 7;
            break;
        case WATER:
            tile = 8;
            break;
    }
    while (!valid_position) 
    {
        x = -280 + (200 * (tile % 3)) + (static_cast<float>(rand()) / RAND_MAX) * 180;
        z = -280 + (200 * (tile / 3)) + (static_cast<float>(rand()) / RAND_MAX) * 180;
        for (const auto& creature : creatures) 
        {
            distance = glm::distance(glm::vec2(x, z), glm::vec2(creature->GetPosition().x, creature->GetPosition().z));
            if (distance < Creature::MIN_DISTANCE) 
            {
                valid_position = false;
                break;
            }
        } 
        valid_position = true;
    }

    creature = nullptr;
    switch (type) 
    {
        case ANEMO:
            creature = new Anemo_Slime(x, Creature::GROUND_LEVEL, z);
            break;
        case CRYO:
            creature = new Cryo_Slime(x, Creature::GROUND_LEVEL, z);
            break;
        case DENDRO:
            creature = new Dendro_Slime(x, Creature::GROUND_LEVEL, z);
            break;
        case PLASMA:
            creature = new Plasma_Slime(x, Creature::GROUND_LEVEL, z);
            break;
        case FIRE:
            creature = new Fire_Slime(x, Creature::GROUND_LEVEL, z);
            break;
        case GEO:
            creature = new Geo_Slime(x, Creature::GROUND_LEVEL, z);
            break;
        case ELECTRO:
            creature = new Electro_Slime(x, Creature::GROUND_LEVEL, z);
            break;
        case WATER:
            creature = new Water_Slime(x, Creature::GROUND_LEVEL, z);
            break;
    }
    return creature;
}