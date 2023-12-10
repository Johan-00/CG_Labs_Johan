#include "assignment3.hpp"
#include "interpolation.hpp"
#include "parametric_shapes.hpp"

#include "config.hpp"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/node.hpp"
#include "core/ShaderProgramManager.hpp"

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <tinyfiledialogs.h>

#include <clocale>
#include <cstdlib>
#include <stdexcept>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>



static GLuint quad_vao = 0;
static GLuint quad_vbo = 0;




void render_quad()
{
	if (quad_vao == 0) {
		GLfloat quadVertices[] = {
			// Positions        // Texture Coords
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		glGenVertexArrays(1, &quad_vao);
		glGenBuffers(1, &quad_vbo);
		glBindVertexArray(quad_vao);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}
	glBindVertexArray(quad_vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

edaf80::Assignment3::Assignment3(WindowManager& windowManager) :
	mCamera(0.5f * glm::half_pi<float>(),
	        static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	        0.01f, 1000.0f),
	inputHandler(), mWindowManager(windowManager), window(nullptr)
{
	WindowManager::WindowDatum window_datum{ inputHandler, mCamera, config::resolution_x, config::resolution_y, 0, 0, 0, 0};

	window = mWindowManager.CreateGLFWWindow("EDAF80: Assignment 3", window_datum, config::msaa_rate);
	if (window == nullptr) {
		throw std::runtime_error("Failed to get a window: aborting!");
	}

	bonobo::init();
}

edaf80::Assignment3::~Assignment3()
{
	bonobo::deinit();
}


void
edaf80::Assignment3::run()
{
	// Set up the camera
	mCamera.mWorld.SetTranslate(glm::vec3(0.0f, 0.0f, 6.0f));
	mCamera.mMouseSensitivity = glm::vec2(0.003f);
	mCamera.mMovementSpeed = glm::vec3(3.0f); // 3 m/s => 10.8 km/h

	// Create the shader programs
	ShaderProgramManager program_manager;



GLuint fallback_shader = 0u;
	program_manager.CreateAndRegisterProgram("Fallback",
	                                         { { ShaderType::vertex, "common/fallback.vert" },
	                                           { ShaderType::fragment, "common/fallback.frag" } },
	                                         fallback_shader);
	if (fallback_shader == 0u) {
		LogError("Failed to load fallback shader");
		return;
	}

	GLuint diffuse_shader = 0u;
	program_manager.CreateAndRegisterProgram("Diffuse",
	                                         { { ShaderType::vertex, "EDAF80/diffuse.vert" },
	                                           { ShaderType::fragment, "EDAF80/diffuse.frag" } },
	                                         diffuse_shader);
	if (diffuse_shader == 0u)
		LogError("Failed to load diffuse shader");

	GLuint normal_shader = 0u;
	program_manager.CreateAndRegisterProgram("Normal",
	                                         { { ShaderType::vertex, "EDAF80/normal.vert" },
	                                           { ShaderType::fragment, "EDAF80/normal.frag" } },
	                                         normal_shader);
	if (normal_shader == 0u)
		LogError("Failed to load normal shader");

	GLuint texcoord_shader = 0u;
	program_manager.CreateAndRegisterProgram("Texture coords",
	                                         { { ShaderType::vertex, "EDAF80/texcoord.vert" },
	                                           { ShaderType::fragment, "EDAF80/texcoord.frag" } },
	                                         texcoord_shader);
	if (texcoord_shader == 0u)
		LogError("Failed to load texcoord shader");

	GLuint skybox_shader = 0u;
	program_manager.CreateAndRegisterProgram("Skybox",
		{ { ShaderType::vertex, "EDAF80/skybox.vert" },
			{ ShaderType::fragment, "EDAF80/skybox.frag" } },
		skybox_shader);

	if (skybox_shader == 0u)
		LogError("Failed to load texcoord shader");


	GLuint phong_shader = 0u;
	program_manager.CreateAndRegisterProgram("Phong",
											{ { ShaderType::vertex, "EDAF80/phong_texture.vert" },
											   { ShaderType::fragment, "EDAF80/phong_texture.frag" } },
												phong_shader);

	if (phong_shader == 0u) {
		LogError("Failed to load phong shader");
	}



	GLuint cloud_shader2 = 0u;
	program_manager.CreateAndRegisterProgram("cloud",
		{ { ShaderType::vertex, "EDAN35/resolve.vert" },
		   { ShaderType::fragment, "EDAN35/cloud.frag" } },
		cloud_shader2);

	if (cloud_shader2 == 0u) {
		LogError("Failed to load cloud shader");
	}

	GLuint cloud_shader = 0u;
	program_manager.CreateAndRegisterProgram("cloud",
		{ { ShaderType::vertex, "EDAN35/cloud.vert" },
		   { ShaderType::fragment, "EDAN35/cloud.frag" } },
		cloud_shader);
	
	if (cloud_shader == 0u) {
		LogError("Failed to load cloud shader");
	}

	GLuint sky_shader = 0u;
	program_manager.CreateAndRegisterProgram("sky",
		{ { ShaderType::vertex, "EDAN35/sky.vert" },
		   { ShaderType::fragment, "EDAN35/sky.frag" } },
		sky_shader);

	if (sky_shader == 0u) {
		LogError("Failed to load cloud shader");
	}


	auto light_position = glm::vec3(10000.0f, 10000.0f, 10000.0f);
	auto const set_uniforms = [&light_position](GLuint program){
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
	};

	bool use_normal_mapping = true;
	bool use_texture = true;
	auto camera_position = mCamera.mWorld.GetTranslation();
	auto const phong_set_uniforms = [&use_normal_mapping, &use_texture, &light_position, &camera_position](GLuint program) {
		glUniform1i(glGetUniformLocation(program, "use_texture"), use_texture ? 1 : 0);
		glUniform1i(glGetUniformLocation(program, "use_normal_mapping"), use_normal_mapping ? 1 : 0);
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));

	};

	auto const sky_set_uniforms = [&light_position, &camera_position](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
		};

	//
	// Set up the two spheres used.
	//
	auto skybox_shape = parametric_shapes::createSphere(50.0f, 100u, 100u);
	if (skybox_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the skybox");
		return;
	}

	Node skybox;
	skybox.set_geometry(skybox_shape);
	skybox.set_program(&sky_shader, sky_set_uniforms);

	auto demo_shape = parametric_shapes::createSphere(1.5f, 40u, 40u);
	if (demo_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the demo sphere");
		return;
	}

	bonobo::material_data demo_material;
	demo_material.ambient = glm::vec3(0.1f, 0.1f, 0.1f);
	demo_material.diffuse = glm::vec3(0.7f, 0.2f, 0.4f);
	demo_material.specular = glm::vec3(1.0f, 1.0f, 1.0f);
	demo_material.shininess = 10.0f;

	Node demo_sphere;
	demo_sphere.set_geometry(demo_shape);
	demo_sphere.set_material_constants(demo_material);
	demo_sphere.set_program(&phong_shader, phong_set_uniforms);


	// Load cube map and other textures
	//std::string skyboxtexture = "LarnacaCastle";
	//std::string skyboxtexture = "Maskonaive2";
	//std::string skyboxtexture = "NissiBeach2";
	//std::string skyboxtexture = "Teide";
	//auto cube_map_id = bonobo::loadTextureCubeMap(
	//	config::resources_path("cubemaps/"+skyboxtexture+"/posx.jpg"),
	//	config::resources_path("cubemaps/"+skyboxtexture+"/negx.jpg"),
	//	config::resources_path("cubemaps/"+skyboxtexture+"/posy.jpg"),
	//	config::resources_path("cubemaps/"+skyboxtexture+"/negy.jpg"),
	//	config::resources_path("cubemaps/"+skyboxtexture+"/posz.jpg"),
	//	config::resources_path("cubemaps/"+skyboxtexture+"/negz.jpg"));

	auto diffuse_texture_id = bonobo::loadTexture2D(config::resources_path("textures/leather_red_02_coll1_2k.jpg"));
	auto specular_texture_id = bonobo::loadTexture2D(config::resources_path("textures/leather_red_02_rough_2k.jpg"));
	auto normal_map_id = bonobo::loadTexture2D(config::resources_path("textures/leather_red_02_nor_2k.jpg"));

	//add the textures
	//skybox.add_texture("cubemap", cube_map_id, GL_TEXTURE_CUBE_MAP);

	demo_sphere.add_texture("diffuse_texture", diffuse_texture_id, GL_TEXTURE_2D);
	demo_sphere.add_texture("specular_texture", specular_texture_id, GL_TEXTURE_2D);
	demo_sphere.add_texture("normal_map", normal_map_id, GL_TEXTURE_2D);



	glClearDepthf(1.0f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);




	auto lastTime = std::chrono::high_resolution_clock::now();

	bool use_orbit_camera = false;
	std::int32_t demo_sphere_program_index = 0;
	auto cull_mode = bonobo::cull_mode_t::disabled;
	auto polygon_mode = bonobo::polygon_mode_t::fill;
	bool show_logs = true;
	bool show_gui = true;
	bool shader_reload_failed = false;
	bool show_basis = false;
	float basis_thickness_scale = 1.0f;
	float basis_length_scale = 1.0f;

	auto BoundsMin = glm::vec3(-50.0f, 15.0f, -50.0f);
	auto BoundsMax = glm::vec3(50.0f, 25.0f, 50.0f);
	auto cloudOffset = glm::vec3(0.0f, 0.0f, 0.0f);
	float cloudScale = 15.0f;
	
	float cloudDensityThreshold = 0.385f;
	float cloudDensityMultiplier = 0.33f;
	int cloudSampleCount = 20;

	float cloudDetailScale = 0.25f;
	float cloudDetailMultiplier = 0.5f; 

	changeCullMode(cull_mode);

	while (!glfwWindowShouldClose(window)) {
		auto const nowTime = std::chrono::high_resolution_clock::now();
		auto const deltaTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - lastTime);
		lastTime = nowTime;
		// Assume these variables are declared elsewhere in your code

		auto& io = ImGui::GetIO();
		inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

		glfwPollEvents();
		inputHandler.Advance();
		mCamera.Update(deltaTimeUs, inputHandler);
		if (use_orbit_camera) {
			mCamera.mWorld.LookAt(glm::vec3(0.0f));
		}
		camera_position = mCamera.mWorld.GetTranslation();

		if (inputHandler.GetKeycodeState(GLFW_KEY_R) & JUST_PRESSED) {
			shader_reload_failed = !program_manager.ReloadAllPrograms();
			if (shader_reload_failed)
				tinyfd_notifyPopup("Shader Program Reload Error",
				                   "An error occurred while reloading shader programs; see the logs for details.\n"
				                   "Rendering is suspended until the issue is solved. Once fixed, just reload the shaders again.",
				                   "error");
		}
		if (inputHandler.GetKeycodeState(GLFW_KEY_F3) & JUST_RELEASED)
			show_logs = !show_logs;
		if (inputHandler.GetKeycodeState(GLFW_KEY_F2) & JUST_RELEASED)
			show_gui = !show_gui;
		if (inputHandler.GetKeycodeState(GLFW_KEY_F11) & JUST_RELEASED)
			mWindowManager.ToggleFullscreenStatusForWindow(window);


		// Retrieve the actual framebuffer size: for HiDPI monitors,
		// you might end up with a framebuffer larger than what you
		// actually asked for. For example, if you ask for a 1920x1080
		// framebuffer, you might get a 3840x2160 one instead.
		// Also it might change as the user drags the window between
		// monitors with different DPIs, or if the fullscreen status is
		// being toggled.
		int framebuffer_width, framebuffer_height;
		glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
		glViewport(0, 0, framebuffer_width, framebuffer_height);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		mWindowManager.NewImGuiFrame();
		
		bonobo::changePolygonMode(polygon_mode);

		skybox.get_transform().SetTranslate(camera_position);

		skybox.render(mCamera.GetWorldToClipMatrix());
		demo_sphere.render(mCamera.GetWorldToClipMatrix());


		glEnable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glUseProgram(cloud_shader2);

		//-----------------cloud shader-----------------
		// Create a custom framebuffer object (FBO)
		//GLuint customFBO;
		//glGenFramebuffers(1, &customFBO);
		//glBindFramebuffer(GL_FRAMEBUFFER, customFBO);
		//
		//// Create and bind a depth texture buffer
		//GLuint depthTexture;
		//glGenTextures(1, &depthTexture);
		//glBindTexture(GL_TEXTURE_2D, depthTexture);
		//
		//// Set the base level of the texture
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0); // Set base level to 1 or the appropriate level
		//

		//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, framebuffer_width, framebuffer_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, framebuffer_width, framebuffer_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//
		//// Attach the depth texture to the custom FBO
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
		//
		//// Check if framebuffer is complete
		//if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		//	// Handle framebuffer incomplete error here
		//	printf("Framebuffer incomplete\n");
		//}
		//
		//// Bind your custom FBO for rendering
		//glBindFramebuffer(GL_FRAMEBUFFER, customFBO);
		//
		//// Use your cloud shader program
		//glUseProgram(cloud_shader2);


		// Pass depth texture to cloud shader
		//glActiveTexture(GL_TEXTURE0); // Use texture unit 0
		//glBindTexture(GL_TEXTURE_2D, depthTexture);
		//glUniform1i(glGetUniformLocation(cloud_shader2, "depthTexture"), 0); // Bind to texture unit 0

		// Other uniform assignments for cloud shader

		glUniform3fv(glGetUniformLocation(cloud_shader2, "camera_position"), 1, glm::value_ptr(camera_position));
		glUniform3fv(glGetUniformLocation(cloud_shader2, "light_position"), 1, glm::value_ptr(light_position));
		glUniform2fv(glGetUniformLocation(cloud_shader2, "view_port"), 1, glm::value_ptr(glm::vec3(framebuffer_width, framebuffer_height, 0.0f)));
		//printf("%d %d\n", framebuffer_width, framebuffer_height);
		glUniformMatrix4fv(glGetUniformLocation(cloud_shader2, "inv_proj"), 1, GL_FALSE, glm::value_ptr(mCamera.GetClipToViewMatrix()));
		glUniformMatrix4fv(glGetUniformLocation(cloud_shader2, "inv_view"), 1, GL_FALSE, glm::value_ptr(mCamera.GetViewToWorldMatrix()));

		glUniform3fv(glGetUniformLocation(cloud_shader2, "BoundsMin"), 1, glm::value_ptr(BoundsMin)); 
		glUniform3fv(glGetUniformLocation(cloud_shader2, "BoundsMax"), 1, glm::value_ptr(BoundsMax)); 
		glUniform3fv(glGetUniformLocation(cloud_shader2, "cloudOffset"), 1, glm::value_ptr(cloudOffset)); 
		glUniform1f(glGetUniformLocation(cloud_shader2, "cloudScale"), cloudScale);
		
		glUniform1f(glGetUniformLocation(cloud_shader2, "cloudDensityThreshold"), cloudDensityThreshold);
		glUniform1f(glGetUniformLocation(cloud_shader2, "cloudDensityMultiplier"), cloudDensityMultiplier);
		glUniform1i(glGetUniformLocation(cloud_shader2, "cloudSampleCount"), cloudSampleCount);

		glUniform1f(glGetUniformLocation(cloud_shader2, "cloudDetailScale"), cloudDetailScale);
		glUniform1f(glGetUniformLocation(cloud_shader2, "cloudDetailMultiplier"), cloudDetailMultiplier);

		// Unbind custom FBO and depth texture
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glBindTexture(GL_TEXTURE_2D, 0);

		render_quad();
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		bool opened = ImGui::Begin("Scene Control", nullptr, ImGuiWindowFlags_None);
		if (opened) {
			auto const cull_mode_changed = bonobo::uiSelectCullMode("Cull mode", cull_mode);
			if (cull_mode_changed) {
				changeCullMode(cull_mode);
			}
			bonobo::uiSelectPolygonMode("Polygon mode", polygon_mode);

			auto demo_sphere_selection_result = program_manager.SelectProgram("Demo sphere", demo_sphere_program_index);
			if (demo_sphere_selection_result.was_selection_changed) {
				demo_sphere.set_program(demo_sphere_selection_result.program, phong_set_uniforms);
			}

			ImGui::Separator();
			ImGui::Checkbox("Use normal mapping", &use_normal_mapping);
			ImGui::ColorEdit3("Ambient", glm::value_ptr(demo_material.ambient));
			ImGui::ColorEdit3("Diffuse", glm::value_ptr(demo_material.diffuse));
			ImGui::ColorEdit3("Specular", glm::value_ptr(demo_material.specular));
			ImGui::SliderFloat("Shininess", &demo_material.shininess, 1.0f, 1000.0f);
			ImGui::SliderFloat3("Light Position", glm::value_ptr(light_position), -10000.0f, 10000.0f);
			ImGui::Separator();
			ImGui::Checkbox("Use orbit camera", &use_orbit_camera);
			ImGui::Separator();
			ImGui::Checkbox("Show basis", &show_basis);
			ImGui::SliderFloat("Basis thickness scale", &basis_thickness_scale, 0.0f, 100.0f);
			ImGui::SliderFloat("Basis length scale", &basis_length_scale, 0.0f, 100.0f);
			ImGui::Separator();
			ImGui::SliderFloat3("BoundsMin", glm::value_ptr(BoundsMin), -1000.0f, 1000.0f);
			ImGui::SliderFloat3("BoundsMax", glm::value_ptr(BoundsMax), -1000.0f, 1000.0f);
			ImGui::SliderFloat3("cloudOffset", glm::value_ptr(cloudOffset), -100.0f, 100.0f);
			ImGui::SliderFloat("cloudScale", &cloudScale, 0.0f, 100.0f);
			ImGui::SliderFloat("cloudDensityThreshold", &cloudDensityThreshold, 0.0f, 1.0f);
			ImGui::SliderFloat("cloudDensityMultiplier", &cloudDensityMultiplier, 0.0f, 1.0f);
			ImGui::SliderInt("cloudSampleCount", &cloudSampleCount, 0, 1000);
			ImGui::Separator();
			ImGui::SliderFloat("cloudDetailScale", &cloudDetailScale, 0.0f, 5);
			ImGui::SliderFloat("cloudDetailMultiplier", &cloudDetailMultiplier, 0.0f, 10.0f);
			
			ImGui::Separator();
		}
		ImGui::End();

		demo_sphere.set_material_constants(demo_material);

		if (show_basis)
			bonobo::renderBasis(basis_thickness_scale, basis_length_scale, mCamera.GetWorldToClipMatrix());

		opened = ImGui::Begin("Render Time", nullptr, ImGuiWindowFlags_None);
		if (opened)
			ImGui::Text("%.3f ms", std::chrono::duration<float, std::milli>(deltaTimeUs).count());
		ImGui::End();

		if (show_logs)
			Log::View::Render();
		mWindowManager.RenderImGuiFrame(show_gui);

		glfwSwapBuffers(window);
	}
}

int main()
{
	std::setlocale(LC_ALL, "");

	Bonobo framework;

	try {
		edaf80::Assignment3 assignment3(framework.GetWindowManager());
		assignment3.run();
	} catch (std::runtime_error const& e) {
		LogError(e.what());
	}
}
