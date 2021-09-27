#include"ModelGltf.h"
#include"ModelObj.h"
#include"FaceDetect.h"
#include"FaceMesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

const unsigned int windowWidth = 600;
const unsigned int windowHeight = 800;

const float windowAspectRatio = float(windowWidth) / float(windowHeight);

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

int main()
{
	/***********************GLFW************************/
	// Initialize GLFW
	glfwInit();

	// Tell GLFW what version of OpenGL we are using 
	// In this case we are using OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Tell GLFW we are using the CORE profile
	// So that means we only have the modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a GLFWwindow object of 800 by 800 pixels, naming it "YoutubeOpenGL"
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "OpenGL-Sandbox", NULL, NULL);
	// Error check if the window fails to create
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	// Introduce the window into the current context
	glfwMakeContextCurrent(window);

	//Load GLAD so it configures OpenGL
	gladLoadGL();
	// Specify the viewport of OpenGL in the Window
	// In this case the viewport goes from x = 0, y = 0, to x = 800, y = 800
	glViewport(0, 0, windowWidth, windowHeight);

	/***************************************************************************/

	/************Initialize Shader Objects and Image Parameters*************/
	// Generate shader objects
	Shader shaderProgramObj("model.vert", "model.frag");
	Shader shaderProgramImg("image.vert", "image.frag");
	Shader shaderProgramMask("image.vert", "image_trans.frag");
	Shader shaderProgramFaceMask("point.vert", "point.frag");
	Shader shaderProgramFaceMesh("point.vert", "point.frag");
	Shader shaderProgramPoint("point.vert", "point.frag");

	// Take care of all the light related things
	glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec3 lightPos = glm::vec3(0.5f, 0.5f, 0.5f);

	// Stores the width, height, and the number of color channels of the image
	int widthImg, heightImg, numColCh;

	// Flips the image so it appears right side up
	//stbi_set_flip_vertically_on_load(true);

	// Reads the image from a file and stores it in bytes
	unsigned char* imgBytes = stbi_load("assets/image/face1.jpg", &widthImg, &heightImg, &numColCh, 0);	//TODO::Replace this with live camera feed images

	float imageAspectRatio = (float)widthImg / (float)heightImg;
	
	/***************************************************************/

	/******************Face Detect Landmarks***********************/

	// Initialize face detect object
	FaceDetect fdetect = FaceDetect();									// FaceDetect object
	std::vector<FaceDetectObj> faceObjs;								// Array for all faces
	std::vector<Mesh> faceDetectMeshes;									// Each face has it's own mesh
	std::vector<glm::mat4> faceDetectModels;							// Each face has it's own model

	faceObjs = fdetect.getFaceLandmarks(imgBytes, widthImg, heightImg);	// Get faces in image

	Texture faceText[]													// Texture for face detect
	{
		Texture("assets/colors/red.png", "diffuse", 0)
	};

	float sideLength = glm::tan(glm::radians(22.5f)) * 1.0f;			// calculate scaling face detect
	float faceMaskScaledLength = 2.0f - (2.0f * sideLength);

	// Create a mesh object from the face detect
	for (size_t i = 0; i < faceObjs.size(); i++)							// Loop through all the faces
	{
		Vertex faceVertex[70];											// Extra two points are for the neck
		for (size_t j = 0; j < faceObjs[i].shape.size(); j++)					// Loop through landmarks in face
		{
			faceVertex[j]= Vertex{ glm::vec3((float)faceObjs[i].shape[j].x * imageAspectRatio /(float)RESIZED_IMAGE_WIDTH, (float)faceObjs[i].shape[j].y/(float)RESIZED_IMAGE_HEIGHT,  0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f) };
		}

		faceVertex[68] = Vertex{ glm::vec3((float)faceObjs[i].shape[5].x * imageAspectRatio / (float)RESIZED_IMAGE_WIDTH, (float)(faceObjs[i].shape[5].y+500) / (float)RESIZED_IMAGE_HEIGHT,  0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f) };

		faceVertex[69] = Vertex{ glm::vec3((float)faceObjs[i].shape[11].x * imageAspectRatio / (float)RESIZED_IMAGE_WIDTH, (float)(faceObjs[i].shape[11].y + 500) / (float)RESIZED_IMAGE_HEIGHT,  0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f) };

		// Store mesh data in vectors for the mesh
		std::vector <Vertex> fVerts(faceVertex, faceVertex + sizeof(faceVertex) / sizeof(Vertex));
		std::vector <Texture> fTex(faceText, faceText + sizeof(faceText) / sizeof(Texture));
		// Create and store face mesh
		faceDetectMeshes.push_back(Mesh(fVerts, fdetect.indices, fTex));
	
		// Activate shader for Face Mask and configure the model matrix
		shaderProgramFaceMask.Activate();
		glm::mat4 facemaskModel = glm::mat4(1.0f);
		facemaskModel = glm::scale(facemaskModel, glm::vec3(faceMaskScaledLength, faceMaskScaledLength, faceMaskScaledLength));
		facemaskModel = glm::translate(facemaskModel, glm::vec3(0.375f, -0.5f, -0.875f));
		facemaskModel = glm::rotate(facemaskModel, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		faceDetectModels.push_back(facemaskModel);
	}

	/****************************************************/

	Texture maskTexture[]
	{
		Texture(imgBytes, "image", 0, widthImg, heightImg, numColCh),						// Texture object deletes imgBytes afterwards
		Texture(fdetect.face_mask_image, "transparency", 1, RESIZED_IMAGE_WIDTH, RESIZED_IMAGE_HEIGHT, 1)			// Texture object deletes imgBytes afterwards
	};
	for (size_t i = 0; i < sizeof(imgVerts); i++) {		// Height stays as 2.0, Width needs to change based on aspect ratio
		imgVerts[i].position.x *= imageAspectRatio;
	}
	// Store mesh data in vectors for the mesh
	std::vector <Vertex> mVerts(imgVerts, imgVerts + sizeof(imgVerts) / sizeof(Vertex));
	std::vector <GLuint> mInds(imgInds, imgInds + sizeof(imgInds) / sizeof(GLuint));
	std::vector <Texture> mTex(maskTexture, maskTexture + sizeof(maskTexture) / sizeof(Texture));
	// Create image mesh
	Mesh maskMesh(mVerts, mInds, mTex);

	shaderProgramMask.Activate();
	glm::mat4 maskModel = glm::mat4(1.0f);
	maskModel = glm::rotate(maskModel, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));	// Flip the image
	maskModel = glm::rotate(maskModel, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));	// Flip the image
	maskModel = glm::scale(maskModel, glm::vec3(0.6f, 0.6f, 1.0f));
	glm::mat4 translation = glm::translate(glm::mat4(1.f), glm::vec3(0.0f, 0.0f, -1.0f));
	maskModel = translation * maskModel;

	glm::mat4 testModel = glm::mat4(1.0f);
	testModel = glm::scale(testModel, glm::vec3(0.6f, 0.6f, 1.0f));
	glm::mat4 trans = glm::translate(glm::mat4(1.f), glm::vec3(0.0f, 0.0f, -1.0f));
	testModel = trans * testModel;
	for (size_t i = 0; i < faceObjs.size(); i++)							// Loop through all the faces
	{
		cv::Point th = faceObjs[i].topHeadPoint;
		cv::Point bh = faceObjs[i].botHeadPoint;
		cv::Point rc = faceObjs[i].rightHeadPoint;
		cv::Point lc = faceObjs[i].leftHeadPoint;
		glm::vec4 vth(glm::vec3(0.75 - (th.x * imageAspectRatio / (float)RESIZED_IMAGE_WIDTH * 2), 1 - (th.y / (float)RESIZED_IMAGE_HEIGHT * 2), 1.0f), 1.0f);
		glm::vec4 vbh(glm::vec3(0.75 - (bh.x * imageAspectRatio / (float)RESIZED_IMAGE_WIDTH * 2), 1 - (bh.y / (float)RESIZED_IMAGE_HEIGHT * 2), 1.0f), 1.0f);
		glm::vec4 vrc(glm::vec3(0.75 - (rc.x * imageAspectRatio / (float)RESIZED_IMAGE_WIDTH * 2), 1 - (rc.y / (float)RESIZED_IMAGE_HEIGHT * 2), 1.0f), 1.0f);
		glm::vec4 vlc(glm::vec3(0.75 - (lc.x * imageAspectRatio / (float)RESIZED_IMAGE_WIDTH * 2), 1 - (lc.y / (float)RESIZED_IMAGE_HEIGHT * 2), 1.0f), 1.0f);
		vth = testModel * vth;
		vbh = testModel * vbh;
		vrc = testModel * vrc;
		vlc = testModel * vlc;
		faceObjs[i].topHeadCoord = vth;
		faceObjs[i].width = glm::length(vrc - vlc);
		faceObjs[i].height = glm::length(vth - vbh);
	}

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
	glm::mat4 trp = glm::translate(glm::mat4(1.f), glm::vec3(faceObjs[0].topHeadCoord.x, faceObjs[0].topHeadCoord.y, -1.001f));
	pointModel = trp * pointModel;
#endif	//ENABLE_DEBUG_POINTS
	/******************************************************************/

	/**************************Image Texture***********************/

	// Image texture data
	Texture imgText[]
	{
		Texture(imgBytes, "diffuse", 0, widthImg, heightImg, numColCh)						// Texture object deletes imgBytes afterwards
	};
	// Store mesh data in vectors for the mesh
	std::vector <Vertex> iVerts(imgVerts, imgVerts + sizeof(imgVerts) / sizeof(Vertex));
	std::vector <GLuint> iInds(imgInds, imgInds + sizeof(imgInds) / sizeof(GLuint));
	std::vector <Texture> iTex(imgText, imgText + sizeof(imgText) / sizeof(Texture));
	// Create image mesh
	Mesh imgMesh(iVerts, iInds, iTex);

	// Activate shader for Image and configure the model matrix
	shaderProgramImg.Activate();
	glm::mat4 imgModel = glm::mat4(1.0f);
	imgModel = glm::rotate(imgModel, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));	// Flip the image
	imgModel = glm::rotate(imgModel, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));	// Flip the image

	/*****************************************************************/

	/**************************Hair Object**********************************/

	std::vector<ModelObj> hairObjs;

	for (auto fom : faceObjs)	// loop through each face object model
	{
		// Initialize texture
		Texture hairText[]
		{
			Texture("assets/hair/dark_blonde_2.jpg", "diffuse", 0)
		};
		std::vector <Texture> hairTex(hairText, hairText + sizeof(hairText) / sizeof(Texture));

		// Initialize model object
		std::string filename = "assets/hair/hair_bob.obj";
		ModelObj hairBob(filename, hairTex);

		// Transfer face mesh data to hair object
		hairBob.model->topHeadCoord = fom.topHeadCoord;
		hairBob.model->faceWidth = fom.width;
		hairBob.model->faceHeight = fom.height;

		// Activate shader for Object and configure the model matrix
		shaderProgramObj.Activate();
		glm::mat4 hairObjectModel = glm::mat4(1.0f);

		// Calculate the scale of the hair object
		hairBob.model->savedRatioWidth = 1.770502;				// Width ratio value based on developer preference			// TODO::Store values in json file
		hairBob.model->savedRatioHeight = 1.448357;			// Height ratio value based on developer preference
		hairBob.model->savedScaleZ = 0.036920;				// Constant Z scale value based on developer preference
		hairBob.model->originalModelWidth = glm::length(hairBob.model->originalBb.max.x - hairBob.model->originalBb.min.x);
		hairBob.model->originalModelHeight = glm::length(hairBob.model->originalBb.max.y - hairBob.model->originalBb.min.y);
		float scaleMultWidth = hairBob.model->savedRatioWidth * hairBob.model->faceWidth / hairBob.model->originalModelWidth;
		float scaleMultHeight = hairBob.model->savedRatioHeight * hairBob.model->faceHeight / hairBob.model->originalModelHeight;
		hairObjectModel = glm::scale(hairObjectModel, glm::vec3(scaleMultWidth, scaleMultHeight, hairBob.model->savedScaleZ));	// TODO::Find calculation for Z component

		// Rotate object to match face direction
		hairBob.model->facePitch = fom.pitch;
		hairBob.model->faceYaw = fom.yaw;
		hairBob.model->faceRoll = -fom.roll;
		hairBob.model->savedPitch = 179.599976;
		hairBob.model->savedYaw = 10.400003;
		hairBob.model->savedRoll = 0;

		hairObjectModel = glm::rotate(hairObjectModel, glm::radians(hairBob.model->facePitch + hairBob.model->savedPitch), glm::vec3(1.0f, 0.0f, 0.0f));
		hairObjectModel = glm::rotate(hairObjectModel, glm::radians(hairBob.model->faceYaw + hairBob.model->savedYaw), glm::vec3(0.0f, 1.0f, 0.0f));
		hairObjectModel = glm::rotate(hairObjectModel, glm::radians(hairBob.model->faceRoll + hairBob.model->savedRoll), glm::vec3(0.0f, 0.0f, 1.0f));

		hairBob.model->UpdateModel(hairObjectModel);

		hairBob.model->savedTopHeadDist.x = -0.062701;		// Value obtained from fixedVertex distance from topHeadCoord	#TODO::Save values in a json file and retrieve it
		hairBob.model->savedTopHeadDist.y = 0.002821;		// Value obtained from fixedVertex distance from topHeadCoord
		hairBob.model->savedTopHeadDist.z = -0.980658;		// Value obtained from fixedVertex distance from topHeadCoord
		
		float transX = (hairBob.model->savedTopHeadDist.x + hairBob.model->topHeadCoord.x) - hairBob.model->fixedVertex.x;
		float transY = (hairBob.model->savedTopHeadDist.y + hairBob.model->topHeadCoord.y) - hairBob.model->fixedVertex.y;
		float transZ = (hairBob.model->savedTopHeadDist.z + hairBob.model->topHeadCoord.z) - hairBob.model->fixedVertex.z;

		glm::mat4 translation = glm::translate(glm::mat4(1.f), glm::vec3(transX, transY, transZ));
		hairObjectModel = translation * hairObjectModel;

		glUniform4f(glGetUniformLocation(shaderProgramObj.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
		glUniform3f(glGetUniformLocation(shaderProgramObj.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		hairBob.model->UpdateModel(hairObjectModel);																// Update object model
		hairObjs.push_back(hairBob);
	}
	/*************************************************************/

	/***********************Other Initializations*************************/
	// Creates camera object
	Camera camera(windowWidth, windowHeight, glm::vec3(0.0f, 0.0f, 2.5f));	// x, y, z position, move camera so we can see image

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
		}

		imgMesh.Draw(shaderProgramImg, camera, imgModel);		// Draw the image
		maskMesh.Draw(shaderProgramMask, camera, maskModel);
		//for (size_t i = 0; i < faceDetectMeshes.size(); i++)
		//{
		//	faceDetectMeshes[i].Draw(shaderProgramFaceMask, camera, faceDetectModels[i]);
		//}
#if ENABLE_DEBUG_POINTS
		pointMesh.Draw(shaderProgramPoint, camera, pointModel);
#endif
		for (size_t i = 0; i < hairObjs.size(); i++)
		{
			if (selected == 1) {			// Current selected input controls is Object
				hairObjs[i].model->Inputs(window, windowWidth, windowHeight); //TODO::remove model parameter

				glm::mat4 newModel = glm::mat4(1.0f);

				// Calculate the scale of the hair object
				hairObjs[i].model->originalModelWidth = glm::length(hairObjs[i].model->originalBb.max.x - hairObjs[i].model->originalBb.min.x);
				hairObjs[i].model->originalModelHeight = glm::length(hairObjs[i].model->originalBb.max.y - hairObjs[i].model->originalBb.min.y);
				float scaleMultWidth = hairObjs[i].model->savedRatioWidth * hairObjs[i].model->faceWidth / hairObjs[i].model->originalModelWidth;
				float scaleMultHeight = hairObjs[i].model->savedRatioHeight * hairObjs[i].model->faceHeight / hairObjs[i].model->originalModelHeight;
				newModel = glm::scale(newModel, glm::vec3(scaleMultWidth, scaleMultHeight, hairObjs[i].model->savedScaleZ));	// TODO::Find calculation for Z component

				// Rotate object to match face direction
				newModel = glm::rotate(newModel, glm::radians(hairObjs[i].model->facePitch + hairObjs[i].model->savedPitch), glm::vec3(1.0f, 0.0f, 0.0f));
				newModel = glm::rotate(newModel, glm::radians(hairObjs[i].model->faceYaw + hairObjs[i].model->savedYaw), glm::vec3(0.0f, 1.0f, 0.0f));
				newModel = glm::rotate(newModel, glm::radians(hairObjs[i].model->faceRoll + hairObjs[i].model->savedRoll), glm::vec3(0.0f, 0.0f, 1.0f));

				hairObjs[i].model->UpdateModel(newModel);

				float transX = (hairObjs[i].model->savedTopHeadDist.x + hairObjs[i].model->topHeadCoord.x) - hairObjs[i].model->fixedVertex.x;
				float transY = (hairObjs[i].model->savedTopHeadDist.y + hairObjs[i].model->topHeadCoord.y) - hairObjs[i].model->fixedVertex.y;
				float transZ = (hairObjs[i].model->savedTopHeadDist.z + hairObjs[i].model->topHeadCoord.z) - hairObjs[i].model->fixedVertex.z;

				glm::mat4 newtrans = glm::translate(glm::mat4(1.f), glm::vec3(transX, transY, transZ));
				newModel = newtrans * newModel;

				hairObjs[i].model->UpdateModel(newModel);
			}
			hairObjs[i].model->Draw(shaderProgramObj, camera);					// Draw the object
		}


		// Swap the back buffer with the front buffer
		glfwSwapBuffers(window);
		// Take care of all GLFW events
		glfwPollEvents();
	}

	// Delete all the objects we've created
	shaderProgramImg.Delete();
	shaderProgramFaceMask.Delete();
	shaderProgramFaceMask.Delete();
	shaderProgramFaceMesh.Delete();
	shaderProgramObj.Delete();
	// Delete window before ending the program
	glfwDestroyWindow(window);
	// Terminate GLFW before ending the program
	glfwTerminate();
	return 0;
}