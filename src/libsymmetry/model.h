#ifndef MODEL_H
#define MODEL_H

struct Model;

void model_init(struct Model* model, const char* geometry_name, int material_type);
void model_reset(struct Model* model);

#endif
