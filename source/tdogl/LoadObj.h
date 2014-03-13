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

#pragma once

#include <glm/glm.hpp>
 #include <iostream>
  #include <vector>

 namespace tdogl {

 	class LoadObj {
 		public:
 			LoadObj();

 			bool loadMtl( std::string filename,
									float Ka[],    
    								float Kd[],    
    								float Ks[],    
    								float &Ns);

 			int loadObj(const std::string filename,
    				std::vector < glm::vec3 > & out_vertices,
    				std::vector < glm::vec2 > & out_uvs,
    				std::vector < glm::vec3 > & out_normals,
    				int n);

 		private:
 	};
 }