#pragma once

#include "Model.h"

// glm
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

// std
#include <cstdint>
#include <memory>

namespace VE
{
    struct TransformComponent
    {
        glm::vec3 translation{};  // position offset
        glm::vec3 scale{1.0F};
        glm::vec3 rotation{};

        // Make a mat that scale then rotate around z,x,y and then translate
        [[nodiscard]] glm::mat4 mat4(void) const;
    };

    class GameObject final
    {
    private:  // Private variables
        id_t m_id;

    public:  // Public variables
        using id_t = std::uint32_t;

        std::shared_ptr<Model> model;
        glm::vec3 objColor;
        TransformComponent transform;

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

        static GameObject createGameObject(void);
        ~GameObject(void) = default;

        [[nodiscard]] id_t getId(void) const { return m_id; }
    };
}
