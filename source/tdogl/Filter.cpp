#include "Filter.h"
#include <stdexcept>
#include <cmath>
#include <GL/glfw.h>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <string.h>

using namespace tdogl;

Filter::Filter() 
    
{
}


bool Filter::filter(glm::vec3 &cVec, int erMin, int erMax) {
	int coord = cVec[0];
	int p	  = cVec[1];
	int pp	  = cVec[2];

	if (coord == 0) {
		cVec[0] = p;
		return true;
	}

	int dif = ( coord - p );

	if ( ( dif >= -erMin && dif <= erMin ) ) { 
    	
		coord = p;    
    
    }

    else if ( dif >= -erMax && dif <= erMax )  {

    	if ( dif < 0 ) {
    		coord = p - (erMax + erMin)/3;
    	}
    	else {
    		coord = p + (erMax + erMin)/3;
    	}
    }   
    else {
    	p = coord;
    }  

    cVec[0] = coord;
    cVec[1] = p;
    cVec[2] = pp;

    return true;

}