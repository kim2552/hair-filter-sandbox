#pragma once
#ifndef HAIR_OBJ_H
#define HAIR_OBJ_H

#include <glm/glm.hpp>

#include "Model.h"
#include "ModelObj.h"
#include "FaceMask.h"
#include "AppConfig.h"

class HairObject
{
public:
	void loadImageAspectRatio(float imageAspectRatio);
	void loadTextures(std::vector<Texture>* textureCameraImage);
	void loadShaders(Shader* shaderCameraImage);
	void initHairObject(FaceMask* faceMask, AppConfig* appConfig);
	void drawHairObject(Camera camera);

	RenderObject m_hairRenderObj;
	ModelObj* m_hairModelObj;
private:
	float m_imageAspectRatio;
};

#endif