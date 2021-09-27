#include "Model.h"

Model::Model(std::vector<Mesh> m, int winWidth, int winHeight)
{
	meshes = m;
	windowWidth = winWidth;
	windowHeight = winHeight;
	faceRoll = 0;
	facePitch = 0;
	faceYaw = 0;
}

void Model::Draw(Shader& shader, Camera& camera)
{
	for (size_t i = 0; i < meshes.size(); i++) {
		meshes[i].Draw(shader, camera, modelMat);
	}
}

void Model::UpdateModel(glm::mat4 model)
{
	modelMat = model;			// Update the modelMat

	glm::vec4 vertexPoint = glm::vec4(meshes[0].vertices[0].position, 1.0f);
	fixedVertex = modelMat * vertexPoint;

	glm::vec4 vmin = modelMat * glm::vec4(originalBb.min,1.0f);
	bb.min = glm::vec3(vmin.x, vmin.y, vmin.z);
	glm::vec4 vmax = modelMat * glm::vec4(originalBb.max, 1.0f);
	bb.max = glm::vec3(vmax.x, vmax.y, vmax.z);

	modelWidth = abs(bb.max.x - bb.min.x);
	modelHeight = abs(bb.max.y - bb.min.y);

	position = modelMat * glm::vec4(originalPosition,1.0f);

	printf("--------Model Properties-----------\n");
	printf("topHeadCoord={%f,%f,%f}\n", topHeadCoord.x, topHeadCoord.y, topHeadCoord.z);
	printf("faceWidth=%f\n", faceWidth);
	printf("faceHeight=%f\n", faceHeight);
	printf("faceYaw,facePitch,faceRoll={%f,%f,%f}\n", faceYaw, facePitch, faceRoll);
	printf("bb minimum: {%f, %f, %f}\n", bb.min.x, bb.min.y, bb.min.z);
	printf("bb maximum: {%f, %f, %f}\n", bb.max.x, bb.max.y, bb.max.z);
	printf("model width: %f\n", modelWidth);
	printf("model height: %f\n", modelHeight);
	printf("original model width: %f\n", glm::length(originalBb.max.x - originalBb.min.x));
	printf("original model height: %f\n", glm::length(originalBb.max.y - originalBb.min.y));
	printf("scaledValueWidth: %f\n", sqrt(model[0][0] * model[0][0] + model[0][1] * model[0][1] + model[0][2] * model[0][2]));
	printf("scaledValueHeight: %f\n", sqrt(model[1][0] * model[1][0] + model[1][1] * model[1][1] + model[1][2] * model[1][2]));
	printf("position: {%f, %f, %f}\n", position.x, position.y, position.z);
	printf("ratio object to face width: %f\n", modelWidth / faceWidth);
	printf("ratio object to face height: %f\n", modelHeight / faceHeight);
	printf("position distance from topHeadCoord={%f,%f,%f}\n", position.x - topHeadCoord.x, position.y - topHeadCoord.y, position.z - topHeadCoord.z);
	printf("*SAVE*savedRatioWidth: %f\n", sqrt(model[0][0] * model[0][0] + model[0][1] * model[0][1] + model[0][2] * model[0][2]) * glm::length(originalBb.max.x - originalBb.min.x) / faceWidth);
	printf("*SAVE*savedRatioHeight: %f\n", sqrt(model[1][0] * model[1][0] + model[1][1] * model[1][1] + model[1][2] * model[1][2]) * glm::length(originalBb.max.y - originalBb.min.y) / faceHeight);
	printf("*SAVE*savedScaleZ: %f\n", sqrt(model[2][0] * model[2][0] + model[2][1] * model[2][1] + model[2][2] * model[2][2]));
	printf("*SAVE*savedTopHeadDist={%f,%f,%f}\n", fixedVertex.x - topHeadCoord.x, fixedVertex.y - topHeadCoord.y, fixedVertex.z - topHeadCoord.z);
	printf("*SAVE*savedPitch,savedYaw,savedRoll={%f,%f,%f}\n", savedPitch, savedYaw, savedRoll);
	printf("-------------------------------\n");

}

void Model::Inputs(GLFWwindow* window, int width, int height)
{
	// Handles key inputs
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		savedTopHeadDist.y += -0.01f;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		savedTopHeadDist.x += 0.01f;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		savedTopHeadDist.y += 0.01f;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		savedTopHeadDist.x += -0.01f;
	}
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
	{
		savedTopHeadDist.z += 0.01f;
	}
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
	{
		savedTopHeadDist.z += -0.01f;
	}
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
	{
		scale += glm::vec3(0.001);
		savedRatioWidth = scale.x * originalModelWidth / faceWidth;
		savedRatioHeight = scale.y * originalModelHeight / faceHeight;
		savedScaleZ = scale.z;
	}
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
	{
		scale += glm::vec3(-0.001);
		savedRatioWidth = scale.x * originalModelWidth / faceWidth;
		savedRatioHeight = scale.y * originalModelHeight / faceHeight;
		savedScaleZ = scale.z;
	}
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
	{
		scale.x += 0.001;
		savedRatioWidth = scale.x * originalModelWidth / faceWidth;
	}
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
	{
		scale.x += -0.001;
		savedRatioWidth = scale.x * originalModelWidth / faceWidth;
	}
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
	{
		scale.y += 0.001;
		savedRatioHeight = scale.y * originalModelHeight / faceHeight;
	}
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
	{
		scale.y += -0.001;
		savedRatioHeight = scale.y * originalModelHeight / faceHeight;
	}
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
	{
		scale.z += 0.001;
		savedScaleZ = scale.z;
	}
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
	{
		scale.z += -0.001;
		savedScaleZ = scale.z;
	}
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
	{
		savedPitch += 0.1f;
	}
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
	{
		savedPitch += -0.1f;
	}
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
	{
		savedYaw += 0.1f;
	}
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
	{
		savedYaw += -0.1f;
	}
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
	{
		savedRoll += 0.1f;
	}
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
	{
		savedRoll += -0.1f;
	}
}