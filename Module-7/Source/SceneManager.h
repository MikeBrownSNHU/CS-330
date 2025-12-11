///////////////////////////////////////////////////////////////////////////////
// scenemanager.h
// ============
// manage the preparing and rendering of 3D scenes - textures, materials, lighting
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ShaderManager.h"
#include "ShapeMeshes.h"

#include <string>
#include <vector>

/***********************************************************
 *  SceneManager
 *
 *  This class contains the code for preparing and rendering
 *  3D scenes, including the shader settings.
 ***********************************************************/
class SceneManager
{
public:
	// constructor
	SceneManager(ShaderManager *pShaderManager);
	// destructor
	~SceneManager();

	struct TEXTURE_INFO
	{
		std::string tag;
		uint32_t ID;
	};

	struct OBJECT_MATERIAL
	{
		glm::vec3 diffuseColor;
		glm::vec3 specularColor;
		float shininess;
		std::string tag;
	};

private:
	// pointer to shader manager object
	ShaderManager* m_pShaderManager;
	// pointer to basic shapes object
	ShapeMeshes* m_basicMeshes;
	// total number of loaded textures
	int m_loadedTextures;
	// loaded textures info
	TEXTURE_INFO m_textureIDs[16];
	// defined object materials
	std::vector<OBJECT_MATERIAL> m_objectMaterials;

	// load texture images and convert to OpenGL texture data
	bool CreateGLTexture(const char* filename, std::string tag);
	// bind loaded OpenGL textures to slots in memory
	void BindGLTextures();
	// free the loaded OpenGL textures
	void DestroyGLTextures();
	// find a loaded texture by tag
	int FindTextureID(std::string tag);
	int FindTextureSlot(std::string tag);

	// -----------------------------
	// Material helpers
	// -----------------------------
	// find a defined material by tag
	bool FindMaterial(std::string tag, OBJECT_MATERIAL& material);

	// define all scene materials (wood, metal, plastic, carpet, etc.)
	void DefineObjectMaterials();

	// -----------------------------
	// Lighting helpers
	// -----------------------------
	// configure the main scene lights (directional + point/fill)
	void ConfigureSceneLights();
	// enable or disable lighting in the shader
	void EnableLighting(bool enabled);

	// set the transformation values 
	// into the transform buffer
	void SetTransformations(
		glm::vec3 scaleXYZ,
		float XrotationDegrees,
		float YrotationDegrees,
		float ZrotationDegrees,
		glm::vec3 positionXYZ);

	// set the color values into the shader
	void SetShaderColor(
		float redColorValue,
		float greenColorValue,
		float blueColorValue,
		float alphaValue);

	// set the texture data into the shader
	void SetShaderTexture(
		std::string textureTag);

	// set the UV scale for the texture mapping
	void SetTextureUVScale(
		float u, float v);

	// set the object material into the shader
	void SetShaderMaterial(
		std::string materialTag);


	// Tracks whether the monitor screen is on or off
	bool m_monitorOn;

public:

	// The following methods are for the students to 
	// customize for their own 3D scene
	void PrepareScene();
	void RenderScene();



	/*** (Mike Brown - Nov 12, 2025) Milestone 2 ***/
	void DrawMonitor(const glm::vec3& basePos);
	/*** (Mike Brown - Nov 19, 2025) Milestone 3 ***/
	void DrawDesk(const glm::vec3& basePos);

	/*** (Mike Brown - Nov 30, 2025) Milestone 4 ***/
	void DrawCamera(const glm::vec3 basePos, float cameraYRotationDegrees);

	void DrawMousePad(const glm::vec3& basePos);

	void DrawKeyboard(const glm::vec3& basePos);

	void DrawMouse(const glm::vec3& basePos);

	/*** (Mike Brown - Dec 02, 2025) Milestone 5 ***/
	// Toggle monitor power state
	void ToggleMonitor() { m_monitorOn = !m_monitorOn; }

	// Optional convenience helpers if you want them
	void SetMonitorOn(bool on) { m_monitorOn = on; }
	bool IsMonitorOn() const { return m_monitorOn; }

	// loads textures from image files
	void LoadSceneTextures();
};