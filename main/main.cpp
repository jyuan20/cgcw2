#include <glad.h>
#include <GLFW/glfw3.h>

#include <typeinfo>
#include <stdexcept>

#include <cstdio>
#include <cstdlib>

#include "../support/error.hpp"
#include "../support/program.hpp"
#include "../support/checkpoint.hpp"
#include "../support/debug_output.hpp"

#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"

#include "defaults.hpp"
#include "simple_mesh.hpp"
#include "loadobj.hpp"
#include "stb_image.h"
#include "cylinder.hpp"
#include "cone.hpp"
#include "box.hpp"

namespace
{
	constexpr char const* kWindowTitle = "COMP3811 - CW2";

	constexpr float kPi_ = 3.1415926f;

	constexpr float kMovementPerSecond_ = 5.f; // units per second
	constexpr float kMouseSensitivity_ = 0.01f; // radians per pixel

	struct State_
	{
		enum class CameraMode { Default, FixedDistance, GroundFixed };
		CameraMode currentCameraMode = CameraMode::Default;
		ShaderProgram* prog;

		struct CamCtrl_
		{
			bool cameraActive;
			bool actionZoomIn, actionZoomOut;

			float phi, theta;
			float radius;

			float lastX, lastY;
			float movementSpeed = 5.f;

		} camControl;
		bool isAnimating;
		float animationTime;
		Vec3f initialPosition; // Store initial position for reset
		Vec3f cylinderPosition; // Add cylinder position here
	};

	void glfw_callback_error_(int, char const*);

	void glfw_callback_key_(GLFWwindow*, int, int, int, int);
	void glfw_callback_motion_(GLFWwindow*, double, double);

	struct GLFWCleanupHelper
	{
		~GLFWCleanupHelper();
	};
	struct GLFWWindowDeleter
	{
		~GLFWWindowDeleter();
		GLFWwindow* window;
	};
}

GLuint loadTexture(const char* aPath) {
	assert(aPath);

	stbi_set_flip_vertically_on_load(true);
	int w, h, channels;
	unsigned char* ptr = stbi_load(aPath, &w, &h, &channels, 4);
	if (!ptr) {
		throw Error("Unable to load image '%s'\n", aPath);
	}

	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, ptr);
	stbi_image_free(ptr);

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 6.f);

	return tex;
}


int main() try
{
	// Initialize GLFW
	if (GLFW_TRUE != glfwInit())
	{
		char const* msg = nullptr;
		int ecode = glfwGetError(&msg);
		throw Error("glfwInit() failed with '%s' (%d)", msg, ecode);
	}

	// Ensure that we call glfwTerminate() at the end of the program.
	GLFWCleanupHelper cleanupHelper;

	// Configure GLFW and create window
	glfwSetErrorCallback(&glfw_callback_error_);

	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

	//glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_DEPTH_BITS, 24);

#	if !defined(NDEBUG)
	// When building in debug mode, request an OpenGL debug context. This
	// enables additional debugging features. However, this can carry extra
	// overheads. We therefore do not do this for release builds.
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#	endif // ~ !NDEBUG

	GLFWwindow* window = glfwCreateWindow(
		1280,
		720,
		kWindowTitle,
		nullptr, nullptr
	);

	if (!window)
	{
		char const* msg = nullptr;
		int ecode = glfwGetError(&msg);
		throw Error("glfwCreateWindow() failed with '%s' (%d)", msg, ecode);
	}

	GLFWWindowDeleter windowDeleter{ window };

	// Set up event handling
	// TODO: Additional event handling setup

	State_ state{};
	state.isAnimating = false;
	state.animationTime = 0.0f;
	state.initialPosition = { 0.0f, -0.85f, 16.0f }; // Replace this with your initial position

	glfwSetWindowUserPointer(window, &state);

	glfwSetKeyCallback(window, &glfw_callback_key_);
	glfwSetCursorPosCallback(window, &glfw_callback_motion_);

	// Set up drawing stuff
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // V-Sync is on.

	// Initialize GLAD
	// This will load the OpenGL API. We mustn't make any OpenGL calls before this!
	if (!gladLoadGLLoader((GLADloadproc)&glfwGetProcAddress))
		throw Error("gladLoaDGLLoader() failed - cannot load GL API!");

	std::printf("RENDERER %s\n", glGetString(GL_RENDERER));
	std::printf("VENDOR %s\n", glGetString(GL_VENDOR));
	std::printf("VERSION %s\n", glGetString(GL_VERSION));
	std::printf("SHADING_LANGUAGE_VERSION %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	SimpleMeshData parlahti = load_wavefront_obj("assets/parlahti.obj");
	std::size_t vertexCountParlahti = parlahti.positions.size();
	GLuint parlahti_vao = create_vao(parlahti);

	SimpleMeshData landingpad = load_wavefront_obj("assets/landingpad.obj");
	std::size_t vertexCountLandingpad = landingpad.positions.size();
	GLuint landingpad_vao = create_vao(landingpad);


	// Ddebug output
#	if !defined(NDEBUG)
	setup_gl_debug_output();
#	endif // ~ !NDEBUG

	// Global GL state
	OGL_CHECKPOINT_ALWAYS();

	// TODO: global GL setup goes here
	glEnable(GL_FRAMEBUFFER_SRGB);



	glEnable(GL_DEPTH_TEST);

	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	OGL_CHECKPOINT_ALWAYS();

	// Get actual framebuffer size.
	// This can be different from the window size, as standard window
	// decorations (title bar, borders, ...) may be included in the window size
	// but not be part of the drawable surface area.
	int iwidth, iheight;
	glfwGetFramebufferSize(window, &iwidth, &iheight);

	glViewport(0, 0, iwidth, iheight);

	// Other initialization & loading

	OGL_CHECKPOINT_ALWAYS();
	ShaderProgram prog({
		{ GL_VERTEX_SHADER, "assets/default.vert" },
		{ GL_FRAGMENT_SHADER, "assets/default.frag" }
		});

	state.prog = &prog;
	state.camControl.radius = 10.f;

	auto last = Clock::now();

	float angle = 0.f;

	const char* texturePath = "assets/L4343A-4k.jpeg";
	GLuint texture = loadTexture(texturePath);
	Vec3f landingPadPosition1{ 0.0f, -0.95f, -16.0f };
	Vec3f landingPadPosition2{ 0.0f, -0.95f, 16.0f };

	// Create shape
	auto xcyl = make_cylinder(true, 16, { 1.f, 1.f, 1.f }, make_scaling(0.55f, 0.2f, 0.2f));
	auto xcone = make_cone(true, 16, { 1.f, 1.f, 1.f }, make_scaling(0.6f, 0.2f, 0.2f) * make_translation({ 1.115f, 0.0f, 0.0f }) * make_rotation_z(270.0f * (kPi_ / 180.0f)));
	auto xcyl2 = make_cylinder(true, 16, { 1.f, 1.f, 1.f }, make_scaling(0.03f, 0.03f, 0.03f) * make_translation({ -1.0f,0.0f,0.0f }));
	auto xbox = make_box(4.0f, 2.0f, 2.0f, Vec3f{ 1.f, 1.f, 1.f }, make_scaling(0.03f, 0.03f, 0.03f) * make_translation({ -4.0f,-6.0f,-1.0f }));
	auto xbox2 = make_box(4.0f, 2.0f, 2.0f, Vec3f{ 1.f, 1.f, 1.f }, make_scaling(0.03f, 0.03f, 0.03f) * make_translation({ -4.0f,4.0f,-1.0f }));
	auto xbox3 = make_box(4.0f, 2.0f, 2.0f, Vec3f{ 1.f, 1.f, 1.f }, make_scaling(0.03f, 0.03f, 0.03f) * make_translation({ -4.0f,-1.0f,4.0f }));
	auto xbox4 = make_box(4.0f, 2.0f, 2.0f, Vec3f{ 1.f, 1.f, 1.f }, make_scaling(0.03f, 0.03f, 0.03f) * make_translation({ -4.0f,-1.0f,-6.0f }));
	auto xcone2 = make_cone(true, 16, { 1.f, 1.f, 1.f }, make_scaling(0.6f, 0.2f, 0.2f) * make_translation({ 0.2f, 0.7f, 0.0f }) * make_rotation_z(270.0f * (kPi_ / 180.0f)));
	auto xcone3 = make_cone(true, 16, { 1.f, 1.f, 1.f }, make_scaling(0.6f, 0.2f, 0.2f) * make_translation({ 0.2f, -0.7f, 0.0f }) * make_rotation_z(270.0f * (kPi_ / 180.0f)));

	//Concatenate shape
	auto con1 = concatenate(std::move(xcyl), xcone);
	auto con2 = concatenate(std::move(con1), xcyl2);
	auto con3 = concatenate(std::move(con2), xbox);
	auto con4 = concatenate(std::move(con3), xbox2);
	auto con5 = concatenate(std::move(con4), xbox3);
	auto con6 = concatenate(std::move(con5), xbox4);
	auto con7 = concatenate(std::move(con6), xcone2);
	auto xspace = concatenate(std::move(con7), xcone3);

	GLuint vao = create_vao(xspace);
	std::size_t vertexCount = xspace.positions.size();
	Vec3f cylinderPosition = { 0.0f, -0.85f, 16.0f };
	Vec3f initialPosition = { 0.0f, -0.85f, 16.0f };

	Vec3f lightPos1 = { 2.0f, 1.0f, 0.0f };
	Vec3f lightColor1 = { 1.f, 0.0f, 0.0f }; // Red cylinder

	Vec3f lightPos2 = { 0.0f, 1.5f, 16.0f };
	Vec3f lightColor2 = { 0.0f, 0.0f, 1.f }; // Blue cone

	Vec3f lightPos3 = { 0.0f, 0.0f, 0.0f };
	Vec3f lightColor3 = { 1.0f, 1.0f, 0.f }; // Yellow box

	// TODO: global GL setup goes here

	OGL_CHECKPOINT_ALWAYS();

	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Let GLFW process events
		glfwPollEvents();

		// Check if window was resized.
		float fbwidth, fbheight;
		{
			int nwidth, nheight;
			glfwGetFramebufferSize(window, &nwidth, &nheight);

			fbwidth = float(nwidth);
			fbheight = float(nheight);

			if (0 == nwidth || 0 == nheight)
			{
				// Window minimized? Pause until it is unminimized.
				// This is a bit of a hack.
				do
				{
					glfwWaitEvents();
					glfwGetFramebufferSize(window, &nwidth, &nheight);
				} while (0 == nwidth || 0 == nheight);
			}

			glViewport(0, 0, nwidth, nheight);

		}

		// Update state
		auto const now = Clock::now();
		float dt = std::chrono::duration_cast<Secondsf>(now - last).count();
		last = now;

		angle += dt * kPi_ * 0.3f;
		if (angle >= 2.f * kPi_)
			angle -= 2.f * kPi_;
		//TODO: update state
		if (state.camControl.actionZoomIn)
			state.camControl.radius -= state.camControl.movementSpeed * dt;
		else if (state.camControl.actionZoomOut)
			state.camControl.radius += state.camControl.movementSpeed * dt;

		if (state.camControl.radius <= 0.1f)
			state.camControl.radius = 0.1f;

		if (state.isAnimating) {
			// Define animation duration and curve
			float animationDuration = 5.0f; // Change this value as needed
			float t = state.animationTime / animationDuration;

			// Use a curve function to create smooth movement (e.g., quadratic ease-in-out)
			float curve = t * t * (3.0f - 2.0f * t);

			// Update object's position based on the curve
			cylinderPosition.x = cylinderPosition.x + curve * (0.f - cylinderPosition.x); // Define newX
			cylinderPosition.y = cylinderPosition.y + curve * (3.85f - cylinderPosition.y); // Define newY
			cylinderPosition.z = cylinderPosition.z + curve * (16.0f - cylinderPosition.z); // Define newZ

			// Increment animation time
			state.animationTime += dt;

			// Check if animation is complete
			if (state.animationTime >= animationDuration) {
				state.isAnimating = false; // Animation finished
				state.animationTime = 0.0f; // Reset animation time
			}
		}

		else if (!state.isAnimating) {
			cylinderPosition.x = initialPosition.x;
			cylinderPosition.y = initialPosition.y;
			cylinderPosition.z = initialPosition.z;
		}
		// Update: compute matrices
		//TODO: define and compute projCameraWorld matrix

		Mat44f Rx = make_rotation_x(state.camControl.theta);
		Mat44f Ry = make_rotation_y(state.camControl.phi);
		Mat44f T = make_translation({ 0.0f, 0.0f, -state.camControl.radius });

		Mat44f worldToCamera = T * Rx * Ry;

		Mat44f projection = make_perspective_projection(
			60.0f * (kPi_ / 180.0f), // converting FOV from degrees to radians
			fbwidth / float(fbheight),
			0.1f,
			100.0f
		);

		Mat44f projCameraWorld = projection * worldToCamera;
		Mat44f landingPadTransform1 = make_translation(landingPadPosition1);
		Mat44f projCameraWorld1 = projection * worldToCamera * landingPadTransform1;
		Mat44f landingPadTransform2 = make_translation(landingPadPosition2);
		Mat44f projCameraWorld2 = projection * worldToCamera * landingPadTransform2;

		Mat44f cylinderRotation = make_rotation_z(90.0f * (kPi_ / 180.0f));
		Mat44f cylinderTransform = make_translation(cylinderPosition) * cylinderRotation;
		Mat44f projCameraWorldCylinder = projection * worldToCamera * cylinderTransform;

		glUseProgram(state.prog->programId());

		Vec3f dirLightDirection = normalize(Vec3f{ 0.0f, -1.0f, -1.0f });
		Vec3f dirLightColor = Vec3f{ 1.0f, 1.0f, 1.0f };

		GLint dirLightDirectionLoc = glGetUniformLocation(prog.programId(), "dirLightDirection");
		GLint dirLightColorLoc = glGetUniformLocation(prog.programId(), "dirLightColor");
		glUniform3fv(dirLightDirectionLoc, 1, &dirLightDirection.x);
		glUniform3fv(dirLightColorLoc, 1, &dirLightColor.x);

		GLint lightPosLoc1 = glGetUniformLocation(state.prog->programId(), "lightPos1");
		GLint lightColorLoc1 = glGetUniformLocation(state.prog->programId(), "lightColor1");
		GLint lightPosLoc2 = glGetUniformLocation(state.prog->programId(), "lightPos2");
		GLint lightColorLoc2 = glGetUniformLocation(state.prog->programId(), "lightColor2");
		GLint lightPosLoc3 = glGetUniformLocation(state.prog->programId(), "lightPos3");
		GLint lightColorLoc3 = glGetUniformLocation(state.prog->programId(), "lightColor3");


		glUniform3fv(lightPosLoc1, 1, &lightPos1.x);
		glUniform3fv(lightColorLoc1, 1, &lightColor1.x);

		glUniform3fv(lightPosLoc2, 1, &lightPos2.x);
		glUniform3fv(lightColorLoc2, 1, &lightColor2.x);

		glUniform3fv(lightPosLoc3, 1, &lightPos3.x);
		glUniform3fv(lightColorLoc3, 1, &lightColor3.x);

		GLint projCameraLoc = glGetUniformLocation(state.prog->programId(), "projCameraWorld");
		glUniformMatrix4fv(projCameraLoc, 1, GL_TRUE, &projCameraWorld.v[0]);
		glBindVertexArray(parlahti_vao);
		glDrawArrays(GL_TRIANGLES, 0, vertexCountParlahti);
		glBindVertexArray(0);

		glUniformMatrix4fv(projCameraLoc, 1, GL_TRUE, &projCameraWorld1.v[0]);
		glBindVertexArray(landingpad_vao);
		glDrawArrays(GL_TRIANGLES, 0, vertexCountLandingpad);
		glBindVertexArray(0);

		GLint applyLightingLoc = glGetUniformLocation(state.prog->programId(), "applyLighting");
		glUniform1i(applyLightingLoc, 1);
		glUniformMatrix4fv(projCameraLoc, 1, GL_TRUE, &projCameraWorld2.v[0]);
		glBindVertexArray(landingpad_vao);
		glDrawArrays(GL_TRIANGLES, 0, vertexCountLandingpad);
		glBindVertexArray(0);

		

		GLint projCameraLocCylinder = glGetUniformLocation(state.prog->programId(), "projCameraWorld");
		glUniformMatrix4fv(projCameraLocCylinder, 1, GL_TRUE, &projCameraWorldCylinder.v[0]);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, vertexCount);
		glBindVertexArray(0);

		glUniform1i(applyLightingLoc, 0);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		OGL_CHECKPOINT_DEBUG();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		OGL_CHECKPOINT_DEBUG();

		// Display results
		glfwSwapBuffers(window);
	}

	// Cleanup.
	//TODO: additional cleanup
	state.prog = nullptr;
	glDeleteTextures(1, &texture);
	glDeleteVertexArrays(1, &parlahti_vao);
	glDeleteVertexArrays(1, &vao);
	glDeleteVertexArrays(1, &landingpad_vao);
	return 0;
}
catch (std::exception const& eErr)
{
	std::fprintf(stderr, "Top-level Exception (%s):\n", typeid(eErr).name());
	std::fprintf(stderr, "%s\n", eErr.what());
	std::fprintf(stderr, "Bye.\n");
	return 1;
}


namespace
{
	void glfw_callback_error_(int aErrNum, char const* aErrDesc)
	{
		std::fprintf(stderr, "GLFW error: %s (%d)\n", aErrDesc, aErrNum);
	}

	void glfw_callback_key_(GLFWwindow* aWindow, int aKey, int, int aAction, int)
	{
		if (GLFW_KEY_ESCAPE == aKey && GLFW_PRESS == aAction)
		{
			glfwSetWindowShouldClose(aWindow, GLFW_TRUE);
			return;
		}

		if (auto* state = static_cast<State_*>(glfwGetWindowUserPointer(aWindow)))
		{
			// R-key reloads shaders.
			if (GLFW_KEY_R == aKey && GLFW_PRESS == aAction)
			{
				if (state->prog)
				{
					try
					{
						state->prog->reload();
						std::fprintf(stderr, "Shaders reloaded and recompiled.\n");
					}
					catch (std::exception const& eErr)
					{
						std::fprintf(stderr, "Error when reloading shader:\n");
						std::fprintf(stderr, "%s\n", eErr.what());
						std::fprintf(stderr, "Keeping old shader.\n");
					}
				}

				state->isAnimating = false;
			}

			// Space toggles camera
			if (GLFW_KEY_SPACE == aKey && GLFW_PRESS == aAction)
			{
				state->camControl.cameraActive = !state->camControl.cameraActive;

				if (state->camControl.cameraActive)
					glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
				else
					glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}

			// Camera controls if camera is active
			if (state->camControl.cameraActive)
			{
				if (GLFW_KEY_W == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.actionZoomIn = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.actionZoomIn = false;
				}
				else if (GLFW_KEY_S == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.actionZoomOut = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.actionZoomOut = false;
				}
				else if (GLFW_KEY_LEFT_SHIFT == aKey) {
					if (GLFW_PRESS == aAction)
						state->camControl.movementSpeed *= 2.0f; // Double the speed
					else if (GLFW_RELEASE == aAction)
						state->camControl.movementSpeed /= 2.0f; // Restore normal speed
				}
				// Ctrl for slowing down movement
				else if (GLFW_KEY_LEFT_CONTROL == aKey) {
					if (GLFW_PRESS == aAction)
						state->camControl.movementSpeed /= 2.0f; // Halve the speed
					else if (GLFW_RELEASE == aAction)
						state->camControl.movementSpeed *= 2.0f; // Restore normal speed
				}
				else if (GLFW_KEY_F == aKey && GLFW_PRESS == aAction) {
					state->isAnimating = true;
					state->animationTime = 0.0f; // Reset animation time
				}
			}
		}
	}

	void glfw_callback_motion_(GLFWwindow* aWindow, double aX, double aY)
	{
		if (auto* state = static_cast<State_*>(glfwGetWindowUserPointer(aWindow)))
		{
			if (state->camControl.cameraActive)
			{
				auto const dx = float(aX - state->camControl.lastX);
				auto const dy = float(aY - state->camControl.lastY);

				state->camControl.phi += dx * kMouseSensitivity_;

				state->camControl.theta += dy * kMouseSensitivity_;
				if (state->camControl.theta > kPi_ / 2.f)
					state->camControl.theta = kPi_ / 2.f;
				else if (state->camControl.theta < -kPi_ / 2.f)
					state->camControl.theta = -kPi_ / 2.f;
			}

			state->camControl.lastX = float(aX);
			state->camControl.lastY = float(aY);
		}
	}

}

namespace
{
	GLFWCleanupHelper::~GLFWCleanupHelper()
	{
		glfwTerminate();
	}

	GLFWWindowDeleter::~GLFWWindowDeleter()
	{
		if (window)
			glfwDestroyWindow(window);
	}
}