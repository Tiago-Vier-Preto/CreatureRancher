#include "creature.hpp"

#ifndef SLIME_TYPES_HPP
#define SLIME_TYPES_HPP
enum Slime_Type {ANEMO, CRYO, DENDRO, ELECTRO, FIRE, GEO, MUTATED_ELECTRO, WATER};

class Anemo_Slime: public Creature {
    public:
        Anemo_Slime(float x, float y, float z);
        int GetType() const override;
};

class Cryo_Slime: public Creature {
    public:
        Cryo_Slime(float x, float y, float z);
        int GetType() const override;
};

class Dendro_Slime: public Creature {
    public:
        Dendro_Slime(float x, float y, float z);
        int GetType() const override;
};

class Electro_Slime: public Creature {
    public:
        Electro_Slime(float x, float y, float z);
        int GetType() const override;
};

class Fire_Slime: public Creature {
    public:
        Fire_Slime(float x, float y, float z);
        int GetType() const override;
};

class Geo_Slime: public Creature {
    public:
        Geo_Slime(float x, float y, float z);
        int GetType() const override;
};

class Mutated_Electro_Slime: public Creature {
    public:
        Mutated_Electro_Slime(float x, float y, float z);
        int GetType() const override;
};

class Water_Slime: public Creature {
    public:
        Water_Slime(float x, float y, float z);
        int GetType() const override;
};

std::vector<Creature*> SpawnCreatures(int count, float map_width, float map_height);

#endif
