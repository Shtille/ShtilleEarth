#include "../../sht/include/sht.h"
#include "../../sht/graphics/include/model/sphere_model.h"
#include "../../sht/graphics/include/renderer/text.h"
#include "../../sht/utility/include/console.h"
#include "../../sht/utility/include/camera.h"
#include "../../sht/math/frustum.h"
#include "../../sht/geo/include/constants.h"
#include "../../sht/geo/include/planet_navigation.h"
#include "../../sht/geo/include/planet_cube.h"
#include "earth_service.h"

#include <cmath>

namespace {
    const float kCameraDistance = sht::geo::kEarthRadius * 5.0f;
    const sht::math::Vector3 kEarthPosition(0.0f, 0.0f, 0.0f);
    /*
     The distance from Earth to Sun is 1.52*10^11 meters, so practically we dont need to compute
     vector to Sun for each vertex. Thus we just use sun direction vector.
     */
    const sht::math::Vector3 kSunDirection(1.0f, 0.0f, 0.0f);
}

class ShtilleEarthApp : public sht::OpenGlApplication
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

		fps_text_ = sht::graphics::DynamicText::Create(renderer_, 30);
        if (!fps_text_)
            return false;

        camera_manager_ = new sht::utility::CameraManager();

		earth_service_ = new EarthService();
        
        planet_navigation_ = new sht::geo::PlanetNavigation(camera_manager_, sht::geo::kEarthRadius, kCameraDistance, 100.0f);

		planet_ = new sht::geo::PlanetCube(earth_service_, renderer_, planet_shader_, camera_manager_, &frustum_, sht::geo::kEarthRadius);
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

		frustum_.Load(projection_view_matrix_);

		planet_->Update();

		BindShaderVariables();
    }
	void RenderPlanetCube()
	{
		renderer_->PushMatrix();
		renderer_->Translate(kEarthPosition);

		// Model matrix should be taken after all transforms took place
		sht::math::Matrix4 mvp = projection_view_matrix_ * renderer_->model_matrix();

		planet_shader_->Bind();
		planet_shader_->UniformMatrix4fv("u_projection_view_model", mvp);

		//renderer_->EnableWireframeMode();
		planet_->Render();
		//renderer_->DisableWireframeMode();

		planet_shader_->Unbind();

		renderer_->PopMatrix();
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
    void OnKeyDown(sht::PublicKey key, int mods) final
    {
        if (key == sht::PublicKey::kF)
        {
            ToggleFullscreen();
        }
        else if (key == sht::PublicKey::kEscape)
        {
            Application::Terminate();
        }
		else if (key == sht::PublicKey::kF5)
		{
			renderer_->TakeScreenshot("screenshots");
		}
        else if (key == sht::PublicKey::kEqual)
        {
            planet_navigation_->SmoothZoomIn();
        }
        else if (key == sht::PublicKey::kMinus)
        {
            planet_navigation_->SmoothZoomOut();
        }
        else if (key == sht::PublicKey::kSpace)
        {
            camera_animation_stopped_ = !camera_animation_stopped_;
        }
        else if (key == sht::PublicKey::kR)
        {
            bool shift_presseed = (mods & sht::ModifierKey::kShift) == sht::ModifierKey::kShift;
            float angle_x = 0.25f * sht::math::kPi; // rotation by Pi/4
            if (shift_presseed)
                angle_x = -angle_x; // opposite direction
            planet_navigation_->SmoothRotation(angle_x);
        }
    }
    void OnMouseDown(sht::MouseButton button, int modifiers) final
    {
        if (mouse_.button_down(sht::MouseButton::kLeft))
        {
            const sht::math::Vector4& viewport = renderer_->viewport();
            const sht::math::Matrix4& proj = renderer_->projection_matrix();
            const sht::math::Matrix4& view = renderer_->view_matrix();
            planet_navigation_->PanBegin(mouse_.x(), mouse_.y(), viewport, proj, view);
        }
    }
    void OnMouseUp(sht::MouseButton button, int modifiers) final
    {
        if (mouse_.button_down(sht::MouseButton::kLeft))
        {
            planet_navigation_->PanEnd();
        }
    }
    void OnMouseMove() final
    {
        if (mouse_.button_down(sht::MouseButton::kLeft))
        {
            const sht::math::Vector4& viewport = renderer_->viewport();
            const sht::math::Matrix4& proj = renderer_->projection_matrix();
            const sht::math::Matrix4& view = renderer_->view_matrix();
            planet_navigation_->PanMove(mouse_.x(), mouse_.y(), viewport, proj, view);
        }
    }
    void OnSize(int w, int h) final
    {
        Application::OnSize(w, h);
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
            renderer_->SetProjectionMatrix(sht::math::PerspectiveMatrix(45.0f, width(), height(), znear, zfar));
        }
    }
    
private:
	sht::geo::PlanetCube * planet_;
	sht::graphics::Shader * planet_shader_;
    sht::graphics::Shader * text_shader_;
    sht::graphics::Font * font_;
    sht::graphics::DynamicText * fps_text_;
    sht::utility::CameraManager * camera_manager_;
    sht::geo::PlanetNavigation * planet_navigation_;

	EarthService * earth_service_;

	sht::math::Frustum frustum_;
    
    sht::math::Matrix4 projection_view_matrix_;
    
    bool need_update_projection_matrix_;
    bool camera_animation_stopped_;
};

DECLARE_MAIN(ShtilleEarthApp);