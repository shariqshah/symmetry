#ifndef MODEL_H
#define MODEL_H

struct Model;
struct Static_Mesh;

void model_init(struct Model* model, struct Static_Mesh* mesh, const char* geometry_name, int material_type);
void model_reset(struct Model* model, struct Static_Mesh* mesh);

#endif
