#include"ModelGltf.h"
#include"ModelObj.h"
#include"FaceDetect.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

const unsigned int width = 600;
const unsigned int height = 800;

const float imageHeightScale = float(width) / float(height);

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

// Vertices coordinates for points
Vertex pointVerts[] =
{ //               COORDINATES           /            NORMALS          /           COLORS         /       TEXCOORDS         //
	Vertex{glm::vec3(-1.0f, 1.0f,  -0.01f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
	Vertex{glm::vec3(-1.0f, -1.0f, -0.01f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f)},
	Vertex{glm::vec3(1.0f, -1.0f, -0.01f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
	Vertex{glm::vec3(1.0f, 1.0f,  -0.01f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)}
};

// Vertices coordinates for points
GLuint pointInds[] =
{
	0, 1, 2,
	0, 2, 3
};


int main()
{
	// Stores the width, height, and the number of color channels of the image
	int fwidthImg, fheightImg, fnumColCh;
	// Reads the image from a file and stores it in bytes
	unsigned char* fbytes = stbi_load("assets/image/testside.jpg", &fwidthImg, &fheightImg, &fnumColCh, 0);
	FaceDetect fdetect = FaceDetect();
	fdetect.getFaceLandmarks(fbytes, fwidthImg, fheightImg);

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

	/*-----------------------------------------------------------------------*/



	// Generate shader objects
	Shader shaderProgramObj("model.vert", "model.frag");
	Shader shaderProgramImg("image.vert", "image.frag");
	Shader shaderProgramPoint("point.vert", "point.frag");
	Shader shaderProgramFace("image.vert", "image.frag");



	// Take care of all the light related things
	glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec3 lightPos = glm::vec3(0.5f, 0.5f, 0.5f);



	// Image texture data
	Texture imgText[]	//TODO::Replace this with live camera feed images
	{
		Texture("assets/image/face.jpg", "diffuse", 0)
	};
	for (size_t i = 0; i < sizeof(imgVerts); i++) {		// Width stays as 1.0, Height needs to change based on aspect ratio
		imgVerts[i].position.y *= imageHeightScale;
	}
	// Store mesh data in vectors for the mesh
	std::vector <Vertex> iVerts(imgVerts, imgVerts + sizeof(imgVerts) / sizeof(Vertex));
	std::vector <GLuint> iInds(imgInds, imgInds + sizeof(imgInds) / sizeof(GLuint));
	std::vector <Texture> iTex(imgText, imgText + sizeof(imgText) / sizeof(Texture));
	// Create image mesh
	Mesh imgMesh(iVerts, iInds, iTex);

	// Hair Texture data
	Texture hairText[]
	{
		Texture("assets/hair/dark_blonde_2.jpg", "diffuse", 0)
	};
	std::vector <Texture> hairTex(hairText, hairText + sizeof(hairText) / sizeof(Texture));

	std::string filename = "assets/hair/hair_bob.obj";
	ModelObj hairBob(filename, hairTex);

	// Point Texture data
	Texture pointText[]
	{
		Texture("assets/colors/red.png", "diffuse", 0)
	};
	// Store mesh data in vectors for the mesh
	std::vector <Vertex> pVerts(pointVerts, pointVerts + sizeof(pointVerts) / sizeof(Vertex));
	std::vector <GLuint> pInds(pointInds, pointInds + sizeof(pointInds) / sizeof(GLuint));
	std::vector <Texture> pTex(pointText, pointText + sizeof(pointText) / sizeof(Texture));
	// Create image mesh
	Mesh pointMesh(pVerts, pInds, pTex);



	// Activate shader for Image and configure the model matrix
	shaderProgramImg.Activate();
	glm::mat4 imgModel = glm::mat4(1.0f);
	imgModel = glm::scale(imgModel, glm::vec3(1.0f, 1.0f, 1.0f));
	imgModel = glm::rotate(imgModel, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	imgModel = glm::rotate(imgModel, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));	// Flip the image
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramImg.ID, "model"), 1, GL_FALSE, glm::value_ptr(imgModel));

#if ENABLE_DEBUG
	// Print out min and max positions for image mesh	TODO::Move this outside of main
	glm::mat4 minVertexMat = glm::mat4(1.0f);
	glm::vec3 minVertexPos = imgVerts[1].position;
	minVertexMat = glm::translate(minVertexMat, minVertexPos);
	minVertexMat = minVertexMat * imgModel;

	std::cout << "image" << " min model coordinates:" << std::endl;
	std::cout << "x=" << minVertexMat[3].x << std::endl;
	std::cout << "y=" << minVertexMat[3].y << std::endl;
	std::cout << "z=" << minVertexMat[3].z << std::endl;

	glm::mat4 maxVertexMat = glm::mat4(1.0f);
	glm::vec3 maxVertexPos = imgVerts[3].position;
	maxVertexMat = glm::translate(maxVertexMat, maxVertexPos);
	maxVertexMat = maxVertexMat * imgModel;

	std::cout << "image" << " max model coordinates:" << std::endl;
	std::cout << "x=" << maxVertexMat[3].x << std::endl;
	std::cout << "y=" << maxVertexMat[3].y << std::endl;
	std::cout << "z=" << maxVertexMat[3].z << std::endl;
#endif

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

	// Activate shader for Point and configure the model matrix
	shaderProgramPoint.Activate();											//TODO::Width and Height matches that of image?
	glm::vec3 pointPos = glm::vec3(0.0f, -0.2300347222222, 0.0f);			//TODO::Create points to represent facial landmarks
	glm::mat4 pointModel = glm::mat4(1.0f);
	pointModel = glm::translate(pointModel, pointPos);
	pointModel = glm::scale(pointModel, glm::vec3(1.0f / 200.0f, 1.0f / 200.0f, 1.0f / 200.0f));		// Points scaled down
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramPoint.ID, "model"), 1, GL_FALSE, glm::value_ptr(pointModel));



	// Enables the Depth Buffer
	glEnable(GL_DEPTH_TEST);

	// Draw in wireframe polygons
#if ENABLE_WIREFRAME
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif

	// Creates camera object
	Camera camera(width, height, glm::vec3(0.0f, 0.0f, 2.5f));	// x, y, z position, move camera so we can see image

	int num_options = 2;	// Camera, Hair
	int selected = 0;		// Selection vars to select input controls

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

		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		//TODO::Insert "invisible" face mesh here
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		if (selected == 1) {			// Current selected input controls is Object
			hairBob.Inputs(window, width, height);
			hairBob.UpdateModel(hairObjectModel);
		}
		hairBob.Draw(shaderProgramObj, camera);					// Draw the object

		pointMesh.Draw(shaderProgramPoint, camera, pointModel);	// Draw the points

		// Swap the back buffer with the front buffer
		glfwSwapBuffers(window);
		// Take care of all GLFW events
		glfwPollEvents();
	}

	// Delete all the objects we've created
	shaderProgramObj.Delete();
	shaderProgramImg.Delete();
	// Delete window before ending the program
	glfwDestroyWindow(window);
	// Terminate GLFW before ending the program
	glfwTerminate();
	return 0;
}