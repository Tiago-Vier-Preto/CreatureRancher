#include <cstdlib>
#include <ctime>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <unordered_map>
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

    constexpr int TILE_COUNT = 9;
    const float tile_width = map_width / 3;
    const float tile_height = map_height / 3;

    auto getTileIndex = [&](float x, float z) {
        int row = static_cast<int>((z + map_height / 2) / tile_height);
        int col = static_cast<int>((x + map_width / 2) / tile_width);
        return row * 3 + col;
    };

    std::unordered_map<int, Slime_Type> tile_to_slime_type = {
        {0, ANEMO}, {1, CRYO}, {2, DENDRO},
        {3, ELECTRO}, {4, static_cast<Slime_Type>(-1)}, {5, FIRE},
        {6, GEO}, {7, MUTATED_ELECTRO}, {8, WATER}
    };
    
    for (int i = 0; i < count; ++i) {
        Slime_Type type;
        bool valid_position = false;
        float x, z;

        while (!valid_position) {
            int tile_index;
            do {
                tile_index = rand() % TILE_COUNT;
            } while (tile_index == 4 || tile_to_slime_type[tile_index] == static_cast<Slime_Type>(-1));
            type = tile_to_slime_type[tile_index];
            float tile_x_min = (tile_index % 3) * tile_width - map_width / 2;
            float tile_z_min = (tile_index / 3) * tile_height - map_height / 2;
            
            x = tile_x_min + static_cast<float>(rand()) / RAND_MAX * tile_width;
            z = tile_z_min + static_cast<float>(rand()) / RAND_MAX * tile_height;
            valid_position = true;

            for (const auto& creature : creatures) {
                float distance = glm::distance(glm::vec2(x, z), glm::vec2(creature->GetPosition().x, creature->GetPosition().z));
                if (distance < Creature::MIN_DISTANCE) {
                    valid_position = false;
                    break;
                }
            }
        }
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

