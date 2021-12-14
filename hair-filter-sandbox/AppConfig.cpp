#include "AppConfig.h"

void AppConfig::LoadConfig(std::string file_path)
{
	std::string configstext = get_file_contents(file_path.c_str());
	json configsJSON = json::parse(configstext);

	params.face_image = configsJSON["configs"]["image"];
	params.hair_texture = configsJSON["configs"]["hair_texture"];
	params.hair_obj = configsJSON["configs"]["hair_obj"];
	params.enable_front_indx = configsJSON["configs"]["enable_front_idx"];
	params.ratio_width = configsJSON["hairs"][params.hair_obj]["transformations"]["ratio_width"];
	params.ratio_height = configsJSON["hairs"][params.hair_obj]["transformations"]["ratio_height"];
	params.scale_z = configsJSON["hairs"][params.hair_obj]["transformations"]["scale_z"];
	params.pitch = configsJSON["hairs"][params.hair_obj]["transformations"]["pitch"];
	params.yaw = configsJSON["hairs"][params.hair_obj]["transformations"]["yaw"];
	params.roll = configsJSON["hairs"][params.hair_obj]["transformations"]["roll"];
	params.topheadx = configsJSON["hairs"][params.hair_obj]["transformations"]["topheadx"];
	params.topheady = configsJSON["hairs"][params.hair_obj]["transformations"]["topheady"];
	params.topheadz = configsJSON["hairs"][params.hair_obj]["transformations"]["topheadz"];
	params.front_vert_index = configsJSON["hairs"][params.hair_obj]["transformations"]["front_vert_index"];
}