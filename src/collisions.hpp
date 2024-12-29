#ifndef __COLLISIONS_H__
#define __COLLISIONS_H__

#include <glm/gtc/type_ptr.hpp>
#include <string>

struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};

AABB ComputeAABB(glm::vec3 position, glm::vec3 size);
bool CheckAABBOverlap(const AABB& a, const AABB& b);
bool CheckSphereSphereOverlap(glm::vec3 posA, float radiusA, glm::vec3 posB, float radiusB);

#endif