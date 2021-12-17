#include "AppConfig.h"

void AppConfig::LoadConfig(std::string file_path)
{
	std::string assets_path = "assets/";

	std::string configstext = get_file_contents(file_path.c_str());
	json configsJSON = json::parse(configstext);

	params.face_image = configsJSON["configs"]["image"];
	params.hair_selection = configsJSON["configs"]["hair_selection"];
	params.enable_wireframe = configsJSON["configs"]["enable_wireframe"];
	params.hair_obj = assets_path + std::string(configsJSON["hairs"][params.hair_selection]["path"]);
	params.hair_texture = assets_path+std::string(configsJSON["hairs"][params.hair_selection]["texture"]);
	params.ratio_width = configsJSON["hairs"][params.hair_selection]["transformations"]["ratio_width"];
	params.ratio_height = configsJSON["hairs"][params.hair_selection]["transformations"]["ratio_height"];
	params.ratio_length = configsJSON["hairs"][params.hair_selection]["transformations"]["ratio_length"];
	params.pitch = configsJSON["hairs"][params.hair_selection]["transformations"]["pitch"];
	params.yaw = configsJSON["hairs"][params.hair_selection]["transformations"]["yaw"];
	params.roll = configsJSON["hairs"][params.hair_selection]["transformations"]["roll"];
	params.topheadx = configsJSON["hairs"][params.hair_selection]["transformations"]["topheadx"];
	params.topheady = configsJSON["hairs"][params.hair_selection]["transformations"]["topheady"];
	params.topheadz = configsJSON["hairs"][params.hair_selection]["transformations"]["topheadz"];
	params.front_vert_index = configsJSON["hairs"][params.hair_selection]["transformations"]["front_vert_index"];
}