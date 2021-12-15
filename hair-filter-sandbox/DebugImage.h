#pragma once
#ifndef DEBUG_IMAGE_H
#define DEBUG_IMAGE_H

#include <glm/glm.hpp>

#include "Model.h"
#include "Texture.h"

class DebugImage
{
public:
	void loadImageAspectRatio(float imageAspectRatio);
	void loadTextures(std::vector<Texture>* textureCameraImage);
	void loadShaders(Shader* shaderCameraImage);
	void initDebugImage(Model* objectModel);
	void updateDebugImage(Model* objectModel);
	void drawDebugImage(Camera camera);

	RenderObject m_debugImage;
private:
	float m_imageAspectRatio;
};

#endif