#pragma once

#include <GL/glew.h>
#include <string>
#include "shadermanager.h"
#include <SOIL.h>

/**
@class Texture

Holds the texture of a mesh
*/
class Texture
{
public:
	/// Constructor
	Texture();
	/// Destructor
	~Texture();

	/// Bind Texture
	void BindHeightmap();
	void Unbind();
	/// Loads a texture to be used by a mesh
	void LoadHeightmap(const std::string& file_name, int& out_width, int& out_height);

private:
	//Holds the texture that should be used by this mesh
	GLuint texture_id;
};

