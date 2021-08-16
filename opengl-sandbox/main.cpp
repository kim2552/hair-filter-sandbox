#include"ModelGltf.h"
#include"ModelObj.h"
#include"FaceDetect.h"
#include"FaceMesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

const unsigned int width = 600;
const unsigned int height = 800;

const float windowAspectRatio = float(width) / float(height);

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

// Vertices coordinates for face
GLuint faceInds[] =
{
	0, 17, 36,
	0, 1, 36,
	1, 41, 36,
	1, 31, 41,
	1, 2, 31,
	2, 3, 31,
	3, 4, 31,
	4, 48, 31,
	4, 5, 48,
	5, 60, 48,
	5, 6, 60,
	6, 59, 60,
	6, 7, 59,
	7, 58, 59,
	7, 57, 58,
	7, 8, 57,
	8, 56, 57,
	8, 9, 56,
	9, 55, 56,
	9, 10, 55,
	10, 64, 55,
	10, 54, 64,
	10, 11, 54,
	11, 12, 54,
	11, 12, 54,
	13, 54, 12,
	13, 35, 54,
	13, 14, 35,
	14, 15, 35,
	15, 46, 35,
	15, 45, 46,
	15, 16, 45,
	16, 26, 45,
	26, 25, 45,
	25, 24, 45,
	24, 44, 45,
	24, 23, 44,
	23, 43, 44,
	23, 22, 43,
	22, 42, 43,
	22, 27, 42,
	27, 21, 39,
	21, 38, 39,
	21, 20, 38,
	20, 19, 38,
	19, 37, 38,
	19, 18, 37,
	18, 36, 37,
	18, 17, 36,
	36, 37, 41,
	37, 41, 40,
	37, 40, 38,
	38, 40, 39,
	42, 47, 43,
	47, 46, 43,
	43, 46, 44,
	44, 46, 45,
	27, 39, 28,
	39, 40, 28,
	40, 28, 29,
	41, 40, 29,
	41, 30, 29,
	41, 30, 31,
	27, 42, 28,
	28, 47, 42,
	28, 29, 47,
	29, 46, 47,
	29, 30, 46,
	30, 35, 46,
	30, 31, 32,
	30, 32, 33,
	30, 33, 34,
	30, 34, 35,
	31, 48, 60,
	31, 60, 49,
	31, 49, 32,
	32, 49, 50,
	32, 50, 33,
	33, 50, 51,
	33, 51, 52,
	33, 52, 34,
	34, 52, 35,
	35, 52, 53,
	35, 53, 64,
	35, 64, 54,
	60, 49, 59,
	49, 59, 67,
	49, 67, 61,
	49, 61, 50,
	50, 61, 51,
	51, 61, 62,
	51, 62, 63,
	51, 63, 52,
	52, 63, 53,
	61, 67, 62,
	59, 67, 58,
	67, 58, 66,
	58, 66, 57,
	67, 62, 66,
	66, 65, 62,
	62, 65, 63,
	63, 65, 53,
	66, 57, 65,
	65, 57, 56,
	65, 56, 55,
	53, 65, 55,
	53, 55, 64,
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
	GLFWwindow* window = glfwCreateWindow(width, height, "OpenGL-Sandbox", NULL, NULL);
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
	glViewport(0, 0, width, height);

	/***************************************************************************/

	/************Initialize Shader Objects and Image Parameters*************/
	// Generate shader objects
	Shader shaderProgramObj("model.vert", "model.frag");
	Shader shaderProgramImg("image.vert", "image.frag");
	Shader shaderProgramFaceMask("point.vert", "point.frag");
	Shader shaderProgramFaceMesh("point.vert", "point.frag");

	// Take care of all the light related things
	glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec3 lightPos = glm::vec3(0.5f, 0.5f, 0.5f);

	// Stores the width, height, and the number of color channels of the image
	int widthImg, heightImg, numColCh;

	// Flips the image so it appears right side up
	//stbi_set_flip_vertically_on_load(true);

	// Reads the image from a file and stores it in bytes
	unsigned char* imgBytes = stbi_load("assets/image/testside2.jpg", &widthImg, &heightImg, &numColCh, 0);	//TODO::Replace this with live camera feed images

	float imageAspectRatio = (float)widthImg / (float)heightImg;
	
	/***************************************************************/

	/******************Face Detect Landmarks***********************/

	// Initialize face detect object
	FaceDetect fdetect = FaceDetect();
	std::vector<std::vector<cv::Point2f>> faces;						// Array for all faces
	std::vector<Mesh> facesdetectMesh;									// Each face has it's own mesh

	faces = fdetect.getFaceLandmarks(imgBytes, widthImg, heightImg);	// Get faces in image

	Texture faceText[]
	{
		Texture("assets/colors/red.png", "diffuse", 0)
	};
	for (size_t i = 0; i < faces.size(); i++)							// Loop through all the faces
	{
		Vertex faceVertex[68];
		for (size_t j = 0; j < faces[i].size(); j++)					// Loop through landmarks in face
		{
			faceVertex[j]= Vertex{ glm::vec3((float)faces[i][j].x* imageAspectRatio /(float)460.0f, ((float)faces[i][j].y/(460.0f* (float)height / (float)width)),  0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f) };
		}
		// Store mesh data in vectors for the mesh
		std::vector <Vertex> fVerts(faceVertex, faceVertex + sizeof(faceVertex) / sizeof(Vertex));
		std::vector <GLuint> fInds(faceInds, faceInds + sizeof(faceInds) / sizeof(GLuint));
		std::vector <Texture> fTex(faceText, faceText + sizeof(faceText) / sizeof(Texture));
		// Create and store face mesh
		facesdetectMesh.push_back(Mesh(fVerts, fInds, fTex));
	}

	// Activate shader for Face Mask and configure the model matrix
	shaderProgramFaceMask.Activate();
	glm::vec3 facemaskPos = glm::vec3(1.0f * imageAspectRatio, -1.0f, -0.1f);			//TODO::Create points to represent facial landmarks
	glm::mat4 facemaskModel = glm::mat4(1.0f);
	facemaskModel = glm::translate(facemaskModel, facemaskPos);
	facemaskModel = glm::scale(facemaskModel, glm::vec3(2.0f, 2.0f, 2.0f));
	//facemaskModel = glm::rotate(facemaskModel, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	facemaskModel = glm::rotate(facemaskModel, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramFaceMask.ID, "model"), 1, GL_FALSE, glm::value_ptr(facemaskModel));

	/****************************************************/

	/*********************Face Mesh Object*******************************/

	//Initialize face mesh object
	FaceMesh fmesh = FaceMesh();		// TODO:: Create face meshes for all faces
	eos::core::LandmarkCollection<Eigen::Vector2f> landmarkCollection = fmesh.processLandmarks(faces[0]);
	std::vector<Mesh> facesobjMesh;									// Each face has it's own mesh

	FaceMeshObj faceMeshObj = fmesh.getFaceMeshObj(landmarkCollection, widthImg, heightImg);

	Texture facemeshText[]
	{
		Texture("assets/colors/red.png", "diffuse", 0)
	};
	std::vector<Vertex> facemeshVerts;
	for (size_t i = 0; i < faceMeshObj.mesh.vertices.size(); i++)
	{
		facemeshVerts.push_back(Vertex{ glm::vec3(faceMeshObj.mesh.vertices[i](0)/(float)460.0f, faceMeshObj.mesh.vertices[i](1)/ (460.0f * (float)height / (float)width), faceMeshObj.mesh.vertices[i](2) / (float)heightImg), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(faceMeshObj.mesh.texcoords[i](0), faceMeshObj.mesh.texcoords[i](1)) });
	}
	std::vector<GLuint> fInds;
	for (size_t i = 0; i < faceMeshObj.mesh.tvi.size(); i++)
	{
		for (size_t j = 0; j < faceMeshObj.mesh.tvi[i].size(); j++)
		{
			fInds.push_back(faceMeshObj.mesh.tvi[i][j]);
		}
	}
	std::vector <Texture> fTex(facemeshText, facemeshText + sizeof(facemeshText) / sizeof(Texture));
	// Create and store face mesh
	facesobjMesh.push_back(Mesh(facemeshVerts, fInds, fTex));

	// Activate shader for Face Mesh and configure the model matrix
	shaderProgramFaceMesh.Activate();
	glm::vec3 facemeshPos = glm::vec3(1.0f * imageAspectRatio, -1.0f, -0.1f);			//TODO::Create points to represent facial landmarks
	glm::mat4 facemeshModel = glm::mat4(1.0f);
	facemeshModel = glm::translate(facemeshModel, facemeshPos);
	facemeshModel = glm::scale(facemeshModel, glm::vec3(2.0f, 2.0f, 2.0f));
	//facemeshModel = glm::rotate(facemeshModel, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	facemeshModel = glm::rotate(facemeshModel, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramFaceMesh.ID, "model"), 1, GL_FALSE, glm::value_ptr(facemeshModel));

	/******************************************************************/

	/**************************Image Texture***********************/

	// Image texture data
	Texture imgText[]
	{
		Texture(imgBytes, "diffuse", 0, widthImg, heightImg, numColCh)						// Texture object deletes imgBytes afterwards
	};
	for (size_t i = 0; i < sizeof(imgVerts); i++) {		// Width stays as 1.0, Height needs to change based on aspect ratio
		imgVerts[i].position.x *= imageAspectRatio;
	}
	// Store mesh data in vectors for the mesh
	std::vector <Vertex> iVerts(imgVerts, imgVerts + sizeof(imgVerts) / sizeof(Vertex));
	std::vector <GLuint> iInds(imgInds, imgInds + sizeof(imgInds) / sizeof(GLuint));
	std::vector <Texture> iTex(imgText, imgText + sizeof(imgText) / sizeof(Texture));
	// Create image mesh
	Mesh imgMesh(iVerts, iInds, iTex);

	// Activate shader for Image and configure the model matrix
	shaderProgramImg.Activate();
	glm::mat4 imgModel = glm::mat4(1.0f);
	imgModel = glm::scale(imgModel, glm::vec3(1.0f, 1.0f, 1.0f));
	imgModel = glm::rotate(imgModel, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));	// Flip the image
	imgModel = glm::rotate(imgModel, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));	// Flip the image
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramImg.ID, "model"), 1, GL_FALSE, glm::value_ptr(imgModel));

	/*****************************************************************/

	/**************************Hair Object**********************************/
	// Hair Texture data
	Texture hairText[]
	{
		Texture("assets/hair/dark_blonde_2.jpg", "diffuse", 0)
	};
	std::vector <Texture> hairTex(hairText, hairText + sizeof(hairText) / sizeof(Texture));

	std::string filename = "assets/hair/hair_bob.obj";
	ModelObj hairBob(filename, hairTex);

	// Activate shader for Object and configure the model matrix
	shaderProgramObj.Activate();
	glm::vec3 hairObjectPos = glm::vec3(0.0f, 0.0f, -1.0f);									//TODO::Translate object to fit face landmarks
	glm::mat4 hairObjectModel = glm::mat4(1.0f);
	hairObjectModel = glm::translate(hairObjectModel, hairObjectPos);
	hairObjectModel = glm::scale(hairObjectModel, glm::vec3(1.0f / 21.0f, 1.0f / 21.0f, 1.0f / 21.0f));	//TODO::Scale object to fit face
	hairObjectModel = glm::rotate(hairObjectModel, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));	//TODO::Rotate object to match face direction
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramObj.ID, "model"), 1, GL_FALSE, glm::value_ptr(hairObjectModel));
	glUniform4f(glGetUniformLocation(shaderProgramObj.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(shaderProgramObj.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
	hairBob.UpdateModel(hairObjectModel);																// Update object model

	/*************************************************************/

	/***********************Other Initializations*************************/
	// Creates camera object
	Camera camera(width, height, glm::vec3(0.0f, 0.0f, 2.5f));	// x, y, z position, move camera so we can see image

	// Enables the Depth Buffer
	glEnable(GL_DEPTH_TEST);

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

		for (size_t i = 0; i < facesdetectMesh.size(); i++)
		{
			facesdetectMesh[i].Draw(shaderProgramFaceMask, camera, facemaskModel);
		}

		for (size_t i = 0; i < facesobjMesh.size(); i++)
		{
			facesobjMesh[i].Draw(shaderProgramFaceMesh, camera, facemeshModel);
		}
		//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		//TODO::Insert facesdetectMesh and facesobjMesh
		//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		//if (selected == 1) {			// Current selected input controls is Object
		//	hairBob.Inputs(window, width, height);
		//	hairBob.UpdateModel(hairObjectModel);
		//}
		//hairBob.Draw(shaderProgramObj, camera);					// Draw the object

		// Swap the back buffer with the front buffer
		glfwSwapBuffers(window);
		// Take care of all GLFW events
		glfwPollEvents();
	}

	// Delete all the objects we've created
	shaderProgramImg.Delete();
	shaderProgramFaceMask.Delete();
	shaderProgramFaceMesh.Delete();
	shaderProgramObj.Delete();
	// Delete window before ending the program
	glfwDestroyWindow(window);
	// Terminate GLFW before ending the program
	glfwTerminate();
	return 0;
}