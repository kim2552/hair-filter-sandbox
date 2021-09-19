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
	unsigned char* imgBytes = stbi_load("assets/image/face3.jpg", &widthImg, &heightImg, &numColCh, 0);	//TODO::Replace this with live camera feed images

	float imageAspectRatio = (float)widthImg / (float)heightImg;
	
	/***************************************************************/

	/******************Face Detect Landmarks***********************/

	// Initialize face detect object
	FaceDetect fdetect = FaceDetect();									// FaceDetect object
	std::vector<std::vector<cv::Point>> faces;						// Array for all faces
	std::vector<Mesh> faceDetectMeshes;									// Each face has it's own mesh
	std::vector<glm::mat4> faceDetectModels;							// Each face has it's own model

	faces = fdetect.getFaceLandmarks(imgBytes, widthImg, heightImg);	// Get faces in image

	Texture faceText[]													// Texture for face detect
	{
		Texture("assets/colors/red.png", "diffuse", 0)
	};

	float sideLength = glm::tan(glm::radians(22.5f)) * 1.0f;			// calculate scaling face detect
	float faceMaskScaledLength = 2.0f - (2.0f * sideLength);

	// Create a mesh object from the face detect
	for (size_t i = 0; i < faces.size(); i++)							// Loop through all the faces
	{
		Vertex faceVertex[70];											// Extra two points are for the neck
		for (size_t j = 0; j < faces[i].size(); j++)					// Loop through landmarks in face
		{
			faceVertex[j]= Vertex{ glm::vec3((float)faces[i][j].x * imageAspectRatio /(float)RESIZED_IMAGE_WIDTH, (float)faces[i][j].y/(float)RESIZED_IMAGE_HEIGHT,  0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f) };
		}

		faceVertex[68] = Vertex{ glm::vec3((float)faces[i][5].x * imageAspectRatio / (float)RESIZED_IMAGE_WIDTH, (float)(faces[i][5].y+500) / (float)RESIZED_IMAGE_HEIGHT,  0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f) };

		faceVertex[69] = Vertex{ glm::vec3((float)faces[i][11].x * imageAspectRatio / (float)RESIZED_IMAGE_WIDTH, (float)(faces[i][11].y + 500) / (float)RESIZED_IMAGE_HEIGHT,  0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f) };

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
		Texture(fdetect.face_mask_image, "transparency", 1, widthImg, heightImg, 1)			// Texture object deletes imgBytes afterwards
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
	maskModel = glm::translate(maskModel, glm::vec3(0.0f, 0.0f, -0.5f));

	/*********************Face Mesh Object*******************************/
	//std::vector<Model> faceObjModels;								// Each face has it's own Model

	//for (size_t i = 0; i < faces.size(); i++)						// Loop through all the faces
	//{
	//	//Initialize face mesh object
	//	FaceMesh fmesh = FaceMesh();
	//	eos::core::LandmarkCollection<Eigen::Vector2f> landmarkCollection = fmesh.processLandmarks(faces[i]);
	//	FaceMeshObj faceMeshObj = fmesh.getFaceMeshObj(landmarkCollection, RESIZED_IMAGE_WIDTH, RESIZED_IMAGE_HEIGHT);

	//	Texture facemeshText[]										// Texture for face detect
	//	{
	//		Texture("assets/colors/blue.png", "diffuse", 0)
	//	};

	//	// Initialize all of the vertices
	//	std::vector<Vertex> facemeshVerts;
	//	for (size_t i = 0; i < faceMeshObj.mesh.vertices.size(); i++)
	//	{
	//		// Get original location of the vertex
	//		const auto p = glm::project(
	//			{ faceMeshObj.mesh.vertices[i][0], faceMeshObj.mesh.vertices[i][1], faceMeshObj.mesh.vertices[i][2] },
	//			faceMeshObj.rendering_parameters.get_modelview(), faceMeshObj.rendering_parameters.get_projection(), glm::vec4(0, RESIZED_IMAGE_HEIGHT, RESIZED_IMAGE_WIDTH, -RESIZED_IMAGE_HEIGHT));

	//		// Add the vertex after modifying based on aspect ratio
	//		facemeshVerts.push_back(Vertex{ glm::vec3(p.x * imageAspectRatio / (float)RESIZED_IMAGE_WIDTH, p.y / (float)RESIZED_IMAGE_HEIGHT, -p.z / (float)RESIZED_IMAGE_HEIGHT), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(faceMeshObj.mesh.texcoords[i](0), faceMeshObj.mesh.texcoords[i](1)) });
	//	}

	//	// Initialize all of the indices
	//	std::vector<GLuint> facemeshInds;
	//	for (size_t i = 0; i < faceMeshObj.mesh.tvi.size(); i++)
	//	{
	//		for (size_t j = 0; j < faceMeshObj.mesh.tvi[i].size(); j++)
	//		{
	//			facemeshInds.push_back(faceMeshObj.mesh.tvi[i][j]);
	//		}
	//	}

	//	// Initialize the textures
	//	std::vector <Texture> fTex(facemeshText, facemeshText + sizeof(facemeshText) / sizeof(Texture));

	//	// Create and store face mesh
	//	std::vector<Mesh> newFaceMeshes;
	//	newFaceMeshes.push_back(Mesh(facemeshVerts, facemeshInds, fTex));

	//	// calculate scaling face object
	//	sideLength = glm::tan(glm::radians(22.5f)) * 0.95f;
	//	float faceMeshScaledLength = 2.0f - (2.0f * sideLength);

	//	// Activate shader for Face Mesh and configure the model matrix
	//	shaderProgramFaceMesh.Activate();
	//	glm::mat4 faceMeshModel = glm::mat4(1.0f);
	//	faceMeshModel = glm::scale(faceMeshModel, glm::vec3(faceMeshScaledLength, faceMeshScaledLength, faceMeshScaledLength));
	//	faceMeshModel = glm::translate(faceMeshModel, glm::vec3(0.375f, -0.5f, -0.875f));
	//	faceMeshModel = glm::rotate(faceMeshModel, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	//	// Create a new Model object
	//	Model newFaceObj(newFaceMeshes);
	//	newFaceObj.roll = faceMeshObj.roll;
	//	newFaceObj.pitch = faceMeshObj.pitch;
	//	newFaceObj.yaw = faceMeshObj.yaw;

	//	const auto th = glm::project(
	//		{ faceMeshObj.mesh.vertices[FACE_MESH_TOPHEAD_INDEX][0], faceMeshObj.mesh.vertices[FACE_MESH_TOPHEAD_INDEX][1], faceMeshObj.mesh.vertices[FACE_MESH_TOPHEAD_INDEX][2] },
	//		faceMeshObj.rendering_parameters.get_modelview(), faceMeshObj.rendering_parameters.get_projection(), glm::vec4(0, RESIZED_IMAGE_HEIGHT, RESIZED_IMAGE_WIDTH, -RESIZED_IMAGE_HEIGHT));
	//	const auto bh = glm::project(
	//		{ faceMeshObj.mesh.vertices[FACE_MESH_BOTHEAD_INDEX][0], faceMeshObj.mesh.vertices[FACE_MESH_BOTHEAD_INDEX][1], faceMeshObj.mesh.vertices[FACE_MESH_BOTHEAD_INDEX][2] },
	//		faceMeshObj.rendering_parameters.get_modelview(), faceMeshObj.rendering_parameters.get_projection(), glm::vec4(0, RESIZED_IMAGE_HEIGHT, RESIZED_IMAGE_WIDTH, -RESIZED_IMAGE_HEIGHT));
	//	const auto rc = glm::project(
	//		{ faceMeshObj.mesh.vertices[FACE_MESH_RIGHTCONTOUR_INDEX][0], faceMeshObj.mesh.vertices[FACE_MESH_RIGHTCONTOUR_INDEX][1], faceMeshObj.mesh.vertices[FACE_MESH_RIGHTCONTOUR_INDEX][2] },
	//		faceMeshObj.rendering_parameters.get_modelview(), faceMeshObj.rendering_parameters.get_projection(), glm::vec4(0, RESIZED_IMAGE_HEIGHT, RESIZED_IMAGE_WIDTH, -RESIZED_IMAGE_HEIGHT));
	//	const auto lc = glm::project(
	//		{ faceMeshObj.mesh.vertices[FACE_MESH_LEFTCONTOUR_INDEX][0], faceMeshObj.mesh.vertices[FACE_MESH_LEFTCONTOUR_INDEX][1], faceMeshObj.mesh.vertices[FACE_MESH_LEFTCONTOUR_INDEX][2] },
	//		faceMeshObj.rendering_parameters.get_modelview(), faceMeshObj.rendering_parameters.get_projection(), glm::vec4(0, RESIZED_IMAGE_HEIGHT, RESIZED_IMAGE_WIDTH, -RESIZED_IMAGE_HEIGHT));
	//	glm::vec4 vth(glm::vec3(th.x * imageAspectRatio / (float)RESIZED_IMAGE_WIDTH, th.y / (float)RESIZED_IMAGE_HEIGHT, -th.z / (float)RESIZED_IMAGE_HEIGHT), 1.0f);
	//	glm::vec4 vbh(glm::vec3(bh.x* imageAspectRatio / (float)RESIZED_IMAGE_WIDTH, bh.y / (float)RESIZED_IMAGE_HEIGHT, -bh.z / (float)RESIZED_IMAGE_HEIGHT), 1.0f);
	//	glm::vec4 vrc(glm::vec3(rc.x* imageAspectRatio / (float)RESIZED_IMAGE_WIDTH, rc.y / (float)RESIZED_IMAGE_HEIGHT, -rc.z / (float)RESIZED_IMAGE_HEIGHT), 1.0f);
	//	glm::vec4 vlc(glm::vec3(lc.x* imageAspectRatio / (float)RESIZED_IMAGE_WIDTH, lc.y / (float)RESIZED_IMAGE_HEIGHT, -lc.z / (float)RESIZED_IMAGE_HEIGHT), 1.0f);
	//	vth = faceMeshModel * vth;
	//	vbh = faceMeshModel * vbh;
	//	vrc = faceMeshModel * vrc;
	//	vlc = faceMeshModel * vlc;
	//	newFaceObj.topHeadCoord = vth;
	//	newFaceObj.faceWidth = glm::length(vrc - vlc);
	//	newFaceObj.faceHeight = glm::length(vth - vbh);

	//	newFaceObj.UpdateModel(faceMeshModel);

	//	faceObjModels.push_back(newFaceObj);	// Store the Model

	//}
	/******************************************************************/

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
	glm::mat4 translation = glm::translate(glm::mat4(1.f), glm::vec3(faceObjModels[0].topHeadCoord.x, faceObjModels[0].topHeadCoord.y, faceObjModels[0].topHeadCoord.z));
	pointModel = translation * pointModel;
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

	//std::vector<ModelObj> hairObjs;

	//for (auto fom : faceObjModels)
	//{
	//	// Initialize texture
	//	Texture hairText[]
	//	{
	//		Texture("assets/hair/dark_blonde_2.jpg", "diffuse", 0)
	//	};
	//	std::vector <Texture> hairTex(hairText, hairText + sizeof(hairText) / sizeof(Texture));

	//	// Initialize model object
	//	std::string filename = "assets/hair/hair_bob.obj";
	//	ModelObj hairBob(filename, hairTex);

	//	// Transfer face mesh data to hair object
	//	hairBob.model->topHeadCoord = fom.topHeadCoord;
	//	hairBob.model->faceWidth = fom.faceWidth;
	//	hairBob.model->faceHeight = fom.faceHeight;

	//	// Activate shader for Object and configure the model matrix
	//	shaderProgramObj.Activate();
	//	glm::mat4 hairObjectModel = glm::mat4(1.0f);

	//	// Calculate the scale of the hair object
	//	float goldenRatioWidth = 1.7723179;				// Width ratio value based on developer preference			// TODO::Store values in json file
	//	float goldenRatioHeight = 1.8758706;			// Height ratio value based on developer preference
	//	float goldenZScale = 1.0f / 25.0f;				// Constant Z scale value based on developer preference
	//	float objectWidth = glm::length(hairBob.model->originalBb.max.x - hairBob.model->originalBb.min.x);
	//	float objectHeight = glm::length(hairBob.model->originalBb.max.y - hairBob.model->originalBb.min.y);
	//	float scaleMultWidth = goldenRatioWidth * hairBob.model->faceWidth / objectWidth;
	//	float scaleMultHeight = goldenRatioHeight * hairBob.model->faceHeight / objectHeight;
	//	hairObjectModel = glm::scale(hairObjectModel, glm::vec3(scaleMultWidth, scaleMultHeight, goldenZScale));	// TODO::Find calculation for Z component

	//	// Rotate object to match face direction
	//	hairObjectModel = glm::rotate(hairObjectModel, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//	hairObjectModel = glm::rotate(hairObjectModel, glm::radians(fom.pitch), glm::vec3(1.0f, 0.0f, 0.0f));
	//	hairObjectModel = glm::rotate(hairObjectModel, glm::radians(fom.yaw), glm::vec3(0.0f, -1.0f, 0.0f));
	//	hairObjectModel = glm::rotate(hairObjectModel, glm::radians(fom.roll), glm::vec3(0.0f, 0.0f, -1.0f));

	//	hairBob.model->UpdateModel(hairObjectModel);			// Updates the position and bounding box of the scaled, rotated object

	//	float goldenDiffX = -0.063447;		// Value obtained from fixedVertex distance from topHeadCoord	#TODO::Save values in a json file and retrieve it
	//	float goldenDiffY = 0.021071;		// Value obtained from fixedVertex distance from topHeadCoord
	//	float goldenDiffZ = 0.007910;		// Value obtained from fixedVertex distance from topHeadCoord
	//	
	//	float transX = (goldenDiffX + hairBob.model->topHeadCoord.x) - hairBob.model->fixedVertex.x;
	//	float transY = (goldenDiffY + hairBob.model->topHeadCoord.y) - hairBob.model->fixedVertex.y;
	//	float transZ = (goldenDiffZ + hairBob.model->topHeadCoord.z) - hairBob.model->fixedVertex.z;

	//	glm::mat4 translation = glm::translate(glm::mat4(1.f), glm::vec3(transX, transY, transZ));
	//	hairObjectModel = translation * hairObjectModel;

	//	glUniform4f(glGetUniformLocation(shaderProgramObj.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	//	glUniform3f(glGetUniformLocation(shaderProgramObj.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
	//	hairBob.model->UpdateModel(hairObjectModel);																// Update object model
	//	hairObjs.push_back(hairBob);
	//}
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
#if ENABLE_FACE_MESH
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
#endif
		for (size_t i = 0; i < faceDetectMeshes.size(); i++)
		{
			faceDetectMeshes[i].Draw(shaderProgramFaceMask, camera, faceDetectModels[i]);
		}
#if ENABLE_DEBUG_POINTS
		pointMesh.Draw(shaderProgramPoint, camera, pointModel);
#endif
#if ENABLE_FACE_MESH
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
		//for (size_t i = 0; i < hairObjs.size(); i++)
		//{
		//	if (selected == 1) {			// Current selected input controls is Object
		//		hairObjs[i].model->Inputs(window, windowWidth, windowHeight, hairObjs[i].model->modelMat); //TODO::remove model parameter
		//		hairObjs[i].model->UpdateModel(hairObjs[i].model->modelMat);
		//	}
		//	hairObjs[i].model->Draw(shaderProgramObj, camera);					// Draw the object
		//}


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