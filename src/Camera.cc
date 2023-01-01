#include "Camera.h"

#include <cmath>
#include <stdexcept>

namespace VE
{
    void Camera::setOrthographicProjection(float left, float right, float top, float bottom, float near, float far)
    {
        m_projectionMat = glm::mat4{1.0F};

        m_projectionMat[0][0] = 2.0F / (right - left);
        m_projectionMat[1][1] = 2.0F / (bottom - top);
        m_projectionMat[2][2] = 1.0F / (far - near);
        m_projectionMat[3][0] = -(right + left) / (right - left);
        m_projectionMat[3][1] = -(bottom + top) / (bottom - top);
        m_projectionMat[3][2] = -near / (far - near);
    }

    void Camera::setPerspectiveProjection(float verticalFieldOfView, float aspect, float near, float far)
    {
        if(glm::abs(aspect - std::numeric_limits<float>::epsilon()) < 0.0F)
        {
            throw std::runtime_error{"setPerspectiveProjection() in Camera.cc"};
        }
        m_projectionMat = glm::mat4{0.0F};
        const float tanHalfFovy{std::tan(verticalFieldOfView / 2.0F)};

        m_projectionMat[0][0] = 1.0F / (aspect * tanHalfFovy);
        m_projectionMat[1][1] = 1.0F / (tanHalfFovy);
        m_projectionMat[2][2] = far / (far - near);
        m_projectionMat[2][3] = 1.0F;
        m_projectionMat[3][2] = -(far * near) / (far - near);
    }

    void Camera::setViewDirection(glm::vec3 camPosition, glm::vec3 camPointingDirection, glm::vec3 upDirection)
    {
        // Construct orthonormal basis (Have unit length and orthogonal to each other)
        const glm::vec3 w{glm::normalize(camPointingDirection)};
        const glm::vec3 u{glm::normalize(glm::cross(w, upDirection))};
        const glm::vec3 v{glm::cross(w, u)};

        m_viewMat = glm::mat4{1.0f};
        m_viewMat[0][0] = u.x;
        m_viewMat[1][0] = u.y;
        m_viewMat[2][0] = u.z;
        m_viewMat[0][1] = v.x;
        m_viewMat[1][1] = v.y;
        m_viewMat[2][1] = v.z;
        m_viewMat[0][2] = w.x;
        m_viewMat[1][2] = w.y;
        m_viewMat[2][2] = w.z;
        m_viewMat[3][0] = -glm::dot(u, camPosition);
        m_viewMat[3][1] = -glm::dot(v, camPosition);
        m_viewMat[3][2] = -glm::dot(w, camPosition);
    }

    void Camera::setViewTarget(glm::vec3 camPosition, glm::vec3 targetPosition, glm::vec3 upDirection)
    {
        if(glm::all(glm::equal(targetPosition - camPosition, glm::vec3{0.0F})))
        {
            throw std::runtime_error{"Direction can't be zero!"};
        }

        setViewDirection(camPosition, targetPosition - camPosition, upDirection);
    }

    void Camera::setViewYXZ(glm::vec3 camPosition, glm::vec3 rotation)
    {
        const float c3{glm::cos(rotation.z)};
        const float s3{glm::sin(rotation.z)};
        const float c2{glm::cos(rotation.x)};
        const float s2{glm::sin(rotation.x)};
        const float c1{glm::cos(rotation.y)};
        const float s1{glm::sin(rotation.y)};

        const glm::vec3 u{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
        const glm::vec3 v{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
        const glm::vec3 w{(c2 * s1), (-s2), (c1 * c2)};

        m_viewMat = glm::mat4{1.0f};
        m_viewMat[0][0] = u.x;
        m_viewMat[1][0] = u.y;
        m_viewMat[2][0] = u.z;
        m_viewMat[0][1] = v.x;
        m_viewMat[1][1] = v.y;
        m_viewMat[2][1] = v.z;
        m_viewMat[0][2] = w.x;
        m_viewMat[1][2] = w.y;
        m_viewMat[2][2] = w.z;
        m_viewMat[3][0] = -glm::dot(u, camPosition);
        m_viewMat[3][1] = -glm::dot(v, camPosition);
        m_viewMat[3][2] = -glm::dot(w, camPosition);
    }
}
