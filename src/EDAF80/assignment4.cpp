#include "assignment4.hpp"
#include "parametric_shapes.hpp"

#include "config.hpp"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/helpers.hpp"
#include "core/node.hpp"
#include "core/ShaderProgramManager.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <tinyfiledialogs.h>

#include <clocale>
#include <stdexcept>

edaf80::Assignment4::Assignment4(WindowManager& windowManager) :
	mCamera(0.5f * glm::half_pi<float>(),
		static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
		0.01f, 1000.0f),
	inputHandler(), mWindowManager(windowManager), window(nullptr)
{
	WindowManager::WindowDatum window_datum{ inputHandler, mCamera, config::resolution_x, config::resolution_y, 0, 0, 0, 0 };

	window = mWindowManager.CreateGLFWWindow("EDAF80: Assignment 4", window_datum, config::msaa_rate);
	if (window == nullptr) {
		throw std::runtime_error("Failed to get a window: aborting!");
	}

	bonobo::init();
}

edaf80::Assignment4::~Assignment4()
{
	bonobo::deinit();
}

void
edaf80::Assignment4::run()
{

	// Set up the camera
	mCamera.mWorld.SetTranslate(glm::vec3(0.0f, 0.0f, 6.0f));
	mCamera.mMouseSensitivity = glm::vec2(0.003f);
	mCamera.mMovementSpeed = glm::vec3(3.0f); // 3 m/s => 10.8 km/h

	glm::vec3 deep_color = glm::vec3(0.0f, 0.0f, 0.1f);
	glm::vec3 shallow_color = glm::vec3(0.0f, 0.5f, 0.5f);

	float amplitudes[2] = { 1.0, 0.5 };
	float frequencies[2] = { 0.2, 0.4 };
	float phases[2] = { 0.5, 1.3 };
	float sharpness[2] = { 2.0, 2.0 };
	glm::vec2 directions[2] = { glm::vec2(-1.0,0.0),glm::vec2(-0.7,0.7) };

	float elapsed_time_s = 0.0f;

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


	GLuint water_shader = 0u;
	program_manager.CreateAndRegisterProgram("Water",
		{ { ShaderType::vertex, "EDAF80/water.vert" },
		   { ShaderType::fragment, "EDAF80/water.frag" } },
		water_shader);

	if (water_shader == 0u) {
		LogError("Failed to load water shader");
	}


	auto light_position = glm::vec3(-2.0f, 4.0f, 2.0f);
	auto const set_uniforms = [&light_position](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
	};


	//
	// Set up shapes.
	//
	auto skybox_shape = parametric_shapes::createSphere(20.0f, 100u, 100u);
	if (skybox_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the skybox");
		return;
	}
	auto water_shape = parametric_shapes::createQuad(100.0f, 100.0f, 1000.0f, 1000.0f);
	if (water_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the demo sphere");
		return;
	}


	auto camera_position = mCamera.mWorld.GetTranslation();
	auto const water_set_uniforms = [&amplitudes, &frequencies, &sharpness, &phases, &directions,
		&elapsed_time_s, &deep_color, &shallow_color, &camera_position](GLuint program) {
		glUniform1fv(glGetUniformLocation(program, "amplitudes"), 2, amplitudes);
		glUniform1fv(glGetUniformLocation(program, "frequencies"), 2, frequencies);
		glUniform1fv(glGetUniformLocation(program, "sharpness"), 2, sharpness);
		glUniform1fv(glGetUniformLocation(program, "phases"), 2, phases);
		glUniform2fv(glGetUniformLocation(program, "directions"), 2, glm::value_ptr(directions[0]));
		glUniform1f(glGetUniformLocation(program, "elapsed_time_s"), elapsed_time_s);
		glUniform3fv(glGetUniformLocation(program, "deep_color"), 1, glm::value_ptr(deep_color));
		glUniform3fv(glGetUniformLocation(program, "shallow_color"), 1, glm::value_ptr(shallow_color));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
	};

	//
	// Set up nodes.
	//
	Node skybox;
	skybox.set_geometry(skybox_shape);
	skybox.set_program(&skybox_shader, set_uniforms);

	Node water;
	water.set_geometry(water_shape);
	water.get_transform().SetTranslate(glm::vec3(-50.0f, -5.0f, -50.0f));
	water.set_program(&water_shader, water_set_uniforms);

	// Load cube map and other textures
	std::string skyboxtexture = "NissiBeach2";
	auto cube_map_id = bonobo::loadTextureCubeMap(
		config::resources_path("cubemaps/" + skyboxtexture + "/posx.jpg"),
		config::resources_path("cubemaps/" + skyboxtexture + "/negx.jpg"),
		config::resources_path("cubemaps/" + skyboxtexture + "/posy.jpg"),
		config::resources_path("cubemaps/" + skyboxtexture + "/negy.jpg"),
		config::resources_path("cubemaps/" + skyboxtexture + "/posz.jpg"),
		config::resources_path("cubemaps/" + skyboxtexture + "/negz.jpg"));

	auto water_texture_id = bonobo::loadTexture2D(config::resources_path("textures/waves.png"));

	//add the textures
	skybox.add_texture("cubemap", cube_map_id, GL_TEXTURE_CUBE_MAP);
	water.add_texture("my_reflection_cube", cube_map_id, GL_TEXTURE_CUBE_MAP);
	water.add_texture("my_ripple", water_texture_id, GL_TEXTURE_2D);

	glClearDepthf(1.0f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);


	auto lastTime = std::chrono::high_resolution_clock::now();

	bool pause_animation = false;
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

	changeCullMode(cull_mode);

	while (!glfwWindowShouldClose(window)) {
		auto const nowTime = std::chrono::high_resolution_clock::now();
		auto const deltaTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - lastTime);
		lastTime = nowTime;
		if (!pause_animation) {
			elapsed_time_s += std::chrono::duration<float>(deltaTimeUs).count();
		}

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
		if (inputHandler.GetKeycodeState(GLFW_KEY_1) & JUST_PRESSED) {
			water.set_program(&fallback_shader, set_uniforms);
		}
		/*if (inputHandler.GetKeycodeState(GLFW_KEY_2) & JUST_PRESSED) {
			quad.set_program(&default_shader, set_uniforms);
		}
		if (inputHandler.GetKeycodeState(GLFW_KEY_3) & JUST_PRESSED) {
			quad.set_program(&diffuse_shader, set_uniforms);
		}*/
		if (inputHandler.GetKeycodeState(GLFW_KEY_5) & JUST_PRESSED) {
			water.set_program(&water_shader, water_set_uniforms);
		}
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

		//
		// Todo: If you need to handle inputs, you can do it here
		//


		mWindowManager.NewImGuiFrame();

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		bonobo::changePolygonMode(polygon_mode);



		if (!shader_reload_failed) {
			skybox.render(mCamera.GetWorldToClipMatrix());
			water.render(mCamera.GetWorldToClipMatrix());
		}


		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//
		// Todo: If you want a custom ImGUI window, you can set it up
		//       here
		//


		bool opened = ImGui::Begin("Scene Control", nullptr, ImGuiWindowFlags_None);
		if (opened) {
			ImGui::Checkbox("Pause animation", &pause_animation);
			ImGui::Checkbox("Use orbit camera", &use_orbit_camera);
			ImGui::Separator();
			auto const cull_mode_changed = bonobo::uiSelectCullMode("Cull mode", cull_mode);
			if (cull_mode_changed) {
				changeCullMode(cull_mode);
			}
			bonobo::uiSelectPolygonMode("Polygon mode", polygon_mode);
			
			ImGui::Separator();
			ImGui::Checkbox("Show basis", &show_basis);
			ImGui::SliderFloat("Basis thickness scale", &basis_thickness_scale, 0.0f, 100.0f);
			ImGui::SliderFloat("Basis length scale", &basis_length_scale, 0.0f, 100.0f);
			ImGui::Separator();
			ImGui::ColorEdit3("Deep", glm::value_ptr(deep_color));
			ImGui::ColorEdit3("Shallow", glm::value_ptr(shallow_color));
			ImGui::Separator();
			float* p = amplitudes;
			for (size_t i = 0; i < 2; ++i) {
				ImGui::SliderFloat((std::string("Amplitude ") + std::to_string(i + 1)).c_str(), &amplitudes[i], 0.0f, 10.0f);
				ImGui::SliderFloat((std::string("Frequency ") + std::to_string(i + 1)).c_str(), &frequencies[i], 0.0f, 10.0f);
				ImGui::SliderFloat((std::string("Phase ") + std::to_string(i + 1)).c_str(), &phases[i], 0.0f, 10.0f);
				ImGui::SliderFloat((std::string("Sharpness ") + std::to_string(i + 1)).c_str(), &sharpness[i], 0.0f, 10.0f);
				ImGui::SliderFloat2((std::string("Direction ") + std::to_string(i + 1)).c_str(), glm::value_ptr(directions[i]), -1.0, 1.0);
			}
		}
		ImGui::End();

		if (show_basis)
			bonobo::renderBasis(basis_thickness_scale, basis_length_scale, mCamera.GetWorldToClipMatrix());
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
		edaf80::Assignment4 assignment4(framework.GetWindowManager());
		assignment4.run();
	}
	catch (std::runtime_error const& e) {
		LogError(e.what());
	}
}
