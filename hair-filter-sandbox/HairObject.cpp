#include "HairObject.h"

void HairObject::loadImageAspectRatio(float imageAspectRatio)
{
	m_imageAspectRatio = imageAspectRatio;
}

void HairObject::loadTextures(std::vector<Texture>* textureHairObject)
{
	// Texture for face mask points
	m_hairRenderObj.textures = textureHairObject;
}

void HairObject::loadShaders(Shader* shaderHairObject)
{
	m_hairRenderObj.shader = shaderHairObject;
}

void HairObject::drawHairObject(Camera camera)
{
	m_hairModelObj->model->Draw(*m_hairRenderObj.shader, camera);
}

void HairObject::initHairObject(FaceMask* faceMask, AppConfig* appConfig)
{
	m_hairModelObj = new ModelObj(appConfig->params.hair_obj.c_str(), *m_hairRenderObj.textures);

	// Transfer face mesh data to hair object
	m_hairModelObj->model->topHeadCoord = faceMask->m_faceDetectObj.topCoord;
	m_hairModelObj->model->faceWidth = faceMask->m_faceDetectObj.width;
	m_hairModelObj->model->faceHeight = faceMask->m_faceDetectObj.height;

	// Activate shader for Object and configure the model matrix
	m_hairRenderObj.shader->Activate();
	glm::mat4 hairObjectModel = glm::mat4(1.0f);

	m_hairModelObj->model->front_head_vertex_index = appConfig->params.front_vert_index;

	// Calculate the scale of the hair object
	m_hairModelObj->model->savedRatioWidth = appConfig->params.ratio_width;				// Width ratio value based on developer preference			// TODO::Store values in json file
	m_hairModelObj->model->savedRatioHeight = appConfig->params.ratio_height;			// Height ratio value based on developer preference
	m_hairModelObj->model->savedScaleZ = appConfig->params.scale_z;				// Constant Z scale value based on developer preference

	m_hairModelObj->model->originalModelWidth = glm::length(m_hairModelObj->model->originalBb.max.x - m_hairModelObj->model->originalBb.min.x);
	m_hairModelObj->model->originalModelHeight = glm::length(m_hairModelObj->model->originalBb.max.y - m_hairModelObj->model->originalBb.min.y);
	float scaleMultWidth = m_hairModelObj->model->savedRatioWidth * m_hairModelObj->model->faceWidth / m_hairModelObj->model->originalModelWidth;
	float scaleMultHeight = m_hairModelObj->model->savedRatioHeight * m_hairModelObj->model->faceHeight / m_hairModelObj->model->originalModelHeight;
	hairObjectModel = glm::scale(hairObjectModel, glm::vec3(scaleMultWidth, scaleMultHeight, m_hairModelObj->model->savedScaleZ));	// TODO::Find calculation for Z component

	m_hairModelObj->model->scale.x = appConfig->params.ratio_width * m_hairModelObj->model->faceWidth / m_hairModelObj->model->originalModelWidth;
	m_hairModelObj->model->scale.y = appConfig->params.ratio_height * m_hairModelObj->model->faceHeight / m_hairModelObj->model->originalModelHeight;
	m_hairModelObj->model->scale.z = appConfig->params.scale_z;

	// Rotate object to match face direction
	m_hairModelObj->model->facePitch = faceMask->m_faceDetectObj.pitch;
	m_hairModelObj->model->faceYaw = faceMask->m_faceDetectObj.yaw;
	m_hairModelObj->model->faceRoll = faceMask->m_faceDetectObj.roll;
	m_hairModelObj->model->savedPitch = appConfig->params.pitch;
	m_hairModelObj->model->savedYaw = appConfig->params.yaw;
	m_hairModelObj->model->savedRoll = appConfig->params.roll;

	hairObjectModel = glm::rotate(hairObjectModel, glm::radians(m_hairModelObj->model->facePitch + m_hairModelObj->model->savedPitch), glm::vec3(1.0f, 0.0f, 0.0f));
	hairObjectModel = glm::rotate(hairObjectModel, glm::radians(m_hairModelObj->model->faceYaw + m_hairModelObj->model->savedYaw), glm::vec3(0.0f, 1.0f, 0.0f));
	hairObjectModel = glm::rotate(hairObjectModel, glm::radians(m_hairModelObj->model->faceRoll + m_hairModelObj->model->savedRoll), glm::vec3(0.0f, 0.0f, 1.0f));

	m_hairModelObj->model->UpdateModel(hairObjectModel);

	m_hairModelObj->model->savedTopHeadDist.x = appConfig->params.topheadx;		// Value obtained from fixedVertex distance from topHeadCoord	#TODO::Save values in a json file and retrieve it
	m_hairModelObj->model->savedTopHeadDist.y = appConfig->params.topheady;		// Value obtained from fixedVertex distance from topHeadCoord
	m_hairModelObj->model->savedTopHeadDist.z = appConfig->params.topheadz;		// Value obtained from fixedVertex distance from topHeadCoord

	float transX = (m_hairModelObj->model->savedTopHeadDist.x + m_hairModelObj->model->topHeadCoord.x) - m_hairModelObj->model->fixedVertex.x;
	float transY = (m_hairModelObj->model->savedTopHeadDist.y + m_hairModelObj->model->topHeadCoord.y) - m_hairModelObj->model->fixedVertex.y;
	float transZ = (m_hairModelObj->model->savedTopHeadDist.z + m_hairModelObj->model->topHeadCoord.z) - m_hairModelObj->model->fixedVertex.z;

	glm::mat4 translation_2 = glm::translate(glm::mat4(1.f), glm::vec3(transX, transY, transZ));
	hairObjectModel = translation_2 * hairObjectModel;

	// Take care of all the light related things
	glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec3 lightPos = glm::vec3(0.5f, 0.5f, 0.5f);

	glUniform4f(glGetUniformLocation(m_hairRenderObj.shader->ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(m_hairRenderObj.shader->ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(glGetUniformLocation(m_hairRenderObj.shader->ID, "filterColor"), 1, 1, 1);
	m_hairModelObj->model->UpdateModel(hairObjectModel);																// Update object model
}