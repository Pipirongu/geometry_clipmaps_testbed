#version 330

// Interpolated values from the vertex shaders
in float heightvalue;

// Ouput data
layout(location = 0) out vec3 color;


void main(){
	color = vec3(heightvalue,heightvalue,heightvalue);
}
