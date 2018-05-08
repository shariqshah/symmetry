#ifndef MATERIAL_H
#define MATERIAL_H

#include "../common/linmath.h"
#include "../common/num_types.h"
#include "../common/variant.h"

struct Model;

#define MAX_UNIFORM_NAME_LEN 64
#define MAX_MATERIAL_REGISTERED_MODELS 1024

struct Uniform
{
	int  location;
	int  type;
};

enum Mat_Type
{
	MAT_BLINN = 0,
	MAT_UNSHADED,
	MAT_MAX
};

enum Mat_Model_Param
{
	MMP_DIFFUSE_TEX = 0,
	MMP_DIFFUSE_COL,
	MMP_DIFFUSE,
	MMP_SPECULAR_STRENGTH,
	MMP_SPECULAR,
	MMP_MAX
};

enum Mat_Pipeline_Param
{
	MPP_MODEL_MAT = 0,
	MPP_INV_MODEL_MAT,
	MPP_VIEW_MAT,
	MPP_MVP,
	MPP_FOG_MODE,
	MPP_FOG_DENSITY,
	MPP_FOG_START_DIST,
	MPP_FOG_MAX_DIST,
	MPP_FOG_COLOR,
	MPP_CAM_POS,
	MPP_TOTAL_LIGHTS,
	MPP_AMBIENT_LIGHT,
	MPP_MAX
};

struct Material
{
	int            type;
	int            shader;
	struct Model*  registered_models[MAX_MATERIAL_REGISTERED_MODELS];
	bool           lit;
	struct Uniform model_params[MMP_MAX];
	struct Uniform pipeline_params[MPP_MAX];
};

bool material_init(struct Material* material, int material_type);
void material_reset(struct Material* material);
bool material_register_model(struct Material* material, struct Model* model);
void material_unregister_model(struct Material* material, struct Model* model);


#endif
