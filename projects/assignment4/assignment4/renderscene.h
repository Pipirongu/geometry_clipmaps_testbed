//------------------------------------------------------------------------------
/**
	Application class used for example application.
	
	(C) 2015 Individual contributors, see AUTHORS file
*/
//------------------------------------------------------------------------------
#pragma once

#include "core/app.h"
#include "render/window.h"
#include "root.h"
#include "rigidbody.h"
#include "camera.h"
#include "mesh.h"
#include "texture.h"
#include "light.h"
#include "shadermanager.h"
#include "debugrenderer.h"
#include "mypersonalmathlib.h"
#include "textrenderer.h"
#include <unordered_set>
#include <algorithm>
#include <limits>

namespace Graphics
{
class RenderScene : public Core::App
{
public:
	/// destructor
	~RenderScene();

	///Static function to get the static instance
	static RenderScene* Instance();

	/// open app
	bool Open();
	/// run app
	void Run();

private:
	std::string fps;
	std::string pos;
	std::string objects;

	bool is_open = false;
	int debug_rendering_toggle = 2;
	int wireframe_rendering_toggle = -1;

	//view, projection matrices
	Matrix44 view;
	Matrix44 projection;

	//Nodes
	Root* root;
	Camera* camera;
	Mesh* plane_mesh;
	Texture* rigidbody_texture;
	Texture* heightmap;
	Light* light;
	
	std::map<int, RigidBody*> object_list;

	float delta_time;

	double current_time;
	double fps_timer;
	int window_width, window_height;

	
	Display::Window* window;

	/// Constructor
	RenderScene();
	///Private copy constructor to prevent creating an instance other than calling the Instance() function
	RenderScene(const RenderScene&);
	///Private assignment constructor to prevent creating an instance other than calling the Instance() function
	RenderScene& operator=(const RenderScene&);
	void AddPlaneToScene(bool is_kinematic, Vector3 position, float degrees = 0, Vector3 axis = Vector3(1, 1, 1));
	void RenderPass();
	void CameraControls();

	int TestAABB_AABB(AABB *a, AABB *b);
};

} // namespace Example
