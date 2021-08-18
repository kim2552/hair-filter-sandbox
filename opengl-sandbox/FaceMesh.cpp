#include "FaceMesh.h"

FaceMesh::FaceMesh()
{
	modelfile = "assets/data/sfm_shape_3448.bin";
	mappingsfile = "assets/data/ibug_to_sfm.txt";
	contourfile = "assets/data/sfm_model_contours.json";
	edgetopologyfile = "assets/data/sfm_3448_edge_topology.json";
	blendshapesfile = "assets/data/expression_blendshapes_3448.bin";

	morphable_model = eos::morphablemodel::load_model(modelfile);

	landmark_mapper = eos::core::LandmarkMapper(mappingsfile);

	blendshapes = eos::morphablemodel::load_blendshapes(blendshapesfile);

	morphable_model_with_expressions = eos::morphablemodel::MorphableModel(
		morphable_model.get_shape_model(), blendshapes, morphable_model.get_color_model(), eos::cpp17::nullopt,
		morphable_model.get_texture_coordinates());

	// These two are used to fit the front-facing contour to the ibug contour landmarks:
	model_contour = eos::fitting::ModelContour::load(contourfile);
	 ibug_contour = eos::fitting::ContourLandmarks::load(mappingsfile);

	 // The edge topology is used to speed up computation of the occluding face contour fitting:
	 //edge_topology = eos::morphablemodel::load_edge_topology(edgetopologyfile);
}

/* Process facial landmarks and store them in a LandmarkCollection object*/
eos::core::LandmarkCollection<Eigen::Vector2f> FaceMesh::processLandmarks(std::vector<cv::Point2f> face)
{
	eos::core::LandmarkCollection<Eigen::Vector2f> landmarks;
	landmarks.reserve(68);

	int ibugId = 1;
	for (size_t i = 0; i < face.size(); i++) {
		eos::core::Landmark<Eigen::Vector2f> landmark;

		landmark.name = std::to_string(ibugId);
		landmark.coordinates[0] = face[i].x;
		landmark.coordinates[1] = face[i].y;	//TODO::Might have to shift by 1.0f
		landmarks.emplace_back(landmark);
		++ibugId;
	}

	return landmarks;
}

FaceMeshObj FaceMesh::getFaceMeshObj(eos::core::LandmarkCollection<Eigen::Vector2f> landmarks, int imgWidth, int imgHeight)
{
	FaceMeshObj meshObj;

	// Fit the model, get back a mesh and the pose:
	std::tie(meshObj.mesh, meshObj.rendering_parameters) = eos::fitting::fit_shape_and_pose(
		morphable_model_with_expressions, landmarks, landmark_mapper, imgWidth, imgHeight, edge_topology,
		ibug_contour, model_contour, 5, eos::cpp17::nullopt, 30.0f);

	// The 3D head pose can be recovered as follows:
	meshObj.yaw = glm::degrees(glm::yaw(meshObj.rendering_parameters.get_rotation()));
	meshObj.pitch = glm::degrees(glm::pitch(meshObj.rendering_parameters.get_rotation()));
	meshObj.roll = glm::degrees(glm::roll(meshObj.rendering_parameters.get_rotation()));

#if ENABLE_DEBUG
	printf("-------------Face Mesh Object Properties------------\n");
	printf("yaw=%f\n", meshObj.yaw);
	printf("pitch=%f\n", meshObj.pitch);
	printf("roll=%f\n", meshObj.roll);
	printf("----------------------------------------------------");
#endif

	return meshObj;
}