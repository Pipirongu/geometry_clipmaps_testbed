#include "clipmapgrid.h"

ClipmapGrid::ClipmapGrid(unsigned int size, unsigned int levels, float clip_scale)
{
	this->size = size;
	this->level_size = 4 * size - 1; // mm that's (n) from the paper (4 blocks * size of each block - 1) = 255
	this->levels = levels;
	this->clipmap_scale = clip_scale;
	this->syncObj = 0;

	this->setup_vertex_buffer(size);
	this->setup_index_buffer(size);
	//this->setup_block_ranges(size); //for frustum culling
	this->setup_uniform_buffer();
	this->setup_vertex_array();

	// UBOs must be bound with aligned length and offset, and it varies per vendor.
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniform_buffer_align);
}

ClipmapGrid::~ClipmapGrid(void)
{
}

void ClipmapGrid::Render(const Matrix44& projection, const Matrix44& view)
{
	/************************************************************************/
	/* Update Draw List                                                     */
	/************************************************************************/
	this->update_draw_list();






	/************************************************************************/
	/* Render Draw List                                                     */
	/************************************************************************/
	//Form the mvp matrix from matrices supplies with draw function
	Matrix44 model_matrix = Matrix44::Translation(Vector3(0, 0, 0));
	Matrix44 mvp = projection * view * model_matrix;


	Matrix44 view_matrix = view;
	Matrix44 modelview_matrix = view * model_matrix;

	//Send matrices to shader uniforms
	glUniformMatrix4fv(ShaderManager::Instance()->model_loc, 1, GL_TRUE, &model_matrix[0][0]);
	glUniformMatrix4fv(ShaderManager::Instance()->view_loc, 1, GL_TRUE, &view_matrix[0][0]);
	glUniformMatrix4fv(ShaderManager::Instance()->modelview_loc, 1, GL_TRUE, &modelview_matrix[0][0]);
	glUniformMatrix4fv(ShaderManager::Instance()->mvp_loc, 1, GL_TRUE, &mvp[0][0]);


	//! [Placeholder render code]
	//Bind this mesh VAO
	glBindVertexArray(this->vao);
	this->render_draw_list();
	//Draw the triangles using the index buffer(EBO)
	//glDrawElements(GL_TRIANGLE_STRIP, this->indices.size(), GL_UNSIGNED_INT, 0);
	//glDrawArrays(GL_TRIANGLE_STRIP, 0, this->vertices.size());
	//Unbind the VAO
	glBindVertexArray(0);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);

	//! [Placeholder render code]
}

void ClipmapGrid::setup_vertex_buffer(unsigned int size)
{
	// The ground consists of many smaller tesselated quads.
	// These smaller quads can be instanced to stamp out a big area (clipmap) where quads further away from camera
	// can be larger, and hence, less detail.
	// The grid is completely flat (XZ-plane), but they are offset in Y direction with a heightmap in vertex shader.
	// We also need padding/fixup regions to fill the missing space which
	// shows up when the clipmap is put together.

	// See Doxygen for an illustration on how these blocks are laid out to form the terrain.

	unsigned int num_vertices = size * size; // Regular block

	// Ring fixups (vertical and horiz). The regions are 3-by-N vertices.
	unsigned int ring_vertices = size * 3;
	num_vertices += 2 * ring_vertices;

	// Trim regions are thin stripes which surround blocks from the lower LOD level.
	// Need (2 * size + 1)-by-2 vertices. One stripe for each four sides are needed.
	unsigned int trim_vertices = (2 * size + 1) * 2;
	num_vertices += trim_vertices * 4;

	// Degenerate triangles. These are run on the edge between clipmap levels.
	// These are needed to avoid occasional "missing pixels" between clipmap levels as
	// imperfections in precision can cause the terrain to not perfectly overlap at the clipmap level boundary.
	//
	// 5 vertices are used per vertex to create a suitable triangle strip.
	// (This is somewhat redundant, but it simplifies the implementation).
	// Two different strips are needed for left/right and top/bottom.
	unsigned int degenerate_vertices = 2 * (size - 1) * 5;
	num_vertices += degenerate_vertices * 2;

	GLubyte *vertices = new GLubyte[2 * num_vertices];
	//! [Generating vertex buffer]
	GLubyte *pv = vertices;

	// Block
	for (unsigned int z = 0; z < size; z++)
	{
		for (unsigned int x = 0; x < size; x++)
		{
			pv[0] = x;
			pv[1] = z;
			pv += 2;
		}
	}
	//! [Generating vertex buffer]

	// Vertical ring fixup
	for (unsigned int z = 0; z < size; z++)
	{
		for (unsigned int x = 0; x < 3; x++)
		{
			pv[0] = x;
			pv[1] = z;
			pv += 2;
		}
	}

	// Horizontal ring fixup
	for (unsigned int z = 0; z < 3; z++)
	{
		for (unsigned int x = 0; x < size; x++)
		{
			pv[0] = x;
			pv[1] = z;
			pv += 2;
		}
	}

	// Full interior trim
	// Top
	for (unsigned int z = 0; z < 2; z++)
	{
		for (unsigned int x = 0; x < 2 * size + 1; x++)
		{
			pv[0] = x;
			pv[1] = z;
			pv += 2;
		}
	}

	// Right
	for (int x = 1; x >= 0; x--)
	{
		for (unsigned int z = 0; z < 2 * size + 1; z++)
		{
			pv[0] = x + 2 * size - 1;
			pv[1] = z;
			pv += 2;
		}
	}

	// Bottom
	for (int z = 1; z >= 0; z--)
	{
		for (unsigned int x = 0; x < 2 * size + 1; x++)
		{
			pv[0] = 2 * size - x;
			pv[1] = z + 2 * size - 1;
			pv += 2;
		}
	}

	// Left
	for (unsigned int x = 0; x < 2; x++)
	{
		for (unsigned int z = 0; z < 2 * size + 1; z++)
		{
			pv[0] = x;
			pv[1] = 2 * size - z;
			pv += 2;
		}
	}

	// Degenerate triangles.
	// Left, right
	for (unsigned int y = 0; y < (size - 1) * 2; y++)
	{
		pv[0] = 0;
		pv[1] = y * 2;
		pv[2] = 0;
		pv[3] = y * 2;
		pv[4] = 0;
		pv[5] = y * 2 + 1;
		pv[6] = 0;
		pv[7] = y * 2 + 2;
		pv[8] = 0;
		pv[9] = y * 2 + 2;
		pv += 10;
	}

	// Top, bottom
	for (unsigned int x = 0; x < (size - 1) * 2; x++)
	{
		pv[0] = x * 2;
		pv[1] = 0;
		pv[2] = x * 2;
		pv[3] = 0;
		pv[4] = x * 2 + 1;
		pv[5] = 0;
		pv[6] = x * 2 + 2;
		pv[7] = 0;
		pv[8] = x * 2 + 2;
		pv[9] = 0;
		pv += 10;
	}

	// Right and bottom share vertices with left and top respectively.

	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, 2 * num_vertices * sizeof(GLubyte), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	delete[] vertices;


	////Block
	//for (unsigned int z = 0; z < size; z++) // rows
	//{
	//	for (unsigned int x = 0; x < size; x++) // cols
	//	{
	//		Vector3 vertex1(x, 0, z);
	//		this->vertices.push_back(vertex1);

	//		//Vector2 uv1(x / (float)width, z / (float)height);
	//		//this->uvs.push_back(uv1);
	//	}
	//}
}

// Returns number of indices needed to create a triangle stripped mesh using generate_block_indices() below.
static unsigned int block_index_count(unsigned int width, unsigned int height)
{
	unsigned int strips = height - 1;
	return strips * (2 * width - 1) + 1;
}

//! [Generating index buffer]
static GLushort *generate_block_indices(GLushort *pi, unsigned int vertex_buffer_offset,
	unsigned int width, unsigned int height)
{
	// Stamp out triangle strips back and forth.
	int pos = vertex_buffer_offset;
	unsigned int strips = height - 1;

	// After even indices in a strip, always step to next strip.
	// After odd indices in a strip, step back again and one to the right or left.
	// Which direction we take depends on which strip we're generating.
	// This creates a zig-zag pattern.
	for (unsigned int z = 0; z < strips; z++)
	{
		int step_even = width;
		int step_odd = ((z & 1) ? -1 : 1) - step_even;

		// We don't need the last odd index.
		// The first index of the next strip will complete this strip.
		for (unsigned int x = 0; x < 2 * width - 1; x++)
		{
			*pi++ = pos;
			pos += (x & 1) ? step_odd : step_even;
		}
	}
	// There is no new strip, so complete the block here.
	*pi++ = pos;

	// Return updated index buffer pointer.
	// More explicit than taking reference to pointer.
	return pi;
}
//! [Generating index buffer]

void ClipmapGrid::setup_index_buffer(unsigned int size)
{
	unsigned int vertex_buffer_offset = 0;

	block.count = block_index_count(size, size);

	vertical.count = block_index_count(3, size);
	horizontal.count = block_index_count(size, 3);

	unsigned int trim_region_indices = block_index_count(2 * size + 1, 2);
	trim_full.count = 4 * trim_region_indices;
	trim_top_left.count = 2 * trim_region_indices;
	trim_bottom_right = trim_bottom_left = trim_top_right = trim_top_left;

	// 6 indices are used here per vertex.
	// Need to repeat one vertex to get correct winding when
	// connecting the triangle strips.
	degenerate_left.count = (size - 1) * 2 * 6;
	degenerate_right = degenerate_bottom = degenerate_top = degenerate_left;

	num_indices = block.count + vertical.count + horizontal.count + trim_full.count +
		4 * trim_top_left.count +
		4 * degenerate_left.count;

	GLushort *indices = new GLushort[num_indices];
	GLushort *pi = indices;

	// Main block
	block.offset = pi - indices;
	pi = generate_block_indices(pi, vertex_buffer_offset, size, size);
	vertex_buffer_offset += size * size;

	// Vertical fixup
	vertical.offset = pi - indices;
	pi = generate_block_indices(pi, vertex_buffer_offset, 3, size);
	vertex_buffer_offset += 3 * size;

	// Horizontal fixup
	horizontal.offset = pi - indices;
	pi = generate_block_indices(pi, vertex_buffer_offset, size, 3);
	vertex_buffer_offset += 3 * size;

	// Full interior trim
	// All trims can be run after each other.
	// The vertex buffer is generated such that this creates a "ring".
	// The full trim is only used to connect clipmap level 0 to level 1. See Doxygen for more detail.
	trim_full.offset = pi - indices;
	unsigned int full_trim_offset = vertex_buffer_offset;
	unsigned int trim_vertices = (2 * size + 1) * 2;
	pi = generate_block_indices(pi, full_trim_offset, 2 * size + 1, 2); // Top
	full_trim_offset += trim_vertices;
	pi = generate_block_indices(pi, full_trim_offset, 2 * size + 1, 2); // Right
	full_trim_offset += trim_vertices;
	pi = generate_block_indices(pi, full_trim_offset, 2 * size + 1, 2); // Bottom
	full_trim_offset += trim_vertices;
	pi = generate_block_indices(pi, full_trim_offset, 2 * size + 1, 2); // Left
	full_trim_offset += trim_vertices;

	// Top-right interior trim
	// This is a half ring (L-shaped).
	trim_top_right.offset = pi - indices;
	pi = generate_block_indices(pi, vertex_buffer_offset, 2 * size + 1, 2); // Top
	pi = generate_block_indices(pi, vertex_buffer_offset + (2 * size + 1) * 2, 2 * size + 1, 2); // Right
	vertex_buffer_offset += trim_vertices;

	// Right-bottom interior trim
	// This is a half ring (L-shaped).
	trim_bottom_right.offset = pi - indices;
	pi = generate_block_indices(pi, vertex_buffer_offset, 2 * size + 1, 2); // Right
	pi = generate_block_indices(pi, vertex_buffer_offset + (2 * size + 1) * 2, 2 * size + 1, 2); // Bottom
	vertex_buffer_offset += trim_vertices;

	// Bottom-left interior trim
	// This is a half ring (L-shaped).
	trim_bottom_left.offset = pi - indices;
	pi = generate_block_indices(pi, vertex_buffer_offset, 2 * size + 1, 2); // Bottom
	pi = generate_block_indices(pi, vertex_buffer_offset + (2 * size + 1) * 2, 2 * size + 1, 2); // Left
	vertex_buffer_offset += trim_vertices;

	// Left-top interior trim
	// This is a half ring (L-shaped).
	trim_top_left.offset = pi - indices;
	pi = generate_block_indices(pi, vertex_buffer_offset, 2 * size + 1, 2); // Left
	pi = generate_block_indices(pi, vertex_buffer_offset - 6 * (2 * size + 1), 2 * size + 1, 2); // Top
	vertex_buffer_offset += trim_vertices;

	// One of the trim regions will be used to connect level N with level N + 1.

	// Degenerates. Left and right share vertices (with different offsets in vertex shader). Top and bottom share.
	// Left
	degenerate_left.offset = pi - indices;
	for (unsigned int z = 0; z < (size - 1) * 2; z++)
	{
		pi[0] = (5 * z) + 0 + vertex_buffer_offset;
		pi[1] = (5 * z) + 1 + vertex_buffer_offset;
		pi[2] = (5 * z) + 2 + vertex_buffer_offset;
		pi[3] = (5 * z) + 3 + vertex_buffer_offset;
		pi[4] = (5 * z) + 4 + vertex_buffer_offset;
		pi[5] = (5 * z) + 4 + vertex_buffer_offset;
		pi += 6;
	}

	// Right
	degenerate_right.offset = pi - indices;
	unsigned int start_z = (size - 1) * 2 - 1;
	for (unsigned int z = 0; z < (size - 1) * 2; z++)
	{
		// Windings are in reverse order on this side.
		pi[0] = (5 * (start_z - z)) + 4 + vertex_buffer_offset;
		pi[1] = (5 * (start_z - z)) + 3 + vertex_buffer_offset;
		pi[2] = (5 * (start_z - z)) + 2 + vertex_buffer_offset;
		pi[3] = (5 * (start_z - z)) + 1 + vertex_buffer_offset;
		pi[4] = (5 * (start_z - z)) + 0 + vertex_buffer_offset;
		pi[5] = (5 * (start_z - z)) + 0 + vertex_buffer_offset;
		pi += 6;
	}

	vertex_buffer_offset += (size - 1) * 2 * 5;

	// Top
	degenerate_top.offset = pi - indices;
	for (unsigned int x = 0; x < (size - 1) * 2; x++)
	{
		pi[0] = (5 * x) + 0 + vertex_buffer_offset;
		pi[1] = (5 * x) + 1 + vertex_buffer_offset;
		pi[2] = (5 * x) + 2 + vertex_buffer_offset;
		pi[3] = (5 * x) + 3 + vertex_buffer_offset;
		pi[4] = (5 * x) + 4 + vertex_buffer_offset;
		pi[5] = (5 * x) + 4 + vertex_buffer_offset;
		pi += 6;
	}

	// Bottom
	degenerate_bottom.offset = pi - indices;
	unsigned int start_x = (size - 1) * 2 - 1;
	for (unsigned int x = 0; x < (size - 1) * 2; x++)
	{
		// Windings are in reverse order on this side.
		pi[0] = (5 * (start_x - x)) + 4 + vertex_buffer_offset;
		pi[1] = (5 * (start_x - x)) + 3 + vertex_buffer_offset;
		pi[2] = (5 * (start_x - x)) + 2 + vertex_buffer_offset;
		pi[3] = (5 * (start_x - x)) + 1 + vertex_buffer_offset;
		pi[4] = (5 * (start_x - x)) + 0 + vertex_buffer_offset;
		pi[5] = (5 * (start_x - x)) + 0 + vertex_buffer_offset;
		pi += 6;
	}

	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(GLushort), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	delete[] indices;

	//for (int y = 0; y < size - 1; y++) { //rows
	//	//Skip the first vertex of the grid(we don't want a degenerate triangle if it's the start or end vertex. Which is why the loop is rows-1)
	//	if (y > 0) {
	//		// Degenerate begin: repeat first vertex
	//		this->indices.push_back(y * size);
	//	}

	//	for (int x = 0; x < size; x++) { // cols
	//		// One part of the strip
	//		this->indices.push_back((y * size) + x);
	//		this->indices.push_back(((y + 1) * size) + x);
	//	}

	//	//no skip, adds the last vertex of the strip as a degenerate, which will connect with the next row's degenerate vertex)
	//	if (y < size - 2) {
	//		// Degenerate end: repeat last vertex
	//		this->indices.push_back(((y + 1) * size) + (size - 1));
	//	}
	//}
}

void ClipmapGrid::setup_uniform_buffer()
{
	glGenBuffers(1, &uniform_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);

	// Per level we can draw up to 12 regular blocks, 4 vert/horiz rings, one trim, and four degenerate strips.
	// Double the UBO size just in case we have very high levels for UBO buffer alignment.
	uniform_buffer_size = 2 * (12 + 4 + 1 + 4) * levels * sizeof(InstanceData);
	//glBufferData(GL_UNIFORM_BUFFER, uniform_buffer_size, NULL, GL_STREAM_DRAW);
	glBufferStorage(GL_UNIFORM_BUFFER, uniform_buffer_size, NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	data = static_cast<InstanceData*>(glMapBufferRange(GL_UNIFORM_BUFFER, 0, uniform_buffer_size, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT)); //GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_WRITE_BIT
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

// Already defined in the shader.
#define LOCATION_VERTEX 0

void ClipmapGrid::setup_vertex_array()
{
	glGenVertexArrays(1, &this->vao);
	glBindVertexArray(this->vao);
	glBindBuffer(GL_ARRAY_BUFFER, this->vertex_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->index_buffer);

	glVertexAttribPointer(LOCATION_VERTEX, 2, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(LOCATION_VERTEX);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Element array buffer state is part of the vertex array object, have to unbind it after the vertex array.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

//! [Snapping clipmap level to a grid]
// The clipmap levels only move in steps of texture coordinates.
// Computes top-left world position for the levels.
Vector2 ClipmapGrid::get_offset_level(const Vector2& camera_pos, unsigned int level)
{
	if (level == 0) // Must follow level 1 as trim region is fixed.
		return get_offset_level(camera_pos, 1) + Vector2(size << 1);
	else
	{
		Vector2 scaled_pos = camera_pos / Vector2(clipmap_scale); // Snap to grid in the appropriate space.

		// Snap to grid of next level. I.e. we move the clipmap level in steps of two.
		Vector2 snapped_pos = Vector2::vec_floor(scaled_pos / Vector2(1 << (level + 1))) * Vector2(1 << (level + 1));

		// Apply offset so all levels align up neatly.
		// If snapped_pos is equal for all levels,
		// this causes top-left vertex of level N to always align up perfectly with top-left interior corner of level N + 1.
		// This gives us a bottom-right trim region.

		// Due to the flooring, snapped_pos might not always be equal for all levels.
		// The flooring has the property that snapped_pos for level N + 1 is less-or-equal snapped_pos for level N.
		// If less, the final position of level N + 1 will be offset by -2 ^ N, which can be compensated for with changing trim-region to top-left.
		Vector2 pos = snapped_pos - Vector2((2 * (size - 1)) << level);
		return pos;
	}
}
//! [Snapping clipmap level to a grid]

void ClipmapGrid::update_level_offsets(const Vector2& camera_pos)
{
	level_offsets.resize(levels);
	for (unsigned int i = 0; i < levels; i++){
		level_offsets[i] = get_offset_level(camera_pos, i);
	}
}

// Since we use instanced drawing, all the different instances of various block types
// can be grouped together to form one draw call per block type.
//
// For the get_draw_info* calls, we look through all possible places where blocks can be rendered
// and push this information to a draw list and a uniform buffer.
//
// The draw list struct (DrawInfo) contains information such as the number of instances for a block type,
// and from where in the uniform buffer to get per-instance data. The per-instance data contains information
// of offset and scale values required to render the blocks at correct positions and at correct scale.
//
// The get_draw_info_* calls are sort of repetitive so comments are only introduced when
// something different is done.
//
// It is important to note that instance.offset is a pre-scaled offset which denotes the
// world-space X/Z position of the top-left vertex in the block.
// instance.scale is used to scale vertex data in a block (which are just integers).
//
// World space X/Z coordinates are computed as instance.offset + vertex_coord * instance.scale.

ClipmapGrid::DrawInfo ClipmapGrid::get_draw_info_horiz_fixup(InstanceData *instances)
{
	DrawInfo info;
	InstanceData instance;
	instance.debug_color = Vector3(0, 1, 0);

	// Horizontal
	info.index_buffer_offset = horizontal.offset;
	info.indices = horizontal.count;
	info.instances = 0;

	// We don't have any fixup regions for the lowest clipmap level.
	for (unsigned int i = 1; i < levels; i++)
	{
		// Left side horizontal fixup region.
		// Texel coordinates are derived by just dividing the world space offset with texture size.
		// The 0.5 texel offset required to sample exactly at the texel center is done in vertex shader.
		instance.texture_scale = 1.0f / level_size;
		instance.scale = clipmap_scale * float(1 << i);
		instance.level = i;

		instance.offset = level_offsets[i];
		instance.offset += Vector2(0, 2 * (size - 1)) * Vector2(1 << i);
		// Avoid texture coordinates which are very large as this can be difficult for the texture sampler
		// to handle (float precision). Since we use GL_REPEAT, fract() does not change the result.
		// Scale the offset down by 2^level first to get the appropriate texel.
		instance.texture_offset = Vector2::vec_fract((instance.offset / Vector2(1 << i)) * instance.texture_scale);
		instance.offset *= Vector2(clipmap_scale);

		// Only add the instance if it's visible.
		//if (intersects_frustum(instance.offset, horizontal.range, i))
		//{
		*instances++ = instance;
		info.instances++;
		//}

		// Right side horizontal fixup region.
		instance.offset = level_offsets[i];
		instance.offset += Vector2(3 * (size - 1) + 2, 2 * (size - 1)) * Vector2(1 << i);
		instance.texture_offset = Vector2::vec_fract((instance.offset / Vector2(1 << i)) * instance.texture_scale);
		instance.offset *= Vector2(clipmap_scale);

		// Only add the instance if it's visible.
		//if (intersects_frustum(instance.offset, horizontal.range, i))
		//{
		*instances++ = instance;
		info.instances++;
		//}
	}

	return info;
}

// Same as horizontal, just different vertex data and offsets.
ClipmapGrid::DrawInfo ClipmapGrid::get_draw_info_vert_fixup(InstanceData *instances)
{
	DrawInfo info;
	InstanceData instance;
	instance.debug_color = Vector3(0, 1, 0);

	// Vertical
	info.index_buffer_offset = vertical.offset;
	info.indices = vertical.count;
	info.instances = 0;

	for (unsigned int i = 1; i < levels; i++)
	{
		// Top region
		instance.texture_scale = 1.0f / level_size;
		instance.scale = clipmap_scale * float(1 << i);
		instance.level = i;

		instance.offset = level_offsets[i];
		instance.offset += Vector2(2 * (size - 1), 0) * Vector2(1 << i);
		instance.texture_offset = Vector2::vec_fract((instance.offset / Vector2(1 << i)) * instance.texture_scale);
		instance.offset *= Vector2(clipmap_scale);

		//if (intersects_frustum(instance.offset, vertical.range, i))
		//{
		*instances++ = instance;
		info.instances++;
		//}

		// Bottom region
		instance.offset = level_offsets[i];
		instance.offset += Vector2(2 * (size - 1), 3 * (size - 1) + 2) * Vector2(1 << i);
		instance.texture_offset = Vector2::vec_fract((instance.offset / Vector2(1 << i)) * instance.texture_scale);
		instance.offset *= Vector2(clipmap_scale);

		//if (intersects_frustum(instance.offset, vertical.range, i))
		//{
		*instances++ = instance;
		info.instances++;
		//}
	}

	return info;
}

ClipmapGrid::DrawInfo ClipmapGrid::get_draw_info_degenerate(InstanceData *instances, const Block& block, const Vector2& offset, const Vector2& ring_offset)
{
	DrawInfo info;
	info.instances = 0;
	info.index_buffer_offset = block.offset;
	info.indices = block.count;

	InstanceData instance;
	instance.debug_color = Vector3(0, 0, 1);
	instance.texture_scale = 1.0f / level_size;

	// No need to connect the last clipmap level to next level (there is none).
	for (unsigned int i = 0; i < levels - 1; i++)
	{
		instance.level = i;
		instance.offset = level_offsets[i];
		instance.offset += offset * Vector2(1 << i);

		// This is required to differentiate between level 0 and the other levels.
		// In clipmap level 0, we only have tightly packed N-by-N blocks.
		// In other levels however, there are horizontal and vertical fixup regions, therefore a different
		// offset (2 extra texels) is required.
		if (i > 0)
		instance.offset += ring_offset * Vector2(1 << i);
		instance.texture_offset = Vector2::vec_fract((instance.offset / Vector2(1 << i)) * instance.texture_scale);
		instance.offset *= Vector2(clipmap_scale);
		instance.scale = clipmap_scale * float(1 << i);

		//if (intersects_frustum(instance.offset, block.range, i))
		//{
		*instances++ = instance;
		info.instances++;
		//}
	}

	return info;
}

// Use the generalized get_draw_info_degenerate().
ClipmapGrid::DrawInfo ClipmapGrid::get_draw_info_degenerate_left(InstanceData *instances)
{
	return get_draw_info_degenerate(instances, degenerate_left, Vector2(0.0f), Vector2(0.0f));
}

ClipmapGrid::DrawInfo ClipmapGrid::get_draw_info_degenerate_right(InstanceData *instances)
{
	return get_draw_info_degenerate(instances, degenerate_right, Vector2(4 * (size - 1), 0.0f), Vector2(2.0f, 0.0f));
}

ClipmapGrid::DrawInfo ClipmapGrid::get_draw_info_degenerate_top(InstanceData *instances)
{
	return get_draw_info_degenerate(instances, degenerate_top, Vector2(0.0f), Vector2(0.0f));
}

ClipmapGrid::DrawInfo ClipmapGrid::get_draw_info_degenerate_bottom(InstanceData *instances)
{
	return get_draw_info_degenerate(instances, degenerate_bottom, Vector2(0.0f, 4 * (size - 1) + 2), Vector2(0.0f, 2.0f));
}

// Only used for cliplevel 1 to encapsulate cliplevel 0.
ClipmapGrid::DrawInfo ClipmapGrid::get_draw_info_trim_full(InstanceData *instances)
{
	DrawInfo info;
	info.index_buffer_offset = trim_full.offset;
	info.indices = trim_full.count;
	info.instances = 0;

	InstanceData instance;
	instance.debug_color = Vector3(1, 1, 0);
	instance.texture_scale = 1.0f / level_size;
	instance.level = 1;
	instance.offset = level_offsets[1];
	instance.offset += Vector2((size - 1) << 1);
	instance.texture_offset = Vector2::vec_fract((instance.offset / Vector2(1 << 1)) * instance.texture_scale);
	instance.offset *= Vector2(clipmap_scale);
	instance.scale = clipmap_scale * float(1 << 1);

	//if (intersects_frustum(instance.offset, trim_full.range, 1))
	//{
	*instances = instance;
	info.instances++;
	//}

	return info;
}

ClipmapGrid::DrawInfo ClipmapGrid::get_draw_info_trim(InstanceData *instances, const Block& block, TrimConditional cond)
{
	DrawInfo info;
	info.index_buffer_offset = block.offset;
	info.indices = block.count;
	info.instances = 0;

	// Level 1 always fills in the gap to level 0 using get_draw_info_trim_full().
	// From level 2 and out, we only need a single L-shaped trim region as levels 1 and up
	// use horizontal/vertical trim regions as well, which increases the size slightly (get_draw_info_blocks()).
	for (unsigned int i = 2; i < levels; i++)
	{
		Vector2 offset_prev_level = level_offsets[i - 1];
		Vector2 offset_current_level = level_offsets[i] + Vector2((size - 1) << i);

		// There are four different ways (top-right, bottom-right, top-left, bottom-left)
		// to apply a trim region depending on how camera snapping is done in get_offset_level().
		// A function pointer is introduced here so we can check if a particular trim type
		// should be used for this level. Only one conditional will return true for a given level.
		if (!cond(offset_prev_level - offset_current_level))
			continue;

		InstanceData instance;
		instance.debug_color = Vector3(1, 1, 0);
		instance.texture_scale = 1.0f / level_size;
		instance.level = i;
		instance.offset = level_offsets[i];
		instance.offset += Vector2((size - 1) << i);
		instance.texture_offset = Vector2::vec_fract((instance.offset / Vector2(1 << i)) * instance.texture_scale);
		instance.offset *= Vector2(clipmap_scale);
		instance.scale = clipmap_scale * float(1 << i);

		//if (intersects_frustum(instance.offset, block.range, i))
		//{
		*instances++ = instance;
		info.instances++;
		//}
	}

	return info;
}

// offset.x and offset.y are either 0 or at least 1.
// Using 0.5f as threshold is a safe way to check for this difference.
static inline bool trim_top_right_cond(const Vector2& offset)
{
	return offset.x < 0.5f && offset.y > 0.5f;
}

static inline bool trim_top_left_cond(const Vector2& offset)
{
	return offset.x > 0.5f && offset.y > 0.5f;
}

static inline bool trim_bottom_right_cond(const Vector2& offset)
{
	return offset.x < 0.5f && offset.y < 0.5f;
}

static inline bool trim_bottom_left_cond(const Vector2& offset)
{
	return offset.x > 0.5f && offset.y < 0.5f;
}

ClipmapGrid::DrawInfo ClipmapGrid::get_draw_info_trim_top_right(InstanceData *instances)
{
	return get_draw_info_trim(instances, trim_top_right, trim_top_right_cond);
}

ClipmapGrid::DrawInfo ClipmapGrid::get_draw_info_trim_top_left(InstanceData *instances)
{
	return get_draw_info_trim(instances, trim_top_left, trim_top_left_cond);
}

ClipmapGrid::DrawInfo ClipmapGrid::get_draw_info_trim_bottom_right(InstanceData *instances)
{
	return get_draw_info_trim(instances, trim_bottom_right, trim_bottom_right_cond);
}

ClipmapGrid::DrawInfo ClipmapGrid::get_draw_info_trim_bottom_left(InstanceData *instances)
{
	return get_draw_info_trim(instances, trim_bottom_left, trim_bottom_left_cond);
}

// These are the basic N-by-N tesselated quads.
ClipmapGrid::DrawInfo ClipmapGrid::get_draw_info_blocks(InstanceData *instances)
{
	// Special case for level 0, here we draw the base quad in a tight 4x4 grid. This needs to be padded with a full trim (get_draw_info_trim_full()).
	DrawInfo info;
	info.instances = 0;
	info.index_buffer_offset = block.offset;
	info.indices = block.count;

	InstanceData instance;
	instance.debug_color = Vector3(1, 0, 0);
	instance.scale = clipmap_scale;
	instance.texture_scale = 1.0f / level_size;

	for (unsigned int z = 0; z < 4; z++)
	{
		for (unsigned int x = 0; x < 4; x++)
		{
			instance.level = 0;
			instance.offset = level_offsets[0];
			instance.offset += Vector2(x, z) * Vector2(size - 1);
			instance.texture_offset = Vector2::vec_fract(instance.offset * instance.texture_scale);
			instance.offset *= Vector2(clipmap_scale);

			//if (intersects_frustum(instance.offset, block.range, 0))
			//{
			*instances++ = instance; //moves pointer once to next available index
			info.instances++;
			//}
		}
	}

	// From level 1 and out, the four center blocks are already filled with the lower clipmap level, so
	// skip these.
	for (unsigned int i = 1; i < levels; i++)
	{
		instance.texture_scale = 1.0f / level_size;

		for (unsigned int z = 0; z < 4; z++)
		{
			for (unsigned int x = 0; x < 4; x++)
			{
				if (z != 0 && z != 3 && x != 0 && x != 3)
				{
					// Already occupied, skip.
					continue;
				}

				instance.scale = clipmap_scale * float(1 << i);
				instance.level = i;
				instance.offset = level_offsets[i];
				instance.offset += Vector2(x, z) * Vector2((size - 1) << i);

				// Skip 2 texels horizontally and vertically at the middle to get a symmetric structure.
				// These regions are filled with horizontal and vertical fixup regions.
				if (x >= 2)
					instance.offset.x += 2 << i;
				if (z >= 2)
					instance.offset.y += 2 << i;

				instance.texture_offset = Vector2::vec_fract((instance.offset / Vector2(1 << i)) * instance.texture_scale);
				instance.offset *= Vector2(clipmap_scale);

				//if (intersects_frustum(instance.offset, block.range, i))
				//{
				*instances++ = instance;
				info.instances++;
				//}
			}
		}
	}

	return info;
}

// Helper template to keep the ugly pointer casting to one place.
template<typename T>
static inline T *buffer_offset(T *buffer, size_t offset)
{
	return reinterpret_cast<T*>(reinterpret_cast<unsigned char*>(buffer)+offset);
}

// Round up to nearest aligned offset.
static inline unsigned int realign_offset(size_t offset, size_t align)
{
	return (offset + align - 1) & ~(align - 1);
}

void ClipmapGrid::update_draw_list(DrawInfo& info, size_t& uniform_buffer_offset)
{
	info.uniform_buffer_offset = uniform_buffer_offset;
	draw_list.push_back(info);

	// Have to ensure that the uniform buffer is always bound at aligned offsets.
	uniform_buffer_offset = realign_offset(uniform_buffer_offset + info.instances * sizeof(InstanceData), uniform_buffer_align);
}

void ClipmapGrid::update_draw_list()
{
	if (syncObj){
		// waiting for the buffer
		while (true)
		{
			GLenum waitReturn = glClientWaitSync(syncObj, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
			if (waitReturn == GL_ALREADY_SIGNALED || waitReturn == GL_CONDITION_SATISFIED)
			{

				break;
			}
		}
		glDeleteSync(syncObj);
	}

	draw_list.clear();

	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);

	// Map the uniform buffer. //gets a pointer so we can modify the data on gpu instead of copy it
	//InstanceData *data = static_cast<InstanceData*>(glMapBufferRange(GL_UNIFORM_BUFFER, 0, uniform_buffer_size, GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_WRITE_BIT));

	if (!data)
	{
		printf("Failed to map uniform buffer.\n");
		return;
	}

	DrawInfo info;
	size_t uniform_buffer_offset = 0;

	// Create a draw list. The number of draw calls is equal to the different types
	// of blocks. The blocks are instanced as necessary in the get_draw_info* calls.

	// Main blocks
	info = get_draw_info_blocks(buffer_offset(data, uniform_buffer_offset)); //buffer_offset typecast instance pointer to char pointer and offsets it. It's PerInstanceData array in shader that gets modified.
	update_draw_list(info, uniform_buffer_offset);

	// Vertical ring fixups
	info = get_draw_info_vert_fixup(buffer_offset(data, uniform_buffer_offset));
	update_draw_list(info, uniform_buffer_offset);

	// Horizontal ring fixups
	info = get_draw_info_horiz_fixup(buffer_offset(data, uniform_buffer_offset));
	update_draw_list(info, uniform_buffer_offset);

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	// Left-side degenerates
	info = get_draw_info_degenerate_left(buffer_offset(data, uniform_buffer_offset));
	update_draw_list(info, uniform_buffer_offset);

	// Right-side degenerates
	info = get_draw_info_degenerate_right(buffer_offset(data, uniform_buffer_offset));
	update_draw_list(info, uniform_buffer_offset);

	// Top-side degenerates
	info = get_draw_info_degenerate_top(buffer_offset(data, uniform_buffer_offset));
	update_draw_list(info, uniform_buffer_offset);

	// Bottom-side degenerates
	info = get_draw_info_degenerate_bottom(buffer_offset(data, uniform_buffer_offset));
	update_draw_list(info, uniform_buffer_offset);

	// Full trim
	info = get_draw_info_trim_full(buffer_offset(data, uniform_buffer_offset));
	update_draw_list(info, uniform_buffer_offset);

	// Top-right trim
	info = get_draw_info_trim_top_right(buffer_offset(data, uniform_buffer_offset));
	update_draw_list(info, uniform_buffer_offset);

	// Top-left trim
	info = get_draw_info_trim_top_left(buffer_offset(data, uniform_buffer_offset));
	update_draw_list(info, uniform_buffer_offset);

	// Bottom-right trim
	info = get_draw_info_trim_bottom_right(buffer_offset(data, uniform_buffer_offset));
	update_draw_list(info, uniform_buffer_offset);

	// Bottom-left trim
	info = get_draw_info_trim_bottom_left(buffer_offset(data, uniform_buffer_offset));
	update_draw_list(info, uniform_buffer_offset);

	// lock the buffer:
	syncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	//glUnmapBuffer(GL_UNIFORM_BUFFER);
}

void ClipmapGrid::render_draw_list()
{
	for (std::vector<DrawInfo>::const_iterator itr = draw_list.begin(); itr != draw_list.end(); ++itr)
	{
		if (itr->instances){
			// Bind uniform buffer at same binding point as in shader
			//glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniform_buffer);
			glBindBufferRange(GL_UNIFORM_BUFFER, 0, uniform_buffer, itr->uniform_buffer_offset, realign_offset(itr->instances * sizeof(InstanceData), uniform_buffer_align));

			// Draw all instances.
			glDrawElementsInstanced(GL_TRIANGLE_STRIP, itr->indices, GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid*>(itr->index_buffer_offset * sizeof(GLushort)), itr->instances);
		}
	}
}