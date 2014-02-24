/*
 main
 
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

// third-party libraries
#include <GL/glew.h>
#include <GL/glfw.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// standard C++ libraries
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <cmath>

// tdogl classes
#include "Helper.h"
#include "tdogl/Program.h"
#include "tdogl/Texture.h"
#include "tdogl/Camera.h"
#include "face.h"

#include "tdogl/LoadObj.h"

// constants
//#define M_PI 3.1415926535897932384626433832795
const glm::vec2 SCREEN_SIZE(1920, 1080);

// globals
tdogl::Texture* gTexture = NULL;


tdogl::Program* gProgram = NULL;
tdogl::Camera gCamera;
GLuint gVAO = 0;
GLuint gVBO = 0;

GLfloat gDegreesRotated = 0.0f;

GLuint gVAO1 = 0;
GLuint gVBO1 = 0;
GLuint vbo_normals = 0;
GLuint vbo_uv = 0;
int monkeyT = 0;
tdogl::Texture* gTexture1 = NULL;
tdogl::LoadObj load;

// returns the full path to the file `fileName` in the resources directory of the app bundle
static std::string ResourcePath(std::string fileName) {
    return GetProcessPath() + "/../resources/" + fileName;
}

// loads the vertex shader and fragment shader, and links them to make the global gProgram
static void LoadShaders() {
    std::vector<tdogl::Shader> shaders;
    shaders.push_back(tdogl::Shader::shaderFromFile(ResourcePath("vertex-shader.txt"), GL_VERTEX_SHADER));
    shaders.push_back(tdogl::Shader::shaderFromFile(ResourcePath("fragment-shader.txt"), GL_FRAGMENT_SHADER));
    //shaders.push_back(tdogl::Shader::shaderFromFile(ResourcePath("box-shader.txt"), GL_FRAGMENT_SHADER));
    gProgram = new tdogl::Program(shaders);
}

static void LoadModel() {

    

    // Read our .obj file
    std::vector< glm::vec3 > vertices;
    std::vector< glm::vec2 > uvs;
    std::vector< glm::vec3 > normals; // Won't be used at the moment.
    bool res = load.loadObj(ResourcePath("monkey.obj"), vertices, uvs, normals);

    //make and bind VAO
    glGenVertexArrays(1, &gVAO1);
    glBindVertexArray(gVAO1);
    
    // make and bind the VBO
    glGenBuffers(1, &gVBO1);
    glBindBuffer(GL_ARRAY_BUFFER, gVBO1);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    monkeyT = vertices.size();
    std::cerr << vertices.size() << std::endl;
    glEnableVertexAttribArray(gProgram->attrib("vert"));
    glVertexAttribPointer(gProgram->attrib("vert"), 3, GL_FLOAT, GL_TRUE, 3*sizeof(GLfloat), NULL);


    //make and bind vbo for normals   
    glGenBuffers(1, &vbo_normals);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
    
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW); //normals is std::vector<float>
    glNormalPointer(   GL_FLOAT, 3*sizeof(GLfloat) , NULL);

    //make and bind vbo for uv coordinates
    glGenBuffers(1, &vbo_uv);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_uv);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec3), &uvs[0], GL_STATIC_DRAW); //normals is std::vector<float>
    
    glEnableVertexAttribArray(gProgram->attrib("vertTexCoord"));
    glVertexAttribPointer(gProgram->attrib("vertTexCoord"), 2, GL_FLOAT, GL_TRUE,  2*sizeof(GLfloat), NULL);

    GLfloat spec[] = {0.5f, 0.5f, 0.5f};
    GLfloat diff[] = {0.64f, 0.64f, 0.64f};
    glMaterialfv( GL_FRONT, GL_SPECULAR, spec );
    glMaterialfv( GL_FRONT, GL_DIFFUSE, diff );
    

    // unbind the VAO
    glBindVertexArray(0);
    
}
// loads a cube into the VAO and VBO globals: gVAO and gVBO
static void LoadCube(float nP, float fB,float ar) {
    // make and bind the VAO
    glGenVertexArrays(1, &gVAO);
    glBindVertexArray(gVAO);
    
    // make and bind the VBO
    glGenBuffers(1, &gVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gVBO);

    
    float n = -nP;
    float f = n - fB;
    
    float r = 1.0f;
    float l = -1.0f;
    float t = ar;
    float b = -ar;

    float la = 0.3f; //lato cubo
    
    /** coordinate cubo vertice sx basso avanti **/
    float x1 = -0.55f;
    float y1 = -0.15f;
    float z1 = n + 0.4f;

    float x2 =  0.0f;
    float y2 =  0.2f;
    float z2 = n - 1.6f;

   
    std::cerr << l << " , " << r << " , " << t << " , " << b << std::endl;
    std::cerr << n << " , " << f << std::endl;

    
    // Make a cube out of triangles (two triangles per side)
    GLfloat vertexData[] = {
        //  X     Y     Z       U     V

      
       
        // bottom
        x1   , y1   , z1   , 0.0f, 0.0f,
        x1+la, y1   , z1   , 1.0f, 0.0f,
        x1+la, y1   , z1-la, 1.0f, 1.0f,
        x1   , y1   , z1-la, 0.0f, 1.0f,
        x1   , y1   , z1   , 0.0f, 0.0f,
        x1+la, y1   , z1-la, 1.0f, 1.0f,

        // top
        x1   , y1+la, z1   , 0.0f, 0.0f,
        x1+la, y1+la, z1   , 1.0f, 0.0f,
        x1+la, y1+la, z1-la, 1.0f, 1.0f,
        x1   , y1+la, z1-la, 0.0f, 1.0f,
        x1   , y1+la, z1   , 0.0f, 0.0f,
        x1+la, y1+la, z1-la, 1.0f, 1.0f,

        // front
        x1   , y1   , z1   , 0.0f, 0.0f,
        x1+la, y1   , z1   , 1.0f, 0.0f,
        x1+la, y1+la, z1   , 1.0f, 1.0f,
        x1   , y1+la, z1   , 0.0f, 1.0f,
        x1   , y1   , z1   , 0.0f, 0.0f,
        x1+la, y1+la, z1   , 1.0f, 1.0f,

        // back
        x1   , y1   , z1-la, 0.0f, 0.0f,
        x1+la, y1   , z1-la, 1.0f, 0.0f,
        x1+la, y1+la, z1-la, 1.0f, 1.0f,
        x1   , y1+la, z1-la, 0.0f, 1.0f,
        x1   , y1   , z1-la, 0.0f, 0.0f,
        x1+la, y1+la, z1-la, 1.0f, 1.0f,

        // left
        x1   , y1   , z1   , 0.0f, 0.0f,
        x1   , y1   , z1-la, 1.0f, 0.0f,
        x1   , y1+la, z1-la, 1.0f, 1.0f,
        x1   , y1+la, z1   , 0.0f, 1.0f,
        x1   , y1   , z1   , 0.0f, 0.0f,
        x1   , y1+la, z1-la, 1.0f, 1.0f,

        // right
        x1+la, y1   , z1-la, 0.0f, 0.0f,
        x1+la, y1   , z1   , 1.0f, 0.0f,
        x1+la, y1+la, z1   , 1.0f, 1.0f,
        x1+la, y1+la, z1-la, 0.0f, 1.0f,
        x1+la, y1   , z1-la, 0.0f, 0.0f,
        x1+la, y1+la, z1   , 1.0f, 1.0f,


        //  X     Y     Z       U     V
          // bottom
        x2   , y2   , z2   , 0.0f, 0.0f,
        x2+la, y2   , z2   , 1.0f, 0.0f,
        x2+la, y2   , z2-la, 1.0f, 1.0f,
        x2   , y2   , z2-la, 0.0f, 1.0f,
        x2   , y2   , z2   , 0.0f, 0.0f,
        x2+la, y2   , z2-la, 1.0f, 1.0f,

        // top
        x2   , y2+la, z2   , 0.0f, 0.0f,
        x2+la, y2+la, z2   , 1.0f, 0.0f,
        x2+la, y2+la, z2-la, 1.0f, 1.0f,
        x2   , y2+la, z2-la, 0.0f, 1.0f,
        x2   , y2+la, z2   , 0.0f, 0.0f,
        x2+la, y2+la, z2-la, 1.0f, 1.0f,

        // front
        x2   , y2   , z2   , 0.0f, 0.0f,
        x2+la, y2   , z2   , 1.0f, 0.0f,
        x2+la, y2+la, z2   , 1.0f, 1.0f,
        x2   , y2+la, z2   , 0.0f, 1.0f,
        x2   , y2   , z2   , 0.0f, 0.0f,
        x2+la, y2+la, z2   , 1.0f, 1.0f,

        // back
        x2   , y2   , z2-la, 0.0f, 0.0f,
        x2+la, y2   , z2-la, 1.0f, 0.0f,
        x2+la, y2+la, z2-la, 1.0f, 1.0f,
        x2   , y2+la, z2-la, 0.0f, 1.0f,
        x2   , y2   , z2-la, 0.0f, 0.0f,
        x2+la, y2+la, z2-la, 1.0f, 1.0f,

        // left
        x2   , y2   , z2   , 0.0f, 0.0f,
        x2   , y2   , z2-la, 1.0f, 0.0f,
        x2   , y2+la, z2-la, 1.0f, 1.0f,
        x2   , y2+la, z2   , 0.0f, 1.0f,
        x2   , y2   , z2   , 0.0f, 0.0f,
        x2   , y2+la, z2-la, 1.0f, 1.0f,

        // right
        x2+la, y2   , z2-la, 0.0f, 0.0f,
        x2+la, y2   , z2   , 1.0f, 0.0f,
        x2+la, y2+la, z2   , 1.0f, 1.0f,
        x2+la, y2+la, z2-la, 0.0f, 1.0f,
        x2+la, y2   , z2-la, 0.0f, 0.0f,
        x2+la, y2+la, z2   , 1.0f, 1.0f,

    

         //BOX
         // bottom
        l    , b    , n    , 0.0f, 0.0f,
        r    , b    , n    , 1.0f, 0.0f,
        r    , b    , f    , 1.0f, 1.0f,
        l    , b    , f    , 0.0f, 1.0f,
        l    , b    , n    , 0.0f, 0.0f,
        r    , b    , f    , 1.0f, 1.0f,

        // top
        l    , t    , n    , 0.0f, 0.0f,
        r    , t    , n    , 1.0f, 0.0f,
        r    , t    , f    , 1.0f, 1.0f,
        l    , t    , f    , 0.0f, 1.0f,
        l    , t    , n    , 0.0f, 0.0f,
        r    , t    , f    , 1.0f, 1.0f,
        

        // back
        l    , b    , f    , 0.0f, 0.0f,
        r    , b    , f    , 1.0f, 0.0f,
        r    , t    , f    , 1.0f, 1.0f,
        l    , t    , f    , 0.0f, 1.0f,
        l    , b    , f    , 0.0f, 0.0f,
        r    , t    , f    , 1.0f, 1.0f,

        // left
        l    , b    , n    , 0.0f, 0.0f,
        l    , b    , f    , 1.0f, 0.0f,
        l    , t    , f    , 1.0f, 1.0f,
        l    , t    , n    , 0.0f, 1.0f,
        l    , b    , n    , 0.0f, 0.0f,
        l    , t    , f    , 1.0f, 1.0f,

        // right
        r    , b    , f    , 0.0f, 0.0f,
        r    , b    , n    , 1.0f, 0.0f,
        r    , t    , n    , 1.0f, 1.0f,
        r    , t    , f    , 0.0f, 1.0f,
        r    , b    , f    , 0.0f, 0.0f,
        r    , t    , n    , 1.0f, 1.0f,
   
    };

    GLfloat plane[] = {
        x1 +1  , y1   , z1   , 0.0f, 0.0f,
        x1+la+1, y1   , z1   , 1.0f, 0.0f,
        x1+la+1, y1+la, z1   , 1.0f, 1.0f,
        x1  +1 , y1+la, z1   , 0.0f, 1.0f,
        x1  +1 , y1   , z1   , 0.0f, 0.0f,
        x1+la+1, y1+la, z1   , 1.0f, 1.0f,

    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

    // connect the xyz to the "vert" attribute of the vertex shader
    glEnableVertexAttribArray(gProgram->attrib("vert"));
    glVertexAttribPointer(gProgram->attrib("vert"), 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), NULL);

    
    // connect the uv coords to the "vertTexCoord" attribute of the vertex shader
    glEnableVertexAttribArray(gProgram->attrib("vertTexCoord"));
    glVertexAttribPointer(gProgram->attrib("vertTexCoord"), 2, GL_FLOAT, GL_TRUE,  5*sizeof(GLfloat), (const GLvoid*)(3 * sizeof(GLfloat)));

    glBindVertexArray(0);
    


    /***BIND SECOND OBJECT ***/
    /*glGenVertexArrays(1, &gVAO1);
    glBindVertexArray(gVAO1);
    
    // make and bind the VBO
    glGenBuffers(1, &gVBO1);
    glBindBuffer(GL_ARRAY_BUFFER, gVBO1);

    glBufferData(GL_ARRAY_BUFFER, sizeof(plane), plane, GL_STATIC_DRAW);

    glEnableVertexAttribArray(gProgram->attrib("vert"));
    glVertexAttribPointer(gProgram->attrib("vert"), 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), NULL);

    glEnableVertexAttribArray(gProgram->attrib("vertTexCoord"));
    glVertexAttribPointer(gProgram->attrib("vertTexCoord"), 2, GL_FLOAT, GL_TRUE,  5*sizeof(GLfloat), (const GLvoid*)(3 * sizeof(GLfloat)));

    // unbind the VAO
    glBindVertexArray(0);*/
}


// loads the file "wooden-crate.jpg" into gTexture
static void LoadTexture() {
    tdogl::Bitmap bmp = tdogl::Bitmap::bitmapFromFile(ResourcePath("grid2.jpg"));
    bmp.flipVertically();
    gTexture = new tdogl::Texture(bmp);

     tdogl::Bitmap bmp1 = tdogl::Bitmap::bitmapFromFile(ResourcePath("box.jpg"));
    bmp.flipVertically();
    gTexture1 = new tdogl::Texture(bmp1);
}


// draws a single frame
static void Render() {
    // clear everything
    glClearColor(0, 0, 0, 1); // black
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // bind the program (the shaders)
    gProgram->use();

    // set the "camera" uniform
    gProgram->setUniform("camera", gCamera.matrix());

    // set the "model" uniform in the vertex shader, based on the gDegreesRotated global
    gProgram->setUniform("model", glm::translate(glm::mat4(1.0f),                       
        glm::vec3(0.0f, 0.0f, 0.0f)));
      
    //gProgram->setUniform("model", glm::rotate(glm::mat4(), gDegreesRotated, glm::vec3(0,1,0)));
        
    // bind the texture and set the "tex" uniform in the fragment shader
     glActiveTexture(GL_TEXTURE0);
     glBindTexture(GL_TEXTURE_2D, gTexture->object());
     gProgram->setUniform("tex", 0); //set to 0 because the texture is bound to GL_TEXTURE0

    
    // bind the VAO (the triangle)
    glBindVertexArray(gVAO);
    
    // draw the VAO
    //glDrawArrays(GL_TRIANGLES, 0, 6*2*3);
    glDrawArrays(GL_TRIANGLES, 0, 17*2*3  );
    
    // unbind the VAO, the program and the texture
    glBindVertexArray(0);


    /*** RENDER SECOND OBJECT***/

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTexture1->object());
    gProgram->setUniform("tex", 0); //set to 0 because the texture is bound to GL_TEXTURE0

    glBindVertexArray(gVAO1);
    
    glDrawArrays(GL_TRIANGLES, 0, monkeyT/*500*2*3*/  );
    
    // unbind the VAO, the program and the texture
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    gProgram->stopUsing();
    
    // swap the display buffers (displays what was just drawn)
    glfwSwapBuffers();
}


// update the scene based on the time elapsed since last update
void Update(float secondsElapsed) {
    //rotate the cube
    /*
    const GLfloat degreesPerSecond = 180.0f;
    gDegreesRotated += secondsElapsed * degreesPerSecond;
    while(gDegreesRotated > 360.0f) gDegreesRotated -= 360.0f;
    */
    //move position of camera based on WASD keys, and XZ keys for up and down
   /* const float moveSpeed = 2.0; //units per second
    if(glfwGetKey('S')){
        gCamera.offsetPosition(secondsElapsed * moveSpeed * -gCamera.forward());
    } else if(glfwGetKey('W')){
        gCamera.offsetPosition(secondsElapsed * moveSpeed * gCamera.forward());
    }
    if(glfwGetKey('A')){
        gCamera.offsetPosition(secondsElapsed * moveSpeed * -gCamera.right());
    } else if(glfwGetKey('D')){
        gCamera.offsetPosition(secondsElapsed * moveSpeed * gCamera.right());
    }
    if(glfwGetKey('Z')){
        gCamera.offsetPosition(secondsElapsed * moveSpeed * -glm::vec3(0,1,0));
    } else if(glfwGetKey('X')){
        gCamera.offsetPosition(secondsElapsed * moveSpeed * glm::vec3(0,1,0));
    }*/
    if(glfwGetKey('R')){
        gCamera.setNearAndFarPlanes(gCamera.nearPlane() + 0.001f , gCamera.farPlane() );
    }
    if(glfwGetKey('G')){
        gCamera.setNearAndFarPlanes(gCamera.nearPlane() - 0.001f, gCamera.farPlane() );
    }

    //increase or decrease field of view based on mouse wheel
    /*const float zoomSensitivity = -0.2;

    float fieldOfView = gCamera.fieldOfView() + zoomSensitivity * 10 *(float)glfwGetMouseWheel();
    if(fieldOfView < 5.0f) fieldOfView = 5.0f;
    if(fieldOfView > 130.0f) fieldOfView = 130.0f;
    gCamera.setFieldOfView(fieldOfView);
    glfwSetMouseWheel(0);*/
}

// the program starts here
void AppMain(int screenX,int screenY,int camX,int camY,float n,float fB) {
    // initialise GLFW
    if(!glfwInit())
        throw std::runtime_error("glfwInit failed");
    
    // open a window with GLFW
    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
    glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
    if(!glfwOpenWindow((int)SCREEN_SIZE.x, (int)SCREEN_SIZE.y
                        /*screenX, screenY*/, 8, 8, 8, 8, 16, 0, /*GLFW_WINDOW*/GLFW_FULLSCREEN))
        throw std::runtime_error("glfwOpenWindow failed. Can your hardware handle OpenGL 3.2?");
    
    // GLFW settings
    //glfwDisable(GLFW_MOUSE_CURSOR);
    glfwSetMousePos(0, 0);
    glfwSetMouseWheel(0);

    init(); //init cam module Opencv

    // initialise GLEW
    glewExperimental = GL_TRUE; //stops glew crashing on OSX :-/
    if(glewInit() != GLEW_OK)
        throw std::runtime_error("glewInit failed");
    
    // GLEW throws some errors, so discard all the errors so far
    while(glGetError() != GL_NO_ERROR) {}

    // print out some info about the graphics drivers
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

    // make sure OpenGL version 3.2 API is available
    if(!GLEW_VERSION_3_2)
        throw std::runtime_error("OpenGL 3.2 API is not available.");

    // OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // load vertex and fragment shaders into opengl
    LoadShaders();

    // load the texture
    LoadTexture();

    // create buffer and fill it with the points of the triangle
    float ar = (float)screenY/(float)screenX;
    LoadModel();
    LoadCube(n,fB,ar);

    // setup gCamera
    gCamera.setPosition(glm::vec3(0,0,4));
    gCamera.setViewportAspectRatio(SCREEN_SIZE.x / SCREEN_SIZE.y);

    // run while the window is open
    double lastTime = glfwGetTime();
    while(glfwGetWindowParam(GLFW_OPENED)){
        // update the scene based on the time elapsed since last update
        double thisTime = glfwGetTime();
        Update(thisTime - lastTime);
        lastTime = thisTime;
        
        // draw one frame
        Render();

        // check for errors
        GLenum error = glGetError();
        if(error != GL_NO_ERROR)
            glPrintError();

        //exit program if escape key is pressed
        if(glfwGetKey(GLFW_KEY_ESC)) {
            finalize(); //finalize cam module Opencv
            glfwCloseWindow();
        }
    }

    // clean up and exit
    glfwTerminate();
}


int main(int argc, char *argv[]) {
    try {
        int screenX = atoi(argv[1]);
        int screenY = atoi(argv[2]);
        int camX = atoi(argv[3]);
        int camY = atoi(argv[4]);
        float n = atof(argv[5]);
        float fB =atof(argv[6]);

        gCamera.init(screenX,screenY,camX,camY,n);
        std::cerr << screenX << std::endl;
        AppMain(screenX,screenY,camY,camY,n,fB);
    } catch (const std::exception& e){
        std::cerr << "ERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}