#include "GfxDecal.h"

glm::mat4 GfxDecal::CalculateDecalProjectionMatrix(const glm::vec3& decalPosition, const glm::vec3& decalDirection, const glm::vec3& decalUp, float decalWidth, float decalHeight, float decalDepth)
{
    // 1. Model Matrix (Position and Scale)
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, decalPosition);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(decalWidth, decalHeight, decalDepth));

    // 2. View Matrix (From decal's perspective)
    glm::vec3 decalRight = glm::normalize(glm::cross(decalUp, decalDirection)); // Right vector
    glm::vec3 decalAdjustedUp = glm::cross(decalDirection, decalRight);          // Recalculated up vector

    glm::mat4 viewMatrix = glm::lookAt(
        decalPosition,
        decalPosition + decalDirection,
        decalAdjustedUp
    );

    // 3. Projection Matrix (Orthographic, since decals often use orthographic projection)
    float nearPlane = 0.0f;    // Start of the decal depth range
    float farPlane = decalDepth; // End of the decal depth range

    glm::mat4 projectionMatrix = glm::ortho(
        -decalWidth / 2.0f, decalWidth / 2.0f,
        -decalHeight / 2.0f, decalHeight / 2.0f,
        nearPlane, farPlane
    );

    // Combine matrices
    glm::mat4 decalProjectionMatrix = projectionMatrix * viewMatrix * modelMatrix;

    return decalProjectionMatrix;
}