#include "Application.h"
#include "FrameTime.h"
#include "SimpleRenderSystem.h"
#include "Camera.h"
#include "KeyboardMovementController.h"

// glm
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cstdio>
#include <iostream>
#include <stdexcept>

namespace VE
{
    // Constructor
    Application::Application(void)
        : m_window{WIDTH, HEIGHT, "VulkanEngine"}, m_device{m_window}, m_renderer{m_window, m_device}
    {
        loadGameObjects();
    }

    // Destructor
    Application::~Application(void) = default;

    // temporary helper function, creates a 1x1x1 cube centered at offset
    static std::unique_ptr<Model> createCubeModel(Device& device, glm::vec3 offset)
    {
        std::vector<Model::Vertex> vertices{

              // left face (white)
              {{-.5F, -.5F, -.5F}, {.9F, .9F, .9F}},
              {{-.5F, .5F, .5F}, {.9F, .9F, .9F}},
              {{-.5F, -.5F, .5F}, {.9F, .9F, .9F}},
              {{-.5F, -.5F, -.5F}, {.9F, .9F, .9F}},
              {{-.5F, .5F, -.5F}, {.9F, .9F, .9F}},
              {{-.5F, .5F, .5F}, {.9F, .9F, .9F}},

              // right Face (yellow)
              {{.5F, -.5F, -.5F}, {.8F, .8F, .1F}},
              {{.5F, .5F, .5F}, {.8F, .8F, .1F}},
              {{.5F, -.5F, .5F}, {.8F, .8F, .1F}},
              {{.5F, -.5F, -.5F}, {.8F, .8F, .1F}},
              {{.5F, .5F, -.5F}, {.8F, .8F, .1F}},
              {{.5F, .5F, .5F}, {.8F, .8F, .1F}},

              // top Face (orange, remember y axis points down)
              {{-.5F, -.5F, -.5F}, {.9F, .6F, .1F}},
              {{.5F, -.5F, .5F}, {.9F, .6F, .1F}},
              {{-.5F, -.5F, .5F}, {.9F, .6F, .1F}},
              {{-.5F, -.5F, -.5F}, {.9F, .6F, .1F}},
              {{.5F, -.5F, -.5F}, {.9F, .6F, .1F}},
              {{.5F, -.5F, .5F}, {.9F, .6F, .1F}},

              // bottom Face (red)
              {{-.5F, .5F, -.5F}, {.8F, .1F, .1F}},
              {{.5F, .5F, .5F}, {.8F, .1F, .1F}},
              {{-.5F, .5F, .5F}, {.8F, .1F, .1F}},
              {{-.5F, .5F, -.5F}, {.8F, .1F, .1F}},
              {{.5F, .5F, -.5F}, {.8F, .1F, .1F}},
              {{.5F, .5F, .5F}, {.8F, .1F, .1F}},

              // nose Face (blue)
              {{-.5F, -.5F, 0.5F}, {.1F, .1F, .8F}},
              {{.5F, .5F, 0.5F}, {.1F, .1F, .8F}},
              {{-.5F, .5F, 0.5F}, {.1F, .1F, .8F}},
              {{-.5F, -.5F, 0.5F}, {.1F, .1F, .8F}},
              {{.5F, -.5F, 0.5F}, {.1F, .1F, .8F}},
              {{.5F, .5F, 0.5F}, {.1F, .1F, .8F}},

              // tail Face (green)
              {{-.5F, -.5F, -0.5F}, {.1F, .8F, .1F}},
              {{.5F, .5F, -0.5F}, {.1F, .8F, .1F}},
              {{-.5F, .5F, -0.5F}, {.1F, .8F, .1F}},
              {{-.5F, -.5F, -0.5F}, {.1F, .8F, .1F}},
              {{.5F, -.5F, -0.5F}, {.1F, .8F, .1F}},
              {{.5F, .5F, -0.5F}, {.1F, .8F, .1F}},

        };
        for(auto& vertex : vertices)
        {
            vertex.position += offset;
        }
        return std::make_unique<Model>(device, vertices);
    }

    void Application::loadGameObjects(void)
    {
        std::shared_ptr<Model> model{createCubeModel(m_device, glm::vec3{0.0F})};

        auto cube{GameObject::createGameObject()};
        cube.model = model;
        cube.transform.translation = {0.0F, 0.0F, 2.5F};
        cube.transform.scale = {0.5F, 0.5F, 0.5F};

        m_gameObjects.push_back(std::move(cube));
    }

    void Application::run(void)
    {
        FrameTime frameTime{};
        SimpleRenderSystem simpleRenderSystem{m_device, m_renderer.getSwapChainRenderPass()};
        Camera camera{};
        KeyboardMovementController cameraController{};

        auto cameraCurrentStat{GameObject::createGameObject()};

        while(!m_window.shouldClose())
        {
            m_window.pollEvents();
            frameTime.gameLoopStarted();

            cameraController.moveInPlaneXZ(m_window.getGLFWwindow(), frameTime.getFrameTime(), cameraCurrentStat);
            camera.setViewYXZ(cameraCurrentStat.transform.translation, cameraCurrentStat.transform.rotation);

            camera.setPerspectiveProjection(glm::radians(50.0F), m_renderer.getSwapChainAspectRatio(), 0.1F, 100.0F);

            if(VkCommandBuffer commandBuffer{m_renderer.beginFrame()})
            {
                m_renderer.beginSwapChainRenderPass(commandBuffer);

                simpleRenderSystem.renderGameObjects(commandBuffer, m_gameObjects, camera);

                m_renderer.endSwapChainRenderPass(commandBuffer);
                m_renderer.endFrame();
            }

            performance(frameTime.getFrameTime());
        }

        vkDeviceWaitIdle(m_device.device());
    }

    void Application::performance(float frameTime, bool inSeconds)
    {
        static float time{frameTime};
        static std::size_t numOfFrames{};
        ++numOfFrames;

        if(time >= 1)
        {
            if(inSeconds)
            {
                std::cerr << static_cast<double>(numOfFrames) << " FPS" << '\r';
            }
            else
            {
                std::cerr << 1000 / static_cast<double>(numOfFrames) << " ms/Frame" << '\r';
            }

            // Reset the timer
            numOfFrames = 0;
            time = 0;
        }
        else
        {
            time += frameTime;
        }
    }
}
