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

void Model::UpdateModel(glm::mat4 model, bool print_log)
{
	modelMat = model;			// Update the modelMat

	glm::vec4 vertexPoint = glm::vec4(meshes[0].vertices[front_head_vertex_index].position, 1.0f);
	fixedVertex = modelMat * vertexPoint;

	glm::vec4 vmin = modelMat * glm::vec4(originalBb.min, 1.0f);
	bb.min = glm::vec3(vmin.x, vmin.y, vmin.z);
	glm::vec4 vmax = modelMat * glm::vec4(originalBb.max, 1.0f);
	bb.max = glm::vec3(vmax.x, vmax.y, vmax.z);

	modelWidth = abs(bb.max.x - bb.min.x);
	modelHeight = abs(bb.max.y - bb.min.y);
	modelLength = abs(bb.max.z - bb.min.z);

	position = modelMat * glm::vec4(originalPosition,1.0f);

	if (print_log) {
		printf("--------Model Properties-----------\n");
		printf("topHeadCoord={%f,%f,%f}\n", topHeadCoord.x, topHeadCoord.y, topHeadCoord.z);
		printf("faceWidth=%f\n", faceWidth);
		printf("faceHeight=%f\n", faceHeight);
		printf("faceLength=%f\n", faceLength);
		printf("faceYaw,facePitch,faceRoll={%f,%f,%f}\n", faceYaw, facePitch, faceRoll);
		printf("bb minimum: {%f, %f, %f}\n", bb.min.x, bb.min.y, bb.min.z);
		printf("bb maximum: {%f, %f, %f}\n", bb.max.x, bb.max.y, bb.max.z);
		printf("fixed vertex: {%f, %f, %f}\n", fixedVertex.x, fixedVertex.y, fixedVertex.z);
		printf("model width: %f\n", modelWidth);
		printf("model height: %f\n", modelHeight);
		printf("model length: %f\n", modelLength);
		printf("original model width: %f\n", glm::length(originalBb.max.x - originalBb.min.x));
		printf("original model height: %f\n", glm::length(originalBb.max.y - originalBb.min.y));
		printf("original model Z: %f\n", glm::length(originalBb.max.z - originalBb.min.z));
		printf("scaledValueWidth: %f\n", sqrt(model[0][0] * model[0][0] + model[0][1] * model[0][1] + model[0][2] * model[0][2]));
		printf("scaledValueHeight: %f\n", sqrt(model[1][0] * model[1][0] + model[1][1] * model[1][1] + model[1][2] * model[1][2]));
		printf("position: {%f, %f, %f}\n", position.x, position.y, position.z);
		printf("ratio object to face width: %f\n", modelWidth / faceWidth);
		printf("ratio object to face height: %f\n", modelHeight / faceHeight);
		printf("position distance from topHeadCoord={%f,%f,%f}\n", position.x - topHeadCoord.x, position.y - topHeadCoord.y, position.z - topHeadCoord.z);
		printf("-------------------------------\n");
		printf("--------Save Configuration--------\n");
		printf("\"ratio_width\": %f,\n", sqrt(model[0][0] * model[0][0] + model[0][1] * model[0][1] + model[0][2] * model[0][2]) * glm::length(originalBb.max.x - originalBb.min.x) / faceWidth);
		printf("\"ratio_height\": %f,\n", sqrt(model[1][0] * model[1][0] + model[1][1] * model[1][1] + model[1][2] * model[1][2]) * glm::length(originalBb.max.y - originalBb.min.y) / faceHeight);
		printf("\"ratio_length\": %f,\n", sqrt(model[2][0] * model[2][0] + model[2][1] * model[2][1] + model[2][2] * model[2][2]) * glm::length(originalBb.max.z - originalBb.min.z) / faceLength);
		printf("\"pitch\": %f,\n", savedPitch);
		printf("\"yaw\": %f,\n", savedYaw);
		printf("\"roll\": %f,\n", savedRoll);
		//printf("\"w_ratio\": %f,\n", (bb.max.x - topHeadCoord.x) / modelWidth);
		//printf("\"h_ratio\": %f,\n", (bb.max.y - topHeadCoord.y) / modelHeight);
		//printf("\"l_ratio\": %f,\n", (bb.max.z - topHeadCoord.z) / modelLength);
		printf("\"topheadx\": %f,\n", fixedVertex.x - topHeadCoord.x);
		printf("\"topheady\": %f,\n", fixedVertex.y - topHeadCoord.y);
		printf("\"topheadz\": %f,\n", fixedVertex.z - topHeadCoord.z);
		printf("\"front_vert_index\": %i\n", front_head_vertex_index);
		printf("-------------------------------\n");
	}
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
		savedRatioLength = scale.z * originalModelLength / faceLength;
	}
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
	{
		scale += glm::vec3(-0.001);
		savedRatioWidth = scale.x * originalModelWidth / faceWidth;
		savedRatioHeight = scale.y * originalModelHeight / faceHeight;
		savedRatioLength = scale.z * originalModelLength / faceLength;
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
		savedRatioLength = scale.z * originalModelLength / faceLength;
	}
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
	{
		scale.z += -0.001;
		savedRatioLength = scale.z * originalModelLength / faceLength;
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