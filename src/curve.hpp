#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

bool inWeaponRange(glm::vec4 weapon_position, glm::vec4 weapon_direction, glm::vec4 slime_position, float range, float minAngle);

glm::vec3 cubicBezierCurve(const glm::vec3& P0, const glm::vec3& P1, const glm::vec3& P2, const glm::vec3& P3, float t);

float randomOffset(int seed, float t);

glm::vec3 bezierSpiralPosition(const glm::vec3& start, const glm::vec3& end, float t, int numSegments, float GROUND_LEVEL);