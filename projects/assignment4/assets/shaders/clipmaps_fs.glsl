// #version 330

// // Interpolated values from the vertex shaders
// in float heightvalue;

// // Ouput data
// layout(location = 0) out vec3 color;


// void main(){
	// //color = vec3(heightvalue,heightvalue,heightvalue);
	// color = vec3(1,1,1);
// }

#version 330
layout(std140) uniform;
//precision highp float;

in float height_value;
//in float frag_debug_color;
layout(location = 0) out vec3 color;
// in float vHeight;
// in vec2 vLod;
// in float vFog;

// // Compress (-inf, +inf) to (0, 1).
// float map_height(float h)\n"
// {
	// return 1.0 / (1.0 + exp(-h / 20.0));
// }

// Make the heightmap look somewhat cloudy and fluffy.

void main()
{
	//vec3 color = vec3(1.2, 1.2, 1.0) * vec3(map_height(vHeight) + (vLod.x + vLod.y) * 0.1);
	//vec3 final_color = mix(color, vec3(0.5), vFog);
	color = vec3(1,0,0);
	//color = vec3(height_value,height_value,height_value);//vec3(1,1,1);
}