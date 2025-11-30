///////////////////////////////////////////////////////////////////////////////
// viewmanager.cpp
// ============
// manage the viewing of 3D objects within the viewport - camera, projection
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"

// GLM Math Header inclusions
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>    

// declaration of the global variables and defines
namespace
{
	// Variables for window width and height
	const int WINDOW_WIDTH = 1000;
	const int WINDOW_HEIGHT = 800;
	const char* g_ViewName = "view";
	const char* g_ProjectionName = "projection";

	// camera object used for viewing and interacting with
	// the 3D scene
	Camera* g_pCamera = nullptr;

	// these variables are used for mouse movement processing
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	// time between current frame and last frame
	float gDeltaTime = 0.0f;
	float gLastFrame = 0.0f;

	// the following variable is false when orthographic projection
	// is off and true when it is on
	bool bOrthographicProjection = false;

	/*** (Mike Brown - Nov 19, 2025) Milestone 3 ***/
	// optional limits for camera movement speed when using mouse scroll
	const float MIN_CAMERA_SPEED = 5.0f;
	const float MAX_CAMERA_SPEED = 60.0f;
}

/***********************************************************
 *  ViewManager()
 *
 *  The constructor for the class
 ***********************************************************/
ViewManager::ViewManager(
	ShaderManager* pShaderManager)
{
	// initialize the member variables
	m_pShaderManager = pShaderManager;
	m_pWindow = NULL;
	g_pCamera = new Camera();
	// default camera view parameters
	g_pCamera->Position = glm::vec3(0.0f, 5.0f, 12.0f);
	g_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
	g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
	g_pCamera->Zoom = 80;
	g_pCamera->MovementSpeed = 20;
}

/***********************************************************
 *  ~ViewManager()
 *
 *  The destructor for the class
 ***********************************************************/
ViewManager::~ViewManager()
{
	// free up allocated memory
	m_pShaderManager = NULL;
	m_pWindow = NULL;
	if (NULL != g_pCamera)
	{
		delete g_pCamera;
		g_pCamera = NULL;
	}
}

/***********************************************************
 *  CreateDisplayWindow()
 *
 *  This method is used to create the main display window.
 ***********************************************************/
GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
	GLFWwindow* window = nullptr;

	// try to create the displayed OpenGL window
	window = glfwCreateWindow(
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		windowTitle,
		NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);

	// tell GLFW to capture all mouse events
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// this callback is used to receive mouse moving events
	glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);

	/*** (Mike Brown - Nov 19, 2025) Milestone 3 ***/
	// this callback is used to receive mouse scroll events (for camera speed)
	glfwSetScrollCallback(window, &ViewManager::Mouse_Scroll_Callback);

	// enable blending for supporting tranparent rendering
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_pWindow = window;

	return(window);
}

/***********************************************************
 *  Mouse_Position_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse is moved within the active GLFW display window.
 ***********************************************************/
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
	// when the first mouse move event is received, this needs to be recorded so that
	// all subsequent mouse moves can correctly calculate the X position offset and Y
	// position offset for proper operation
	if (gFirstMouse)
	{
		gLastX = xMousePos;
		gLastY = yMousePos;
		gFirstMouse = false;
	}

	// calculate the X offset and Y offset values for moving the 3D camera accordingly
	float xOffset = xMousePos - gLastX;
	float yOffset = gLastY - yMousePos; // reversed since y-coordinates go from bottom to top

	// set the current positions into the last position variables
	gLastX = xMousePos;
	gLastY = yMousePos;

	// move the 3D camera according to the calculated offsets
	g_pCamera->ProcessMouseMovement(xOffset, yOffset);
}

/***********************************************************
 *  Mouse_Scroll_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse scroll wheel is used within the active GLFW
 *  display window.
 ***********************************************************/
void ViewManager::Mouse_Scroll_Callback(GLFWwindow* window, double xOffset, double yOffset)
{
	/*** (Mike Brown - Nov 19, 2025) Milestone 3 ***/
	// use the vertical scroll wheel to adjust the camera movement speed
	// scrolling up increases speed, scrolling down decreases speed
	if (nullptr == g_pCamera)
	{
		return;
	}

	float newSpeed = g_pCamera->MovementSpeed + static_cast<float>(yOffset) * 2.0f;

	// clamp the new speed to stay within reasonable bounds
	if (newSpeed < MIN_CAMERA_SPEED)
	{
		newSpeed = MIN_CAMERA_SPEED;
	}
	if (newSpeed > MAX_CAMERA_SPEED)
	{
		newSpeed = MAX_CAMERA_SPEED;
	}

	g_pCamera->MovementSpeed = newSpeed;
}

/***********************************************************
 *  ProcessKeyboardEvents()
 *
 *  This method is called to process any keyboard events
 *  that may be waiting in the event queue.
 ***********************************************************/
void ViewManager::ProcessKeyboardEvents()
{
	// close the window if the escape key has been pressed
	if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(m_pWindow, true);
	}

	// process camera zooming in and out
	if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(FORWARD, gDeltaTime);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(BACKWARD, gDeltaTime);
	}

	// process camera panning left and right
	if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(LEFT, gDeltaTime);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(RIGHT, gDeltaTime);
	}

	/*** (Mike Brown - Nov 19, 2025) Milestone 3 ***/
	// process camera vertical movement (up and down)
	if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
	{
		// move camera straight down
		g_pCamera->Position.y -= g_pCamera->MovementSpeed * gDeltaTime;
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
	{
		// move camera straight up
		g_pCamera->Position.y += g_pCamera->MovementSpeed * gDeltaTime;
	}

	/*** (Mike Brown - Nov 19, 2025) Milestone 3 ***/
	// switch between perspective (P) and orthographic (O) projection
	if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS)
	{
		// perspective mode on
		bOrthographicProjection = false;

		// restore the original perspective camera settings
		g_pCamera->Position = glm::vec3(0.0f, 5.0f, 12.0f);
		g_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
		g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
		g_pCamera->Zoom = 80.0f;
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_PRESS)
	{
		// orthographic mode on
		bOrthographicProjection = true;

		// camera looks directly at the 3D object from the front.
		// Desk is centered roughly around x=3, y=4, z≈1.5, so we
		// place the camera in front of it on +Z looking straight in.
		g_pCamera->Position = glm::vec3(3.0f, 4.0f, 30.0f);
		g_pCamera->Front = glm::vec3(0.0f, 0.0f, -1.0f);
		g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
		g_pCamera->Zoom = 45.0f;
	}
}

/***********************************************************
 *  PrepareSceneView()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void ViewManager::PrepareSceneView()
{
	glm::mat4 view;
	glm::mat4 projection;

	// per-frame timing
	float currentFrame = glfwGetTime();
	gDeltaTime = currentFrame - gLastFrame;
	gLastFrame = currentFrame;

	// process any keyboard events that may be waiting in the 
	// event queue
	ProcessKeyboardEvents();

	/*** (Mike Brown - Nov 19, 2025) Milestone 3 ***/
	// set the viewport for the current window size
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	// get the current view matrix from the camera
	view = g_pCamera->GetViewMatrix();

	// define the current projection matrix
	if (!bOrthographicProjection)
	{
		// perspective projection for 3D view
		projection = glm::perspective(
			glm::radians(g_pCamera->Zoom),
			(GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT,
			0.1f,
			100.0f);
	}
	else
	{
		// orthographic projection for 2D-style view
		// adjust the "size" to frame the scene appropriately
		float orthoSize = 15.0f;
		float aspect = (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT;

		projection = glm::ortho(
			-orthoSize * aspect, orthoSize * aspect,
			-orthoSize, orthoSize,
			0.1f,
			100.0f);
	}

	// if the shader manager object is valid
	if (NULL != m_pShaderManager)
	{
		// set the view matrix into the shader for proper rendering
		m_pShaderManager->setMat4Value(g_ViewName, view);
		// set the projection matrix into the shader for proper rendering
		m_pShaderManager->setMat4Value(g_ProjectionName, projection);
		// set the view position of the camera into the shader for proper rendering
		m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
	}
}