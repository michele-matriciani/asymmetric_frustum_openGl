#pragma once

#include <glm/glm.hpp>
 #include <iostream>


namespace tdogl {

 	class Filter {
 		public:
 			Filter();

 			bool filter( glm::vec3 &coord ,int erMin, int erMax);

 			

 		private:
 	};
 }