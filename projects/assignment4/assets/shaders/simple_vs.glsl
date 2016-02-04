#version 330

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertex_position_modelspace;
layout(location = 1) in vec2 vertex_uv;

// Output data ; will be interpolated for each fragment.
out float heightvalue;

// Values that stay constant for the whole mesh.
uniform mat4 mvp;
uniform mat4 view;
uniform mat4 model;
uniform mat4 modelview;

uniform sampler2D height_map;

void main(){
	// float x_coord = vertex_position_modelspace.x * 0.001 * float(64-1);
    // float z_coord = vertex_position_modelspace.z * 0.001 * float(64-1);
	float x_coord = 0.5 * vertex_position_modelspace.x + 0.5;
    float z_coord = 0.5 * vertex_position_modelspace.z + 0.5;
	// float x_coord = vertex_position_modelspace.x * (1.f/63.f) + 63;
    // float z_coord = vertex_position_modelspace.z * (1.f/63.f) + 63;

	
	float height = texture(height_map, vec2(x_coord, z_coord)).r;
	
	vec4 vertex = vec4(vertex_position_modelspace.x, height, vertex_position_modelspace.z, 1);
	//Output position of vertex in clip space
	gl_Position = mvp * vertex;

	//UVs are sent to the fragment shader
	heightvalue = height;
}
