#ifndef MODEL_H
#define MODEL_H

#include <stdbool.h>

struct Model;
struct Static_Mesh;

void model_init(struct Model* model, struct Static_Mesh* mesh, const char* geometry_name, int material_type);
bool model_geometry_set(struct Model* model, const char* geometry_name);
void model_reset(struct Model* model, struct Static_Mesh* mesh);

#endif
