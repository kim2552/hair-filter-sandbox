#include "FaceMask.h"

void FaceMask::loadImageAspectRatio(float imageAspectRatio)
{
	m_imageAspectRatio = imageAspectRatio;
}

void FaceMask::loadTextures(std::vector<Texture>* textureFaceMaskPoints, std::vector<Texture>* textureFaceMaskImage)
{
	// Texture for face mask points
	m_faceMaskPoints.textures = textureFaceMaskPoints;

	// Texture for face mask image
	m_faceMaskImage.textures = textureFaceMaskImage;
}

void FaceMask::loadShaders(Shader* shaderFaceMaskPoints, Shader* shaderFaceMaskImage)
{
	m_faceMaskPoints.shader = shaderFaceMaskPoints;
	m_faceMaskImage.shader = shaderFaceMaskImage;
}

void FaceMask::drawMaskImage(Camera camera)
{
	m_faceMaskImage.mesh->Draw(*m_faceMaskImage.shader, camera, m_faceMaskImage.model);
}

void FaceMask::loadFaceDetectObj(FaceDetectObj faceDetectObj, FaceDetect* faceDetect)
{
	m_faceDetectObj = faceDetectObj;

	std::vector<Vertex> faceMaskImageVertices{
		Vertex{glm::vec3(-1.0f * m_imageAspectRatio, 1.0f,  0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
		Vertex{glm::vec3(-1.0f * m_imageAspectRatio, -1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f)},
		Vertex{glm::vec3(1.0f * m_imageAspectRatio, -1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
		Vertex{glm::vec3(1.0f * m_imageAspectRatio, 1.0f,  0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)}
	};
	std::vector<GLuint> faceMaskImageIndices{
		0, 1, 2,
		0, 2, 3
	};

	//------------------------------- Initialize the Face Mask Points Model ----------------------------------//

	// Texture for face mask points
	Texture textureFaceMaskPoints[]
	{
		Texture("assets/colors/red.png", "diffuse", 0)
	};

	// Scale the face detection landmarks
	float sideLength = glm::tan(glm::radians(22.5f)) * 1.0f;			// calculate scaling face detect
	float faceMaskScaledLength = 2.0f - (2.0f * sideLength);

	// Create a mesh object from the face detection landmarks
	std::vector<Vertex> faceVertices;												// Extra two points are for the neck
	for (size_t j = 0; j < m_faceDetectObj.landmarks.size(); j++)		// Loop through landmarks in face
	{
		faceVertices.push_back( Vertex{ glm::vec3((float)m_faceDetectObj.landmarks[j].x * m_imageAspectRatio / (float)RESIZED_IMAGE_WIDTH, (float)m_faceDetectObj.landmarks[j].y / (float)RESIZED_IMAGE_HEIGHT,  0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f) } );
	}
	// Update landmarks for the neck
	faceVertices[68] = Vertex{ glm::vec3((float)m_faceDetectObj.landmarks[5].x * m_imageAspectRatio / (float)RESIZED_IMAGE_WIDTH, (float)(m_faceDetectObj.landmarks[5].y + 500) / (float)RESIZED_IMAGE_HEIGHT,  0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f) };
	faceVertices[69] = Vertex{ glm::vec3((float)m_faceDetectObj.landmarks[11].x * m_imageAspectRatio / (float)RESIZED_IMAGE_WIDTH, (float)(m_faceDetectObj.landmarks[11].y + 500) / (float)RESIZED_IMAGE_HEIGHT,  0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f) };
	
	// Create and store face mesh
	m_faceMaskPoints.mesh = new Mesh(faceVertices, faceDetect->m_indices, *m_faceMaskPoints.textures);

	// Activate shader for Face Mask and configure the model matrix
	m_faceMaskPoints.shader->Activate();
	glm::mat4 faceMaskPointsModel = glm::mat4(1.0f);
	faceMaskPointsModel = glm::scale(faceMaskPointsModel, glm::vec3(faceMaskScaledLength, faceMaskScaledLength, faceMaskScaledLength));
	faceMaskPointsModel = glm::translate(faceMaskPointsModel, glm::vec3(0.375f, -0.5f, -0.875f));
	faceMaskPointsModel = glm::rotate(faceMaskPointsModel, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	m_faceMaskPoints.model = faceMaskPointsModel;

	//------------------------------- Initialize the Face Mask Image Model ----------------------------------//

	// Create image mesh
	m_faceMaskImage.mesh = new Mesh(faceMaskImageVertices, faceMaskImageIndices, *m_faceMaskImage.textures);

	m_faceMaskImage.shader->Activate();
	glm::mat4 faceMaskImageModel = glm::mat4(1.0f);
	faceMaskImageModel = glm::rotate(faceMaskImageModel, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));	// Flip the image
	faceMaskImageModel = glm::rotate(faceMaskImageModel, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));	// Flip the image
	faceMaskImageModel = glm::scale(faceMaskImageModel, glm::vec3(0.6f, 0.6f, 1.0f));
	glm::mat4 translation_1 = glm::translate(glm::mat4(1.f), glm::vec3(0.0f, 0.0f, -1.0f));
	faceMaskImageModel = translation_1 * faceMaskImageModel;

	glm::mat4 testModel = glm::mat4(1.0f);
	testModel = glm::scale(testModel, glm::vec3(0.6f, 0.6f, 1.0f));
	/*glm::mat4 trans = glm::translate(glm::mat4(1.f), glm::vec3(0.0f, 0.0f, -1.0f));
	testModel = trans * testModel;*/

	cv::Point th = m_faceDetectObj.topPoint;
	cv::Point bh = m_faceDetectObj.botPoint;
	cv::Point rc = m_faceDetectObj.rightPoint;
	cv::Point lc = m_faceDetectObj.leftPoint;
	glm::vec4 vth(glm::vec3(0.75 - (th.x * m_imageAspectRatio / (float)RESIZED_IMAGE_WIDTH * 2), 1 - (th.y / (float)RESIZED_IMAGE_HEIGHT * 2), -1.0f), 1.0f);
	glm::vec4 vbh(glm::vec3(0.75 - (bh.x * m_imageAspectRatio / (float)RESIZED_IMAGE_WIDTH * 2), 1 - (bh.y / (float)RESIZED_IMAGE_HEIGHT * 2), -1.0f), 1.0f);
	glm::vec4 vrc(glm::vec3(0.75 - (rc.x * m_imageAspectRatio / (float)RESIZED_IMAGE_WIDTH * 2), 1 - (rc.y / (float)RESIZED_IMAGE_HEIGHT * 2), -1.0f), 1.0f);
	glm::vec4 vlc(glm::vec3(0.75 - (lc.x * m_imageAspectRatio / (float)RESIZED_IMAGE_WIDTH * 2), 1 - (lc.y / (float)RESIZED_IMAGE_HEIGHT * 2), -1.0f), 1.0f);
	vth = testModel * vth;
	vbh = testModel * vbh;
	vrc = testModel * vrc;
	vlc = testModel * vlc;
	m_faceDetectObj.topCoord = vth;
	m_faceDetectObj.width = glm::length(vrc - vlc);
	m_faceDetectObj.height = glm::length(vth - vbh);
	m_faceMaskImage.model = faceMaskImageModel;
}