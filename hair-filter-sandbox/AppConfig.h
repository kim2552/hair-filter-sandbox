#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "globals.h"

#include <string>

#include "ModelGltf.h"

struct Parameters
{
	std::string face_image;
	std::string hair_selection;
	std::string hair_texture;
	std::string hair_obj;
	int enable_wireframe;
	float ratio_width;
	float ratio_height;
	float scale_z;
	float pitch;
	float yaw;
	float roll;
	float topheadx;
	float topheady;
	float topheadz;
	int front_vert_index;
};

class AppConfig
{
public:
	void LoadConfig(std::string file_path);
	Parameters params;
};

#endif