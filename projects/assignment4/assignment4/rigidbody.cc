#include "rigidbody.h"

RigidBody::RigidBody()
{
	this->mesh = nullptr;
	this->texture = nullptr;
	this->id = 0;
	
	//Set default Material properties for mesh
	this->ambient_material.Insert(0.25, 0.25, 0.25);
	this->specular_material.Insert(0.5, 0.5, 0.5);
	this->shininess = 5;
	this->SetID();
}

RigidBody::~RigidBody(void)
{
	this->mesh = nullptr;
	this->texture = nullptr;

	//Clear children list
	this->children.clear();
}

//------------------------------------------------------------------------------
/**
First parameter is only used if the transform instance represents a camera. The view will store the inverse of the parents model matrix
The second parameter is the parent's model matrix
*/
void RigidBody::Update(Matrix44& view, const Matrix44& model)
{
	/*
	If the model matrix isn't reset and just multiplied with the parents model matrix.
	The matrices you just want to do once to place it in the world will add up and undesired effects occurs.
	For example the parent has a translation -10 and the child inherits it by matrix multiplication. 
	But the next render loop the child will have -10 and inherit additional translation from parent.
	Instead transform matrices are implemented to hold the updated transforms. Model matrix is reset and inherits these transforms + the parent's.
	*/
	this->model = this->rotation  * this->look_at * Matrix44::Translation(this->position) * this->rotation_around_own_axis * this->scaling; //A rotation is applied first to make the object rotate around itself.

	//Multiply the parent's model matrix with this
	this->model = model * this->model;
	
	//Calls the childrens' update
	if (!this->children.empty())
	{
		for (int i = 0; i < this->children.size(); i++)
		{
			if (this->children[i] != nullptr)
			{
				this->children[i]->Update(view, this->model);
			}
		}
	}
	
}

void RigidBody::Integrate(float timestep)
{
	//empti
}

void RigidBody::Draw(const Matrix44& projection, const Matrix44& view)
{	
	//Send the material properties to shader program
	glUniform3fv(ShaderManager::Instance()->ambient_material_loc, 1, &this->ambient_material[0]);
	glUniform3fv(ShaderManager::Instance()->specular_material_loc, 1, &this->specular_material[0]);
	glUniform1f(ShaderManager::Instance()->shininess_loc, this->shininess);
	glUniform1f(ShaderManager::Instance()->height_scale_loc, this->mesh->height_scale);
	glUniform1f(ShaderManager::Instance()->step_size_loc, this->mesh->STEP_SIZE);
	
	//Form the mvp matrix from matrices supplies with draw function
	Matrix44 model_matrix = this->model;
	Matrix44 mvp = projection * view * model_matrix;


	Matrix44 view_matrix = view;
	Matrix44 modelview_matrix = view * model_matrix;

	//Send matrices to shader uniforms
	glUniformMatrix4fv(ShaderManager::Instance()->model_loc, 1, GL_TRUE, &model_matrix[0][0]);
	glUniformMatrix4fv(ShaderManager::Instance()->view_loc, 1, GL_TRUE, &view_matrix[0][0]);
	glUniformMatrix4fv(ShaderManager::Instance()->modelview_loc, 1, GL_TRUE, &modelview_matrix[0][0]);
	glUniformMatrix4fv(ShaderManager::Instance()->mvp_loc, 1, GL_TRUE, &mvp[0][0]);

	this->heightmap->BindHeightmap();

	//if(texture){
	//	this->texture->Bind();	
	//}
	if(mesh){
		this->mesh->Draw();
	}
	
	//Calls the childrens' update
	if (!this->children.empty())
	{
		for (int i = 0; i < this->children.size(); i++)
		{
			if (this->children[i] != nullptr)
			{
				this->children[i]->Draw(projection, view);
			}
		}
	}
}

void RigidBody::SetMesh(Mesh* mesh)
{
	this->mesh = mesh;
}

void RigidBody::SetID()
{
	static int current_id = 0;
	this->id = ++current_id;
}

void RigidBody::SetTexture(Texture* texture)
{
	this->texture = texture;
}

int RigidBody::ID()
{
	return this->id;
}

Matrix44 RigidBody::GetModel()
{
	return this->model;
}

void RigidBody::Rotate(float angle, int x, int y, int z, RotateAround space)
{
	if (space == Self){ //Rotates around own axis
		this->rotation_around_own_axis.Rotate(angle, x, y, z);
	}
	else if (space == World){ //Rotate around world
		this->rotation.Rotate(angle, x, y, z);
	}
	
}

void RigidBody::Rotate(float angle, Vector3 vector, RotateAround space)
{
	if (space == Self){ //Rotates around own axis
		this->rotation_around_own_axis.Rotate(angle, vector);
	}
	else if (space == World){ //Rotate around world
		this->rotation.Rotate(angle, vector);
	}
	
}

void RigidBody::SetPosition(float x, float y, float z)
{
	this->position.Insert(x, y, z);
	//this->translation.Translate(x, y, z);
}


void RigidBody::SetPosition(Vector3 vector)
{
	this->position = vector;
	//this->translation.Translate(vector);
}

void RigidBody::Translate(float x, float y, float z)
{
	this->position += Vector3(x, y, z);
}

void RigidBody::Translate(Vector3 vector)
{
	this->position += vector;
}

void RigidBody::Scale(float factor)
{
	this->scaling.Scale(factor);
}

void RigidBody::SetScale(float factor)
{
	this->scaling.SetToIdentity();
	this->scaling.Scale(factor);
}

void RigidBody::Scale(float x, float y, float z)
{
	this->scaling.Scale(x, y, z);
}

void RigidBody::Scale(Vector3 vector)
{
	this->scaling.Scale(vector);
}

//------------------------------------------------------------------------------
/**
arg1: position of object
arg2: direction of object
arg3: up vector of object
*/
void RigidBody::LookAt(Vector3 eye_position, Vector3 eye_target, Vector3 eye_up)
{
	this->look_at.LookAt(eye_position, eye_target, eye_up);

	//Invert the lookAt matrix so it can be used to move an object
	this->look_at = this->look_at.Inverse();
}