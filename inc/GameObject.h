#pragma once

#include "Model.h"

// std
#include <cstdint>
#include <memory>

namespace VE
{
    struct Transform2dComponent
    {
        glm::vec2 translation{};  // position offset
        glm::vec2 scale{1.0F, 1.0F};
        float rotation{};

        glm::mat2 mat2(void)
        {
            const float sin{glm::sin(rotation)};
            const float cos{glm::cos(rotation)};

            glm::mat2 rotationMat{{cos, sin}, {-sin, cos}};

            glm::mat2 scaleMat{{scale.x, 0.0F}, {0.0F, scale.y}};

            return rotationMat * scaleMat;
        }
    };

    class GameObject final
    {
    private:  // Private variables
        id_t m_id;

    public:  // Public variables
        using id_t = std::uint32_t;

        std::shared_ptr<Model> model;
        glm::vec3 objColor;
        Transform2dComponent transform2d;

    private:  // Private methods
        // Constructor
        GameObject(id_t objId) : m_id{objId}, objColor{} {}

    public:  // Public methods
        /*------------------------------------------------------------------*/
        /*                       Don't copy my class!                       */

        GameObject(const GameObject& copy) = delete;
        GameObject& operator=(const GameObject& copy) = delete;
        /*------------------------------------------------------------------*/

        GameObject(GameObject&& move) = default;
        GameObject& operator=(GameObject&& move) = default;

        static GameObject createGameObject(void)
        {
            static id_t currentId{};
            return {++currentId};
        }

        [[nodiscard]] id_t getId(void) const { return m_id; }
    };
}
