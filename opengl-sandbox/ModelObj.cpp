#include "ModelObj.h"

ModelObj::ModelObj(std::string file, std::vector<Texture> textures)
{
	filename = file;
	tinyobj::ObjReader reader;
	tinyobj::ObjReaderConfig reader_config;

	reader_config.mtl_search_path = "./"; // Path to material files

	if (!reader.ParseFromFile(filename, reader_config)) {
		if (!reader.Error().empty()) {
			std::cerr << "TinyObjReader: " << reader.Error();
		}
		exit(1);
	}

	if (!reader.Warning().empty()) {
		std::cout << "TinyObjReader: " << reader.Warning();
	}

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	auto& materials = reader.GetMaterials();

#if ENABLE_DEBUG
	printf("# of vertices  = %d\n", (int)(attrib.vertices.size()) / 3);
	printf("# of normals   = %d\n", (int)(attrib.normals.size()) / 3);
	printf("# of texcoords = %d\n", (int)(attrib.texcoords.size()) / 2);
	printf("# of materials = %d\n", (int)materials.size());
	printf("# of shapes    = %d\n", (int)shapes.size());
#endif

	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;

		// We create a new mesh per shape
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;
		int idx_counter = 0;

		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
				tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
				tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
				glm::vec3 vertex = { vx, vy, vz };

#if ENABLE_DEBUG
				if (vx > max_vert_x) { max_vert_x = vx; }
				if (vx < min_vert_x) { min_vert_x = vx; }
				if (vy > max_vert_y) { max_vert_y = vy; }
				if (vy < min_vert_y) { min_vert_y = vy; }
				if (vz > max_vert_z) { max_vert_z = vz; }
				if (vz < min_vert_z) { min_vert_z = vz; }
#endif //ENABLE_DEBUG

				// Check if `normal_index` is zero or positive. negative = no normal data
				glm::vec3 normal = { 0.0f, 0.0f, 0.0f };
				if (idx.normal_index >= 0) {
					tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
					tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
					tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];

					normal = { nx, ny, nz };
				}

				// Check if `texcoord_index` is zero or positive. negative = no texcoord data
				glm::vec2 texcoord = { 0.0f, 0.0f };
				if (idx.texcoord_index >= 0) {
					tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
					tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];

					texcoord = { tx, ty };
				}

				// Optional: vertex colors
				tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
				tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
				tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
				glm::vec3 color = { red, green, blue };

				vertices.push_back({
					vertex,
					normal,
					color,
					texcoord
				});
				indices.push_back(idx_counter++);	// Store the index of the vertex
			}
			index_offset += fv;

			// per-face material
			shapes[s].mesh.material_ids[f];
		}

		meshes.push_back(Mesh(vertices, indices, textures));
	}

#if ENABLE_DEBUG
	printf("max_x=%f, min_x=%f, max_y=%f, min_y=%f, max_z=%f, min_z=%f\n", max_vert_x, min_vert_x, max_vert_y, min_vert_y, max_vert_z, min_vert_z);
#endif
}

void ModelObj::Draw(Shader& shader, Camera& camera)
{
	for (size_t i = 0; i < meshes.size(); i++) {
		meshes[i].Draw(shader, camera, modelMat);
	}
}

void ModelObj::UpdateModel(glm::mat4 model)
{
	model = glm::translate(model, Position);
	model = glm::scale(model, glm::vec3(scale, scale, scale));
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::lookAt(Position, Position + Orientation, Up);
	model = model * view;
	modelMat = model;
#if ENABLE_DEBUG
	glm::mat4 minVertexMat = glm::mat4(1.0f);
	minVertexMat = glm::scale(minVertexMat, glm::vec3(1.0f / 21.0f, 1.0f / 21.0f, 1.0f / 21.0f));
	glm::vec3 minVertexPos = glm::vec3(min_vert_x, min_vert_y, min_vert_z);
	minVertexMat = glm::translate(minVertexMat, minVertexPos);
	minVertexMat = minVertexMat * modelMat;

	std::cout << filename << " min model coordinates:" << std::endl;
	std::cout << "x="<<minVertexMat[3].x << std::endl;
	std::cout << "y="<<minVertexMat[3].y << std::endl;
	std::cout << "z="<<minVertexMat[3].z << std::endl;

	glm::mat4 maxVertexMat = glm::mat4(1.0f);
	glm::vec3 maxVertexPos = glm::vec3(max_vert_x, max_vert_y, max_vert_z);
	maxVertexMat = glm::scale(maxVertexMat, glm::vec3(1.0f / 21.0f, 1.0f / 21.0f, 1.0f / 21.0f));
	maxVertexMat = glm::translate(maxVertexMat, maxVertexPos);
	maxVertexMat = maxVertexMat * modelMat;

	std::cout << filename << " max model coordinates:" << std::endl;
	std::cout << "x=" << maxVertexMat[3].x << std::endl;
	std::cout << "y=" << maxVertexMat[3].y << std::endl;
	std::cout << "z=" << maxVertexMat[3].z << std::endl;
#endif
}

void ModelObj::Inputs(GLFWwindow* window, int width, int height)
{
	// Handles key inputs
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		Position += speed * Orientation;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		Position += speed * -glm::normalize(glm::cross(Orientation, Up));
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		Position += speed * -Orientation;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		Position += speed * glm::normalize(glm::cross(Orientation, Up));
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		Position += speed * Up;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		Position += speed * -Up;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		speed = 0.4f;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
	{
		speed = 0.1f;
	}
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
	{
		scale += speed * 0.01;
	}
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
	{
		scale += speed * -0.01;
	}

	// Handles mouse inputs
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		// Hides mouse cursor
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		// Prevents camera from jumping on the first click
		if (firstClick)
		{
			glfwSetCursorPos(window, (width / 2), (height / 2));
			firstClick = false;
		}

		// Stores the coordinates of the cursor
		double mouseX;
		double mouseY;
		// Fetches the coordinates of the cursor
		glfwGetCursorPos(window, &mouseX, &mouseY);

		// Normalizes and shifts the coordinates of the cursor such that they begin in the middle of the screen
		// and then "transforms" them into degrees 
		float rotX = sensitivity * (float)(mouseY - (height / 2)) / height;
		float rotY = sensitivity * (float)(mouseX - (width / 2)) / width;

		// Calculates upcoming vertical change in the Orientation
		glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-rotX), glm::normalize(glm::cross(Orientation, Up)));

		// Decides whether or not the next vertical Orientation is legal or not
		if (abs(glm::angle(newOrientation, Up) - glm::radians(90.0f)) <= glm::radians(85.0f))
		{
			Orientation = newOrientation;
		}

		// Rotates the Orientation left and right
		Orientation = glm::rotate(Orientation, glm::radians(-rotY), Up);

		// Sets mouse cursor to the middle of the screen so that it doesn't end up roaming around
		glfwSetCursorPos(window, (width / 2), (height / 2));
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		// Unhides cursor since camera is not looking around anymore
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		// Makes sure the next time the camera looks around it doesn't jump
		firstClick = true;
	}
}