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
#include "Filter.h" //obj loader

using namespace tdogl;

//#define M_PI 3.1415926535897932384626433832795
static const float MaxVerticalAngle = 85.0f; //must be less than 90 to avoid gimbal lock

Filter filter;

//float _prevX = 0.0f;
//float _prevY= 0.0f;
int _prevX = 320;
int _prevY= 240;
float _prevZ = 1.0f;

int _prevX2 = 320;
int _prevY2= 240;
float _prevZ2 = 1.0f;

int count;

float xCam = 640.0f;  //CAM resolution
float yCam = 480.0f;
float xScreen = 1920;  //SCENE resolution
float yScreen = 1080;
float ar; // 9/16 ASPECT RATIO

float xr;
float yr;

float width = 1.0f;   //FRUSTUM width and height
float height;

//
float n, nP, f, right_edge, left_edge, top_edge, bottom_edge;

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

void Camera::init(int xS, int yS, int xC, int yC,float nPl) {
    xScreen = xS;
    yScreen = yS;
    xCam    = xC;
    yCam    = yC;
    nP      = nPl;
    ar      = yScreen/xScreen; 
    xr      = xScreen/xCam;
    yr      = yScreen/yCam;
    height  = ar;
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



/*** USED FUNCTIONS ***/


glm::mat4 Camera::matrix() const {
    //return projection() * view();
    return projection();
}


glm::vec3 Camera::up() const {
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    //glm::vec4 up = glm::inverse(orientation()) * glm::vec4(0,1,0,1);
    return glm::vec3(up);
}

int Camera::prevX() const {
    return _prevX;
}
int Camera::prevY() const {
    return _prevY;
}
int Camera::prevZ() const {
    return _prevZ;
}

int Camera::prevX2() const {
    return _prevX2;
}
int Camera::prevY2() const {
    return _prevY2;
}
int Camera::prevZ2() const {
    return _prevZ2;
}




float interpolate ( int cz ) {
/*
    if ( cx < 0)
        cx *= -1;
    if ( cz < 0)
        cz *= -1;
    return (cx)/(cz);
*/
    // z max = 480;
    // z giusta = 162;
    return ( (-162+480)*cz*cz + 480 ) /10000000.0f;

}

void Camera::setX(glm::vec3 & f, int &coordX) const{
    coordX  = f[0];
    _prevX  = f[1];
    _prevX2 = f[2];
}

void Camera::setY(glm::vec3 & f, int &coordY) const{
    coordY  = f[0];
    _prevY  = f[1];
    _prevY2 = f[2];
}
void Camera::setZ(glm::vec3 & f, int &coordZ) const {
    coordZ  = f[0];
    _prevZ  = f[1];
    _prevZ2 = f[2];
}

glm::mat4 Camera::projection() const {
    

    
    int coordX = 0, coordY = 0, coordZ = 0; 
    bool change = true;
    change = getFaceCoord( &coordX, &coordY, &coordZ  );

    glm::vec3 fX = glm::vec3( coordX, prevX(), prevX2());
    glm::vec3 fY = glm::vec3( coordY, prevY(), prevY2());
    glm::vec3 fZ = glm::vec3( coordZ, prevZ(), prevZ2());

    filter.filter(fX,5,8);
    filter.filter(fY,5,8);
    filter.filter(fZ,7,15);

    setX(fX,coordX);
    setY(fY,coordY);
    setZ(fZ,coordZ);
    


    /*
    int mouseX, mouseY;
    glfwGetMousePos(&mouseX,   &mouseY);

    glm::vec3 eye = glm::vec3(   (float)(mouseX) / 1000,
                                -(float)(mouseY) / 1000 *0.75f,
                                                 _nearPlane-1.0f);
    coordX = mouseX;
    coordY = mouseY;
*/
    
    //float c = interpolate(coordZ);
    float z = (float)coordZ / 100.0f ;
    
    // con -320.0f non e' allineato
    float x = ((( (float)(coordX) - 320.0f ) / 100.0f ) / (xr*ar));
    
    //x = x*c;

    float y = ((( (float)(coordY) - 240.0f ) / 100.0f ) / (yr*ar));
    
    
    
    std::cerr << x << " , " << y <<  " , " << coordZ <<std::endl;
    
    //EYE VECTOR
    eye = glm::vec3(                x,
                                    y,
                                z-1.0f );

    //std::cerr << coordX << " , " << coordY << " , " << z <<std::endl;
    
    //FRUSTUM COORDINATES
    n = -eye.z + nP;
    f = n     + 1000.0f;
    
    right_edge  = width      - eye.x ;
    left_edge   = right_edge - 2*width;
    top_edge    = height     - eye.y;
    bottom_edge = top_edge   - 2*height;


    //POSITION VECTOR
    positionV = glm::vec3( -eye.x, -eye.y, -eye.z);
    
    //TARGET VECTOR
    target    = glm::vec3( right_edge - width,
                            top_edge - height,
                                         -n-1 );
    
    //FRUSTUM MATRIX
    frustum   = glm::frustum(left_edge, right_edge, bottom_edge,
                                                 top_edge, n, f);
    
    //VIEWMATRIX
    viewMatrix = glm::lookAt(positionV, target, up());

    //TRANSLATION MATRIX
    translate = glm::translate(                      glm::mat4(1.0f), 
                               glm::vec3(-eye.x + right_edge - width,
                                 (-eye.y + top_edge - height)/height,
                                                           1.0f));

    return translate  * frustum * viewMatrix;
}