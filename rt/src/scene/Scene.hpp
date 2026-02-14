#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "../engine/rt_engine.hpp"

namespace rt {

    struct Material
    {
        glm::vec4 color = glm::vec4(1.0);
        int emissive = 0;
    };

    struct Shape
    {
        int type = -1;
        glm::vec3 pos = glm::vec3(0.0);

        int enabled = 1;
        Material mat = Material{glm::vec4(1.0), 0};

        float f0 = 0.0;
        float f1 = 0.0;
        float f2 = 0.0;
        glm::vec3 v0 = glm::vec3(0.0);
        glm::vec3 v1 = glm::vec3(0.0);
        glm::vec3 v2 = glm::vec3(0.0);
    };

    class Scene
    {
    private:
        std::vector<Shape> _shapes;
    public:
        Scene() {}
        ~Scene() {}

        void AddSphere(glm::vec3 pos, float radius, glm::vec4 color, int emissive)
        {
            Shape shape;
            shape.type = 1;
            shape.pos = pos;
            shape.f0 = radius;
            shape.mat.color = color;
            shape.mat.emissive = emissive;
            _shapes.push_back(shape);
        }

        const std::vector<Shape>& GetShapes() const
        {
            return (_shapes);
        }

        int GetSceneSize() const
        {
            return (_shapes.size());
        }
    };

}