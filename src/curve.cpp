#include "curve.hpp"
#include <iostream>
#include <vector>
#include <glm/gtc/noise.hpp> 

bool inWeaponRange(glm::vec4 weapon_position, glm::vec4 weapon_direction, glm::vec4 slime_position, float range, float minAngle) 
{
    glm::vec4 direction = slime_position - weapon_position;
    float distance = glm::length(direction);
    if (distance > range) {
        return false;
    }
    float angle = glm::angle(glm::normalize(direction), weapon_direction);
    return angle < glm::radians(minAngle);
}

glm::vec3 cubicBezierCurve(const glm::vec3& P0, const glm::vec3& P1, const glm::vec3& P2, const glm::vec3& P3, float t) {
    float u = 1.0f - t;
    return u * u * u * P0 + 3.0f * u * u * t * P1 + 3.0f * u * t * t * P2 + t * t * t * P3;
}

float randomOffset(int seed, float t) {
    return glm::perlin(glm::vec2(seed, t * 10.0f)) * 0.1f; // Ajuste o fator multiplicador (0.1f) para controlar o caos
}

glm::vec3 bezierSpiralPosition(const glm::vec3& start, const glm::vec3& end, float t, int numSegments, float GROUND_LEVEL) {
    if (numSegments < 3) numSegments = 3; // Garantir pelo menos 3 segmentos

    std::vector<glm::vec3> controlPoints;
    glm::vec3 direction = glm::normalize(end - start);
    glm::vec3 right = glm::normalize(glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 up = glm::normalize(glm::cross(right, direction));

    float totalLength = glm::length(end - start);
    float angleStep = glm::two_pi<float>() / numSegments; // Ângulo por segmento
    float totalAngle = glm::two_pi<float>() * 4.0f;       // Total de voltas (ajuste conforme desejado)
    float heightStep = totalLength / (numSegments - 1);
    float baseRadius = totalLength * 0.2f; // Ajuste do raio inicial

    for (int i = 0; i < numSegments; ++i) {
        float angle = i * totalAngle / (numSegments - 1); // Ângulo acumulativo
        float height = i * heightStep;

        float radius = baseRadius * (1.0f - static_cast<float>(i) / (numSegments - 1)); // Redução do raio
        glm::vec3 offset = radius * (cos(angle) * right + sin(angle) * up);

        // Adiciona deslocamento caótico (opcional)
        float oscillationStrength = radius * 0.3f;
        glm::vec3 chaoticOffset = oscillationStrength * sin(i * glm::pi<float>() * 0.5f) * up;
        offset += chaoticOffset;

        glm::vec3 point = start + height * direction + offset;

        // Garante que o ponto esteja acima do nível do chão
        if (point.y < GROUND_LEVEL) {
            point.y = GROUND_LEVEL;
        }

        controlPoints.push_back(point);
    }

    int segment = static_cast<int>(t * (numSegments - 1));
    float segmentT = (t * (numSegments - 1)) - segment;

    if (segment >= numSegments - 1) {
        return end;
    }

    glm::vec3 P0 = controlPoints[segment];
    glm::vec3 P1 = P0 + (controlPoints[segment + 1] - P0) / 3.0f;
    glm::vec3 P2 = controlPoints[segment + 1] - (controlPoints[segment + 1] - P0) / 3.0f;
    glm::vec3 P3 = controlPoints[segment + 1];

    glm::vec3 result = cubicBezierCurve(P0, P1, P2, P3, segmentT);

    // Garante que a posição final também esteja acima do chão
    if (result.y < GROUND_LEVEL) {
        result.y = GROUND_LEVEL;
    }

    return result;
}