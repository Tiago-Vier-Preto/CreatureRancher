#include "collisions.hpp"

AABB ComputeAABB(glm::vec3 position, glm::vec3 size) {
    glm::vec3 halfSize = size * 0.5f;
    return { position - halfSize, position + halfSize };
}

bool CheckAABBOverlap(const AABB& a, const AABB& b) {
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
           (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
           (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

bool CheckSphereSphereOverlap(glm::vec3 posA, float radiusA, glm::vec3 posB, float radiusB) {
    float distance = glm::length(posA - posB);
    return distance < (radiusA + radiusB);
}

bool SpherePlaneCollision(const glm::vec3 spherePosition, const float radius, const glm::vec3 normalPlane, const float planeDistance) {
    glm::vec3 spherePosition3D = glm::vec3(spherePosition);
    float distToPlane = glm::dot(normalPlane, spherePosition) - planeDistance;
    return std::abs(distToPlane) <= radius;
}

