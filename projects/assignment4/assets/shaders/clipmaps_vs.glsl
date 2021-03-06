#version 450
//uniform mediump sampler2DArray height_map;
uniform sampler2D height_map;

uniform mat4 mvp;
uniform mat4 model;
uniform vec3 uCameraPos;
uniform float uInvLevelSize[10]; // GL doesn't allow unsized array when accessed from non-constant.

struct PerInstanceData
{
	vec2 offset; // World-space offset in XZ plane.
	float scale; // Scaling factor for vertex offsets (per-instance)
	float level; // lod-level to use when sampling heightmap
	vec3 debug_color;
};

layout(std140, binding = 0) uniform InstanceData // Use std140 packing rules for uniform block. set binding point to 0
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
out vec3 frag_debug_color;
out float height_value;

void main()
{
	vec2 local_offset = aVertex * instance[gl_InstanceID].scale;
	vec2 pos = instance[gl_InstanceID].offset + local_offset;

	float level = instance[gl_InstanceID].level;
	// vec2 tex_offset = (aVertex + 0.5) * instance[gl_InstanceID].texture_scale; // 0.5 offset to sample mid-texel.
	// vec2 texcoord = instance[gl_InstanceID].texture_offset + tex_offset;

	
	vec2 samplingt = pos/4096;
	
	// Find blending factors for heightmap. The detail level must not have any discontinuities or it shows as 'artifacts'.
	// vec2 dist = abs(pos - uCameraPos.xz) * uInvLevelSize[int(level)];
	// vec2 a = clamp((dist - 0.325) * 8.0, 0.0, 1.0);
	// float lod_factor = max(a.x, a.y);
	// //sample twice level and level+1
	// float height1 = textureLod(height_map, samplingt, level).x;
	// float height2 = textureLod(height_map, samplingt, level+1).x;
	// float height = mix(height1, height2, lod_factor);

	//height = clamp(height, HEIGHTMAP_MIN, HEIGHTMAP_MAX); // To ensure frustum culling assumptions are met.
	
	float height = textureLod(height_map, samplingt, level).x;
	vec4 vert = vec4(pos.x, 100*height, pos.y, 1.0);

	gl_Position = mvp * vert;
	
	frag_debug_color = instance[gl_InstanceID].debug_color;
	height_value = height;
	
	//vHeight = height;
	//vLod = vec2(level, lod_factor);

	//vec3 dist_camera = uCameraPos - vert.xyz;
	//vFog = clamp(dot(dist_camera, dist_camera) / 250000.0, 0.0, 1.0); // Simple per-vertex fog.
}