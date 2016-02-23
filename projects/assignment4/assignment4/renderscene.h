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
#include "clipmapgrid.h"

// Sets the size of clipmap blocks, NxN vertices per block. Should be power-of-two and no bigger than 64.
// A clipmap-level is organized roughly as 4x4 blocks with some padding. A clipmap level is a (4N-1) * (4N-1) grid.
#define CLIPMAP_SIZE 16

// Number of LOD levels for clipmap.
#define CLIPMAP_LEVELS 10

// Distance between vertices.
#define CLIPMAP_SCALE 0.25f

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

	int STEP_SIZE = 1;

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
	Light* light;

	//Clipmapgrid and heightmap :)
	ClipmapGrid* clipmaps;
	Texture* heightmap;

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
	void AddPlaneToScene(bool is_kinematic, Vector3 position, float clipmap_scale, float degrees = 0, Vector3 axis = Vector3(1, 1, 1));
	void RenderPass();
	void UpdateClipmaps();
	void RenderClipmaps();
	void CameraControls();

};

} // namespace Example
