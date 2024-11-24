#include "Camera.h"
#include "Log.h"

namespace framework {
Camera::Camera() {
    mYaw = 0.0f;
    mPitch = 0.0f;

    mWorldUp = glm::vec3(0.0, 0.0, 1.0);
    mTargetPoint = glm::vec3(0.0, 0.0, 0.0);
    mTargetDistance = 1.0f;
    UpdateView();

    float aspect = 1.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    float fovyDeg = 60.0f;
    mProjection = glm::perspective(glm::radians(fovyDeg), aspect, nearPlane, farPlane);
}

Camera::~Camera() {

}

void Camera::ProcessMove(glm::vec3 offset) {
    mTargetPoint += offset.x * mSensitiveX * mRight;
    mTargetPoint += offset.y * mSensitiveY * mUp;
    mTargetPoint += offset.z * mSensitiveFront * mFront;
}

void Camera::ProcessRotate(glm::vec2 offset) {
    mYaw = std::fmodf(mYaw - mSensitiveYaw * offset.x, 360.0f);
    mPitch = glm::clamp(mPitch + mSensitivePitch * offset.y, -89.9f, 89.9f);
}

void Camera::ProcessScale(float offset) {
    mTargetPoint += offset * mSensitiveFront * mFront;
}

void Camera::SetPerspective(float aspect, float nearPlane, float mFarPlane, float fovyDeg) {
    mProjection = glm::perspective(glm::radians(fovyDeg), aspect, nearPlane, mFarPlane);
}

void Camera::UpdateView() {
    mFront.x = std::cos(glm::radians(mPitch)) * std::cos(glm::radians(mYaw));
    mFront.y = std::cos(glm::radians(mPitch)) * std::sin(glm::radians(mYaw));
    mFront.z = std::sin(glm::radians(mPitch));
    mFront = -glm::normalize(mFront);
    //LOGI("front %.3f %.3f %.3f", mFront.x, mFront.y, mFront.z);

    mRight = glm::normalize(glm::cross(mFront, mWorldUp));
    mUp = glm::normalize(glm::cross(mRight, mFront));

    mPosition = mTargetPoint - mTargetDistance * mFront;
    mView = glm::lookAt(mPosition, mTargetPoint, mUp);
}
}