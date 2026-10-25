#include "GfxObject.h"
#include <cmath>

bool GfxTransform::TrySetModelMatrix(const glm::mat4& matrix)
{
    for (int column = 0; column < 4; ++column)
        for (int row = 0; row < 4; ++row)
            if (!std::isfinite(matrix[column][row]))
                return false;
    if (std::abs(matrix[0][3]) > 1e-5f || std::abs(matrix[1][3]) > 1e-5f
        || std::abs(matrix[2][3]) > 1e-5f || std::abs(matrix[3][3] - 1.0f) > 1e-5f)
        return false;

    glm::vec3 newScale;
    glm::mat3 basis;
    for (int axis = 0; axis < 3; ++axis)
    {
        const glm::vec3 column(matrix[axis]);
        const double length = std::sqrt(glm::dot(glm::dvec3(column), glm::dvec3(column)));
        if (!std::isfinite(length) || length < 1e-5 || length > 1e30)
            return false;
        newScale[axis] = static_cast<float>(length) * (scale[axis] < 0.0f ? -1.0f : 1.0f);
        basis[axis] = column / newScale[axis];
    }
    // TRS cannot represent shear. World scaling of rotated objects would introduce it.
    if (std::abs(glm::dot(basis[0], basis[1])) > 1e-4f
        || std::abs(glm::dot(basis[0], basis[2])) > 1e-4f
        || std::abs(glm::dot(basis[1], basis[2])) > 1e-4f)
        return false;
    if (glm::determinant(basis) < 0.0f)
    {
        newScale.x = -newScale.x;
        basis[0] = -basis[0];
    }
    const glm::quat newRotation = glm::normalize(glm::quat_cast(basis));
    position = glm::vec3(matrix[3]);
    rotation = newRotation;
    scale = newScale;
    return true;
}

GfxObject::GfxObject(VkPipeline graphicsPipeline, VkPipelineLayout graphicsPipelineLayout, const char* Name)
    : graphicsPipeline(graphicsPipeline), graphicsPipelineLayout(graphicsPipelineLayout), name(Name)
{
}
