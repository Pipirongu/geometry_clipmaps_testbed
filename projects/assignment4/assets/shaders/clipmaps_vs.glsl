// #version 330

// // Input vertex data, different for all executions of this shader.
// layout(location = 0) in vec3 vertex_position_modelspace;
// layout(location = 1) in vec2 vertex_uv;

// // Output data ; will be interpolated for each fragment.
// out float heightvalue;

// // Values that stay constant for the whole mesh.
// uniform mat4 mvp;
// uniform mat4 view;
// uniform mat4 model;
// uniform mat4 modelview;

// uniform float height_scale;
// uniform float step_size;

// uniform sampler2D height_map;

// void main(){
	// // float height = texture(height_map, vertex_uv).r;
	// // // float height = texture(height_map, vec2(x_coord, z_coord)).r;
	
	// // // vec4 vertex = vec4(vertex_position_modelspace.x, step_size*height_scale*height, vertex_position_modelspace.z, 1);
	// // vec4 vertex = vec4(vertex_position_modelspace.x, 0, vertex_position_modelspace.z, 1);
	// // //Output position of vertex in clip space
	// // gl_Position = mvp * vertex;

	// // //UVs are sent to the fragment shader
	// // heightvalue = height;
	

// }

#version 330
layout(std140) uniform;

//uniform mediump sampler2DArray height_map;
uniform sampler2D height_map;

uniform mat4 mvp;
//uniform vec3 uCameraPos;
//uniform float uInvLevelSize[10]; // GL doesn't allow unsized array when accessed from non-constant.

struct PerInstanceData
{
	vec2 offset; // World-space offset in XZ plane.
	vec2 texture_scale;
	vec2 texture_offset; // Same as for world-space offset/scale, just for texture coordinates
	float scale; // Scaling factor for vertex offsets (per-instance)
	float level; // LOD-level to use when sampling heightmap
	float debug_color;
};

uniform InstanceData
{
	PerInstanceData instance[256];
};

#define LOCATION_VERTEX 0
#define HEIGHTMAP_MIN -20.0 // Depends on the heightmap.
#define HEIGHTMAP_MAX 20.0

layout(location = LOCATION_VERTEX) in vec2 aVertex;
// out float vHeight;
// out vec2 vLod;
// out float vFog;
//out float frag_debug_color;
out float height_value;

void main()
{
	vec2 local_offset = aVertex * instance[gl_InstanceID].scale;
	vec2 pos = instance[gl_InstanceID].offset + local_offset;

	float level = instance[gl_InstanceID].level;
	vec2 tex_offset = (aVertex + 0.5) * instance[gl_InstanceID].texture_scale; // 0.5 offset to sample mid-texel.
	vec2 texcoord = instance[gl_InstanceID].texture_offset + tex_offset;

	//vec4 heights = textureLod(height_map, texcoord, level);
	//vec2 heights = texture(height_map, vec3(texcoord, level)).rg;
	//float height = texture(height_map, texcoord).r;

	// Find blending factors for heightmap. The detail level must not have any discontinuities or it shows as 'artifacts'.
	// vec2 dist = abs(pos - uCameraPos.xz) * uInvLevelSize[int(level)];
	// vec2 a = clamp((dist - 0.325) * 8.0, 0.0, 1.0);
	// float lod_factor = max(a.x, a.y);
	// float height = mix(heights.x, heights.y, lod_factor);

	//height = clamp(height, HEIGHTMAP_MIN, HEIGHTMAP_MAX); // To ensure frustum culling assumptions are met.

	vec4 vert = vec4(pos.x, 0, pos.y, 1.0); //0 is height

	gl_Position = mvp * vert;
	
	//frag_debug_color = instance[gl_InstanceID].debug_color;
	//height_value = height;
	
	//vHeight = height;
	//vLod = vec2(level, lod_factor);

	//vec3 dist_camera = uCameraPos - vert.xyz;
	//vFog = clamp(dot(dist_camera, dist_camera) / 250000.0, 0.0, 1.0); // Simple per-vertex fog.
}