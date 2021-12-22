#include"globals.h"
#include"AppConfig.h"
#include"ModelGltf.h"
#include"ModelObj.h"
#include"FaceDetect.h"
#include"FaceMesh.h"
#include"FaceMask.h"
#include"CameraImage.h"
#include"HairObject.h"
#include"DebugImage.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

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
	Shader shaderProgramDebugImage("point.vert", "point.frag");

	// Stores the width, height, and the number of color channels of the image
	int widthImg, heightImg, numColCh;

	// Flips the image so it appears right side up
	//stbi_set_flip_vertically_on_load(true);

	// Reads the image from a file and stores it in bytes
	unsigned char* imgBytes = stbi_load(config.params.face_image.c_str(), &widthImg, &heightImg, &numColCh, 0);

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
	hairObject.initHairObject(&config);
	hairObject.updateHairObject(&faceMask);

	/*************************************************************/

	/**************************DEBUG POINT(s)**************************/

#if ENABLE_DEBUG_POINTS
	std::vector<Texture> textureDebugImage
	{
		Texture("assets/colors/green.png", "diffuse", 0)
	};
	DebugImage debugImage;
	debugImage.loadImageAspectRatio(imageAspectRatio);
	debugImage.loadShaders(&shaderProgramDebugImage);
	debugImage.loadTextures(&textureDebugImage);
	debugImage.initDebugImage(hairObject.m_hairModelObj->model);
#endif	//ENABLE_DEBUG_POINTS

	/******************************************************************/

	/***********************Other Initializations*************************/

	// Creates camera object
	Camera camera(WINDOW_WIDTH, WINDOW_HEIGHT, glm::vec3(0.0f, 0.0f, 2.5f));	// x, y, z position, move camera so we can see image

	// Enables the Depth Buffer
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Draw in wireframe polygons
#if ENABLE_WIREFRAME
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif

	// Control selection parameters
	int num_options = 2;	// Camera, Hair
	int selected = 0;		// Selection vars to select input controls
	int front_head_indx = 0;

	/***********************************************************/
	
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

			// Randomize the color of the object
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

		if(config.params.enable_wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		if(config.params.enable_wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

#if ENABLE_DEBUG_POINTS
		debugImage.drawDebugImage(camera);
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
			if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
			{
				front_head_indx += 100;
				hairObject.m_hairModelObj->model->front_head_vertex_index = front_head_indx;
			}
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			{
				front_head_indx -= 100;
				if (front_head_indx < 0) front_head_indx = 0;
				hairObject.m_hairModelObj->model->front_head_vertex_index = front_head_indx;
			}

			hairObject.m_hairModelObj->model->Inputs(window, WINDOW_WIDTH, WINDOW_HEIGHT);

			// Update the hairObject model
			hairObject.updateHairObject(&faceMask, true);

			// Update the debugImage model
			debugImage.updateDebugImage(hairObject.m_hairModelObj->model);
		}

		if(config.params.enable_wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		hairObject.drawHairObject(camera);

		if(config.params.enable_wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_BLEND);
		faceMask.drawMaskImage(camera);
		glDisable(GL_BLEND);
		// Swap the back buffer with the front buffer
		glfwSwapBuffers(window);
		// Take care of all GLFW events
		glfwPollEvents();
	}

	// Delete all the objects we've created
	cameraImage.m_cameraImage.shader->Delete();
	faceMask.m_faceMaskImage.shader->Delete();
	faceMask.m_faceMaskPoints.shader->Delete();
	hairObject.m_hairRenderObj.shader->Delete();
	debugImage.m_debugImage.shader->Delete();

	// Delete window before ending the program
	glfwDestroyWindow(window);
	// Terminate GLFW before ending the program
	glfwTerminate();
	return 0;
}