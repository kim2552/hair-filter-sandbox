#pragma once
#ifndef FACE_MASK_H
#define FACE_MASK_H

#include <glm/glm.hpp>

#include "Model.h"
#include "Texture.h"
#include "FaceDetect.h"

class FaceMask
{
public:
	void loadImageAspectRatio(float imageAspectRatio);
	void loadTextures(std::vector<Texture>* textureFaceMaskPoints, std::vector<Texture>* textureFaceMaskImage);
	void loadShaders(Shader* shaderFaceMaskPoints, Shader* shaderFaceMaskImage);
	void loadFaceDetectObj(FaceDetectObj faceDetectObj, FaceDetect* faceDetect);
	void drawMaskImage(Camera camera);

	RenderObject m_faceMaskImage;
	RenderObject m_faceMaskPoints;

	FaceDetectObj m_faceDetectObj;
private:
	float m_imageAspectRatio;
};

#endif