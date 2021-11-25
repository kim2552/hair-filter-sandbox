#pragma once
#ifndef MODEL_OBJ_H
#define MODEL_OBJ_H

#include <tinyobj/tiny_obj_loader.h>
#include "Mesh.h"
#include "Model.h"

class ModelObj
{
public:
	// Loads in a model from a file and stores tha information in 'data', 'JSON', and 'file'
	ModelObj(std::string file, std::vector<Texture> textures);
	Model* model;

private:

	std::string filename;

	// Prevents textures from being loaded twice
	std::vector<std::string> loadedTexName;
	std::vector<Texture> loadedTex;
};

#endif //MODEL_OBJ_H