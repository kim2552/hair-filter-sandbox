#include"globals.h"
#include"AppConfig.h"
#include"ModelGltf.h"
#include"ModelObj.h"
#include"FaceDetect.h"
#include"FaceMesh.h"
#include"FaceMask.h"
#include"CameraImage.h"
#include"HairObject.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

// Vertices coordinates
Vertex imgVerts[] =
{ //               COORDINATES           /            NORMALS          /           COLORS         /       TEXCOORDS         //
	Vertex{glm::vec3(-1.0f, 1.0f,  0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
	Vertex{glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f)},
	Vertex{glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
	Vertex{glm::vec3(1.0f, 1.0f,  0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)}
};

// Indices for vertices order
GLuint imgInds[] =
{
	0, 1, 2,
	0, 2, 3
};

GLFWwindow* InitializeGlfw()
{
	// Initialize GLFW
	glfwInit();

	// Tell GLFW what version of OpenGL we are using 
	// In this case we are using OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Tell GLFW we are using the CORE profile
	// So that means we only have the modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a GLFWwindow object
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "hair-filter-sandbox", NULL, NULL);
	// Error check if the window fails to create
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}
	// Introduce the window into the current context
	glfwMakeContextCurrent(window);

	//Load GLAD so it configures OpenGL
	gladLoadGL();
	// Specify the viewport of OpenGL in the Window
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	return window;
}

int main()
{
	// Initialize GLFW window
	GLFWwindow* window = InitializeGlfw();
	if (window == NULL) {
		return -1;
	}

	AppConfig config;
	config.LoadConfig("assets/configs.json");

	/***************************************************************************/

	/************Initialize Shader Objects and Image Parameters*************/
	// Generate shader objects
	Shader shaderProgramHairObject("model.vert", "model.frag");
	Shader shaderProgramCameraImage("image.vert", "image.frag");
	Shader shaderProgramFaceMaskImage("image.vert", "image_trans.frag");
	Shader shaderProgramFaceMaskPoints("point.vert", "point.frag");
	Shader shaderProgramFaceMesh("point.vert", "point.frag");
	Shader shaderProgramPoint("point.vert", "point.frag");

	// Stores the width, height, and the number of color channels of the image
	int widthImg, heightImg, numColCh;

	// Flips the image so it appears right side up
	//stbi_set_flip_vertically_on_load(true);

	// Reads the image from a file and stores it in bytes
	unsigned char* imgBytes = stbi_load(config.params.face_image.c_str(), &widthImg, &heightImg, &numColCh, 0);	//TODO::Replace this with live camera feed images

	float imageAspectRatio = (float)widthImg / (float)heightImg;
	/***************************************************************/

	/******************Face Detect Landmarks***********************/

	// Initialize face detect object
	FaceDetect fdetect = FaceDetect();									// FaceDetect object
	FaceDetectObj faceDetectObj;										// Array for all faces

	// Get FaceDetectObject from the image
	faceDetectObj = fdetect.getFaceObject(imgBytes, widthImg, heightImg);

	std::vector<Texture> textureFaceMaskPoints													// Texture for face detect
	{
		Texture("assets/colors/red.png", "diffuse", 0)
	};
	std::vector<Texture> textureFaceMaskImage
	{
		Texture(imgBytes, "image", 0, widthImg, heightImg, numColCh),						// Texture object deletes imgBytes afterwards
		Texture(fdetect.m_face_mask_image, "transparency", 1, RESIZED_IMAGE_WIDTH, RESIZED_IMAGE_HEIGHT, 1)			// Texture object deletes imgBytes afterwards
	};

	FaceMask faceMask;
	faceMask.loadImageAspectRatio(imageAspectRatio);
	faceMask.loadShaders(&shaderProgramFaceMaskPoints, &shaderProgramFaceMaskImage);
	faceMask.loadTextures(&textureFaceMaskPoints, &textureFaceMaskImage);
	faceMask.loadFaceDetectObj(faceDetectObj, &fdetect);

	/**************************Image Texture***********************/

	// Image texture data
	std::vector<Texture> textureCameraImage
	{
		Texture(imgBytes, "diffuse", 0, widthImg, heightImg, numColCh)						// Texture object deletes imgBytes afterwards
	};
	CameraImage cameraImage;
	cameraImage.loadImageAspectRatio(imageAspectRatio);
	cameraImage.loadShaders(&shaderProgramCameraImage);
	cameraImage.loadTextures(&textureCameraImage);
	cameraImage.initCameraImage();

	/*****************************************************************/

	/**************************Hair Object**********************************/

	// Hair object texture data
	std::vector<Texture> textureHairObject
	{
		Texture(config.params.hair_texture.c_str(), "diffuse", 0)							// Texture object deletes imgBytes afterwards
	};
	HairObject hairObject;
	hairObject.loadImageAspectRatio(imageAspectRatio);
	hairObject.loadShaders(&shaderProgramHairObject);
	hairObject.loadTextures(&textureHairObject);
	hairObject.initHairObject(&faceMask, &config);

	/*************************************************************/

	/**************************DEBUG POINT(s)**************************/
#if ENABLE_DEBUG_POINTS
	// Vertices coordinates
	Vertex pointVerts[] =
	{ //               COORDINATES           /            NORMALS          /           COLORS         /       TEXCOORDS         //
		Vertex{glm::vec3(-1.0f, 1.0f,  0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
		Vertex{glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f)},
		Vertex{glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
		Vertex{glm::vec3(1.0f, 1.0f,  0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)}
	};

	// Indices for vertices order
	GLuint pointInds[] =
	{
		0, 1, 2,
		0, 2, 3
	};

	Texture pointText[]
	{
		Texture("assets/colors/green.png", "diffuse", 0)
	};

	// Store mesh data in vectors for the mesh
	std::vector <Vertex> pVerts(pointVerts, pointVerts + sizeof(pointVerts) / sizeof(Vertex));
	std::vector <GLuint> pInds(pointInds, pointInds + sizeof(pointInds) / sizeof(GLuint));
	std::vector <Texture> pTex(pointText, pointText + sizeof(pointText) / sizeof(Texture));
	// Create image mesh
	Mesh pointMesh(pVerts, pInds, pTex);

	// Activate shader for Face Mask and configure the model matrix
	shaderProgramPoint.Activate();
	glm::mat4 pointModel = glm::mat4(1.0f);
	pointModel = glm::scale(pointModel, glm::vec3(0.01f, 0.01f, 0.01f));
	glm::mat4 trp = glm::translate(glm::mat4(1.f), glm::vec3(hairObject.m_hairModelObj->model->fixedVertex.x, hairObject.m_hairModelObj->model->fixedVertex.y, hairObject.m_hairModelObj->model->fixedVertex.z));
	pointModel = trp * pointModel;
#endif	//ENABLE_DEBUG_POINTS
	/******************************************************************/

	/***********************Other Initializations*************************/
	// Creates camera object
	Camera camera(WINDOW_WIDTH, WINDOW_HEIGHT, glm::vec3(0.0f, 0.0f, 2.5f));	// x, y, z position, move camera so we can see image

	// Enables the Depth Buffer
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Draw in wireframe polygons
#if ENABLE_WIREFRAME
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif

	// Control selection parameters
	int num_options = 2;	// Camera, Hair
	int selected = 0;		// Selection vars to select input controls

	/***********************************************************/
	
	int front_head_indx = 0;
	// Main while loop
	while (!glfwWindowShouldClose(window))
	{
		// Handles key inputs
		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		{
			selected = (selected + 1) % num_options;
			printf("SELECTED %d", selected);
		}
		// Specify the color of the background
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		// Clean the back buffer and assign the new color to it
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (selected == 0) {			// Current selected input controls is Camera
			// Handles camera inputs
			camera.Inputs(window);
			// Updates and exports the camera matrix to the Vertex Shader
			camera.updateMatrix(45.0f, 0.1f, 100.0f);

			if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
			{
				float colorx = ((double)rand() / (RAND_MAX));
				float colory = ((double)rand() / (RAND_MAX));
				float colorz = ((double)rand() / (RAND_MAX));
				hairObject.m_hairRenderObj.shader->Activate();
				glUniform3f(glGetUniformLocation(hairObject.m_hairRenderObj.shader->ID, "filterColor"), colorx, colory, colorz);
			}
		}

		cameraImage.drawCameraImage(camera);

		if(config.params.enable_front_indx)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		if(config.params.enable_front_indx)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//for (size_t i = 0; i < faceDetectMeshes.size(); i++)
		//{
		//	faceDetectMeshes[i].Draw(shaderProgramFaceMask, camera, faceDetectModels[i]);
		//}
#if ENABLE_DEBUG_POINTS
		pointMesh.Draw(shaderProgramPoint, camera, pointModel);
#endif
		if (selected == 1) {			// Current selected input controls is Object
			front_head_indx = hairObject.m_hairModelObj->model->front_head_vertex_index;
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			{
				front_head_indx += 1;
				hairObject.m_hairModelObj->model->front_head_vertex_index = front_head_indx;
			}
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			{
				front_head_indx -= 1;
				if (front_head_indx < 0) front_head_indx = 0;
				hairObject.m_hairModelObj->model->front_head_vertex_index = front_head_indx;
			}
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			{
				front_head_indx += 100;
				hairObject.m_hairModelObj->model->front_head_vertex_index = front_head_indx;
			}
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			{
				front_head_indx -= 100;
				if (front_head_indx < 0) front_head_indx = 0;
				hairObject.m_hairModelObj->model->front_head_vertex_index = front_head_indx;
			}

			hairObject.m_hairModelObj->model->Inputs(window, WINDOW_WIDTH, WINDOW_HEIGHT); //TODO::remove model parameter

			glm::mat4 newModel = glm::mat4(1.0f);

			// Calculate the scale of the hair object
			hairObject.m_hairModelObj->model->originalModelWidth = glm::length(hairObject.m_hairModelObj->model->originalBb.max.x - hairObject.m_hairModelObj->model->originalBb.min.x);
			hairObject.m_hairModelObj->model->originalModelHeight = glm::length(hairObject.m_hairModelObj->model->originalBb.max.y - hairObject.m_hairModelObj->model->originalBb.min.y);
			float scaleMultWidth = hairObject.m_hairModelObj->model->savedRatioWidth * hairObject.m_hairModelObj->model->faceWidth / hairObject.m_hairModelObj->model->originalModelWidth;
			float scaleMultHeight = hairObject.m_hairModelObj->model->savedRatioHeight * hairObject.m_hairModelObj->model->faceHeight / hairObject.m_hairModelObj->model->originalModelHeight;
			newModel = glm::scale(newModel, glm::vec3(scaleMultWidth, scaleMultHeight, hairObject.m_hairModelObj->model->savedScaleZ));	// TODO::Find calculation for Z component

			// Rotate object to match face direction
			newModel = glm::rotate(newModel, glm::radians(hairObject.m_hairModelObj->model->facePitch + hairObject.m_hairModelObj->model->savedPitch), glm::vec3(1.0f, 0.0f, 0.0f));
			newModel = glm::rotate(newModel, glm::radians(hairObject.m_hairModelObj->model->faceYaw + hairObject.m_hairModelObj->model->savedYaw), glm::vec3(0.0f, 1.0f, 0.0f));
			newModel = glm::rotate(newModel, glm::radians(hairObject.m_hairModelObj->model->faceRoll + hairObject.m_hairModelObj->model->savedRoll), glm::vec3(0.0f, 0.0f, 1.0f));

			hairObject.m_hairModelObj->model->UpdateModel(newModel);

			float transX = (hairObject.m_hairModelObj->model->savedTopHeadDist.x + hairObject.m_hairModelObj->model->topHeadCoord.x) - hairObject.m_hairModelObj->model->fixedVertex.x;
			float transY = (hairObject.m_hairModelObj->model->savedTopHeadDist.y + hairObject.m_hairModelObj->model->topHeadCoord.y) - hairObject.m_hairModelObj->model->fixedVertex.y;
			float transZ = (hairObject.m_hairModelObj->model->savedTopHeadDist.z + hairObject.m_hairModelObj->model->topHeadCoord.z) - hairObject.m_hairModelObj->model->fixedVertex.z;

			glm::mat4 newtrans = glm::translate(glm::mat4(1.f), glm::vec3(transX, transY, transZ));
			newModel = newtrans * newModel;

			hairObject.m_hairModelObj->model->UpdateModel(newModel);

			pointModel = glm::mat4(1.0f);
			pointModel = glm::scale(pointModel, glm::vec3(0.01f, 0.01f, 0.01f));
			glm::mat4 trp = glm::translate(glm::mat4(1.f), glm::vec3(hairObject.m_hairModelObj->model->fixedVertex.x, hairObject.m_hairModelObj->model->fixedVertex.y, hairObject.m_hairModelObj->model->fixedVertex.z));
			pointModel = trp * pointModel;
		}

		if(config.params.enable_front_indx)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		hairObject.m_hairModelObj->model->Draw(*hairObject.m_hairRenderObj.shader, camera);					// Draw the object
		if(config.params.enable_front_indx)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		faceMask.drawMaskImage(camera);

		// Swap the back buffer with the front buffer
		glfwSwapBuffers(window);
		// Take care of all GLFW events
		glfwPollEvents();
	}

	// Delete all the objects we've created
	cameraImage.m_cameraImage.shader->Delete();
	faceMask.m_faceMaskImage.shader->Delete();
	faceMask.m_faceMaskPoints.shader->Delete();
	shaderProgramFaceMesh.Delete();
	hairObject.m_hairRenderObj.shader->Delete();
	// Delete window before ending the program
	glfwDestroyWindow(window);
	// Terminate GLFW before ending the program
	glfwTerminate();
	return 0;
}