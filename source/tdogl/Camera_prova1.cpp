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

glm::mat4 Camera::projection() const {
    float leftP   = -0.8f;
    float rightP =  0.8f;
    float topP    =  0.6f;
    float bottomP = -0.6f;

    float d_near = _nearPlane;
    
    int mouseX, mouseY;
    glfwGetMousePos(&mouseX, &mouseY);

    int rh = 1; // set to -1 to right hand

    glm::vec3 eye = glm::vec3(  rh*(float)(mouseX) / 1000,
                                rh*(float)(mouseY) / 1000 *0.75f,
                                            (-1)* rh*   d_near);

    glm::vec3 up  =           glm::vec3(0.0, 1.0, 0.0);
    
   // std::cerr << eye.x << " , " << eye.y << " , " << eye.z << std::endl;
 
    float d_far = eye.z + abs(_farPlane) + 1;

   

    /*** calcolo piani ****/
    
    /*
    float bottom = bottomP + eye.y;
    float top = bottom - 2*bottomP;
    float left = leftP + eye.x;
    float right = left - 2*leftP; 
  */
   
    
    float top    =     topP - 0.75f*eye.y;
    float bottom =     top - 2*topP;
    float right  =   rightP - eye.x;
    float left   = right - 2*rightP;
    
    /*** vettori per view matrix ****/
    //glm::vec3 view_dir = glm::vec3(0.0, 0.0, -1.0);
    //glm::vec3 view_dir = eye - glm::vec3(-(right+left)/2,-(top-bottom)/(2/0.75f)
     //                                      , -dirZ);

    
    
    glm::vec3 zaxis =   glm::normalize(-eye);
    glm::vec3 xaxis =  glm::normalize(glm::cross(up, zaxis)); 
    glm::vec3 yaxis =   glm::normalize(glm::cross(zaxis, xaxis));
   
    std::cerr << zaxis.x << " , " << zaxis.y << " , " << zaxis.z << std::endl;
    
    
    /*** view matrix ***/
    glm::mat4 view_m = glm::mat4(xaxis.x         , yaxis.x         , zaxis.x         , 0,
                            xaxis.y         , yaxis.y         , zaxis.y         , 0,
                            xaxis.z         , yaxis.z         , zaxis.z         , 0,
                        rh*(-glm::dot(xaxis, eye)), rh*(-glm::dot(yaxis, eye)), rh*(-glm::dot(zaxis, eye)), 1);


    /*** perspective matrix ***/
    
    glm::mat4 mp_lh = glm::mat4(
                                2*d_near/(right-left),          0,                              0,                              0,
                                0,                              2*d_near/(top-bottom),          0,                              0,
                                (left+right)/rh*(left-right),   (top+bottom)/rh*(bottom-top),   d_far/rh*(d_far-d_near),        rh*1,
                                0,                              0,                              d_near*d_far/(d_near-d_far),    0);
/*
    glm::mat4 P = glm::mat4(
        (2.0*d_near) / (right-left),                          0.0,         (right+left) / (right-left), (-d_near*(right+left))/ right -left, 
                                0.0,  (2.0*d_near) / (top-bottom),         (top+bottom) / (top-bottom),  (-d_near* (top+bottom))/ top-bottom,
                                0.0,                          0.0,  -(d_far + d_near)/(d_far - d_near), -(2.0*d_far*d_near) / (d_far-d_near),
                                0.0,                          0.0,                                -1.0,                               d_near);
    
*/
    return mp_lh * view_m;
    
       /**** prove ****/
    /*
    float bottom = bottomP + eye.y;
    float top = bottom - 2*bottomP;
    float left = leftP + eye.x;
    float right = left - 2*leftP; 
    */
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
