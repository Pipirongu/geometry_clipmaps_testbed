#pragma once

//For class representing key for map in the vertex indexing algorithm
#include <map>

#include <GL/glew.h>

//For opening files
#include <stdio.h>
#include <stdlib.h>

#include "mypersonalmathlib.h"
#include <vector>
#include <algorithm>

/**
@class Mesh

Represents the mesh of an object. Can load obj files.
*/
class Mesh
{
public:
	/// Constructor
	Mesh();
	/// Destructor
	~Mesh();

	/// Draw the mesh, and then calls the childrens' draw
	void Draw();
	void GenerateNestedRegularGrid(int width, int height);
	void UpdateNestedRegularGrid(int width, int height);
	void setup_index_buffer(int width, int height);

	//vertices, uvs, normals, indices
	std::vector<Vector3> vertices;
	std::vector<Vector2> uvs;
	std::vector<Vector3> normals;
	std::vector<unsigned int> indices;
	float height_scale = 20;
	int STEP_SIZE = 1;

private:
	
	//VAO
	GLuint vao;
	//VBO Vertices
	GLuint vertex_vbo;
	//VBO UVS
	GLuint uv_vbo;
	//VBO Normals
	GLuint normal_vbo;
	//EBO Indices
	GLuint index_ebo;

	void CreateVertexBuffers();
	void UpdateVertexBuffers();
};
