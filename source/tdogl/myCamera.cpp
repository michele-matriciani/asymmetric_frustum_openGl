/*
 tdogl::Camera

 Copyright 2012 Thomas Dalling - http://tomdalling.com/

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */
#define _USE_MATH_DEFINES
#include <cmath>
#include <GL/glfw.h>
#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace tdogl;

//#define M_PI 3.1415926535897932384626433832795
static const float MaxVerticalAngle = 85.0f; //must be less than 90 to avoid gimbal lock

static inline float RadiansToDegrees(float radians) {
    return radians * 180.0f / (float)M_PI;
}


Camera::Camera() :
    _position(0.0f, 0.0f, 1.0f),
    _horizontalAngle(0.0f),
    _verticalAngle(0.0f),
    _fieldOfView(45.0f),

    _eyeZ(1.0f),
    _nearPlane(3.0f),
    _farPlane(100.0f),
    _viewportAspectRatio(4.0f/3.0f)
{
}

const glm::vec3& Camera::position() const {
    return _position;
}

void Camera::setPosition(const glm::vec3& position) {
    _position = position;
}

void Camera::offsetPosition(const glm::vec3& offset) {
    _position += offset;
}

float Camera::fieldOfView() const {
    return _fieldOfView;
}

void Camera::setFieldOfView(float fieldOfView) {
    assert(fieldOfView > 0.0f && fieldOfView < 180.0f);
    _fieldOfView = fieldOfView;
}

float Camera::eyeZ() const {
    return _eyeZ;
}

void Camera::setEyeZ( float eyeZ ) {
    _eyeZ = eyeZ;
}

float Camera::nearPlane() const {
    return _nearPlane;
}

float Camera::farPlane() const {
    return _farPlane;
}

void Camera::setNearAndFarPlanes(float nearPlane, float farPlane) {
    assert(nearPlane > 0.0f);
    assert(farPlane > nearPlane);
    _nearPlane = nearPlane;
    _farPlane = farPlane;
}

glm::mat4 Camera::orientation() const {
    glm::mat4 orientation;
    orientation = glm::rotate(orientation, _verticalAngle, glm::vec3(1,0,0));
    orientation = glm::rotate(orientation, _horizontalAngle, glm::vec3(0,1,0));
    return orientation;
}

void Camera::offsetOrientation(float upAngle, float rightAngle) {
    _horizontalAngle += rightAngle;
    _verticalAngle += upAngle;
    normalizeAngles();
}

void Camera::lookAt(glm::vec3 position) {
    assert(position != _position);
    glm::vec3 direction = glm::normalize(position - _position);
    _verticalAngle = RadiansToDegrees(asinf(-direction.y));
    _horizontalAngle = -RadiansToDegrees(atan2f(-direction.x, -direction.z));
    normalizeAngles();
}

float Camera::viewportAspectRatio() const {
    return _viewportAspectRatio;
}

void Camera::setViewportAspectRatio(float viewportAspectRatio) {
    assert(viewportAspectRatio > 0.0);
    _viewportAspectRatio = viewportAspectRatio;
}

glm::vec3 Camera::forward() const {
    glm::vec4 forward = glm::inverse(orientation()) * glm::vec4(0,0,-1,1);
    return glm::vec3(forward);
}

glm::vec3 Camera::right() const {
    glm::vec4 right = glm::inverse(orientation()) * glm::vec4(1,0,0,1);
    return glm::vec3(right);
}

glm::vec3 Camera::up() const {
    glm::vec4 up = glm::inverse(orientation()) * glm::vec4(0,1,0,1);
    return glm::vec3(up);
}

glm::mat4 Camera::matrix() const {
    //return projection() * view();
    return projection();
}

glm::mat4 tranAndScale(glm::vec3 eye) {
    glm::mat4 translate = glm::translate(glm::mat4(), glm::vec3(-eye.x+1,0.0f,0.0f));
    
    glm::mat4 scale = glm::mat4( 2.0f,  0,  0,  0,
                                    0,  1.0f,   0,  0,
                                    0,  0,  1.0f,   0,
                                    0,  0,  0,  1.0f    );
    glm::mat4 translate2 = glm::translate(glm::mat4() ,
                            glm::vec3(eye.x +1, 0.0f,0.0f) );

    return translate;

}



glm::mat4 Camera::projection() const {
    

    //std::cerr << eye.x << " , " << eye.y << " , " << eye.z << std::endl;

    float alpha = M_PI/6.0f;
    //float alpha = 30.0f; //horizontal field of view
    float a = 0.75f;  //aspect ratio
    float e = 1.0/glm::tan(alpha/2.0);  //focal length
    //float beta = 2.0* glm::atan(a/e);
    
    float n = -e;  //near plane distance
    float f = -3*e;
    //float f = n - 100.0f;    //far plane distance
    
    float x = -n/e;
    float right_edge = x; //right edge of near plane
    float left_edge = -x;
    
    float y = -(a*n)/e;
    float top_edge = y; //top edge of near plane
    float bottom_edge = -y;
    
    n = -1.0f;
    f = -5.0f;
    right_edge = 1.0f;
    left_edge = -1.0f;
    top_edge = 0.75f;
    bottom_edge = -0.75f;
    
    int mouseX, mouseY;
    glfwGetMousePos(&mouseX, &mouseY);

    glm::vec3 eye = glm::vec3(   (float)(mouseX) / 1000,
                                -(float)(mouseY) / 1000 *0.75f,
                                                n);

    std::cerr << eye.x << " , " << eye.y << " , " <<  std::endl;
    
  
       
    right_edge = 1.0f - eye.x ;
    left_edge = right_edge - 2;
    top_edge = 0.75f - eye.y;
    bottom_edge = top_edge - 1.5f;
/*
    n = n - eye.x/10;
    f = n -6.0f;
*/
    glm::mat4 frustum = glm::frustum(left_edge, right_edge, bottom_edge, top_edge, -n, -f);
    
    
    glm::vec3 position = glm::vec3(-eye.x,-eye.y, 0.0f);
    
    glm::vec3 target = glm::vec3(right_edge -1.0f,
                                  top_edge - 0.75f,
                                   -1.0f );
    

    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 viewMatrix = glm::lookAt(position, target, up);

    
    
    glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(-eye.x + right_edge-1,(-eye.y + top_edge -0.75f)/0.75f ,0.0f ));

    
    glm::mat4 proj;
    glm::mat4 transform;
    transform =  translate;
    proj = transform * (frustum * viewMatrix);

    return proj;
}

glm::mat4 Camera::view() const {
    return orientation() * glm::translate(glm::mat4(), -_position);
}

void Camera::normalizeAngles() {
    _horizontalAngle = fmodf(_horizontalAngle, 360.0f);
    //fmodf can return negative values, but this will make them all positive
    if(_horizontalAngle < 0.0f)
        _horizontalAngle += 360.0f;

    if(_verticalAngle > MaxVerticalAngle)
        _verticalAngle = MaxVerticalAngle;
    else if(_verticalAngle < -MaxVerticalAngle)
        _verticalAngle = -MaxVerticalAngle;
}
