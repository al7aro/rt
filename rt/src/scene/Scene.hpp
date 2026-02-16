#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "../engine/rt_engine.hpp"

namespace rt {

    struct Material
    {
        glm::vec4 color;
        int emissive;
        float p0;
        float p1;
        float p2;
    };

    struct Shape
    {
        glm::vec3 pos;
        int type;
        glm::vec3 v0;
        int enabled;
        glm::vec3 v1;
        float f0;
        glm::vec3 v2;
        float f1;
        Material mat;
    };

    class Scene
    {
    private:
        std::vector<Shape> _shapes;
    public:
        Scene() {}
        ~Scene() {}

        /* COMMON FUNCTIONS */
        void SetPosition(unsigned int id, const glm::vec3& pos) { _shapes[id].pos = pos; }
        void SetEnable(unsigned id, int e) { _shapes[id].enabled = e; }
        /* MATERIAL FUNCTIONS */
        void SetColor(unsigned int id, const glm::vec4& c) { _shapes[id].mat.color = c; }
        void SetEmissive(unsigned int id, float e) { _shapes[id].mat.emissive = e; }

        /* SPHERE FUNCTIONS */
        unsigned int CreateSphere(glm::vec3 pos = glm::vec3(0.0), float radius = 1.0)
        {
            unsigned int ret = _shapes.size();
            Shape shape;
            shape.pos = pos;
            shape.type = 1;
            shape.enabled = 1;
            shape.f0 = radius;
            shape.mat.color = glm::vec4(1.0);
            shape.mat.emissive = 0.0;
            _shapes.push_back(shape);
            return (ret);
        }
        void SetRadius(unsigned int id, float r) { _shapes[id].f0 = r; }

        const rt::Shape* GetData() const
        {
            return (_shapes.data());
        }

        int GetShapeCount() const
        {
            return (_shapes.size());
        }

        int GetSceneSizeInBytes() const
        {
            return (_shapes.size() * sizeof(Shape));
        }
    };

}