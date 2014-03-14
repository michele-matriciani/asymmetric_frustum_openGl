#include "Filter.h"
#include <GL/glfw.h>
#include <vector>
#include <iostream>


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
    	//p = coord;
    	pp = p;

    }   
    else {
    	p = coord;
    	pp = p;
    }  

    cVec[0] = coord;
    cVec[1] = p;
    cVec[2] = pp;

    return true;

}