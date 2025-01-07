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

bool CylinderSphereCollision(glm::vec3 cylinderPosition, float cylinderRadius, float cylinderHeight, glm::vec3 spherePosition, float sphereRadius) {
    // Calculate the closest point on the cylinder's axis to the sphere's center
    float halfHeight = cylinderHeight * 0.5f;
    glm::vec3 cylinderTop = cylinderPosition + glm::vec3(0, halfHeight, 0);
    glm::vec3 cylinderBottom = cylinderPosition - glm::vec3(0, halfHeight, 0);

    // Project the sphere's center onto the cylinder's axis
    glm::vec3 axis = glm::normalize(cylinderTop - cylinderBottom);
    float projection = glm::dot(spherePosition - cylinderBottom, axis);
    projection = glm::clamp(projection, 0.0f, cylinderHeight);

    // Find the closest point on the cylinder's axis to the sphere
    glm::vec3 closestPoint = cylinderBottom + axis * projection;

    // Check if the distance from the closest point to the sphere's center is less than the sum of the radii
    float distance = glm::length(closestPoint - spherePosition);
    return distance < (cylinderRadius + sphereRadius);
}


