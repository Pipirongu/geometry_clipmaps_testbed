#include "texture.h"

Texture::Texture()
{
}


Texture::~Texture()
{
	glDeleteTextures(1, &this->texture_id);
}

void Texture::BindHeightmap()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->texture_id);
	glUniform1i(ShaderManager::Instance()->heightmap_sampler_loc, 0);
}

void Texture::Unbind()
{
	//Unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::LoadHeightmap(const std::string& file_name, int& out_width, int& out_height)
{
	//load the heightmap texture using SOIL	
	int texture_width = 0, texture_height = 0, channels = 0;
	GLubyte* pData = SOIL_load_image(file_name.c_str(), &texture_width, &texture_height, &channels, SOIL_LOAD_L);
	out_width = texture_width;
	out_height = texture_height;

	//vertically flip the heightmap image on Y axis since it is inverted 
	for (int j = 0; j * 2 < texture_height; ++j)
	{
		int index1 = j * texture_width;
		int index2 = (texture_height - 1 - j) * texture_width;
		for (int i = texture_width; i > 0; --i)
		{
			GLubyte temp = pData[index1];
			pData[index1] = pData[index2];
			pData[index2] = temp;
			++index1;
			++index2;
		}
	}

	//Create openGL Texture
	glGenTextures(1, &this->texture_id);
	//Bind it
	glBindTexture(GL_TEXTURE_2D, this->texture_id);
	//Fill with data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, texture_width, texture_height, 0, GL_RED, GL_UNSIGNED_BYTE, pData);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Generate mipmaps, by the way.
	glGenerateMipmap(GL_TEXTURE_2D);
	//Unbind
	glBindTexture(GL_TEXTURE_2D, 0);


	//free SOIL image data
	SOIL_free_image_data(pData);
}