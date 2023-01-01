#include "GameObject.h"

namespace VE
{
    GameObject GameObject::createGameObject(void)
    {
        static id_t currentId{};
        return {++currentId};
    }

    [[nodiscard]] glm::mat4 TransformComponent::mat4(void) const
    {
        glm::mat4 transform{glm::translate(glm::mat4{1.0F}, translation)};

        // rotate y
        transform = glm::rotate(transform, rotation.y, glm::vec3{0.0F, 1.0F, 0.0F});

        // rotate x
        transform = glm::rotate(transform, rotation.x, glm::vec3{1.0F, 0.0F, 0.0F});

        // rotate z
        transform = glm::rotate(transform, rotation.z, glm::vec3{0.0F, 0.0F, 1.0F});

        // scale
        transform = glm::scale(transform, scale);

        return transform;
    }
}
