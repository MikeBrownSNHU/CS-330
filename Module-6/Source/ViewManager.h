///////////////////////////////////////////////////////////////////////////////
// viewmanager.h
// ==============
// Defines the `ViewManager` class, which manages the viewing and projection 
// of 3D objects within the OpenGL viewport. This includes handling the camera, 
// projection matrices, and user input for interacting with the 3D scene.
//
// AUTHOR: Brian Battersby
// INSTITUTION: Southern New Hampshire University (SNHU)
// COURSE: CS-330 Computational Graphics and Visualization
//
// INITIAL VERSION: November 1, 2023
// LAST REVISED: December 1, 2024
//
// RESPONSIBILITIES:
// - Manage the OpenGL display window and viewport setup.
// - Integrate with the `ShaderManager` class for rendering configurations.
// - Handle camera control, including position, orientation, and zoom.
// - Process keyboard and mouse input for real-time interaction.
// - Configure projection settings for perspective and orthographic views.
//
// NOTE: This class works closely with `ShaderManager` and the `Camera` class 
// to prepare and update the view and projection matrices for rendering.
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

private:
	// pointer to shader manager object
	ShaderManager* m_pShaderManager;
	// active OpenGL display window
	GLFWwindow* m_pWindow;

	// process keyboard events for interaction with the 3D scene
	void ProcessKeyboardEvents();

public:
	// create the initial OpenGL display window
	GLFWwindow* CreateDisplayWindow(const char* windowTitle);
	
	// prepare the conversion from 3D object display to 2D scene display
	void PrepareSceneView();
};