#include "camera.h"

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(
        transform.position, transform.position + transform.getFront(), transform.getUp());
}

PerspectiveCamera::PerspectiveCamera(float fovy, float aspect, float znear, float zfar)
    : fovy(fovy), aspect(aspect), znear(znear), zfar(zfar) {}

glm::mat4 PerspectiveCamera::getProjectionMatrix() const {
    return glm::perspective(fovy, aspect, znear, zfar);
}

Frustum PerspectiveCamera::getFrustum() const {
    Frustum frustum;
    // TODO: construct the frustum with the position and orientation of the camera
    // Note: this is for Bonus project 'Frustum Culling'
    // write your code here
    // ----------------------------------------------------------------------
    const float halfVSide = zfar * tanf(fovy * .5f);
    const float halfHSide = halfVSide * aspect;
    const glm::vec3 frontMultFar = zfar * transform.getFront();

    frustum.planes[Frustum::NearFace] = { transform.position + znear * transform.getFront(), transform.getFront() };
    frustum.planes[Frustum::FarFace] = { transform.position + frontMultFar, -transform.getFront() };
    frustum.planes[Frustum::LeftFace] = { transform.position, glm::normalize(glm::cross(frontMultFar - transform.getRight() * halfHSide, transform.getUp())) };
    frustum.planes[Frustum::RightFace] = { transform.position, glm::normalize(glm::cross(transform.getUp(), frontMultFar + transform.getRight() * halfHSide)) };
    frustum.planes[Frustum::BottomFace] = { transform.position, glm::normalize(glm::cross(transform.getRight(), frontMultFar - transform.getUp() * halfVSide)) };
    frustum.planes[Frustum::TopFace] = { transform.position, glm::normalize(glm::cross(frontMultFar + transform.getUp() * halfVSide, transform.getRight())) };
    // ----------------------------------------------------------------------

    return frustum;
}

OrthographicCamera::OrthographicCamera(
    float left, float right, float bottom, float top, float znear, float zfar)
    : left(left), right(right), top(top), bottom(bottom), znear(znear), zfar(zfar) {}

glm::mat4 OrthographicCamera::getProjectionMatrix() const {
    return glm::ortho(left, right, bottom, top, znear, zfar);
}

Frustum OrthographicCamera::getFrustum() const {
    Frustum frustum;
    const glm::vec3 fv = transform.getFront();
    const glm::vec3 rv = transform.getRight();
    const glm::vec3 uv = transform.getUp();

    // all of the plane normal points inside the frustum, maybe it's a convention
    frustum.planes[Frustum::NearFace] = {transform.position + znear * fv, fv};
    frustum.planes[Frustum::FarFace] = {transform.position + zfar * fv, -fv};
    frustum.planes[Frustum::LeftFace] = {transform.position - right * rv, rv};
    frustum.planes[Frustum::RightFace] = {transform.position + right * rv, -rv};
    frustum.planes[Frustum::BottomFace] = {transform.position - bottom * uv, uv};
    frustum.planes[Frustum::TopFace] = {transform.position + top * uv, -uv};

    return frustum;
}