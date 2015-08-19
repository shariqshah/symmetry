#ifndef model_H
#define model_H

struct Camera;

struct Model
{
	int node;
	int geometry_index;
	int shader;					/* Temporary, replace with material */
};

struct Model* model_get(int index);
void model_init(void);
int  model_create(int node, const char* geo_name);
void model_remove(int index);
void model_cleanup(void);
void model_render_all(struct Camera* camera);

#endif
