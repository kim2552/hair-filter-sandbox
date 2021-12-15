#include "DebugImage.h"

void DebugImage::loadImageAspectRatio(float imageAspectRatio)
{
	m_imageAspectRatio = imageAspectRatio;
}

void DebugImage::loadTextures(std::vector<Texture>* textureCameraImage)
{
	// Texture for face mask points
	m_debugImage.textures = textureCameraImage;
}

void DebugImage::loadShaders(Shader* shaderCameraImage)
{
	m_debugImage.shader = shaderCameraImage;
}

void DebugImage::drawDebugImage(Camera camera)
{
	m_debugImage.mesh->Draw(*m_debugImage.shader, camera, m_debugImage.model);
}

void DebugImage::initDebugImage(Model* objectModel)
{
	std::vector<Vertex> debugImageVertices{
		Vertex{glm::vec3(-1.0f, 1.0f,  0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
		Vertex{glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f)},
		Vertex{glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
		Vertex{glm::vec3(1.0f, 1.0f,  0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)}
	};
	std::vector<GLuint> debugImageIndices{
		0, 1, 2,
		0, 2, 3
	};

	// Create image mesh
	m_debugImage.mesh = new Mesh(debugImageVertices, debugImageIndices, *m_debugImage.textures);

	// Activate shader for Face Mask and configure the model matrix
	m_debugImage.shader->Activate();
	glm::mat4 pointModel = glm::mat4(1.0f);
	pointModel = glm::scale(pointModel, glm::vec3(0.01f, 0.01f, 0.01f));
	glm::mat4 trp = glm::translate(glm::mat4(1.f), glm::vec3(objectModel->fixedVertex.x, objectModel->fixedVertex.y, objectModel->fixedVertex.z));
	pointModel = trp * pointModel;

	m_debugImage.model = pointModel;
}

void DebugImage::updateDebugImage(Model* objectModel)
{
	m_debugImage.model = glm::mat4(1.0f);
	m_debugImage.model = glm::scale(m_debugImage.model, glm::vec3(0.01f, 0.01f, 0.01f));
	glm::mat4 trp = glm::translate(glm::mat4(1.f), glm::vec3(objectModel->fixedVertex.x, objectModel->fixedVertex.y, objectModel->fixedVertex.z));
	m_debugImage.model = trp * m_debugImage.model;
}