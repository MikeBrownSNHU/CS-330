///////////////////////////////////////////////////////////////////////////////
// viewmanager.h
// ============
// manage the viewing of 3D objects within the viewport - camera, projection
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ShaderManager.h"
#include "camera.h"

// GLFW library
#include "GLFW/glfw3.h" 

class ViewManager
{
public:
	// constructor
	ViewManager(
		ShaderManager* pShaderManager);
	// destructor
	~ViewManager();

	// mouse position callback for mouse interaction with the 3D scene
	static void Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos);

	/*** (Mike Brown - Nov 19, 2025) Milestone 3 ***/
	// mouse scroll callback for zoom speed control
	static void Mouse_Scroll_Callback(GLFWwindow* window, double xoffset, double yoffset);

private:
	// pointer to shader manager object
	ShaderManager* m_pShaderManager;
	// active OpenGL display window
	GLFWwindow* m_pWindow;

	// process keyboard events for interaction with the 3D scene
	void ProcessKeyboardEvents();

	/*** (Mike Brown - Nov 19, 2025) Milestone 3 ***/
	// camera used to navigate the 3D scene
	Camera m_camera;

	// timing for smooth movement
	float m_deltaTime;
	float m_lastFrameTime;

	// mouse tracking for smooth look-around
	bool  m_firstMouse;
	float m_lastX;
	float m_lastY;

	// P = perspective, O = orthographic
	bool m_usePerspective;

public:
	// create the initial OpenGL display window
	GLFWwindow* CreateDisplayWindow(const char* windowTitle);

	// prepare the conversion from 3D object display to 2D scene display
	void PrepareSceneView();
};
