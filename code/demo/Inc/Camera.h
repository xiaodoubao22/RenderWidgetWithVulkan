#ifndef __CAMERA__
#define __CAMERA__

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace render {
class Camera {
public:
    Camera();
    ~Camera();

    void ProcessMove(glm::vec3 offset);
    void ProcessRotate(glm::vec2 offset);
    void ProcessScale(float offset);
    void SetPerspective(float aspect = 1.0f, float nearPlane = 0.1f, float mFarPlane = 100.0f, float fovyDeg = 60.0f);

    glm::mat4& GetView()
    {
        return mView;
    }
    glm::mat4& GetProjection()
    {
        return mProjection;
    }
    glm::vec3& GetUp()
    {
        return mUp;
    }
    glm::vec3& GetRight()
    {
        return mRight;
    }
    glm::vec3& GetFront()
    {
        return mFront;
    }

    void UpdateView();

public:
    float_t mYaw;
    float_t mPitch;
    float_t mSensitiveYaw = 0.1;
    float_t mSensitivePitch = 0.1;
    float_t mSensitiveX = 0.001;
    float_t mSensitiveY = 0.001;
    float_t mSensitiveFront = 0.05;

    glm::vec3 mPosition;
    glm::vec3 mTargetPoint;
    float mTargetDistance;
    glm::vec3 mRight;
    glm::vec3 mUp;
    glm::vec3 mFront;
    glm::vec3 mWorldUp;
    glm::mat4 mView;

    glm::mat4 mProjection;

};
}   // namespace render

#endif
