#include "mesh.h"

//------------------------------------------------------------------------------
/**
*/
Mesh::Mesh()
{
}

//------------------------------------------------------------------------------
/**
*/
Mesh::~Mesh(void)
{

	//Delete VBOs
	glDeleteBuffers(1, &this->vertex_vbo);
	glDeleteBuffers(1, &this->uv_vbo);
	glDeleteBuffers(1, &this->normal_vbo);
	glDeleteBuffers(1, &this->index_ebo);

	//Delete VAO
	glDeleteVertexArrays(1, &this->vao);
}

//------------------------------------------------------------------------------
/**
*/
void Mesh::Draw()
{
	//Bind this mesh VAO
	glBindVertexArray(this->vao);
	//Draw the triangles using the index buffer(EBO)
	glDrawElements(GL_TRIANGLE_STRIP, this->indices.size(), GL_UNSIGNED_INT, 0);
	//glDrawArrays(GL_TRIANGLE_STRIP, 0, this->vertices.size());
	//Unbind the VAO
	glBindVertexArray(0);
}

void Mesh::GenerateNestedRegularGrid(int width, int height)
{
	for (unsigned int z = 0; z < width; z++)
	{
		for (unsigned int x = 0; x < height; x++)
		{
			Vector3 vertex1(x, 1, z);
			Vector3 vertex2(x, 1, z + 1);
			this->vertices.push_back(vertex1);
			//this->vertices.push_back(vertex2);

			Vector2 uv1(x / (float)width, z / (float)height);
			Vector2 uv2(x / (float)width, (z + 1) / (float)height);
			this->uvs.push_back(uv1);
			//this->uvs.push_back(uv2);
		}
	}
	this->setup_index_buffer();

	this->CreateVertexBuffers();
}

void Mesh::setup_index_buffer()
{
	int width = 60;
	int height = 60;

	// Now build the index data
	int numStripsRequired = width - 1;
	int numDegensRequired = 2 * (numStripsRequired - 1);
	int verticesPerStrip = 2 * height;

	//final short[] heightMapIndexData = new short[(verticesPerStrip * numStripsRequired)
	//	+ numDegensRequired];

	//offset = 0;

	for (int y = 0; y <width - 1; y++) {
		if (y > 0) {
			// Degenerate begin: repeat first vertex
			this->indices.push_back(y * height);
		}

		for (int x = 0; x < height; x++) {
			// One part of the strip
			this->indices.push_back((y * height) + x);
			this->indices.push_back(((y + 1) * height) + x);
		}

		if (y < height - 2) {
			// Degenerate end: repeat last vertex
			this->indices.push_back(((y + 1) * height) + (width - 1));
		}
	}
}

void Mesh::CreateVertexBuffers()
{
	//////////////////////////////////////////////////////////////////////////
	//Create VAO
	glGenVertexArrays(1, &this->vao);
	//Bind VAO
	glBindVertexArray(this->vao);

	//////////////////////////////////////////////////////////////////////////
	//Vertex VBO
	//Create VBO on the GPU to store the vertex data
	glGenBuffers(1, &this->vertex_vbo);
	//Bind VBO to make it current
	glBindBuffer(GL_ARRAY_BUFFER, this->vertex_vbo);
	//Set the usage type, allocate VRAM and send the vertex data to the GPU
	glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vector3), &this->vertices[0], GL_STATIC_DRAW);

	//Sets up which shader attribute will received the data. How many elements will form a vertex, type etc
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	//Enable the shader attribute to receive data
	glEnableVertexAttribArray(0);

	//////////////////////////////////////////////////////////////////////////
	//UV VBO
	//Create VBO on the GPU to store the vertex data
	glGenBuffers(1, &this->uv_vbo);
	//Bind VBO to make it current
	glBindBuffer(GL_ARRAY_BUFFER, this->uv_vbo);
	//Set the usage type, allocate VRAM and send the vertex data to the GPU
	glBufferData(GL_ARRAY_BUFFER, this->uvs.size() * sizeof(Vector2), &this->uvs[0], GL_STATIC_DRAW);

	//Sets up which shader attribute will receive the data. How many elements will form a vertex, type etc
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	//Enable the shader attribute to receive data
	glEnableVertexAttribArray(1);

	////////////////////////////////////////////////////////////////////////////
	////Normal VBO
	////Create VBO on the GPU to store the vertex data
	//glGenBuffers(1, &this->normal_vbo);
	////Bind VBO to make it current
	//glBindBuffer(GL_ARRAY_BUFFER, this->normal_vbo);
	////Set the usage type, allocate VRAM and send the vertex data to the GPU
	//glBufferData(GL_ARRAY_BUFFER, this->normals.size() * sizeof(Vector3), &this->normals[0], GL_STATIC_DRAW);

	////Sets up which shader attribute will receive the data. How many elements will form a vertex, type etc
	//glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	////Enable the shader attribute to receive data
	//glEnableVertexAttribArray(2);

	//////////////////////////////////////////////////////////////////////////
	//Indices EBO
	//Create EBO on the GPU to store the vertex data
	glGenBuffers(1, &this->index_ebo);
	//Bind EBO to make it current
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->index_ebo);
	//Set the usage type, allocate VRAM and send the vertex data to the GPU
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned int), &this->indices[0], GL_STATIC_DRAW);

	//////////////////////////////////////////////////////////////////////////
	//Unbind the VAO now that the VBOs have been set up
	glBindVertexArray(0);
}