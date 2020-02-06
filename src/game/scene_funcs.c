#include "scene_funcs.h"

#include "../common/log.h"

void scene_init_stub(struct Scene* scene)
{
	log_warning("Scene Init Stub Called");
}

void scene_cleanup_stub(struct Scene* scene)
{
	log_warning("Scene Cleanup Stub Called");
}

void scene_1_init(struct Scene* scene)
{
	log_message("Scene 1 init called");
}

void scene_1_cleanup(struct Scene* scene)
{
	log_message("Scene 1 cleanup called");
}
