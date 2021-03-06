/*
 tdogl::LoadObj
 
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

#include "LoadObj.h"
#include <stdexcept>
#include <cmath>
#include <GL/glfw.h>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <string.h>

using namespace tdogl;

LoadObj::LoadObj() 
    
{
}





bool LoadObj::loadMtl( std::string filename, 
			GLfloat Ka[],    // Ambient colour
    		GLfloat Kd[],    // Diffuse colour
    		GLfloat Ks[],    // Specular colour
    		GLfloat &Ns) {   // Specular (coeff)) {

	const char* path = filename.c_str();
	FILE* file = fopen(path, "r");

	if( file == NULL ) {
		std::cerr << "Material not founded" << std::endl;
		return false;
	}

	while( 1 ){
 
	    char lineHeader[128];
	    // read the first word of the line
	    int res = fscanf(file, "%s", lineHeader);
	    if (res == EOF)
	        break; // EOF = End Of File. Quit the loop.
	 
	    // else : parse lineHeader

	    if( strcmp( lineHeader, "Ns" ) == 0 ){

	    	int matches = fscanf(file, "%f", &Ns);
	    	if (matches != 1) {
	    		return false;
	    	}
            
        }

        else if( strcmp( lineHeader, "Ka" ) == 0 ){

	    	int matches = fscanf(file, "%f %f %f", &Ka[0], &Ka[1], &Ka[2]);
            if (matches != 3) {
	    		return false;
	    	}
        }

        else if( strcmp( lineHeader, "Kd" ) == 0 ){

	    	int matches = fscanf(file, "%f %f %f", &Kd[0], &Kd[1], &Kd[2]);
            if (matches != 3) {
	    		return false;
	    	}
        }

        else if( strcmp( lineHeader, "Ks" ) == 0 ){

	    	int matches = fscanf(file, "%f %f %f", &Ks[0], &Ks[1], &Ks[2]);
            if (matches != 3) {
	    		return false;
	    	}
        }

	}
}






int LoadObj::loadObj(const std::string filename,
    				std::vector < glm::vec3 > & out_vertices,
    				std::vector < glm::vec2 > & out_uvs,
    				std::vector < glm::vec3 > & out_normals,
    				int n) {


	std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
	std::vector< glm::vec3 > temp_vertices;
	std::vector< glm::vec2 > temp_uvs;
	std::vector< glm::vec3 > temp_normals;
	const char *path = filename.c_str();
	FILE * file = fopen(path, "r");

	bool vt = false;
	bool vn = false;
	int retval = -1;

	if( file == NULL ){
    	printf("Impossible to open the file !\n");
    return -1;
	}

	

	while( 1 ){
 
	    char lineHeader[128];
	    // read the first word of the line
	    int res = fscanf(file, "%s", lineHeader);
	    if (res == EOF)
	        break; // EOF = End Of File. Quit the loop.
	 
	    // else : parse lineHeader

	    if ( strcmp( lineHeader, "v" ) == 0 ){
		    glm::vec3 vertex;
		    int r = fscanf(file, "%f %f %f\n", &vertex.x , &vertex.y, &vertex.z );
		    vertex.z -= n-2;
		    /*
		    vertex.x = ((float)((int)(vertex.x*100)))/100;
		    vertex.y = ((float)((int)(vertex.y*100)))/100;
		    vertex.z = ((float)((int)(vertex.z*100)))/100;*/
		    /*
		    vertex.x = (int)vertex.x;
		    vertex.y = (int)vertex.y;
		    vertex.z = (int)vertex.z;
		    */temp_vertices.push_back(vertex);

	    }
	    else if ( strcmp( lineHeader, "vt" ) == 0 ){
	    	vt = true;
		    glm::vec2 uv;
		    int r = fscanf(file, "%f %f\n", &uv.x, &uv.y );
		    temp_uvs.push_back(uv);
		}
		else if ( strcmp( lineHeader, "vn" ) == 0 ){
		    vn = true;
		    glm::vec3 normal;
		    int r = fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
		    temp_normals.push_back(normal);
		}

		
		else if ( strcmp( lineHeader, "f" ) == 0 ){

			std::string vertex1, vertex2, vertex3;
		    unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];

			if ( vt && vn  ) { //format %d/%d/%d

				retval = 1;				
				int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
		    	
		    	if (matches != 9){
		        	printf(" %d File can't be read by our simple parser : ( Try exporting with other options\n",matches);
		        	return -1;
		    	}

		    	vertexIndices.push_back(vertexIndex[0]);
			    vertexIndices.push_back(vertexIndex[1]);
			    vertexIndices.push_back(vertexIndex[2]);
			    
			    uvIndices    .push_back(uvIndex[0]);
			    uvIndices    .push_back(uvIndex[1]);
			    uvIndices    .push_back(uvIndex[2]);

			    normalIndices.push_back(normalIndex[0]);
			    normalIndices.push_back(normalIndex[1]);
			    normalIndices.push_back(normalIndex[2]);

			}

			else if ( !vt && vn ) { //format %d//%d

				retval = 2;
				int matches = fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1],  &normalIndex[1], &vertexIndex[2], &normalIndex[2] );
		    	
		    	if (matches != 6){
		        	printf(" %d File can't be read by our simple parser : ( Try exporting with other options\n",matches);
		        	return -1;
		    	}

		    	vertexIndices.push_back(vertexIndex[0]);
			    vertexIndices.push_back(vertexIndex[1]);
			    vertexIndices.push_back(vertexIndex[2]);
			   

			    normalIndices.push_back(normalIndex[0]);
			    normalIndices.push_back(normalIndex[1]);
			    normalIndices.push_back(normalIndex[2]);
			}

			else if ( vt && !vn ) { //format %d/%d
				
				retval = 3;
				int matches = fscanf(file, "%d/%d %d/%d %d/%d\n", &vertexIndex[0], &uvIndex[0],  &vertexIndex[1], &uvIndex[1],  &vertexIndex[2], &uvIndex[2] );
		    	
		    	if (matches != 6){
		        	printf(" %d File can't be read by our simple parser : ( Try exporting with other options\n",matches);
		        	return -1;
		    	}

		    	vertexIndices.push_back(vertexIndex[0]);
			    vertexIndices.push_back(vertexIndex[1]);
			    vertexIndices.push_back(vertexIndex[2]);
			    /*** comment uvIndices if uv are not included***/
			    uvIndices    .push_back(uvIndex[0]);
			    uvIndices    .push_back(uvIndex[1]);
			    uvIndices    .push_back(uvIndex[2]);

			    

			}

			else if ( !vt && !vn) { //format %d

				retval = 4;
				int matches = fscanf(file, "%d %d %d\n", &vertexIndex[0],  &vertexIndex[1],  &vertexIndex[2] );
		    	
		    	if (matches != 3){
		        	printf(" %d File can't be read by our simple parser : ( Try exporting with other options\n",matches);
		        	return -1;
		    	}

		    	vertexIndices.push_back(vertexIndex[0]);
			    vertexIndices.push_back(vertexIndex[1]);
			    vertexIndices.push_back(vertexIndex[2]);
		
			}

		
			else {

				return -1;
			}

		}

	}
	// For each vertex of each triangle
	for( unsigned int i=0; i<vertexIndices.size(); i++ ){
			unsigned int vertexIndex = vertexIndices[i];
			glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];
			/*std::cout << i << "   "<< vertex.x << "," << vertex.y << 
		    "," << vertex.z << std::endl;*/
			out_vertices.push_back(vertex);
		}

	for( unsigned int i=0; i<normalIndices.size(); i++ ){
		unsigned int normalIndex = normalIndices[i];
		glm::vec3 vertex = temp_normals[ normalIndex-1 ];
		/*std::cerr << i << "   " << vertex.x << "," << vertex.y << 
	    "," << vertex.z << std::endl;*/
		out_normals.push_back(vertex);
	}

	for( unsigned int i=0; i<uvIndices.size(); i++ ){
		unsigned int uvIndex = uvIndices[i];
		glm::vec2 vertex = temp_uvs[ uvIndex-1 ];
		/*std::cerr << i << "   " << vertex.x << "," << vertex.y << 
	    std::endl;*/
		out_uvs.push_back(vertex);
	}

	fclose (file);

	return retval;


}