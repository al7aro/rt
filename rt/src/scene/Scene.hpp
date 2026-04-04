#pragma once

#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "../engine/rt_engine.hpp"

namespace rt {

    struct Material
    {
        enum BRDFModel
        {
            LAMBERTIAN = 0,
            SPECULAR = 1,
            BLINN_PHONG = 2,
            COOK_TORRANCE = 3,
        };
        glm::vec4 color;
        int emissive;
        float p0;   // BRDF model
        float ks;   // ks componente especular
        float kd;   // kd componente difusa
        float m;    // m rugosidad
        float eta;  //
        float p5;   //
        float p6;   //
    };

    struct Shape
    {
        glm::vec3 pos;
        int type;
        glm::vec3 v0;
        float f0;
        glm::vec3 v1;
        float f1;
        glm::vec3 v2;
        float f2;
        glm::vec3 v3;
        float f3;
        Material mat;
    };

    class Scene
    {
    private:
        std::vector<Shape> _shapes;
        glm::vec4 _ambient_color;
    public:
        Scene()
            : _ambient_color(0.0) {}
        ~Scene() {}

        void SetAmbientColor(const glm::vec4& color) { _ambient_color = color; }
        const glm::vec4& GetAmbientColor() { return (_ambient_color); }

        /* COMMON FUNCTIONS */
        void SetPosition(unsigned int id, const glm::vec3& pos) { _shapes[id].pos = pos; }
        void SetPosition(std::vector<unsigned int> ids, const glm::vec3& pos) { for (unsigned int id : ids) _shapes[id].pos = pos; }

        /* MATERIAL FUNCTIONS */
        void SetColor(unsigned int id, const glm::vec4& c) { _shapes[id].mat.color = c; }
        void SetColor(std::vector<unsigned int> ids, const glm::vec4& c) { for (unsigned int id : ids) _shapes[id].mat.color = c; }

        void SetEmissive(unsigned int id, float e) { _shapes[id].mat.emissive = e; }
        void SetEmissive(std::vector<unsigned int> ids, float e) { for (unsigned int id : ids) _shapes[id].mat.emissive = e; }

        void SetModel(unsigned int id, Material::BRDFModel model) { _shapes[id].mat.p0 = float(model); }
        void SetModel(std::vector<unsigned int> ids, Material::BRDFModel model) { for (unsigned int id : ids) _shapes[id].mat.p0 = model; }
        
        void SetKd(unsigned int id, float kd) { _shapes[id].mat.kd = kd; }
        void SetKd(std::vector<unsigned int> ids, float kd) { for (unsigned int id : ids) _shapes[id].mat.kd = kd; }
        
        void SetKs(unsigned int id, float ks) { _shapes[id].mat.ks = ks; }
        void SetKs(std::vector<unsigned int> ids,  float ks) { for (unsigned int id : ids) _shapes[id].mat.ks = ks; }
        
        void SetParamM(unsigned int id, float m) { _shapes[id].mat.m = m; }
        void SetParamM(std::vector<unsigned int> ids, float m) { for (unsigned int id : ids) _shapes[id].mat.m = m; }
        
        void SetRefracti(unsigned int id, float eta) { _shapes[id].mat.eta = eta; }
        void SetRefracti(std::vector<unsigned int> ids, float eta) { for (unsigned int id : ids) _shapes[id].mat.eta = eta; }


        /* SPHERE FUNCTIONS */
        unsigned int CreateSphere(glm::vec3 pos = glm::vec3(0.0), float radius = 1.0)
        {
            unsigned int ret = _shapes.size();
            Shape shape;
            shape.pos = pos;
            shape.type = 1;
            shape.f0 = radius;
            shape.mat.color = glm::vec4(1.0);
            shape.mat.emissive = 0.0;
            shape.mat.p0 = Material::LAMBERTIAN;
            shape.mat.m = 0.5;
            shape.mat.kd = 0.5;
            shape.mat.ks = 0.5;
            shape.mat.eta = 1.5;
            _shapes.push_back(shape);
            return (ret);
        }
        void SetRadius(unsigned int id, float r) { _shapes[id].f0 = r; }

        /* PLANE FUNCTIONS */
        unsigned int CreatePlane(const glm::vec3& origin = glm::vec3(0.0), const glm::vec3& normal = glm::vec3(0.0, 1.0, 0.0))
        {
            unsigned int ret = _shapes.size();
            Shape shape;
            shape.pos = origin;
            shape.type = 2;
            shape.v0 = normal;
            shape.mat.color = glm::vec4(1.0);
            shape.mat.emissive = 0.0;
            shape.mat.p0 = Material::LAMBERTIAN;
            shape.mat.m = 0.5;
            shape.mat.kd = 0.5;
            shape.mat.ks = 0.5;
            shape.mat.eta = 1.5;
            _shapes.push_back(shape);
            return (ret);
        }

        /* QUAD FUNCTIONS */
        unsigned int CreateQuad(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& d)
        {
            unsigned int ret = _shapes.size();
            Shape shape;
            shape.pos = a;
            shape.type = 3;
            shape.v0 = a;
            shape.v1 = b;
            shape.v2 = c;
            shape.v3 = d;
            shape.mat.color = glm::vec4(1.0);
            shape.mat.emissive = 0.0;
            shape.mat.p0 = Material::LAMBERTIAN;
            shape.mat.m = 0.5;
            shape.mat.kd = 0.5;
            shape.mat.ks = 0.5;
            shape.mat.eta = 1.5;
            _shapes.push_back(shape);
            return (ret);
        }

        // THE BOX WILL BE AN ARRAY OF QUADS
        std::vector<unsigned int> CreateBox(const glm::vec3& center, glm::vec3 scale = glm::vec3(1.0), glm::vec3 rot = glm::vec3(0.0))
        {
            float x = scale.x / 2.0;
            float y = scale.y / 2.0;
            float z = scale.z / 2.0;
            glm::vec3 right = glm::vec3(1, 0, 0);
            glm::vec3 front = glm::vec3(0, 0, 1);
            glm::vec3 up = glm::vec3(0, 1, 0);
            
            right = glm::rotate(right, rot.x, up);
            front = glm::rotate(front, rot.x, up);

            front = glm::rotate(front, rot.y, right);
            up = glm::rotate(up, rot.y, right);

            up = glm::rotate(up, rot.z, front);
            right = glm::rotate(right, rot.z, front);
            
            std::vector<unsigned int> quads;
            glm::vec3 v0;
            glm::vec3 v1;
            glm::vec3 v2;
            glm::vec3 v3;
            v0 = center -x*right -y*up -z*front;
            v1 = center -x*right -y*up +z*front;
            v2 = center -x*right +y*up +z*front;
            v3 = center -x*right +y*up -z*front;
            quads.push_back(CreateQuad(v0, v1, v2, v3));
            v0 = center +x*right -y*up -z*front;
            v1 = center +x*right -y*up +z*front;
            v2 = center +x*right +y*up +z*front;
            v3 = center +x*right +y*up -z*front;
            quads.push_back(CreateQuad(v3, v2, v1, v0));
            v0 = center -x*right -y*up -z*front;
            v1 = center +x*right -y*up -z*front;
            v2 = center +x*right -y*up +z*front;
            v3 = center -x*right -y*up +z*front;
            quads.push_back(CreateQuad(v0, v1, v2, v3));
            v0 = center -x*right +y*up -z*front;
            v1 = center +x*right +y*up -z*front;
            v2 = center +x*right +y*up +z*front;
            v3 = center -x*right +y*up +z*front;
            quads.push_back(CreateQuad(v3, v2, v1, v0));
            v0 = center -x*right -y*up -z*front;
            v1 = center -x*right +y*up -z*front;
            v2 = center +x*right +y*up -z*front;
            v3 = center +x*right -y*up -z*front;
            quads.push_back(CreateQuad(v0, v1, v2, v3));
            v0 = center -x*right -y*up +z*front;
            v1 = center -x*right +y*up +z*front;
            v2 = center +x*right +y*up +z*front;
            v3 = center +x*right -y*up +z*front;
            quads.push_back(CreateQuad(v3, v2, v1, v0));
            return (quads);
        }

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