#pragma once

#include "mesh.h"
#include "texture.h"
#include "mypersonalmathlib.h"
#include "debugrenderer.h"
#include <vector>

/**
@class ClipmapGrid

Geometry clipmaps
-updates and renders the grid 
*/
class ClipmapGrid
{
public:
	/// Constructor
	ClipmapGrid(unsigned int size, unsigned int levels, float clip_scale);
	/// Destructor
	~ClipmapGrid(void);
	
	/// Updates the model/view matrix and then calls its childrens' update
	void Update(Matrix44& view);
	void update_level_offsets(const Vector2& camera_pos);

	/// Does nothing with this class. Just calls its childrens' update and forwards the arguments
	void Render(const Matrix44& projection, const Matrix44& view);

	/// Returns the model
	Matrix44 GetModel();

	/// Multiply the transform's translation matrix with a Translation matrix
	void Translate(float x, float y, float z);
	/// Multiply the transform's translation matrix with a Translation matrix
	void Translate(Vector3 vector);
	
	void SetPosition( float x, float y, float z );
	void SetPosition( Vector3 vector);
	
	/// Multiply the transform's scaling matrix with a Scaling matrix. Saves the scale factor so the bounding sphere can be scaled elsewhere in the program. Scales equally for xyz axis
	void Scale(float factor);
	/// Sets the scale values in the transform's matrix to factor
	/// Sets the scaling matrix to identity before multiplying it with a Scaling matrix. Saves the scale factor so the bounding sphere can be scaled elsewhere in the program. Scales equally for xyz axis
	void SetScale(float factor);
	/// Multiply the scaling matrix with a Scaling matrix. Doesn't have to scale uniformly
	void Scale(float x, float y, float z);
	/// Multiply the scaling matrix with a Scaling matrix. Doesn't have to scale uniformly
	void Scale (Vector3 vector);

	//My Physics Stuff
	Vector3 position;


	//Position and scale as just vectors
	Matrix44 model;
	Matrix44 scaling;

private:
	//vertices, uvs, normals, indices
	std::vector<Vector3> vertices;
	//std::vector<Vector2> uvs;
	//std::vector<Vector3> normals;
	std::vector<unsigned int> indices;

	//vao, vbo, ibo, ubo
	GLuint vao, vertex_buffer, index_buffer, uniform_buffer;
	unsigned int size;
	unsigned int level_size;
	unsigned int levels;
	size_t uniform_buffer_size; //??
	size_t num_indices; //??

	float clipmap_scale;

	struct Block
	{
		size_t offset;
		size_t count;
		Vector2 range;
	};

	struct InstanceData
	{
		Vector2 offset; // Offset of the block in XZ plane (world space). This is prescaled.
		Vector2 texture_scale; // Scale factor of local offsets (vertex coordinates) translated into texture coordinates.
		Vector2 texture_offset; // Offset for texture coordinates, similar to offset. Also prescaled.
		float scale; // Scale factor of local offsets (vertex coordinates).
		float level; // Clipmap LOD level of block.
		//float debug_color = 1;
	};

	//blocks
	Block block;
	Block vertical;
	Block horizontal;
	Block trim_full;
	Block trim_top_right;
	Block trim_bottom_right;
	Block trim_bottom_left;
	Block trim_top_left;
	Block degenerate_left;
	Block degenerate_top;
	Block degenerate_right;
	Block degenerate_bottom;

	//setup buffers
	void setup_vertex_buffer(unsigned int size);
	void setup_index_buffer(unsigned int size);
	void setup_block_ranges(unsigned int size);
	void setup_uniform_buffer();
	void setup_vertex_array();

	//update the instanced draw list structure
	//render the instanced draw list structure
	void update_draw_list();
	void render_draw_list();

	struct DrawInfo
	{
		size_t index_buffer_offset;
		size_t uniform_buffer_offset;
		unsigned int indices;
		unsigned int instances;
		Vector3 debug_color;
	};
	std::vector<DrawInfo> draw_list;

	GLint uniform_buffer_align;

	std::vector<Vector2> level_offsets;

	typedef bool(*TrimConditional)(const Vector2& offset);

	Vector2 get_offset_level(const Vector2& camera_pos, unsigned int level); //snapping grid
	void update_draw_list(DrawInfo& info, size_t& ubo_offset);
	DrawInfo get_draw_info_blocks(InstanceData *instance_data);
	DrawInfo get_draw_info_vert_fixup(InstanceData *instance_data);
	DrawInfo get_draw_info_horiz_fixup(InstanceData *instance_data);
	DrawInfo get_draw_info_degenerate(InstanceData *instance_data, const Block& block, const Vector2& offset, const Vector2& ring_offset);
	DrawInfo get_draw_info_degenerate_left(InstanceData *instance_data);
	DrawInfo get_draw_info_degenerate_right(InstanceData *instance_data);
	DrawInfo get_draw_info_degenerate_top(InstanceData *instance_data);
	DrawInfo get_draw_info_degenerate_bottom(InstanceData *instance_data);
	DrawInfo get_draw_info_trim_full(InstanceData *instance_data);
	DrawInfo get_draw_info_trim(InstanceData *instance_data, const Block& block, TrimConditional cond);
	DrawInfo get_draw_info_trim_top_right(InstanceData *instance_data);
	DrawInfo get_draw_info_trim_top_left(InstanceData *instance_data);
	DrawInfo get_draw_info_trim_bottom_right(InstanceData *instance_data);
	DrawInfo get_draw_info_trim_bottom_left(InstanceData *instance_data);
};