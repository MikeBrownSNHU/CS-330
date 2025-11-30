///////////////////////////////////////////////////////////////////////////////
// scenemanager.cpp
// ================
// This file contains the implementation of the `SceneManager` class, which is 
// responsible for managing the preparation and rendering of 3D scenes. It 
// handles textures, materials, lighting configurations, and object rendering.
//
// AUTHOR: Brian Battersby
// INSTITUTION: Southern New Hampshire University (SNHU)
// COURSE: CS-330 Computational Graphics and Visualization
//
// INITIAL VERSION: November 1, 2023
// LAST REVISED: December 1, 2024
//
// RESPONSIBILITIES:
// - Load, bind, and manage textures in OpenGL.
// - Define materials and lighting properties for 3D objects.
// - Manage transformations and shader configurations.
// - Render complex 3D scenes using basic meshes.
//
// NOTE: This implementation leverages external libraries like `stb_image` for 
// texture loading and GLM for matrix and vector operations.
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationZ * rotationY * rotationX * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/



void SceneManager::LoadSceneTextures()
{
	/*** STUDENTS - add the code BELOW for loading the textures that ***/
	/*** will be used for mapping to objects in the 3D scene. Up to  ***/
	/*** 16 textures can be loaded per scene. Refer to the code in   ***/
	/*** the OpenGL Sample for help.                                 ***/

	bool bReturn = false;

	bReturn = CreateGLTexture(
		"textures/leather2.jpg",
		"leather");
	bReturn = CreateGLTexture(
		"textures/wood.jpg",
		"wood");
	bReturn = CreateGLTexture(
		"textures/metal.jpg",
		"metal");
	bReturn = CreateGLTexture(
		"textures/plastic.jpg",
		"plastic");
	bReturn = CreateGLTexture("textures/monitor_screen.jpg",
		"monitorScreen");
	bReturn = CreateGLTexture("textures/carpet.jpg",
		"carpet");
	bReturn = CreateGLTexture("textures/black_metal.jpg",
		"blackMetal");
	// after the texture image data is loaded into memory, the
	// loaded textures need to be bound to texture slots - there
	// are a total of 16 available slots for scene textures
	BindGLTextures();
}

/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// load the textures for the 3D scene
	LoadSceneTextures();

	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene

	m_basicMeshes->LoadPlaneMesh();

/*** (Mike Brown - Nov 12, 2025) Milestone 2 ***/

	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadPrismMesh();
	m_basicMeshes->LoadSphereMesh();


}



/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(1, 1, 1, 1);

	// draw the mesh with transformation values
	SetShaderTexture("carpet");
	SetTextureUVScale(6.0f, 6.0f);
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/

/*** (Mike Brown - Nov 12, 2025) Milestone 2 ***/
	DrawMonitor(glm::vec3(3.0f, 4.12f, 0.75f));

/*** (Mike Brown - Nov 19, 2025) Milestone 3 ***/
	DrawDesk(glm::vec3(0.0f, 0.0f, 0.0f));

/*** (Mike Brown - Nov 30, 2025) Milestone 4 ***/
	DrawCamera(glm::vec3(0.0f, 4.12f, 1.0f), 15.0f);

}




// --------------------------------------------------------------
// Draw Desk
// The plane where our objects will sit
// Created by Mike Brown - Nov 19, 2025
// Now supports basePos as the world-space origin for the monitor
// --------------------------------------------------------------

void SceneManager::DrawDesk(const glm::vec3& basePos)
{
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	//
	// Desk Legs
	//

	// LF
	scaleXYZ = glm::vec3(0.25f, 4.0f, 0.25f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + glm::vec3(-1.0f, 2.0f, 0.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.05f, 0.05f, 0.06f, 1.0f);
	m_basicMeshes->DrawBoxMesh();

	// LR
	scaleXYZ = glm::vec3(0.25f, 4.0f, 0.25f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + glm::vec3(-1.0f, 2.0f, 3.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.05f, 0.05f, 0.06f, 1.0f);
	m_basicMeshes->DrawBoxMesh();

	// RF
	scaleXYZ = glm::vec3(0.25f, 4.0f, 0.25f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + glm::vec3(7.0f, 2.0f, 0.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.05f, 0.05f, 0.06f, 1.0f);
	m_basicMeshes->DrawBoxMesh();

	// RR
	scaleXYZ = glm::vec3(0.25f, 4.0f, 0.25f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + glm::vec3(7.0f, 2.0f, 3.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.05f, 0.05f, 0.06f, 1.0f);
	m_basicMeshes->DrawBoxMesh();

	//
	// Desk Frame
	//

	// L
	scaleXYZ = glm::vec3(0.3f, 0.25f, 3.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + glm::vec3(-1.0f, 3.875f, 1.5f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.05f, 0.05f, 0.06f, 1.0f);
	m_basicMeshes->DrawBoxMesh();

	// R
	scaleXYZ = glm::vec3(0.3f, 0.25f, 3.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + glm::vec3(7.0f, 3.875f, 1.5f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.05f, 0.05f, 0.06f, 1.0f);
	m_basicMeshes->DrawBoxMesh();

	// Rear
	scaleXYZ = glm::vec3(8.0f, 0.25f, 0.3f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + glm::vec3(3.0f, 3.875f, 0.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.05f, 0.05f, 0.06f, 1.0f);
	m_basicMeshes->DrawBoxMesh();

	// Front
	scaleXYZ = glm::vec3(8.0f, 0.25f, 0.3f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + glm::vec3(3.0f, 3.875f, 3.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.05f, 0.05f, 0.06f, 1.0f);
	m_basicMeshes->DrawBoxMesh();

	//
	// Desk Top
	//
	scaleXYZ = glm::vec3(8.5f, 0.25f, 3.5f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + glm::vec3(3.0f, 4.0f, 1.5f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.75f, 0.5f, 0.0f, 1.0f);
	SetShaderTexture("wood");
	SetTextureUVScale(2.0f, 2.0f);
	m_basicMeshes->DrawBoxMesh();
}



// --------------------------------------------------------------
// Draw Monitor
// Complex object composed of multiple basic shapes
// Created by Mike Brown - Nov 12, 2025
// --------------------------------------------------------------

void SceneManager::DrawMonitor(const glm::vec3& basePos)
{
	// Shared locals (project pattern)
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	// ------------------------------------------------------------
	// Foot / Base 
	// ------------------------------------------------------------
	scaleXYZ = glm::vec3(1.9f, 0.05f, 1.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + glm::vec3(0.0f, 0.05f, 0.15f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);


	SetShaderColor(0.80f, 0.80f, 0.83f, 1.0f);
	SetShaderTexture("metal");
	SetTextureUVScale(0.7f, 0.7f);
	m_basicMeshes->DrawBoxMesh();

	// ------------------------------------------------------------
	// Stand aluminum post, leaning back slightly
	// ------------------------------------------------------------
	scaleXYZ = glm::vec3(0.12f, 1.2f, 0.12f);
	XrotationDegrees = -10.0f; // lean back
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + glm::vec3(0.0f, 0.0f, 0.124f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.78f, 0.78f, 0.80f, 1.0f);
	m_basicMeshes->DrawCylinderMesh();

	// ------------------------------------------------------------
	// Stand aluminum post top
	// ------------------------------------------------------------
	scaleXYZ = glm::vec3(0.75f, 0.4f, 0.5f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 180.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + glm::vec3(0.0f, 1.2f, 0.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.78f, 0.78f, 0.80f, 1.0f);
	m_basicMeshes->DrawPrismMesh();

	// ------------------------------------------------------------
	// Small disc at the bottom of the stand
	// ------------------------------------------------------------
	scaleXYZ = glm::vec3(0.18f, 0.08f, 0.18f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + glm::vec3(0.0f, 0.0f, 0.124f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.78f, 0.78f, 0.80f, 1.0f);
	m_basicMeshes->DrawCylinderMesh();

	// ------------------------------------------------------------
	// Panel backing
	// ------------------------------------------------------------
	scaleXYZ = glm::vec3(3.2f, 1.8f, 0.14f);
	XrotationDegrees = -3.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + glm::vec3(0.0f, 1.80f, 0.25f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.18f, 0.18f, 0.19f, 1.0f);     
	SetShaderTexture("metal");
	SetTextureUVScale(2.0f, 2.0f);
	m_basicMeshes->DrawBoxMesh();

	// ------------------------------------------------------------
	// Power ports
	// ------------------------------------------------------------
	scaleXYZ = glm::vec3(0.05f, -0.02f, 0.05f);
	XrotationDegrees = 87.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + glm::vec3(-0.75f, 1.1f, 0.2f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.11f, 0.11f, 0.12f, 1.0f);

	m_basicMeshes->DrawCylinderMesh();

	scaleXYZ = glm::vec3(0.05f, -0.02f, 0.05f);
	XrotationDegrees = 87.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + glm::vec3(-0.7f, 1.1f, 0.2f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.11f, 0.11f, 0.12f, 1.0f);
	m_basicMeshes->DrawCylinderMesh();

	// ------------------------------------------------------------
	// Screen content (mySNHU)
	// ------------------------------------------------------------
	scaleXYZ = glm::vec3(2.9f, 1.35f, 0.10f);
	XrotationDegrees = -3.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + glm::vec3(0.0f, 1.91f, 0.281f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("monitorScreen");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawBoxMesh();

	// ------------------------------------------------------------
	// Glass screen face / bezel
	// ------------------------------------------------------------
	scaleXYZ = glm::vec3(3.19f, 1.6f, 0.10f);
	XrotationDegrees = -3.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + glm::vec3(0.0f, 1.9f, 0.28f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.06f, 0.06f, 0.07f, 1.0f);     

	m_basicMeshes->DrawBoxMesh();

	// ------------------------------------------------------------
	// Aluminum bottom bezel
	// ------------------------------------------------------------
	scaleXYZ = glm::vec3(3.2f, 0.287f, 0.11f);  
	XrotationDegrees = -3.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	
	positionXYZ = basePos + glm::vec3(0.0f, 1.05f, 0.34f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);

	SetShaderColor(0.80f, 0.80f, 0.83f, 1.0f);
	SetShaderTexture("metal");
	SetTextureUVScale(0.7f, 0.3f);
	m_basicMeshes->DrawBoxMesh();
}


// --------------------------------------------------------------
// Draw Camera
// Complex object composed of multiple basic shapes and textures
// Created by Mike Brown - Nov 30, 2025
// --------------------------------------------------------------

void SceneManager::DrawCamera(const glm::vec3 basePos, float cameraYRotationDegrees)
{
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	glm::mat4 camRotMat = glm::rotate(glm::radians(cameraYRotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	auto rotateOffsetY = [&](const glm::vec3& localOffset)
		{
			return glm::vec3(camRotMat * glm::vec4(localOffset, 1.0f));
		};

	// ----------------------------------------------------
	//  Top Hump
	// ----------------------------------------------------

	scaleXYZ = glm::vec3(0.18f, 0.075f, 0.1f);
	XrotationDegrees = 270.0f;
	YrotationDegrees = cameraYRotationDegrees;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + rotateOffsetY(glm::vec3(0.05f, scaleXYZ.y * 5.0f, -0.037f));
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.78f, 0.78f, 0.80f, 1.0f);
	m_basicMeshes->DrawPrismMesh();

	scaleXYZ = glm::vec3(0.18f, 0.075f, 0.1f);
	XrotationDegrees = 270.0f;
	YrotationDegrees = cameraYRotationDegrees;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + rotateOffsetY(glm::vec3(-0.05f, scaleXYZ.y * 5.0f, -0.037f));
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.78f, 0.78f, 0.80f, 1.0f);
	m_basicMeshes->DrawPrismMesh();

	scaleXYZ = glm::vec3(0.1f, 0.088f, 0.075f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = cameraYRotationDegrees;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + rotateOffsetY(glm::vec3(0.0f, scaleXYZ.y * 4.35f, -0.037f));
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.78f, 0.78f, 0.80f, 1.0f);
	m_basicMeshes->DrawBoxMesh();

	scaleXYZ = glm::vec3(0.05f, 0.04f, 0.07f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = cameraYRotationDegrees;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + rotateOffsetY(glm::vec3(0.0f, scaleXYZ.y * 10.0f, -0.042f));
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.02f, 0.12f, 0.10f, 1.0f);
	m_basicMeshes->DrawBoxMesh();



	// ----------------------------------------------------
	//  Button
	// ----------------------------------------------------

	scaleXYZ = glm::vec3(0.04f, 0.01f, 0.04f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = cameraYRotationDegrees;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + rotateOffsetY(glm::vec3(-0.24f, scaleXYZ.y * 37.5f, 0.02f));
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.06f, 0.06f, 0.06f, 1.0f);
	m_basicMeshes->DrawSphereMesh();


	// ----------------------------------------------------
	//  Base Bottom
	// ----------------------------------------------------

	scaleXYZ = glm::vec3(0.5f, 0.05f, 0.15f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = cameraYRotationDegrees;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + rotateOffsetY(glm::vec3(0.0f, scaleXYZ.y * 0.5f, 0.0f));
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.82f, 0.82f, 0.84f, 1.0f);
	SetShaderTexture("metal");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawBoxMesh();


	scaleXYZ = glm::vec3(0.11f, 0.05f, 0.09f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = cameraYRotationDegrees;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + rotateOffsetY(glm::vec3(-0.23f, 0.0f, 0.0141f));
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.82f, 0.82f, 0.84f, 1.0f);
	SetShaderTexture("metal");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawCylinderMesh();

	scaleXYZ = glm::vec3(0.075f, 0.05f, 0.075f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = cameraYRotationDegrees;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + rotateOffsetY(glm::vec3(0.235f, 0.0f, 0.0f));
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.82f, 0.82f, 0.84f, 1.0f);
	SetShaderTexture("metal");
	m_basicMeshes->DrawCylinderMesh();

	// ----------------------------------------------------
	//  Base Mid
	// ----------------------------------------------------

	scaleXYZ = glm::vec3(0.5f, 0.27f, 0.15f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = cameraYRotationDegrees;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + rotateOffsetY(glm::vec3(0.0f, scaleXYZ.y * 0.7f, 0.0f));
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("leather");
	SetTextureUVScale(1.5f, 1.0f);
	m_basicMeshes->DrawBoxMesh();


	scaleXYZ = glm::vec3(0.11f, 0.27f, 0.09f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = cameraYRotationDegrees;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + rotateOffsetY(glm::vec3(-0.23f, 0.055f, 0.0141f));
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("leather");
	SetTextureUVScale(1.5f, 1.0f);
	m_basicMeshes->DrawCylinderMesh();

	scaleXYZ = glm::vec3(0.075f, 0.27f, 0.075f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = cameraYRotationDegrees;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + rotateOffsetY(glm::vec3(0.235f, 0.055f, 0.0f));
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("leather");
	SetTextureUVScale(1.5f, 1.0f);
	m_basicMeshes->DrawCylinderMesh();

	// ----------------------------------------------------
	//  Base Top
	// ----------------------------------------------------

	scaleXYZ = glm::vec3(0.5f, 0.05f, 0.15f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = cameraYRotationDegrees;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + rotateOffsetY(glm::vec3(0.0f, scaleXYZ.y * 7.0f, 0.0f));
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.82f, 0.82f, 0.84f, 1.0f);
	SetShaderTexture("metal");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawBoxMesh();


	scaleXYZ = glm::vec3(0.11f, 0.05f, 0.09f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = cameraYRotationDegrees;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + rotateOffsetY(glm::vec3(-0.23f, 0.325f, 0.0141f));
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.82f, 0.82f, 0.84f, 1.0f);
	SetShaderTexture("metal");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawCylinderMesh();

	scaleXYZ = glm::vec3(0.075f, 0.05f, 0.075f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = cameraYRotationDegrees;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + rotateOffsetY(glm::vec3(0.235f, 0.325f, 0.0f));
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.82f, 0.82f, 0.84f, 1.0f);
	SetShaderTexture("metal");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawCylinderMesh();

	// ----------------------------------------------------
	//  Lens
	// ----------------------------------------------------

	//Base
	scaleXYZ = glm::vec3(0.125f, 0.07f, 0.125f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = cameraYRotationDegrees;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + rotateOffsetY(glm::vec3(0.0f, 0.185f, 0.0141f));
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.82f, 0.82f, 0.84f, 1.0f);
	SetShaderTexture("metal");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawCylinderMesh();

	//Mid
	scaleXYZ = glm::vec3(0.12f, 0.18f, 0.12f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = cameraYRotationDegrees;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + rotateOffsetY(glm::vec3(0.0f, 0.185f, 0.0141f));
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.04f, 0.04f, 0.04f, 1.0f);
	m_basicMeshes->DrawCylinderMesh();

	//Front
	scaleXYZ = glm::vec3(0.13f, 0.1f, 0.13f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = cameraYRotationDegrees;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + rotateOffsetY(glm::vec3(0.0f, 0.185f, 0.19f));
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.12f, 0.12f, 0.12f, 1.0f);
	m_basicMeshes->DrawCylinderMesh();

	//Glass
	scaleXYZ = glm::vec3(0.11f, 0.01f, 0.11f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = cameraYRotationDegrees;
	ZrotationDegrees = 0.0f;
	positionXYZ = basePos + rotateOffsetY(glm::vec3(0.0f, 0.185f, 0.29f));
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.02f, 0.12f, 0.10f, 1.0f);
	m_basicMeshes->DrawSphereMesh();
}
