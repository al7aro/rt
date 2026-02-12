#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Camera.hpp"

namespace rt {

    class Camera
    {
    private:
        glm::vec3 _pos;
        glm::quat _rot;

        float _aspect, _fov;

        float _yaw, _pitch;

    public:
        Camera();
        ~Camera();

        void Move(const glm::vec3& offset);
        void SetPosition(const glm::vec3& pos);

        void Pitch(float angle);
        void Yaw(float angle);
        void Roll(float angle);

        void SetAspect(float aspect);
        void SetFOV(float fov);

        float GetAspect() const;
        float GetFOV() const;
        glm::vec3 GetPosition() const;  /* Returns the position of the camera */
        glm::mat3 GetRotationMatrix() const;

    private:
        void UpdateRotation();
    };

}