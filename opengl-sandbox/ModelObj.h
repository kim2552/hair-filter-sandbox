#pragma once
#ifndef MODEL_OBJ_H
#define MODEL_OBJ_H

#include <tinyobj/tiny_obj_loader.h>
#include "Mesh.h"

class ModelObj
{
public:
	// Loads in a model from a file and stores tha information in 'data', 'JSON', and 'file'
	ModelObj(std::string file, std::vector<Texture> textures);

	void Draw(Shader& shader, Camera& camera);

	void UpdateModel(glm::mat4 model);

	// Handles object controls
	void Inputs(GLFWwindow* window, int width, int height);

private:
	std::string filename;

	// All the meshes and transformations
	std::vector<Mesh> meshes;
	glm::mat4 modelMat = glm::mat4(1.0f);

	// Prevents textures from being loaded twice
	std::vector<std::string> loadedTexName;
	std::vector<Texture> loadedTex;

	glm::vec3 Position;
	glm::vec3 Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
	float scale = 1.0f;
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
	// Adjust the speed of the camera and it's sensitivity when looking around
	float speed = 0.1f;
	float sensitivity = 100.0f;
	// Prevents the camera from jumping around when first clicking left click
	bool firstClick = true;
	// Stores the width and height of the window
	int width;
	int height;


	float max_vert_x = 0;
	float min_vert_x = 0;
	float max_vert_y = 0;
	float min_vert_y = 0;
	float max_vert_z = 0;
	float min_vert_z = 0;
};

#endif //MODEL_OBJ_H