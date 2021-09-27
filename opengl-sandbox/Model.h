#ifndef MODEL_H_CLASS
#define MODEL_H_CLASS

#include "Mesh.h"

#include "vector"

struct BoundingBox
{
	glm::vec3 min;
	glm::vec3 max;
};

class Model
{
public:
	Model(std::vector<Mesh> meshes, int winWidth=0, int winHeight=0);

	// Draw the model
	void Draw(Shader& shader, Camera& camera);

	// Updates the model matrix
	void UpdateModel(glm::mat4 model);

	// Handles object controls
	void Inputs(GLFWwindow* window, int width, int height);

	// BoundingBox of the object
	BoundingBox originalBb;
	BoundingBox bb;

	// Model Information
	float originalModelWidth;
	float originalModelHeight;
	float modelWidth;
	float modelHeight;

	// Face Information
	glm::vec3 topHeadCoord;
	float faceWidth;
	float faceHeight;
	float faceRoll;
	float facePitch;
	float faceYaw;

	// Saved Model Parameters
	glm::vec3 savedTopHeadDist;	// distance of object from top point in head
	float savedRatioWidth;		// ratio of width between face and object
	float savedRatioHeight;		// ratio of height between face and object
	float savedScaleZ;			// scale value for Z coordinate
	float savedYaw;				// yaw
	float savedPitch;			// pitch
	float savedRoll;			// roll

	// All the meshes and transformations
	std::vector<Mesh> meshes;
	glm::mat4 modelMat = glm::mat4(1.0f);

	// Prevents textures from being loaded twice
	std::vector<std::string> loadedTexName;
	std::vector<Texture> loadedTex;

	// Model world coordinate information
	glm::vec3 fixedVertex;						// This fixed vertex point, is a consistent vertex point on the mesh
	glm::vec3 originalPosition;
	glm::vec3 position;
	glm::vec3 scale = glm::vec3(0);

	// Stores the width and height of the window
	int windowWidth;
	int windowHeight;
};

#endif