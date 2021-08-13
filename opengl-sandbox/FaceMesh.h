#pragma once
#ifndef FACE_MESH_H
#define FACE_MESH_H

#include "eos/core/Image.hpp"
#include "eos/core/image/opencv_interop.hpp"
#include "eos/core/Landmark.hpp"
#include "eos/core/LandmarkMapper.hpp"
#include "eos/core/read_pts_landmarks.hpp"
#include "eos/core/write_obj.hpp"
#include "eos/fitting/fitting.hpp"
#include "eos/morphablemodel/Blendshape.hpp"
#include "eos/morphablemodel/MorphableModel.hpp"
#include "eos/render/opencv/draw_utils.hpp"
#include "eos/render/texture_extraction.hpp"
#include "eos/cpp17/optional.hpp"

struct FaceMeshObj{
	eos::core::Mesh mesh;
	float yaw;
	float pitch;
	float roll;
};

class FaceMesh
{
public:

	eos::morphablemodel::MorphableModel morphable_model;
	eos::core::LandmarkMapper landmark_mapper;
	std::vector<eos::morphablemodel::Blendshape> blendshapes;
	eos::morphablemodel::MorphableModel morphable_model_with_expressions;
	eos::fitting::ModelContour model_contour;
	eos::fitting::ContourLandmarks ibug_contour;
	eos::morphablemodel::EdgeTopology edge_topology;

	FaceMesh();

	eos::core::LandmarkCollection<Eigen::Vector2f> processLandmarks(std::vector<cv::Point2f> faceLandmarks);

	FaceMeshObj getFaceMeshObj(eos::core::LandmarkCollection<Eigen::Vector2f> landmarks, int imgWidth, int imgHeight);

private:
	std::string modelfile;
	std::string mappingsfile;
	std::string contourfile;
	std::string edgetopologyfile;
	std::string blendshapesfile;
};

#endif	//FACE_MESH_H