#pragma once
#ifndef CAMERA_IMAGE_H
#define CAMERA_IMAGE_H

#include <glm/glm.hpp>

#include "Model.h"
#include "Texture.h"

class CameraImage
{
public:
	void loadImageAspectRatio(float imageAspectRatio);
	void loadTextures(std::vector<Texture>* textureCameraImage);
	void loadShaders(Shader* shaderCameraImage);
	void initCameraImage();
	void drawCameraImage(Camera camera);

	RenderObject m_cameraImage;
private:
	float m_imageAspectRatio;
};

#endif