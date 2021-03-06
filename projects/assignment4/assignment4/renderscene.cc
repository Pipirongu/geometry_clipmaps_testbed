//------------------------------------------------------------------------------
// exampleapp.cc
// (C) 2015 Individual contributors, see AUTHORS file
//------------------------------------------------------------------------------
#include "config.h"
#include "renderscene.h"

#include <cstring>

using namespace Display;
namespace Graphics
{

//------------------------------------------------------------------------------
/**
*/
RenderScene::RenderScene()
{
	//Sets radix character to . and not ,
	setlocale(LC_ALL, "POSIX");

}

//------------------------------------------------------------------------------
/**
*/
RenderScene::~RenderScene()
{
	delete this->root;
	delete this->camera;
	delete this->light;
	
	for (std::map<int, RigidBody*>::iterator it = this->object_list.begin(); it != this->object_list.end(); it++){
		delete it->second;
	}
}

//------------------------------------------------------------------------------
/**
*/
RenderScene* RenderScene::Instance()
{
	static RenderScene instance;

	return &instance;
}

//------------------------------------------------------------------------------
/**
*/
bool
RenderScene::Open()
{
	App::Open();
	this->window = new Display::Window;

	// key callback
	this->window->SetKeyPressFunction([this](int32 key, int32 scancode, int32 action, int32 mods)
	{
		if (key == GLFW_KEY_4 && action == GLFW_PRESS){
			this->wireframe_rendering_toggle++;
			if (this->wireframe_rendering_toggle == 2){
				this->wireframe_rendering_toggle = 0;
			}
			if (this->wireframe_rendering_toggle == 0){
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			else if (this->wireframe_rendering_toggle == 1){
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
		}
		if (key == GLFW_KEY_Q && action == GLFW_PRESS){
			if (this->debug_rendering_toggle == 3){
				this->debug_rendering_toggle = 0;
			}
			else{
				this->debug_rendering_toggle++;
			}
		}
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
			this->is_open = false;
		}
	});

	// close callback
	this->window->SetCloseFunction([this]()
	{
		this->is_open = false;
	});

	// window resize callback
	this->window->SetWindowSizeFunction([this](int width, int height)
	{
		if (width != 0 || height != 0){
			this->window->GetWindowSize(&this->window_width, &this->window_height);
			this->window->SetSize(this->window_width, this->window_height);
			float aspect = (float)this->window_width / (float)this->window_height;
			this->projection = Matrix44::Perspective(60, aspect, 0.1, 1000);
			TextRenderer::Instance()->SetProjection(Matrix44::Ortho(0.0f, this->window_width, 0.0f, this->window_height, -1, 1));
		}
	});

	
	//init time
	this->current_time = glfwGetTime();
	this->fps_timer = 0;

	if (this->window->Open())
	{
		glfwSwapInterval(-1);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		//glEnable(GL_CULL_FACE);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
		ShaderManager::Instance()->AddShaderProgram("shaders/simple_vs.glsl", "shaders/simple_fs.glsl", "main");
		ShaderManager::Instance()->AddShaderProgram("shaders/clipmaps_vs.glsl", "shaders/clipmaps_fs.glsl", "clipmaps");
		//// get the index layout for InstanceData
		//GLuint instance_data_index = glGetUniformBlockIndex(ShaderManager::Instance()->shaders["clipmaps"], "InstanceData");
		//// bind the uniform block to binding point
		//glUniformBlockBinding(ShaderManager::Instance()->shaders["clipmaps"], instance_data_index, 0);
		ShaderManager::Instance()->AddShaderProgram("shaders/text_vs.glsl", "shaders/text_fs.glsl", "text");

		//Objects
		this->root = new Root;
		this->camera = new Camera;
		this->clipmaps = new ClipmapGrid(CLIPMAP_SIZE, CLIPMAP_LEVELS, CLIPMAP_SCALE);



		
		this->light = new Light;

		this->root->AddChildNode(this->camera);
		this->root->AddChildNode(this->light);

		//Init the camera vectors just so the objects are rendered in the scene
		camera->UpdateCamera();

		// load projection matrix first time because resize callback won't be called until user resizes window.
		this->window->GetWindowSize(&this->window_width, &this->window_height);
		
		float aspect = (float)this->window_width / (float)this->window_height;
		this->projection = Matrix44::Perspective(60, aspect, 0.1, 1000);

		TextRenderer::Instance()->Init("fonts/font.ttf", 18);
		TextRenderer::Instance()->SetProjection(Matrix44::Ortho(0.0f, this->window_width, 0.0f, this->window_height, -1, 1));

		//plane
		int width, height;
		this->heightmap = new Texture;
		this->heightmap->LoadHeightmap("textures/mountains.png", width, height);

		this->is_open = true;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
/**
*/
void
RenderScene::Run()
{
	while(this->is_open)
	{		
		//deltatime
		double new_time = glfwGetTime();
		this->delta_time = float(new_time - this->current_time);
		this->current_time = new_time;

		//Poll for key input
		this->window->Update();
		//Moves the player
		this->CameraControls();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//Update matrices
		this->root->Update(this->view); //updates the view matrix
		//this->RenderPass();

		this->RenderClipmaps();

		ShaderManager::Instance()->ChangeShader("text");

		if ((current_time - this->fps_timer) > 0.2f){
			this->fps = std::to_string((int)(1 / this->delta_time)) + " FPS";
			this->pos = "[" + std::to_string((int)camera->position[0]) + "," + std::to_string((int)camera->position[1]) + "," + std::to_string((int)camera->position[2]) + "]";
			this->objects = "Objects: " + std::to_string(this->object_list.size());
			this->fps_timer = current_time;
		}

		TextRenderer::Instance()->SetColor(1, 1, 0);
		TextRenderer::Instance()->RenderText(fps, 20.f, this->window_height - 25.f, 1);
		TextRenderer::Instance()->SetColor(0, 1, 1);
		TextRenderer::Instance()->RenderText("Block: Red", 20.f, this->window_height - 50.f, 1);
		TextRenderer::Instance()->RenderText("Ring Fixup: Green", 20.f, this->window_height - 75.f, 1);
		TextRenderer::Instance()->RenderText("Degenerate Triangles: Blue", 20.f, this->window_height - 100.f, 1);
		TextRenderer::Instance()->RenderText("Trim: Yellow", 20.f, this->window_height - 125.f, 1);

		this->window->SwapBuffers();

	}
	this->window->Close();
}

void RenderScene::RenderPass()
{

	ShaderManager::Instance()->ChangeShader("main");

	//Send light properties to shader
	this->light->Draw(this->projection, this->view);

	//Draw objects
	for (std::map<int, RigidBody*>::iterator it = this->object_list.begin(); it != this->object_list.end(); it++){
		it->second->Draw(this->projection, this->view);
	}

	//ShaderManager::Instance()->ChangeShader("debug");
	////Draw light
	//this->light->DrawDebug(this->projection, this->view);
}

//------------------------------------------------------------------------------
/**
*/
void RenderScene::CameraControls()
{
	if (this->window->GetMouseButton(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS){
		double mouse_pos_x, mouse_pos_y;
		int width, height;
		this->window->GetCursorPos(&mouse_pos_x, &mouse_pos_y);
		this->window->GetWindowSize(&width, &height);
		double mid_x = width / 2;
		double mid_y = height / 2;

		bool skip = false;
		//Prevents camera from jumping
		if (abs(mid_x - mouse_pos_x) > 50 || abs(mid_y - mouse_pos_y) > 50){
			this->window->SetCursorPos(mid_x, mid_y);
			skip = true;
		}
		if (!skip){
			camera->MouseInput(mouse_pos_x, mouse_pos_y, width, height, this->delta_time);
			//Hide the cursor
			this->window->SetCursorMode(GLFW_CURSOR_DISABLED);
			//Set the mouse pointer to the middle of the widget
			this->window->SetCursorPos(mid_x, mid_y);
		}
	}
	else if (this->window->GetMouseButton(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE){
		this->window->SetCursorMode(GLFW_CURSOR_NORMAL);
	}

	camera->KeyboardInput(this->window->GetKey(GLFW_KEY_W), this->window->GetKey(GLFW_KEY_S), this->window->GetKey(GLFW_KEY_A), this->window->GetKey(GLFW_KEY_D), this->delta_time);

	//updates the player
	this->camera->UpdateCameraMatrix(); //update camera matrix
}

void RenderScene::RenderClipmaps()
{
	ShaderManager::Instance()->ChangeShader("clipmaps");
	//this->clipmaps->update_level_offsets(Vector2(0,0));
	this->clipmaps->update_level_offsets(Vector2(this->camera->position[0], this->camera->position[2]));

	glUniform3fv(ShaderManager::Instance()->camera_pos_loc, 1, &this->camera->position[0]);

	//update heightmap

	//bind heightmap
	this->heightmap->BindHeightmap();
	this->clipmaps->Render(this->projection, this->view);
	this->heightmap->Unbind();
}

} // namespace Example
