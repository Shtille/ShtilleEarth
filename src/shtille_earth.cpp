#include "earth_service.h"
#include "planet/constants.h"
#include "planet/planet_cube.h"

// I hope in later days all the modules will be moved into scythe library
#include "planet/planet_navigation.h"

#include "declare_main.h"
#include "model/sphere_model.h"
#include "graphics/text.h"
#include "camera.h"
#include "math/frustum.h"
#include "math/constants.h"

#include <cmath>

namespace {
	const float kCameraDistance = kEarthRadius * 5.0f;
	const scythe::Vector3 kEarthPosition(0.0f, 0.0f, 0.0f);
	/*
	 The distance from Earth to Sun is 1.52*10^11 meters, so practically we dont need to compute
	 vector to Sun for each vertex. Thus we just use sun direction vector.
	 */
	const scythe::Vector3 kSunDirection(1.0f, 0.0f, 0.0f);
}

class ShtilleEarthApp : public scythe::OpenGlApplication
					  , public scythe::DesktopInputListener
{
public:
	ShtilleEarthApp()
		: planet_(nullptr)
		, font_(nullptr)
		, fps_text_(nullptr)
		, camera_manager_(nullptr)
		, planet_navigation_(nullptr)
		, earth_service_(nullptr)
		, need_update_projection_matrix_(true)
		, camera_animation_stopped_(false)
	{
		SetInputListener(this);
	}
	const char* GetTitle() final
	{
		return "Shtille Earth";
	}
	const bool IsMultisample() final
	{
		return true;
	}
	void BindShaderConstants()
	{
		planet_shader_->Bind();
		planet_shader_->Uniform1f("u_planet_radius", planet_->radius());
		planet_shader_->Uniform1i("u_texture", 0);
		planet_shader_->Unbind();
	}
	void BindShaderVariables()
	{
	}
	bool Load() final
	{        
		// Load shaders
		const char *attribs[] = {"a_position", "a_normal", "a_texcoord"};
		if (!renderer_->AddShader(planet_shader_, "data/shaders/planet/planet_tile", attribs, 1)) return false;
		if (!renderer_->AddShader(text_shader_, "data/shaders/text", attribs, 1)) return false;

		renderer_->AddFont(font_, "data/fonts/GoodDog.otf");
		if (font_ == nullptr)
			return false;

		fps_text_ = scythe::DynamicText::Create(renderer_, 30);
		if (!fps_text_)
			return false;

		camera_manager_ = new scythe::CameraManager();

		earth_service_ = new EarthService();
		
		const float kAnimationTime = 1.0f;
		planet_navigation_ = new scythe::PlanetNavigation(camera_manager_, kEarthPosition, kEarthRadius, kAnimationTime, kCameraDistance, 100.0f);

		planet_ = new PlanetCube(earth_service_, renderer_, planet_shader_, camera_manager_, &frustum_, kEarthPosition, kEarthRadius);
		if (!planet_->Initialize())
			return false;

		planet_->SetParameters(0.7853981f/*fovy*/, height_);

		// Finally bind constants
		BindShaderConstants();
		
		return true;
	}
	void Unload() final
	{
		if (planet_)
			delete planet_;
		if (planet_navigation_)
			delete planet_navigation_;
		if (earth_service_)
			delete earth_service_;
		if (camera_manager_)
			delete camera_manager_;
		if (fps_text_)
			delete fps_text_;
	}
	void Update() final
	{
		const float kFrameTime = GetFrameTime();

		if (!camera_animation_stopped_)
			camera_manager_->Update(kFrameTime);

		// Update matrices
		renderer_->SetViewMatrix(camera_manager_->view_matrix());
		UpdateProjectionMatrix();
		projection_view_matrix_ = renderer_->projection_matrix() * renderer_->view_matrix();

		frustum_.Set(projection_view_matrix_);

		planet_->Update();

		BindShaderVariables();
	}
	void RenderPlanetCube()
	{
		//renderer_->EnableWireframeMode();
		planet_->Render();
		//renderer_->DisableWireframeMode();
	}
	void RenderInterface()
	{
		renderer_->DisableDepthTest();
		
		// Draw FPS
		text_shader_->Bind();
		text_shader_->Uniform1i("u_texture", 0);
		text_shader_->Uniform4f("u_color", 1.0f, 0.5f, 1.0f, 1.0f);
		fps_text_->SetText(font_, 0.0f, 0.8f, 0.05f, L"fps: %.2f", GetFrameRate());
		fps_text_->Render();
		
		renderer_->EnableDepthTest();
	}
	void Render() final
	{
		renderer_->SetViewport(width_, height_);
		
		renderer_->ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		renderer_->ClearColorAndDepthBuffers();
		
		RenderPlanetCube();

		RenderInterface();
	}
	void OnChar(unsigned short code) final
	{

	}
	void OnKeyDown(scythe::PublicKey key, int mods) final
	{
		if (key == scythe::PublicKey::kF)
		{
			ToggleFullscreen();
		}
		else if (key == scythe::PublicKey::kEscape)
		{
			DesktopApplication::Terminate();
		}
		else if (key == scythe::PublicKey::kF5)
		{
			renderer_->TakeScreenshot("screenshots");
		}
		else if (key == scythe::PublicKey::kEqual)
		{
			planet_navigation_->SmoothZoomIn();
		}
		else if (key == scythe::PublicKey::kMinus)
		{
			planet_navigation_->SmoothZoomOut();
		}
		else if (key == scythe::PublicKey::kSpace)
		{
			camera_animation_stopped_ = !camera_animation_stopped_;
		}
		else if (key == scythe::PublicKey::kR)
		{
			bool shift_presseed = (mods & scythe::ModifierKey::kShift) == scythe::ModifierKey::kShift;
			float angle_x = 0.25f * scythe::kPi; // rotation by Pi/4
			if (shift_presseed)
				angle_x = -angle_x; // opposite direction
			planet_navigation_->SmoothRotation(angle_x);
		}
	}
	void OnKeyUp(scythe::PublicKey key, int modifiers) final
	{

	}
	void OnMouseDown(scythe::MouseButton button, int modifiers) final
	{
		if (mouse_.button_down(scythe::MouseButton::kLeft))
		{
			const scythe::Vector4& viewport = renderer_->viewport();
			const scythe::Matrix4& proj = renderer_->projection_matrix();
			const scythe::Matrix4& view = renderer_->view_matrix();
			planet_navigation_->PanBegin(mouse_.x(), mouse_.y(), viewport, proj, view);
		}
	}
	void OnMouseUp(scythe::MouseButton button, int modifiers) final
	{
		if (mouse_.button_down(scythe::MouseButton::kLeft))
		{
			planet_navigation_->PanEnd();
		}
	}
	void OnMouseMove() final
	{
		if (mouse_.button_down(scythe::MouseButton::kLeft))
		{
			const scythe::Vector4& viewport = renderer_->viewport();
			const scythe::Matrix4& proj = renderer_->projection_matrix();
			const scythe::Matrix4& view = renderer_->view_matrix();
			planet_navigation_->PanMove(mouse_.x(), mouse_.y(), viewport, proj, view);
		}
	}
	void OnScroll(float delta_x, float delta_y) final
	{

	}
	void OnSize(int w, int h) final
	{
		DesktopApplication::OnSize(w, h);
		// To have correct perspective when resizing
		need_update_projection_matrix_ = true;
	}
	void UpdateProjectionMatrix()
	{
		if (need_update_projection_matrix_ || camera_manager_->animated())
		{
			need_update_projection_matrix_ = false;

			float znear, zfar;
			planet_navigation_->ObtainZNearZFar(&znear, &zfar);
			scythe::Matrix4 projection;
			scythe::Matrix4::CreatePerspective(45.0f, aspect_ratio_, znear, zfar, &projection);
			renderer_->SetProjectionMatrix(projection);
		}
	}
	
private:
	PlanetCube * planet_;
	scythe::Shader * planet_shader_;
	scythe::Shader * text_shader_;
	scythe::Font * font_;
	scythe::DynamicText * fps_text_;
	scythe::CameraManager * camera_manager_;
	scythe::PlanetNavigation * planet_navigation_;

	EarthService * earth_service_;

	scythe::Frustum frustum_;
	
	scythe::Matrix4 projection_view_matrix_;
	
	bool need_update_projection_matrix_;
	bool camera_animation_stopped_;
};

DECLARE_MAIN(ShtilleEarthApp);