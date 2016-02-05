#pragma once

#include "node.h"
#include "mesh.h"
#include "texture.h"
#include "mypersonalmathlib.h"
#include "debugrenderer.h"
#include <vector>

/**
@class RigidBody

RigidBody class
*/
class RigidBody : public Node
{
public:
	/// Used by Rotate functions to decide how the object should rotate
	enum RotateAround
	{
		Self,
		World
	};
	
	/// Constructor
	RigidBody();
	/// Destructor
	~RigidBody(void);
	
	/// Updates the model/view matrix and then calls its childrens' update
	void Update(Matrix44& view, const Matrix44& model = Matrix44());

	/// Does nothing with this class. Just calls its childrens' update and forwards the arguments
	void Draw(const Matrix44& projection, const Matrix44& view);

	void Integrate(float timestep);

	//set for mesh, texture
	void SetMesh(Mesh* mesh);
	void SetID();
	void SetTexture(Texture* texture);
	int ID();
	
	/// Returns the model
	Matrix44 GetModel();

	/// Multiply the transform's rotation matrix with a Rotation matrix around an arbitrary axis
	void Rotate(float angle, int x, int y, int z, RotateAround space = Self);
	/// Multiply the transform's rotation matrix with a Rotation matrix around an arbitrary axis
	void Rotate(float angle, Vector3 vector, RotateAround space = Self);

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

	/// inverted camera lookat function for objects
	void LookAt(Vector3 eye_position, Vector3 eye_target, Vector3 eye_up);

	Mesh* mesh;
	Texture* texture;
	Texture* heightmap;

	//My Physics Stuff
	Vector3 position;
	bool is_kinematic = false;


	Matrix44 model;
	//Holds the rotation values of this transform, applied after translation. So rotation speed around planets can be changed
	Matrix44 rotation;
	//Holds the scaling values of this transform
	Matrix44 scaling;
	//Rotation matrix applied first to get effect "rotate around own axis"
	Matrix44 rotation_around_own_axis;

	//A Combination of Translation/Rotation matrices
	Matrix44 look_at;

private:
	int id;
	
	//Material Properties
	Vector3 ambient_material;
	Vector3 specular_material;
	float shininess;


};

//std specialization
namespace std {
	template <>
	struct hash<std::pair<RigidBody*, RigidBody*>>
	{
		inline size_t operator()(const std::pair<RigidBody*, RigidBody*> &v) const
		{
			return (v.first->ID() + v.second->ID());
		}
	};

	template <>
	struct equal_to<std::pair<RigidBody*, RigidBody*>>
	{
		inline bool operator()(const std::pair<RigidBody*, RigidBody*>& lhs, const std::pair<RigidBody*, RigidBody*>& rhs) const
		{
			return (lhs.first == rhs.first && lhs.second == rhs.second ||
				lhs.first == rhs.second && lhs.second == rhs.first);
		}
	};
}