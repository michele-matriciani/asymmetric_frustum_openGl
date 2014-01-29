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
#include "face.h"

using namespace tdogl;

//#define M_PI 3.1415926535897932384626433832795
static const float MaxVerticalAngle = 85.0f; //must be less than 90 to avoid gimbal lock


//float _prevX = 0.0f;
//float _prevY= 0.0f;
int _prevX = 300.0f;
int _prevY= 200.0f;

float const xCam = 600.0f;  //CAM resolution
float const yCam = 400.0f;
float const xScreen = 1920.0f;  //SCENE resolution
float const yScreen = 1080.0f;
float const ar =  0.5625f; // 9/16 ASPECT RATIO

float const xr = xScreen / xCam;
float const yr = yScreen / yCam;

float const width = 1.0f;   //FRUSTUM width and height
float const height = ar;

//
float n, f, right_edge, left_edge, top_edge, bottom_edge;

glm::vec3 eye, positionV, target;

glm::mat4 frustum, viewMatrix, translate;

static inline float RadiansToDegrees(float radians) {
    return radians * 180.0f / (float)M_PI;
}


Camera::Camera() :
    _position(0.0f, 0.0f, 1.0f),
    _horizontalAngle(0.0f),
    _verticalAngle(0.0f),
    _fieldOfView(45.0f),
    
    _nearPlane(1.0f),
    _farPlane(4.0f),
    _viewportAspectRatio(4.0f/3.0f)
    
{
}

const glm::vec3& Camera::position() const {
    return _position;
}

void Camera::setPosition(const glm::vec3& position) {
    _position = position;
}

float Camera::prevX() const {
    return _prevX;
}
float Camera::prevY() const {
    return _prevY;
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
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    //glm::vec4 up = glm::inverse(orientation()) * glm::vec4(0,1,0,1);
    return glm::vec3(up);
}

glm::mat4 Camera::matrix() const {
    //return projection() * view();
    return projection();
}



glm::mat4 Camera::projection() const {
    

/*
    float alpha = M_PI/6.0f;
    //float alpha = 30.0f; //horizontal field of view
*/
    
    
    int coordX = 0, coordY= 0; 
    bool change = true;
    change = getFaceCoord( &coordX, &coordY );
    
    /*
    int mouseX, mouseY;
    glfwGetMousePos(&mouseX, &mouseY);

    glm::vec3 eye = glm::vec3(   (float)(mouseX) / 1000,
                                -(float)(mouseY) / 1000 *0.75f,
                                                _nearPlane-1.0f);

    */
    
    if (coordX == 0 && coordY == 0)
        change = false;

    

    if ( change ) {
        int difX = abs( coordX - prevX() );
        int difY = abs( coordY - prevY() );
        int err = 2;
        if ( difX < err && difY < err ) {// filtro  
            coordX = prevX();
            coordY = prevY();
        }
        else 
            setPrev( coordX, coordY );
    }
    else {
        coordX = prevX();
        coordY = prevY();
    }

    std::cerr << coordX << " , " << coordY << std::endl;


    float x = (( (float)(coordX) - 300.0f ) / 100.0f) / (xr*ar);
    float y = (( (float)(coordY) - 320.0f ) / 100.0f) / (yr*ar);
    
    /*if (change) {
        x = (( (float)(coordX) - 300.0f ) / 100.0f) / xr;
        y = (( (float)(coordY) - 220.0f ) / 100.0f) / yr;
        float difX = x - prevX();
        float difY = y - prevY();
        float err = 0.015f;
        if( difX > err && difX < -err &&
            difY > err && difY < -err ){
            //std::cerr << difX << " , " << difY << std::endl;
            x = prevX();
            y = prevY();


        }
        else {
            setPrev(x,y);
        }
    }
    else {
        x = prevX();
        y = prevY();
    }*/

    //std::cerr << x << " , " << y << std::endl;

    eye = glm::vec3(                x,
                                    y,
                      _nearPlane-1.0f );

    
    //FRUSTUM COORDINATES
    n = eye.z +1.0f;
    f = n + 2.0f;
    
    right_edge = width - eye.x ;
    left_edge = right_edge - 2*width;
    top_edge = height - eye.y;
    bottom_edge = top_edge - 2*height;

    
    //std::cerr << eye.x << " , " << eye.y << " , " << n << std::endl;
    
    //POSITION VECTOR
    positionV = glm::vec3( -eye.x, -eye.y, eye.z );
    
    //TARGET VECTOR
    target = glm::vec3(  right_edge -1.0f,
                            top_edge - ar,
                                       -n );
    
    //FRUSTUM MATRIX
    frustum = glm::frustum(left_edge, right_edge, bottom_edge, top_edge, n, f);
    
    //VIEWMATRIX
    viewMatrix = glm::lookAt(positionV, target, up());

    //TRANSLATION MATRIX
    translate = glm::translate(                      glm::mat4(1.0f), 
                               glm::vec3(-eye.x + right_edge - width,
                                 (-eye.y + top_edge - height)/height,
                                                                   0 ));

    return translate * (frustum * viewMatrix);
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

//void Camera::setPrev( float x,float y) const {
void Camera::setPrev( int x, int y) const {
    _prevX = x;
    _prevY = y;
}