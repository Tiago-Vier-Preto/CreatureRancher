#include "creature.hpp"

#ifndef SLIME_TYPES_HPP
#define SLIME_TYPES_HPP
enum Slime_Type {ANEMO, CRYO, DENDRO, PLASMA, FIRE, GEO, ELECTRO, WATER};
inline std::string to_string(Slime_Type t)
{
    switch (t)
    {
        case ANEMO:   return "Anemo";
        case CRYO:    return "Cryo";
        case DENDRO:  return "Dendro";
        case PLASMA:  return "Plasma";
        case FIRE:    return "Fire";
        case GEO:     return "Geo";
        case ELECTRO: return "Electro";
        case WATER:   return "Water";
        default:      return "Invalid Slime Type";
    }
}

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

class Plasma_Slime: public Creature {
    public:
        Plasma_Slime(float x, float y, float z);
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

class Electro_Slime: public Creature {
    public:
        Electro_Slime(float x, float y, float z);
        int GetType() const override;
};

class Water_Slime: public Creature {
    public:
        Water_Slime(float x, float y, float z);
        int GetType() const override;
};

std::vector<Creature*> InitialCreatureSpawn(int count, float map_width, float map_length);
Creature* SpawnCreature(float map_width, float map_length, std::vector<Creature*> creatures);

#endif
