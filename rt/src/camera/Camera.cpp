#include "Camera.hpp"

namespace rt {

    Camera::Camera()
        : _pos(glm::vec3(0.0)), _rot(glm::quat(1.0, 0.0, 0.0, 0.0)),
        _aspect(1.0), _fov(90), _yaw(0.0), _pitch(0.0) {}

    Camera::~Camera() {}

    void Camera::Move(const glm::vec3& offset)
    {
        _pos += _rot * offset;
    }
    void Camera::SetPosition(const glm::vec3& pos)
    {
        _pos = pos;
    }

    void Camera::Pitch(float angle)
    {
        _pitch += angle;
        UpdateRotation();
    }
    void Camera::Yaw(float angle)
    {
        _yaw += angle;
        UpdateRotation();
    }
    void Camera::Roll(float angle)
    {
    }

    void Camera::UpdateRotation()
    {
        glm::quat q_yaw = glm::angleAxis(_yaw, glm::vec3(0.0, 1.0, 0.0));
        glm::quat q_pitch = glm::angleAxis(_pitch, glm::vec3(1.0, 0.0, 0.0));
        _rot = q_yaw * q_pitch;
    }

    void Camera::SetAspect(float aspect) { _aspect = aspect; }
    void Camera::SetFOV(float fov) { _fov = fov; }
    float Camera::GetAspect() const { return (_aspect); }
    float Camera::GetFOV() const { return (glm::radians(_fov)); }
    glm::vec3 Camera::GetPosition() const { return (_pos); }

    glm::mat3 Camera::GetRotationMatrix() const
    {
        glm::mat4 rot = glm::mat3_cast(glm::inverse(_rot));
        return (rot);
    }

}