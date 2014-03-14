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

#include <dirent.h> //search files in directory 
#include <sys/stat.h>
#include <list> //list for models
#include <vector>
#include <cstring>

// tdogl classes
#include "Helper.h"
#include "tdogl/Program.h"
#include "tdogl/Texture.h"
#include "tdogl/Camera.h"


#include "tdogl/LoadObj.h" //obj loader

#include "face.h" //opencv module





// constants
//#define M_PI 3.1415926535897932384626433832795
const glm::vec2 SCREEN_SIZE(1920, 1080);

//model with all attributes
struct ModelAsset {
    tdogl::Program* shaders;
    tdogl::Texture* texture;
    GLuint vbo_v,vbo_n,vbo_uv; //vbo for vertices,normals and uv
    GLuint vao;
    GLenum drawType;
    GLint drawStart;
    GLint drawCount;

    ModelAsset() :
        shaders(NULL),
        texture(NULL),
        vbo_v(0),
        vbo_n(0),
        vbo_uv(0),
        vao(0),
        drawType(GL_TRIANGLES),
        drawStart(0),
        drawCount(6*3*2)
    {}
};

typedef struct {
    GLfloat Ka[3];    // Ambient colour
    GLfloat Kd[3];    // Diffuse colour
    GLfloat Ks[3];    // Specular colour
    GLfloat Ns;       // Specular (coeff)
}Material;


//contains model and transformation
struct ModelInstance {
    ModelAsset* asset;
    glm::mat4 transform;
    Material* mtl;

    ModelInstance() :
        asset(NULL),
        transform(),
        mtl(NULL)
    {}
};

struct Light {
    glm::vec3 position;
    glm::vec3 intensities; //a.k.a. the color of the light
};
Light gLight;

std::vector<ModelInstance> models;

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

static tdogl::Program* LoadShaders(const char* vertFilename, const char* fragFilename) {
    std::vector<tdogl::Shader> shaders;
    shaders.push_back(tdogl::Shader::shaderFromFile(ResourcePath(vertFilename), GL_VERTEX_SHADER));
    shaders.push_back(tdogl::Shader::shaderFromFile(ResourcePath(fragFilename), GL_FRAGMENT_SHADER));
    return new tdogl::Program(shaders);
}

static void SearchModels( std::vector < std::string > &files ) {
    DIR *dir;
    std::string dirname = ResourcePath("Models");
    const char *d = dirname.c_str();
    dir = opendir (d);
    struct dirent *dirp;
    struct stat filestat;
    std::string filepath;
    int i = 0;

    while ( (dirp = readdir( dir ))) {
        
        filepath = dirname + "/" + dirp->d_name;
        
        if (stat( filepath.c_str(), &filestat )) continue;
        if (S_ISDIR( filestat.st_mode ))         continue;
        
        //check if path ends with ".obj"
        if (filepath.length() >= 4) {
            if (0 == filepath.compare (filepath.length() - 4, 4, ".obj")) {
                std::cerr << i++ << "," << filepath << std::endl;
                    files.push_back( filepath );
            }
        }
        /*if ( filepath.find(".obj") != std::string::npos ) {
            if ( filepath.find(".obj~") == std::string::npos ) {
                std::cerr << i++ << "," << filepath << std::endl;
                files.push_back( filepath );
            }
        }*/
    }
}


static void LoadModels(std::vector < std::string > files,int n) {
    int count = 0;
    

    while (!files.empty()) {
        std::string file = files.back();
        files.pop_back();


        ModelAsset* model = new ModelAsset();
        model->shaders = LoadShaders("vertex-shader.txt", "fragment-shader.txt");
        model->drawType = GL_TRIANGLES;
        model->texture = gTexture1;


        std::vector< glm::vec3 > vertices;
        std::vector< glm::vec2 > uvs;
        std::vector< glm::vec3 > normals; // Won't be used at the moment.
        int res = load.loadObj(file, vertices, uvs, normals,n);

        if ( res < 1 ) {
            std::cerr << "Error loading " << file << std::endl;
            continue;
        }

        glGenBuffers(1, &model->vbo_v);
        glGenVertexArrays(1, &model->vao);

         // bind the VAO
        glBindVertexArray(model->vao);
         // bind the VBO
        glBindBuffer(GL_ARRAY_BUFFER, model->vbo_v);

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
                         &vertices[0], GL_STATIC_DRAW);
        
        model->drawCount = vertices.size();
        std::cerr << model->drawCount << "," << vertices.size() << std::endl;
        
        glEnableVertexAttribArray(model->shaders->attrib("vert"));
        glVertexAttribPointer(model->shaders->attrib("vert"), 3, GL_FLOAT,
                 GL_FALSE, 3*sizeof(GLfloat), NULL);

        
        if (res == 1 || res == 2) {
        //make and bind vbo for normals
            glGenBuffers(1, &model->vbo_n);
            glBindBuffer(GL_ARRAY_BUFFER, model->vbo_n);
            
            glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3),
                         &normals[0], GL_STATIC_DRAW); //normals is std::vector<float>
            
            glEnableVertexAttribArray(model->shaders->attrib("vertNormal"));
        glVertexAttribPointer(model->shaders->attrib("vertNormal"), 3, GL_FLOAT,
                 GL_TRUE, 3*sizeof(GLfloat), NULL);
            //glNormalPointer( GL_FLOAT, 3*sizeof(GLfloat) , NULL);

        }
        //make and bind vbo for uv coordinates

        if ( res == 1 || res == 3) {
            glGenBuffers(1, &model->vbo_uv);
            glBindBuffer(GL_ARRAY_BUFFER, model->vbo_uv);
            glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec3),
             &uvs[0], GL_STATIC_DRAW); //normals is std::vector<float>
            
            glEnableVertexAttribArray(model->shaders->attrib("vertTexCoord"));
            glVertexAttribPointer(model->shaders->attrib("vertTexCoord"), 2, GL_FLOAT,
                         GL_TRUE,  2*sizeof(GLfloat), NULL);

        }
        // unbind the VAO
        glBindVertexArray(0);

/*
        std::cerr << "MODELLO " << count++ << std::endl;
            
        ModelInstance m = ModelInstance(model);
        //m.asset = &model;
        
        m.transform = glm::mat4(); //no transform
        models.push_back(m);*/


        ModelInstance m = ModelInstance();

        m.asset = model;
        m.transform = glm::mat4();

        /***LOAD MATERIAL ***/
        /*
        const std::string mtl = file.replace(file.length() - 4,
                                                 4, ".mtl");
        GLfloat Ka[3];
        GLfloat Kd[3];
        GLfloat Ks[3];
        GLfloat Ns;  

        
        bool ret = load.loadMtl(file,Ka,Kd,Ks,Ns);

            if (ret) {

                std::cerr << "Mtl Loaded " << std::endl;
                Material* mtl = new Material();
                memcpy ( &mtl->Ka, &Ka, sizeof(Ka) );
                memcpy ( &mtl->Kd, &Kd, sizeof(Kd) );
                memcpy ( &mtl->Ks, &Ks, sizeof(Ks) );
                mtl->Ns = Ns;
                m.mtl = mtl;
            }
*/

        
        
        models.push_back(m);
       
    }

    std::cerr<< "NUM COUNT: " << count << std::endl;




}

static void LoadModel() {
    



    // Read our .obj file
    std::vector< glm::vec3 > vertices;
    std::vector< glm::vec2 > uvs;
    std::vector< glm::vec3 > normals; // Won't be used at the moment.
    bool res = load.loadObj(ResourcePath("cuboT.obj"), vertices, uvs, normals,2);

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
    float y1 = 0.15f;
    float z1 = n + 0.4f;

    float x2 =  0.0f;
    float y2 =  0.2f;
    float z2 = n - 1.6f;

   
    std::cerr << l << " , " << r << " , " << t << " , " << b << std::endl;
    std::cerr << n << " , " << f << std::endl;

    
    // Make a cube out of triangles (two triangles per side)
    GLfloat vertexData[] = {
        //  X     Y     Z       U     V

      
       /*
        // bottom
        x1   , y1   , z1   , 0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
        x1+la, y1   , z1   , 1.0f, 0.0f,   0.0f, -1.0f, 0.0f,
        x1+la, y1   , z1-la, 1.0f, 1.0f,   0.0f, -1.0f, 0.0f,
        x1   , y1   , z1-la, 0.0f, 1.0f,   0.0f, -1.0f, 0.0f,
        x1   , y1   , z1   , 0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
        x1+la, y1   , z1-la, 1.0f, 1.0f,   0.0f, -1.0f, 0.0f,

        // top
        x1   , y1+la, z1   , 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        x1+la, y1+la, z1   , 1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        x1+la, y1+la, z1-la, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f,
        x1   , y1+la, z1-la, 0.0f, 1.0f,   0.0f, 1.0f, 0.0f,
        x1   , y1+la, z1   , 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        x1+la, y1+la, z1-la, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f,

        // front
        x1   , y1   , z1   , 0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
        x1+la, y1   , z1   , 1.0f, 0.0f,   0.0f, 0.0f, 1.0f,
        x1+la, y1+la, z1   , 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        x1   , y1+la, z1   , 0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        x1   , y1   , z1   , 0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
        x1+la, y1+la, z1   , 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,

        // back
        x1   , y1   , z1-la, 0.0f, 0.0f,   0.0f, 0.0f, -1.0f,
        x1+la, y1   , z1-la, 1.0f, 0.0f,   0.0f, 0.0f, -1.0f,
        x1+la, y1+la, z1-la, 1.0f, 1.0f,   0.0f, 0.0f, -1.0f,
        x1   , y1+la, z1-la, 0.0f, 1.0f,   0.0f, 0.0f, -1.0f,
        x1   , y1   , z1-la, 0.0f, 0.0f,   0.0f, 0.0f, -1.0f,
        x1+la, y1+la, z1-la, 1.0f, 1.0f,   0.0f, 0.0f, -1.0f,

        // left
        x1   , y1   , z1   , 0.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
        x1   , y1   , z1-la, 1.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
        x1   , y1+la, z1-la, 1.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
        x1   , y1+la, z1   , 0.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
        x1   , y1   , z1   , 0.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
        x1   , y1+la, z1-la, 1.0f, 1.0f,   -1.0f, 0.0f, 0.0f,

        // right
        x1+la, y1   , z1-la, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
        x1+la, y1   , z1   , 1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
        x1+la, y1+la, z1   , 1.0f, 1.0f,   1.0f, 0.0f, 0.0f,
        x1+la, y1+la, z1-la, 0.0f, 1.0f,   1.0f, 0.0f, 0.0f,
        x1+la, y1   , z1-la, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
        x1+la, y1+la, z1   , 1.0f, 1.0f,   1.0f, 0.0f, 0.0f,
    */
    /*
        //  X     Y     Z       U     V
          // bottom
        x2   , y2   , z2   , 0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
        x2+la, y2   , z2   , 1.0f, 0.0f,   0.0f, -1.0f, 0.0f,
        x2+la, y2   , z2-la, 1.0f, 1.0f,   0.0f, -1.0f, 0.0f,
        x2   , y2   , z2-la, 0.0f, 1.0f,   0.0f, -1.0f, 0.0f,
        x2   , y2   , z2   , 0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
        x2+la, y2   , z2-la, 1.0f, 1.0f,   0.0f, -1.0f, 0.0f,

        // top
        x2   , y2+la, z2   , 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        x2+la, y2+la, z2   , 1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        x2+la, y2+la, z2-la, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f,
        x2   , y2+la, z2-la, 0.0f, 1.0f,   0.0f, 1.0f, 0.0f,
        x2   , y2+la, z2   , 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        x2+la, y2+la, z2-la, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f,

        // front
        x2   , y2   , z2   , 0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
        x2+la, y2   , z2   , 1.0f, 0.0f,   0.0f, 0.0f, 1.0f,
        x2+la, y2+la, z2   , 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        x2   , y2+la, z2   , 0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        x2   , y2   , z2   , 0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
        x2+la, y2+la, z2   , 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,

        // back
        x2   , y2   , z2-la, 0.0f, 0.0f,   0.0f, 0.0f, -1.0f,
        x2+la, y2   , z2-la, 1.0f, 0.0f,   0.0f, 0.0f, -1.0f,
        x2+la, y2+la, z2-la, 1.0f, 1.0f,   0.0f, 0.0f, -1.0f,
        x2   , y2+la, z2-la, 0.0f, 1.0f,   0.0f, 0.0f, -1.0f,
        x2   , y2   , z2-la, 0.0f, 0.0f,   0.0f, 0.0f, -1.0f,
        x2+la, y2+la, z2-la, 1.0f, 1.0f,   0.0f, 0.0f, -1.0f,
 
        // left
        x2   , y2   , z2   , 0.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
        x2   , y2   , z2-la, 1.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
        x2   , y2+la, z2-la, 1.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
        x2   , y2+la, z2   , 0.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
        x2   , y2   , z2   , 0.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
        x2   , y2+la, z2-la, 1.0f, 1.0f,   -1.0f, 0.0f, 0.0f,

        // right
        x2+la, y2   , z2-la, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
        x2+la, y2   , z2   , 1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
        x2+la, y2+la, z2   , 1.0f, 1.0f,   1.0f, 0.0f, 0.0f,
        x2+la, y2+la, z2-la, 0.0f, 1.0f,   1.0f, 0.0f, 0.0f,
        x2+la, y2   , z2-la, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
        x2+la, y2+la, z2   , 1.0f, 1.0f,   1.0f, 0.0f, 0.0f,
    */
    

         //BOX
         // bottom
        l    , b    , n    , 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        r    , b    , n    , 1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        r    , b    , f    , 1.0f, 1.0f,   0.0f, 1.0f, 0.0f,
        l    , b    , f    , 0.0f, 1.0f,   0.0f, 1.0f, 0.0f,
        l    , b    , n    , 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        r    , b    , f    , 1.0f, 1.0f,   0.0f, 1.0f, 0.0f,

        // top
        l    , t    , n    , 0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
        r    , t    , n    , 1.0f, 0.0f,   0.0f, -1.0f, 0.0f,
        r    , t    , f    , 1.0f, 1.0f,   0.0f, -1.0f, 0.0f,
        l    , t    , f    , 0.0f, 1.0f,   0.0f, -1.0f, 0.0f,
        l    , t    , n    , 0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
        r    , t    , f    , 1.0f, 1.0f,   0.0f, -1.0f, 0.0f,
        

        // back
        l    , b    , f    , 0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
        r    , b    , f    , 1.0f, 0.0f,   0.0f, 0.0f, 1.0f,
        r    , t    , f    , 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        l    , t    , f    , 0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        l    , b    , f    , 0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
        r    , t    , f    , 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,

        // left
        l    , b    , n    , 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
        l    , b    , f    , 1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
        l    , t    , f    , 1.0f, 1.0f,   1.0f, 0.0f, 0.0f,
        l    , t    , n    , 0.0f, 1.0f,   1.0f, 0.0f, 0.0f,
        l    , b    , n    , 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
        l    , t    , f    , 1.0f, 1.0f,   1.0f, 0.0f, 0.0f,

        // right
        r    , b    , f    , 0.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
        r    , b    , n    , 1.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
        r    , t    , n    , 1.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
        r    , t    , f    , 0.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
        r    , b    , f    , 0.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
        r    , t    , n    , 1.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
   
    };
/*
    GLfloat plane[] = {
        x1 +1.5  , y1   , z1-2   , 0.0f, 0.0f,
        x1+la+1.5, y1   , z1-2   , 1.0f, 0.0f,
        x1+la+1.5, y1+la, z1-2   , 1.0f, 1.0f,
        x1  +1.5 , y1+la, z1-2   , 0.0f, 1.0f,
        x1  +1.5 , y1   , z1-2   , 0.0f, 0.0f,
        x1+la+1.5, y1+la, z1-2   , 1.0f, 1.0f,

    };*/

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

    // connect the xyz to the "vert" attribute of the vertex shader
    glEnableVertexAttribArray(gProgram->attrib("vert"));
    glVertexAttribPointer(gProgram->attrib("vert"), 3, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), NULL);

    
    // connect the uv coords to the "vertTexCoord" attribute of the vertex shader
    glEnableVertexAttribArray(gProgram->attrib("vertTexCoord"));
    glVertexAttribPointer(gProgram->attrib("vertTexCoord"), 2, GL_FLOAT, GL_TRUE,  8*sizeof(GLfloat), (const GLvoid*)(3 * sizeof(GLfloat)));
/*
    // connect the normal to the "vertNormal" attribute of the vertex shader
    glEnableVertexAttribArray(gProgram->attrib("vertNormal"));
    glVertexAttribPointer(gProgram->attrib("vertNormal"), 3, GL_FLOAT, GL_TRUE,  8*sizeof(GLfloat), (const GLvoid*)(5 * sizeof(GLfloat)));
    glBindVertexArray(0);
  */  


    /***BIND SECOND OBJECT ***/
   /* glGenVertexArrays(1, &gVAO1);
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

static void RenderInstance(const ModelInstance& inst) {
    
    ModelAsset* asset = inst.asset;
    tdogl::Program* shaders = asset->shaders;
    
    //bind the shaders
    shaders->use();

    //set the shader uniforms
    shaders->setUniform("camera", gCamera.matrix());

    shaders->setUniform("model", inst.transform);
    //bind the texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, asset->texture->object());
    shaders->setUniform("tex", 0); //set to 0 because the texture will be bound to GL_TEXTURE0
    //shaders->setUniform("light.position", gLight.position);
    //shaders->setUniform("light.intensities", gLight.intensities);

  
    
    
    //bind VAO and draw
    glBindVertexArray(asset->vao);
    glDrawArrays(asset->drawType, asset->drawStart, asset->drawCount);

    //unbind everything
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    shaders->stopUsing();

}

// draws a single frame
static void Render() {
    // clear everything
    glClearColor(0, 0, 0, 1); // black
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    

    /*** RENDER FIRST OBJECT ***/
    // bind the program (the shaders)
    gProgram->use();
    // set the "camera" uniform
    gProgram->setUniform("camera", gCamera.matrix());
    // set the "model" uniform in the vertex shader, based on the gDegreesRotated global
    gProgram->setUniform("model", glm::translate(glm::mat4(1.0f),                       
        glm::vec3(0.0f, 0.0f, 0.0f))); 
    // bind the texture and set the "tex" uniform in the fragment shader
     glActiveTexture(GL_TEXTURE0);
     glBindTexture(GL_TEXTURE_2D, gTexture->object());
     gProgram->setUniform("tex", 0); //set to 0 because the texture is bound to GL_TEXTURE0
     //gProgram->setUniform("light.position", gLight.position);
     //gProgram->setUniform("light.intensities", gLight.intensities);
    // bind the VAO (the triangle)
    glBindVertexArray(gVAO);
    // draw the VAO
    //glDrawArrays(GL_TRIANGLES, 0, 6*2*3);
    glDrawArrays(GL_TRIANGLES, 0, 5*2*3  );
    // unbind the VAO, the program and the texture
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    gProgram->stopUsing();





    /*** RENDER MODELS***/

    std::vector<ModelInstance>::iterator it;
    int count = 0;
    for(it = models.begin(); it != models.end(); ++it){
        std::cerr << "Rendering " << count++ << std::endl; 
        RenderInstance(*it);
    }
    
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
    if(!glfwOpenWindow(/*(int)SCREEN_SIZE.x, (int)SCREEN_SIZE.y*/
                        screenX, screenY, 8, 8, 8, 8, 16, 0, /*GLFW_WINDOW*/GLFW_FULLSCREEN))
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

    std::vector <std::string> files;
    SearchModels(files);

    LoadCube(n,fB,ar);

    LoadModels(files,n);
    

    //LoadModel();
    

    // setup gCamera
    gCamera.setPosition(glm::vec3(0,0,4));
    gCamera.setViewportAspectRatio((float)(screenX) / (float)(screenY));

    //gLight.position = gCamera.position();
    //gLight.position = glm::vec3( 0.5f, 0.5f, -n+2);
    //gLight.intensities = glm::vec3(1,1,1); //white

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