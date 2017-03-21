#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <limits.h>

#include "game.h"
#include "platform.h"
#include "input.h"
#include "renderer.h"
#include "log.h"
#include "file_io.h"
#include "shader.h"
#include "entity.h"
#include "geometry.h"
#include "array.h"
#include "transform.h"
#include "camera.h"
#include "model.h"
#include "scene.h"
#include "utils.h"
#include "texture.h"
#include "material.h"
#include "framebuffer.h"
#include "light.h"
#include "gl_load.h"
#include "gui.h"
#include "editor.h"

#define UNUSED(a) (void)a
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define LEN(a) (sizeof(a)/sizeof(a)[0])


static void run(void);
static void update(float dt, int* window_should_close);
static void render(void);
static void debug(float dt);
static void debug_gui(float dt);
static void scene_setup(void);

static struct Game_State* game_state = NULL;

void game_init(struct Window* window)
{
	/* TODO: Implement dealing with init failures */
	game_state = malloc(sizeof(*game_state));
	game_state->window = window;
								  
	/* Init systems */
	input_init();
	char* base_path = platform_get_base_path();
	io_file_init(base_path);
	free(base_path);
	shader_init();
	texture_init();
	framebuffer_init();
	geom_init();
	renderer_init();
	transform_init();
	light_init();
	camera_init();
	material_init();
	editor_init();
	model_init();
	entity_init();
	scene_init();
	
	/* Debug scene setup */
	scene_setup();

	run();
}

void scene_setup(void)
{
	int forward_keys[2]    = {KEY_W, KEY_UP};
	int backward_keys[2]   = {KEY_S, KEY_DOWN};
	int up_keys[2]         = {KEY_Q};
	int down_keys[2]       = {KEY_E};
	int left_keys[2]       = {KEY_A, KEY_LEFT};
	int right_keys[2]      = {KEY_D, KEY_RIGHT};
	int turn_right_keys[1] = {KEY_L};
	int turn_left_keys[1]  = {KEY_J};
	int turn_up_keys[1]    = {KEY_I};
	int turn_down_keys[1]  = {KEY_K};
	int sprint_keys[2]     = {KEY_LSHIFT, KEY_RSHIFT};
	input_map_create("Move_Forward",  forward_keys,    2);
	input_map_create("Move_Backward", backward_keys,   2);
	input_map_create("Move_Up",       up_keys,         1);
	input_map_create("Move_Down",     down_keys,       1);
	input_map_create("Move_Left",     left_keys,       2);
	input_map_create("Move_Right",    right_keys,      2);
	input_map_create("Turn_Right",    turn_right_keys, 1);
	input_map_create("Turn_Left",     turn_left_keys,  1);
	input_map_create("Turn_Up",       turn_up_keys,    1);
	input_map_create("Turn_Down",     turn_down_keys,  1);
	input_map_create("Sprint",        sprint_keys,     2);
	
	struct Entity* player = scene_add_new("player", "None");
	game_state->player_node = player->node;
	vec3 viewer_pos = {10, 5, 100};
	struct Transform* viewer_tran = entity_component_get(player, C_TRANSFORM);
	/* struct Model* player_model = entity_component_add(player, C_MODEL, "sphere.pamesh", NULL); */
	/* model_set_material_param(player_model, "diffuse_color", &color); */
	/* vec4_fill(&color, 1.f, 1.f, 1.f, 1.f); */
	transform_set_position(viewer_tran, &viewer_pos);
	int render_width, render_height;
	render_width = 1024;
	render_height = 768;
	struct Camera* camera = entity_component_add(player, C_CAMERA, render_width, render_height);
	camera_attach_fbo(camera, render_width, render_height, 1, 1, 1);
	vec4_fill(&camera->clear_color, 0.3f, 0.6f, 0.9f, 1.0f);
	camera_set_primary_viewer(camera);

	vec4 color = {0.f, 1.f, 1.f, 1.f };
	struct Entity* new_ent = scene_add_new("Model_Entity", NULL);
	struct Transform* tran = entity_component_get(new_ent, C_TRANSFORM);
	vec3 position = {0, 0, -5};
	transform_translate(tran, &position, TS_WORLD);
	struct Model* box_model = entity_component_add(new_ent, C_MODEL, "torus.pamesh", "Blinn_Phong");
	model_set_material_param(box_model, "diffuse_color", &color);
	int tex = texture_create_from_file("white.tga", TU_DIFFUSE);
	model_set_material_param(box_model, "diffuse_texture", &tex);
	struct Transform* model_tran = entity_component_get(new_ent, C_TRANSFORM);
	vec3 scale = {1, 1, 1};
	transform_scale(model_tran, &scale);

	int parent_node = new_ent->node;
	int num_suz = 100;
	srand(time(NULL));
	for(int i = 0; i < num_suz; i++)
	{
		int x = rand() % num_suz;
		int y = rand() % num_suz;
		int z = rand() % num_suz;
		x++; y++; z++;
		struct Entity* suz = scene_add_as_child("Suzanne", NULL, parent_node);
		struct Model* suz_model = entity_component_add(suz, C_MODEL, "suzanne.pamesh", "Blinn_Phong");
		model_set_material_param(suz_model, "diffuse_color", &color);
		float spec_str = 80.f;
		model_set_material_param(suz_model, "specular_strength", &spec_str);
		struct Transform* s_tran = entity_component_get(suz, C_TRANSFORM);
		vec3 s_pos = {x, 5, z};
		transform_translate(s_tran, &s_pos, TS_WORLD);
	}
	

	struct Entity* ground = scene_add_new("Ground", NULL);
	struct Model* ground_model = entity_component_add(ground, C_MODEL, "plane.pamesh", "Blinn_Phong");
	model_set_material_param(ground_model, "diffuse_color", &color);
	int white_tex = texture_create_from_file("white.tga", TU_DIFFUSE);
	model_set_material_param(ground_model, "diffuse_texture", &white_tex);
	float spec_str = 80.f;
	model_set_material_param(ground_model, "specular_strength", &spec_str);
	struct Transform* ground_tran = entity_component_get(ground, C_TRANSFORM);
	vec3 pos = {0, -15, 0};
	vec3 scale_ground = {200.f, 200.f, 200.f};
	transform_set_position(ground_tran, &pos);
	transform_scale(ground_tran, &scale_ground);

	/* struct Entity* screen = scene_add_new("Screen", NULL); */
	/* struct Model* screen_model = entity_component_add(screen, C_MODEL, NULL, NULL); */
	/* screen_model->geometry_index = geom_find("Quad"); */
	/* struct Entity* screen_camera = scene_add_as_child("Screen_Camera", NULL, screen->node); */
	/* struct Transform* screen_camera_tran = entity_component_get(screen_camera, C_TRANSFORM); */
	/* transform_rotate(screen_camera_tran, &UNIT_Y, 180.f, TS_WORLD); */
	/* struct Camera* cam = entity_component_add(screen_camera, C_CAMERA, 50, 50); */
	/* cam->nearz = 0.1f; */
	/* cam->farz  = 50.f; */
	/* camera_update_proj(cam); */
	/* camera_attach_fbo(cam, 128, 128, 1, 1, 0); */
	/* model_set_material_param(screen_model, "diffuse_color", &color); */
	/* model_set_material_param(screen_model, "diffuse_texture", &cam->render_tex); */

	const int MAX_LIGHTS = 3;
	for(int i = 0; i < MAX_LIGHTS; i++)
	{
		int x = rand() % MAX_LIGHTS;
		int z = rand() % MAX_LIGHTS;
		x++; z++;
		struct Entity* light_ent = scene_add_new("Light_Ent", NULL);
		struct Transform* light_tran = entity_component_get(light_ent, C_TRANSFORM);
		vec3 lt_pos = {x * 20, 0, z * 20};
		transform_set_position(light_tran, &lt_pos);
		struct Light* light_comp = entity_component_add(light_ent, C_LIGHT, LT_POINT);
		vec3_fill(&light_comp->color, 1.f / (float)x, 1.f / ((rand() % 10) + 1.f), 1.f / (float)z);
		light_comp->intensity = 1.f;
	}

	/* struct Entity* sun = scene_add_new("Sun", NULL); */
	/* struct Light* sun_light = entity_component_add(sun, C_LIGHT, LT_DIR); */
	/* sun_light->intensity = 0.8f; */
}

void debug(float dt)
{
	//struct Entity* entity = entity_get(player_node);
	struct Entity* entity = !input_key_state_get('C', KS_PRESSED) ? entity_get(game_state->player_node) : scene_find("Screen_Camera");
	struct Camera* cam = entity_component_get(entity, C_CAMERA);
	camera_set_primary_viewer(cam);
	struct Transform* transform = entity_component_get(entity, C_TRANSFORM);
	float move_speed = 5.f, move_scale = 3.f, turn_speed = 50.f;
	vec3 offset = {0, 0, 0};
	float turn_up_down = 0.f;
	float turn_left_right = 0.f;
	float max_up_down = 60.f;
	static float total_up_down_rot = 0.f;
	vec3 rot_axis_up_down = {1, 0, 0};
	vec3 rot_axis_left_right = {0, 1, 0};

	/* Look around */
	if(input_map_state_get("Turn_Up",    KS_PRESSED)) turn_up_down += turn_speed;
	if(input_map_state_get("Turn_Down",  KS_PRESSED)) turn_up_down -= turn_speed;
	if(input_map_state_get("Turn_Right", KS_PRESSED)) turn_left_right += turn_speed;
	if(input_map_state_get("Turn_Left",  KS_PRESSED)) turn_left_right -= turn_speed;

	/* if(input_key_state_get(KEY_TAB, KS_PRESSED))  */
	/* if(input_key_state_get(KEY_TAB, KS_PRESSED) && input_key_state_get(KEY_LSHIFT, KS_PRESSED)) input_mouse_mode_set(MM_NORMAL); */
	
	if(input_mousebutton_state_get(MB_RIGHT, KS_PRESSED))
	{
		if(input_mouse_mode_get() != MM_RELATIVE) input_mouse_mode_set(MM_RELATIVE);
		const double scale = 0.25;
		int cursor_lr, cursor_ud;
		input_mouse_delta_get(&cursor_lr, &cursor_ud);
		turn_up_down = -cursor_ud * turn_speed * dt * scale;
		turn_left_right = cursor_lr * turn_speed * dt * scale;
		input_mouse_pos_set(0.0, 0.0);
	}
	else
	{
		input_mouse_mode_set(MM_NORMAL);
		turn_up_down *= dt;
		turn_left_right *= dt;
	}
	
	total_up_down_rot += turn_up_down;
	if(total_up_down_rot >= max_up_down)
	{
		total_up_down_rot = max_up_down;
		turn_up_down = 0.f;
	}
	else if(total_up_down_rot <= -max_up_down)
	{
		total_up_down_rot = -max_up_down;
		turn_up_down = 0.f;
	}
	
	if(turn_left_right != 0.f)
	{
		transform_rotate(transform, &rot_axis_left_right, -turn_left_right, TS_WORLD);
		vec3 up = {0.f, 0.f, 0.f};
		vec3 forward = {0.f, 0.f, 0.f};
		vec3 lookat = {0.f, 0.f, 0.f};
		transform_get_up(transform, &up);
		transform_get_forward(transform, &forward);
		transform_get_lookat(transform, &lookat);
		/* log_message("Up : %s", tostr_vec3(&up)); */
		/* log_message("FR : %s", tostr_vec3(&forward)); */
	}
	if(turn_up_down != 0.f)
	{
		transform_rotate(transform, &rot_axis_up_down, turn_up_down, TS_LOCAL);
		vec3 up = {0.f, 0.f, 0.f};
		vec3 forward = {0.f, 0.f, 0.f};
		vec3 lookat = {0.f, 0.f, 0.f};
		transform_get_up(transform, &up);
		transform_get_forward(transform, &forward);
		transform_get_lookat(transform, &lookat);
		/* log_message("Up : %s", tostr_vec3(&up)); */
		/* log_message("FR : %s", tostr_vec3(&forward)); */
	}
	
	/* Movement */
	if(input_map_state_get("Sprint",        KS_PRESSED)) move_speed *= move_scale;
	if(input_map_state_get("Move_Forward",  KS_PRESSED)) offset.z   -= move_speed;
	if(input_map_state_get("Move_Backward", KS_PRESSED)) offset.z   += move_speed;
	if(input_map_state_get("Move_Left",     KS_PRESSED)) offset.x   -= move_speed;
	if(input_map_state_get("Move_Right",    KS_PRESSED)) offset.x   += move_speed;
	if(input_map_state_get("Move_Up",       KS_PRESSED)) offset.y   += move_speed;
	if(input_map_state_get("Move_Down",     KS_PRESSED)) offset.y   -= move_speed;

	vec3_scale(&offset, &offset, dt);
	if(offset.x != 0 || offset.y != 0 || offset.z != 0)
	{
		transform_translate(transform, &offset, TS_LOCAL);
 		//log_message("Position : %s", tostr_vec3(&transform->position));
	}

	if(input_key_state_get(KEY_SPACE, KS_PRESSED))
	{
		struct Entity* model = scene_find("Model_Entity");
		struct Transform* mod_tran = entity_component_get(model, C_TRANSFORM);
		vec3 x_axis = {1, 0, 0};
		transform_rotate(mod_tran, &x_axis, 25.f * dt, TS_WORLD);
	}

	if(input_key_state_get(KEY_M, KS_PRESSED))
	{
		struct Entity* model = scene_find("Model_Entity");
		struct Transform* mod_tran = entity_component_get(model, C_TRANSFORM);
		//vec3 y_axis = {0, 0, 1};
		//transform_rotate(mod_tran, &y_axis, 25.f * dt, TS_LOCAL);
		vec3 amount = {0, 0, -5 * dt};
		transform_translate(mod_tran, &amount, TS_LOCAL);
	}

	if(input_key_state_get(KEY_N, KS_PRESSED))
	{
		struct Entity* model = scene_find("Model_Entity");
		struct Transform* mod_tran = entity_component_get(model, C_TRANSFORM);
		/* vec3 y_axis = {0, 0, 1}; */
		/* transform_rotate(mod_tran, &y_axis, 25.f * dt, TS_WORLD); */
		vec3 amount = {0, 0, 5 * dt};
		transform_translate(mod_tran, &amount, TS_LOCAL);
	}	

	/* if(input_key_state_get(GLFW_KEY_C, KS_PRESSED)) */
	/* { */
	/* 	struct Entity* cam_ent = scene_find("Screen_Camera"); */
	/* 	struct Camera* cam = entity_component_get(cam_ent, C_CAMERA); */
	/* 	camera_set_primary_viewer(cam); */
	/* } */

	/* if(input_key_state_get(GLFW_KEY_V, KS_PRESSED)) */
	/* { */
	/* 	struct Camera* cam = entity_component_get(entity, C_CAMERA); */
	/* 	camera_set_priimary_viewer(cam); */
	/* } */
}

void run(void)
{
	uint32 last_time = platform_get_ticks();
	int should_window_close = 0;
	while(!should_window_close)
	{
		uint32 curr_time = platform_get_ticks();
		float delta_time = (float)(curr_time - last_time) / 1000.f;
		last_time = curr_time;
		
		update(delta_time, &should_window_close);
		render();
		window_swap_buffers(game_state->window);
		gui_input_begin();
		platform_poll_events(&should_window_close);
		gui_input_end();
	}
}

void update(float dt, int* window_should_close)
{
	input_update();
	
	if(input_key_state_get(KEY_ESCAPE, KS_PRESSED)) *window_should_close = 1;
	if(input_key_state_get(KEY_F1, KS_PRESSED))		editor_toggle();
	
	debug(dt);
	//debug_gui(dt);
	editor_update(dt);
}

void debug_gui(float dt)
{
	struct Gui_State* gui_state = gui_state_get();
	struct nk_context* ctx = &gui_state->context;
	
    /* window flags */
    static int show_menu = nk_true;
    static int titlebar = nk_true;
    static int border = nk_true;
    static int resize = nk_true;
    static int movable = nk_true;
    static int no_scrollbar = nk_false;
    static int scale_left = nk_false;
    static nk_flags window_flags = 0;
    static int minimizable = nk_true;

    /* popups */
    static enum nk_style_header_align header_align = NK_HEADER_RIGHT;
    static int show_app_about = nk_false;

    /* window flags */
    window_flags = 0;
    ctx->style.window.header.align = header_align;
    if (border) window_flags |= NK_WINDOW_BORDER;
    if (resize) window_flags |= NK_WINDOW_SCALABLE;
    if (movable) window_flags |= NK_WINDOW_MOVABLE;
    if (no_scrollbar) window_flags |= NK_WINDOW_NO_SCROLLBAR;
    if (scale_left) window_flags |= NK_WINDOW_SCALE_LEFT;
    if (minimizable) window_flags |= NK_WINDOW_MINIMIZABLE;

    if (nk_begin(ctx, "Overview", nk_rect(70, 70, 400, 600), window_flags))
    {
        if (show_menu)
        {
            /* menubar */
            enum menu_states {MENU_DEFAULT, MENU_WINDOWS};
            static nk_size mprog = 60;
            static int mslider = 10;
            static int mcheck = nk_true;

            nk_menubar_begin(ctx);
            nk_layout_row_begin(ctx, NK_STATIC, 25, 4);
            nk_layout_row_push(ctx, 45);
            if (nk_menu_begin_label(ctx, "MENU", NK_TEXT_LEFT, nk_vec2(120, 200)))
            {
                static size_t prog = 40;
                static int slider = 10;
                static int check = nk_true;
                nk_layout_row_dynamic(ctx, 25, 1);
                if (nk_menu_item_label(ctx, "Hide", NK_TEXT_LEFT))
                    show_menu = nk_false;
                if (nk_menu_item_label(ctx, "About", NK_TEXT_LEFT))
                    show_app_about = nk_true;
                nk_progress(ctx, &prog, 100, NK_MODIFIABLE);
                nk_slider_int(ctx, 0, &slider, 16, 1);
                nk_checkbox_label(ctx, "check", &check);
                nk_menu_end(ctx);
            }
            nk_layout_row_push(ctx, 70);
            nk_progress(ctx, &mprog, 100, NK_MODIFIABLE);
            nk_slider_int(ctx, 0, &mslider, 16, 1);
            nk_checkbox_label(ctx, "check", &mcheck);
            nk_menubar_end(ctx);
        }

        if (show_app_about)
        {
            /* about popup */
            static struct nk_rect s = {20, 100, 300, 190};
            if (nk_popup_begin(ctx, NK_POPUP_STATIC, "About", NK_WINDOW_CLOSABLE, s))
            {
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_label(ctx, "Nuklear", NK_TEXT_LEFT);
                nk_label(ctx, "By Micha Mettke", NK_TEXT_LEFT);
                nk_label(ctx, "nuklear is licensed under the public domain License.",  NK_TEXT_LEFT);
                nk_popup_end(ctx);
            } else show_app_about = nk_false;
        }

        /* window flags */
        if (nk_tree_push(ctx, NK_TREE_TAB, "Window", NK_MINIMIZED)) {
            nk_layout_row_dynamic(ctx, 30, 2);
            nk_checkbox_label(ctx, "Titlebar", &titlebar);
            nk_checkbox_label(ctx, "Menu", &show_menu);
            nk_checkbox_label(ctx, "Border", &border);
            nk_checkbox_label(ctx, "Resizable", &resize);
            nk_checkbox_label(ctx, "Movable", &movable);
            nk_checkbox_label(ctx, "No Scrollbar", &no_scrollbar);
            nk_checkbox_label(ctx, "Minimizable", &minimizable);
            nk_checkbox_label(ctx, "Scale Left", &scale_left);
            nk_tree_pop(ctx);
        }

        if (nk_tree_push(ctx, NK_TREE_TAB, "Widgets", NK_MINIMIZED))
        {
            enum options {A,B,C};
            static int checkbox;
            static int option;
            if (nk_tree_push(ctx, NK_TREE_NODE, "Text", NK_MINIMIZED))
            {
                /* Text Widgets */
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_label(ctx, "Label aligned left", NK_TEXT_LEFT);
                nk_label(ctx, "Label aligned centered", NK_TEXT_CENTERED);
                nk_label(ctx, "Label aligned right", NK_TEXT_RIGHT);
                nk_label_colored(ctx, "Blue text", NK_TEXT_LEFT, nk_rgb(0,0,255));
                nk_label_colored(ctx, "Yellow text", NK_TEXT_LEFT, nk_rgb(255,255,0));
                nk_text(ctx, "Text without /0", 15, NK_TEXT_RIGHT);

                nk_layout_row_static(ctx, 100, 200, 1);
                nk_label_wrap(ctx, "This is a very long line to hopefully get this text to be wrapped into multiple lines to show line wrapping");
                nk_layout_row_dynamic(ctx, 100, 1);
                nk_label_wrap(ctx, "This is another long text to show dynamic window changes on multiline text");
                nk_tree_pop(ctx);
            }

            if (nk_tree_push(ctx, NK_TREE_NODE, "Button", NK_MINIMIZED))
            {
                /* Buttons Widgets */
                nk_layout_row_static(ctx, 30, 100, 3);
                if (nk_button_label(ctx, "Button"))
                    fprintf(stdout, "Button pressed!\n");
                nk_button_set_behavior(ctx, NK_BUTTON_REPEATER);
                if (nk_button_label(ctx, "Repeater"))
                    fprintf(stdout, "Repeater is being pressed!\n");
                nk_button_set_behavior(ctx, NK_BUTTON_DEFAULT);
                nk_button_color(ctx, nk_rgb(0,0,255));

                nk_layout_row_static(ctx, 25, 25, 8);
                nk_button_symbol(ctx, NK_SYMBOL_CIRCLE_SOLID);
                nk_button_symbol(ctx, NK_SYMBOL_CIRCLE_OUTLINE);
                nk_button_symbol(ctx, NK_SYMBOL_RECT_SOLID);
                nk_button_symbol(ctx, NK_SYMBOL_RECT_OUTLINE);
                nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_UP);
                nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_DOWN);
                nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_LEFT);
                nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_RIGHT);

                nk_layout_row_static(ctx, 30, 100, 2);
                nk_button_symbol_label(ctx, NK_SYMBOL_TRIANGLE_LEFT, "prev", NK_TEXT_RIGHT);
                nk_button_symbol_label(ctx, NK_SYMBOL_TRIANGLE_RIGHT, "next", NK_TEXT_LEFT);
                nk_tree_pop(ctx);
            }

            if (nk_tree_push(ctx, NK_TREE_NODE, "Basic", NK_MINIMIZED))
            {
                /* Basic widgets */
                static int int_slider = 5;
                static float float_slider = 2.5f;
                static size_t prog_value = 40;
                static float property_float = 2;
                static int property_int = 10;
                static int property_neg = 10;

                static float range_float_min = 0;
                static float range_float_max = 100;
                static float range_float_value = 50;
                static int range_int_min = 0;
                static int range_int_value = 2048;
                static int range_int_max = 4096;
                static const float ratio[] = {120, 150};

                nk_layout_row_static(ctx, 30, 100, 1);
                nk_checkbox_label(ctx, "Checkbox", &checkbox);

                nk_layout_row_static(ctx, 30, 80, 3);
                option = nk_option_label(ctx, "optionA", option == A) ? A : option;
                option = nk_option_label(ctx, "optionB", option == B) ? B : option;
                option = nk_option_label(ctx, "optionC", option == C) ? C : option;


                nk_layout_row(ctx, NK_STATIC, 30, 2, ratio);
                nk_labelf(ctx, NK_TEXT_LEFT, "Slider int");
                nk_slider_int(ctx, 0, &int_slider, 10, 1);

                nk_label(ctx, "Slider float", NK_TEXT_LEFT);
                nk_slider_float(ctx, 0, &float_slider, 5.0, 0.5f);
                nk_labelf(ctx, NK_TEXT_LEFT, "Progressbar" , prog_value);
                nk_progress(ctx, &prog_value, 100, NK_MODIFIABLE);

                nk_layout_row(ctx, NK_STATIC, 25, 2, ratio);
                nk_label(ctx, "Property float:", NK_TEXT_LEFT);
                nk_property_float(ctx, "Float:", 0, &property_float, 64.0f, 0.1f, 0.2f);
                nk_label(ctx, "Property int:", NK_TEXT_LEFT);
                nk_property_int(ctx, "Int:", 0, &property_int, 100.0f, 1, 1);
                nk_label(ctx, "Property neg:", NK_TEXT_LEFT);
                nk_property_int(ctx, "Neg:", -10, &property_neg, 10, 1, 1);

                nk_layout_row_dynamic(ctx, 25, 1);
                nk_label(ctx, "Range:", NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 25, 3);
                nk_property_float(ctx, "#min:", 0, &range_float_min, range_float_max, 1.0f, 0.2f);
                nk_property_float(ctx, "#float:", range_float_min, &range_float_value, range_float_max, 1.0f, 0.2f);
                nk_property_float(ctx, "#max:", range_float_min, &range_float_max, 100, 1.0f, 0.2f);

                nk_property_int(ctx, "#min:", INT_MIN, &range_int_min, range_int_max, 1, 10);
                nk_property_int(ctx, "#neg:", range_int_min, &range_int_value, range_int_max, 1, 10);
                nk_property_int(ctx, "#max:", range_int_min, &range_int_max, INT_MAX, 1, 10);

                nk_tree_pop(ctx);
            }

            if (nk_tree_push(ctx, NK_TREE_NODE, "Selectable", NK_MINIMIZED))
            {
                if (nk_tree_push(ctx, NK_TREE_NODE, "List", NK_MINIMIZED))
                {
                    static int selected[4] = {nk_false, nk_false, nk_true, nk_false};
                    nk_layout_row_static(ctx, 18, 100, 1);
                    nk_selectable_label(ctx, "Selectable", NK_TEXT_LEFT, &selected[0]);
                    nk_selectable_label(ctx, "Selectable", NK_TEXT_LEFT, &selected[1]);
                    nk_label(ctx, "Not Selectable", NK_TEXT_LEFT);
                    nk_selectable_label(ctx, "Selectable", NK_TEXT_LEFT, &selected[2]);
                    nk_selectable_label(ctx, "Selectable", NK_TEXT_LEFT, &selected[3]);
                    nk_tree_pop(ctx);
                }
                if (nk_tree_push(ctx, NK_TREE_NODE, "Grid", NK_MINIMIZED))
                {
                    int i;
                    static int selected[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
                    nk_layout_row_static(ctx, 50, 50, 4);
                    for (i = 0; i < 16; ++i) {
                        if (nk_selectable_label(ctx, "Z", NK_TEXT_CENTERED, &selected[i])) {
                            int x = (i % 4), y = i / 4;
                            if (x > 0) selected[i - 1] ^= 1;
                            if (x < 3) selected[i + 1] ^= 1;
                            if (y > 0) selected[i - 4] ^= 1;
                            if (y < 3) selected[i + 4] ^= 1;
                        }
                    }
                    nk_tree_pop(ctx);
                }
                nk_tree_pop(ctx);
            }

            if (nk_tree_push(ctx, NK_TREE_NODE, "Combo", NK_MINIMIZED))
            {
                /* Combobox Widgets
                 * In this library comboboxes are not limited to being a popup
                 * list of selectable text. Instead it is a abstract concept of
                 * having something that is *selected* or displayed, a popup window
                 * which opens if something needs to be modified and the content
                 * of the popup which causes the *selected* or displayed value to
                 * change or if wanted close the combobox.
                 *
                 * While strange at first handling comboboxes in a abstract way
                 * solves the problem of overloaded window content. For example
                 * changing a color value requires 4 value modifier (slider, property,...)
                 * for RGBA then you need a label and ways to display the current color.
                 * If you want to go fancy you even add rgb and hsv ratio boxes.
                 * While fine for one color if you have a lot of them it because
                 * tedious to look at and quite wasteful in space. You could add
                 * a popup which modifies the color but this does not solve the
                 * fact that it still requires a lot of cluttered space to do.
                 *
                 * In these kind of instance abstract comboboxes are quite handy. All
                 * value modifiers are hidden inside the combobox popup and only
                 * the color is shown if not open. This combines the clarity of the
                 * popup with the ease of use of just using the space for modifiers.
                 *
                 * Other instances are for example time and especially date picker,
                 * which only show the currently activated time/data and hide the
                 * selection logic inside the combobox popup.
                 */
                static float chart_selection = 8.0f;
                static int current_weapon = 0;
                static int check_values[5];
                static float position[3];
                static struct nk_color combo_color = {130, 50, 50, 255};
                static struct nk_color combo_color2 = {130, 180, 50, 255};
                static size_t prog_a =  20, prog_b = 40, prog_c = 10, prog_d = 90;
                static const char *weapons[] = {"Fist","Pistol","Shotgun","Plasma","BFG"};

                char buffer[64];
                size_t sum = 0;

                /* default combobox */
                nk_layout_row_static(ctx, 25, 200, 1);
                current_weapon = nk_combo(ctx, weapons, LEN(weapons), current_weapon, 25, nk_vec2(200,200));

                /* slider color combobox */
                if (nk_combo_begin_color(ctx, combo_color, nk_vec2(200,200))) {
                    float ratios[] = {0.15f, 0.85f};
                    nk_layout_row(ctx, NK_DYNAMIC, 30, 2, ratios);
                    nk_label(ctx, "R:", NK_TEXT_LEFT);
                    combo_color.r = (nk_byte)nk_slide_int(ctx, 0, combo_color.r, 255, 5);
                    nk_label(ctx, "G:", NK_TEXT_LEFT);
                    combo_color.g = (nk_byte)nk_slide_int(ctx, 0, combo_color.g, 255, 5);
                    nk_label(ctx, "B:", NK_TEXT_LEFT);
                    combo_color.b = (nk_byte)nk_slide_int(ctx, 0, combo_color.b, 255, 5);
                    nk_label(ctx, "A:", NK_TEXT_LEFT);
                    combo_color.a = (nk_byte)nk_slide_int(ctx, 0, combo_color.a , 255, 5);
                    nk_combo_end(ctx);
                }

                /* complex color combobox */
                if (nk_combo_begin_color(ctx, combo_color2, nk_vec2(200,400))) {
                    enum color_mode {COL_RGB, COL_HSV};
                    static int col_mode = COL_RGB;
#ifndef DEMO_DO_NOT_USE_COLOR_PICKER
                    nk_layout_row_dynamic(ctx, 120, 1);
                    combo_color2 = nk_color_picker(ctx, combo_color2, NK_RGBA);
#endif

                    nk_layout_row_dynamic(ctx, 25, 2);
                    col_mode = nk_option_label(ctx, "RGB", col_mode == COL_RGB) ? COL_RGB : col_mode;
                    col_mode = nk_option_label(ctx, "HSV", col_mode == COL_HSV) ? COL_HSV : col_mode;

                    nk_layout_row_dynamic(ctx, 25, 1);
                    if (col_mode == COL_RGB) {
                        combo_color2.r = (nk_byte)nk_propertyi(ctx, "#R:", 0, combo_color2.r, 255, 1,1);
                        combo_color2.g = (nk_byte)nk_propertyi(ctx, "#G:", 0, combo_color2.g, 255, 1,1);
                        combo_color2.b = (nk_byte)nk_propertyi(ctx, "#B:", 0, combo_color2.b, 255, 1,1);
                        combo_color2.a = (nk_byte)nk_propertyi(ctx, "#A:", 0, combo_color2.a, 255, 1,1);
                    } else {
                        nk_byte tmp[4];
                        nk_color_hsva_bv(tmp, combo_color2);
                        tmp[0] = (nk_byte)nk_propertyi(ctx, "#H:", 0, tmp[0], 255, 1,1);
                        tmp[1] = (nk_byte)nk_propertyi(ctx, "#S:", 0, tmp[1], 255, 1,1);
                        tmp[2] = (nk_byte)nk_propertyi(ctx, "#V:", 0, tmp[2], 255, 1,1);
                        tmp[3] = (nk_byte)nk_propertyi(ctx, "#A:", 0, tmp[3], 255, 1,1);
                        combo_color2 = nk_hsva_bv(tmp);
                    }
                    nk_combo_end(ctx);
                }

                /* progressbar combobox */
                sum = prog_a + prog_b + prog_c + prog_d;
                sprintf(buffer, "%lu", sum);
                if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
                    nk_layout_row_dynamic(ctx, 30, 1);
                    nk_progress(ctx, &prog_a, 100, NK_MODIFIABLE);
                    nk_progress(ctx, &prog_b, 100, NK_MODIFIABLE);
                    nk_progress(ctx, &prog_c, 100, NK_MODIFIABLE);
                    nk_progress(ctx, &prog_d, 100, NK_MODIFIABLE);
                    nk_combo_end(ctx);
                }

                /* checkbox combobox */
                sum = (size_t)(check_values[0] + check_values[1] + check_values[2] + check_values[3] + check_values[4]);
                sprintf(buffer, "%lu", sum);
                if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
                    nk_layout_row_dynamic(ctx, 30, 1);
                    nk_checkbox_label(ctx, weapons[0], &check_values[0]);
                    nk_checkbox_label(ctx, weapons[1], &check_values[1]);
                    nk_checkbox_label(ctx, weapons[2], &check_values[2]);
                    nk_checkbox_label(ctx, weapons[3], &check_values[3]);
                    nk_combo_end(ctx);
                }

                /* complex text combobox */
                sprintf(buffer, "%.2f, %.2f, %.2f", position[0], position[1],position[2]);
                if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
                    nk_layout_row_dynamic(ctx, 25, 1);
                    nk_property_float(ctx, "#X:", -1024.0f, &position[0], 1024.0f, 1,0.5f);
                    nk_property_float(ctx, "#Y:", -1024.0f, &position[1], 1024.0f, 1,0.5f);
                    nk_property_float(ctx, "#Z:", -1024.0f, &position[2], 1024.0f, 1,0.5f);
                    nk_combo_end(ctx);
                }

                /* chart combobox */
                sprintf(buffer, "%.1f", chart_selection);
                if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,250))) {
                    size_t i = 0;
                    static const float values[]={26.0f,13.0f,30.0f,15.0f,25.0f,10.0f,20.0f,40.0f, 12.0f, 8.0f, 22.0f, 28.0f, 5.0f};
                    nk_layout_row_dynamic(ctx, 150, 1);
                    nk_chart_begin(ctx, NK_CHART_COLUMN, LEN(values), 0, 50);
                    for (i = 0; i < LEN(values); ++i) {
                        nk_flags res = nk_chart_push(ctx, values[i]);
                        if (res & NK_CHART_CLICKED) {
                            chart_selection = values[i];
                            nk_combo_close(ctx);
                        }
                    }
                    nk_chart_end(ctx);
                    nk_combo_end(ctx);
                }

                {
                    static int time_selected = 0;
                    static int date_selected = 0;
                    static struct tm sel_time;
                    static struct tm sel_date;
                    if (!time_selected || !date_selected) {
                        /* keep time and date updated if nothing is selected */
                        time_t cur_time = time(0);
                        struct tm *n = localtime(&cur_time);
                        if (!time_selected)
                            memcpy(&sel_time, n, sizeof(struct tm));
                        if (!date_selected)
                            memcpy(&sel_date, n, sizeof(struct tm));
                    }

                    /* time combobox */
                    sprintf(buffer, "%02d:%02d:%02d", sel_time.tm_hour, sel_time.tm_min, sel_time.tm_sec);
                    if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,250))) {
                        time_selected = 1;
                        nk_layout_row_dynamic(ctx, 25, 1);
                        sel_time.tm_sec = nk_propertyi(ctx, "#S:", 0, sel_time.tm_sec, 60, 1, 1);
                        sel_time.tm_min = nk_propertyi(ctx, "#M:", 0, sel_time.tm_min, 60, 1, 1);
                        sel_time.tm_hour = nk_propertyi(ctx, "#H:", 0, sel_time.tm_hour, 23, 1, 1);
                        nk_combo_end(ctx);
                    }

                    /* date combobox */
                    sprintf(buffer, "%02d-%02d-%02d", sel_date.tm_mday, sel_date.tm_mon+1, sel_date.tm_year+1900);
                    if (nk_combo_begin_label(ctx, buffer, nk_vec2(350,400)))
                    {
                        int i = 0;
                        const char *month[] = {"January", "February", "March", "Apil", "May", "June", "July", "August", "September", "Ocotober", "November", "December"};
                        const char *week_days[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
                        const int month_days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
                        int year = sel_date.tm_year+1900;
                        int leap_year = (!(year % 4) && ((year % 100))) || !(year % 400);
                        int days = (sel_date.tm_mon == 1) ?
                            month_days[sel_date.tm_mon] + leap_year:
                            month_days[sel_date.tm_mon];

                        /* header with month and year */
                        date_selected = 1;
                        nk_layout_row_begin(ctx, NK_DYNAMIC, 20, 3);
                        nk_layout_row_push(ctx, 0.05f);
                        if (nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_LEFT)) {
                            if (sel_date.tm_mon == 0) {
                                sel_date.tm_mon = 11;
                                sel_date.tm_year = MAX(0, sel_date.tm_year-1);
                            } else sel_date.tm_mon--;
                        }
                        nk_layout_row_push(ctx, 0.9f);
                        sprintf(buffer, "%s %d", month[sel_date.tm_mon], year);
                        nk_label(ctx, buffer, NK_TEXT_CENTERED);
                        nk_layout_row_push(ctx, 0.05f);
                        if (nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_RIGHT)) {
                            if (sel_date.tm_mon == 11) {
                                sel_date.tm_mon = 0;
                                sel_date.tm_year++;
                            } else sel_date.tm_mon++;
                        }
                        nk_layout_row_end(ctx);

                        /* good old week day formula (double because precision) */
                        {int year_n = (sel_date.tm_mon < 2) ? year-1: year;
							int y = year_n % 100;
							int c = year_n / 100;
							int y4 = (int)((float)y / 4);
							int c4 = (int)((float)c / 4);
							int m = (int)(2.6 * (double)(((sel_date.tm_mon + 10) % 12) + 1) - 0.2);
							int week_day = (((1 + m + y + y4 + c4 - 2 * c) % 7) + 7) % 7;

							/* weekdays  */
							nk_layout_row_dynamic(ctx, 35, 7);
							for (i = 0; i < (int)LEN(week_days); ++i)
								nk_label(ctx, week_days[i], NK_TEXT_CENTERED);

							/* days  */
							if (week_day > 0) nk_spacing(ctx, week_day);
							for (i = 1; i <= days; ++i) {
								sprintf(buffer, "%d", i);
								if (nk_button_label(ctx, buffer)) {
									sel_date.tm_mday = i;
									nk_combo_close(ctx);
								}
							}}
                        nk_combo_end(ctx);
                    }
                }

                nk_tree_pop(ctx);
            }

            if (nk_tree_push(ctx, NK_TREE_NODE, "Input", NK_MINIMIZED))
            {
                static const float ratio[] = {120, 150};
                static char field_buffer[64];
                static char text[9][64];
                static int text_len[9];
                static char box_buffer[512];
                static int field_len;
                static int box_len;
                nk_flags active;

                nk_layout_row(ctx, NK_STATIC, 25, 2, ratio);
                nk_label(ctx, "Default:", NK_TEXT_LEFT);

                nk_edit_string(ctx, NK_EDIT_SIMPLE, text[0], &text_len[0], 64, nk_filter_default);
                nk_label(ctx, "Int:", NK_TEXT_LEFT);
                nk_edit_string(ctx, NK_EDIT_SIMPLE, text[1], &text_len[1], 64, nk_filter_decimal);
                nk_label(ctx, "Float:", NK_TEXT_LEFT);
                nk_edit_string(ctx, NK_EDIT_SIMPLE, text[2], &text_len[2], 64, nk_filter_float);
                nk_label(ctx, "Hex:", NK_TEXT_LEFT);
                nk_edit_string(ctx, NK_EDIT_SIMPLE, text[4], &text_len[4], 64, nk_filter_hex);
                nk_label(ctx, "Octal:", NK_TEXT_LEFT);
                nk_edit_string(ctx, NK_EDIT_SIMPLE, text[5], &text_len[5], 64, nk_filter_oct);
                nk_label(ctx, "Binary:", NK_TEXT_LEFT);
                nk_edit_string(ctx, NK_EDIT_SIMPLE, text[6], &text_len[6], 64, nk_filter_binary);

                nk_label(ctx, "Password:", NK_TEXT_LEFT);
                {
                    int i = 0;
                    int old_len = text_len[8];
                    char buffer[64];
                    for (i = 0; i < text_len[8]; ++i) buffer[i] = '*';
                    nk_edit_string(ctx, NK_EDIT_FIELD, buffer, &text_len[8], 64, nk_filter_default);
                    if (old_len < text_len[8])
                        memcpy(&text[8][old_len], &buffer[old_len], (nk_size)(text_len[8] - old_len));
                }

                nk_label(ctx, "Field:", NK_TEXT_LEFT);
                nk_edit_string(ctx, NK_EDIT_FIELD, field_buffer, &field_len, 64, nk_filter_default);

                nk_label(ctx, "Box:", NK_TEXT_LEFT);
                nk_layout_row_static(ctx, 180, 278, 1);
                nk_edit_string(ctx, NK_EDIT_BOX, box_buffer, &box_len, 512, nk_filter_default);

                nk_layout_row(ctx, NK_STATIC, 25, 2, ratio);
                active = nk_edit_string(ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER, text[7], &text_len[7], 64,  nk_filter_ascii);
                if (nk_button_label(ctx, "Submit") ||
                    (active & NK_EDIT_COMMITED))
                {
                    text[7][text_len[7]] = '\n';
                    text_len[7]++;
                    memcpy(&box_buffer[box_len], &text[7], (nk_size)text_len[7]);
                    box_len += text_len[7];
                    text_len[7] = 0;
                }
                nk_tree_pop(ctx);
            }
            nk_tree_pop(ctx);
        }

        if (nk_tree_push(ctx, NK_TREE_TAB, "Chart", NK_MINIMIZED))
        {
            /* Chart Widgets
             * This library has two different rather simple charts. The line and the
             * column chart. Both provide a simple way of visualizing values and
             * have a retained mode and immediate mode API version. For the retain
             * mode version `nk_plot` and `nk_plot_function` you either provide
             * an array or a callback to call to handle drawing the graph.
             * For the immediate mode version you start by calling `nk_chart_begin`
             * and need to provide min and max values for scaling on the Y-axis.
             * and then call `nk_chart_push` to push values into the chart.
             * Finally `nk_chart_end` needs to be called to end the process. */
            float id = 0;
            static int col_index = -1;
            static int line_index = -1;
            float step = (2*3.141592654f) / 32;

            int i;
            int index = -1;
            struct nk_rect bounds;

            /* line chart */
            id = 0;
            index = -1;
            nk_layout_row_dynamic(ctx, 100, 1);
            bounds = nk_widget_bounds(ctx);
            if (nk_chart_begin(ctx, NK_CHART_LINES, 32, -1.0f, 1.0f)) {
                for (i = 0; i < 32; ++i) {
                    nk_flags res = nk_chart_push(ctx, (float)cos(id));
                    if (res & NK_CHART_HOVERING)
                        index = (int)i;
                    if (res & NK_CHART_CLICKED)
                        line_index = (int)i;
                    id += step;
                }
                nk_chart_end(ctx);
            }

            if (index != -1) {
                char buffer[NK_MAX_NUMBER_BUFFER];
                float val = (float)cos((float)index*step);
                sprintf(buffer, "Value: %.2f", val);
                nk_tooltip(ctx, buffer);
            }
            if (line_index != -1) {
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_labelf(ctx, NK_TEXT_LEFT, "Selected value: %.2f", (float)cos((float)index*step));
            }

            /* column chart */
            nk_layout_row_dynamic(ctx, 100, 1);
            bounds = nk_widget_bounds(ctx);
            if (nk_chart_begin(ctx, NK_CHART_COLUMN, 32, 0.0f, 1.0f)) {
                for (i = 0; i < 32; ++i) {
                    nk_flags res = nk_chart_push(ctx, (float)fabs(sin(id)));
                    if (res & NK_CHART_HOVERING)
                        index = (int)i;
                    if (res & NK_CHART_CLICKED)
                        col_index = (int)i;
                    id += step;
                }
                nk_chart_end(ctx);
            }
            if (index != -1) {
                char buffer[NK_MAX_NUMBER_BUFFER];
                sprintf(buffer, "Value: %.2f", (float)fabs(sin(step * (float)index)));
                nk_tooltip(ctx, buffer);
            }
            if (col_index != -1) {
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_labelf(ctx, NK_TEXT_LEFT, "Selected value: %.2f", (float)fabs(sin(step * (float)col_index)));
            }

            /* mixed chart */
            nk_layout_row_dynamic(ctx, 100, 1);
            bounds = nk_widget_bounds(ctx);
            if (nk_chart_begin(ctx, NK_CHART_COLUMN, 32, 0.0f, 1.0f)) {
                nk_chart_add_slot(ctx, NK_CHART_LINES, 32, -1.0f, 1.0f);
                nk_chart_add_slot(ctx, NK_CHART_LINES, 32, -1.0f, 1.0f);
                for (id = 0, i = 0; i < 32; ++i) {
                    nk_chart_push_slot(ctx, (float)fabs(sin(id)), 0);
                    nk_chart_push_slot(ctx, (float)cos(id), 1);
                    nk_chart_push_slot(ctx, (float)sin(id), 2);
                    id += step;
                }
            }
            nk_chart_end(ctx);

            /* mixed colored chart */
            nk_layout_row_dynamic(ctx, 100, 1);
            bounds = nk_widget_bounds(ctx);
            if (nk_chart_begin_colored(ctx, NK_CHART_LINES, nk_rgb(255,0,0), nk_rgb(150,0,0), 32, 0.0f, 1.0f)) {
                nk_chart_add_slot_colored(ctx, NK_CHART_LINES, nk_rgb(0,0,255), nk_rgb(0,0,150),32, -1.0f, 1.0f);
                nk_chart_add_slot_colored(ctx, NK_CHART_LINES, nk_rgb(0,255,0), nk_rgb(0,150,0), 32, -1.0f, 1.0f);
                for (id = 0, i = 0; i < 32; ++i) {
                    nk_chart_push_slot(ctx, (float)fabs(sin(id)), 0);
                    nk_chart_push_slot(ctx, (float)cos(id), 1);
                    nk_chart_push_slot(ctx, (float)sin(id), 2);
                    id += step;
                }
            }
            nk_chart_end(ctx);
            nk_tree_pop(ctx);
        }

        if (nk_tree_push(ctx, NK_TREE_TAB, "Popup", NK_MINIMIZED))
        {
            static struct nk_color color = {255,0,0, 255};
            static int select[4];
            static int popup_active;
            const struct nk_input *in = &ctx->input;
            struct nk_rect bounds;

            /* menu contextual */
            nk_layout_row_static(ctx, 30, 150, 1);
            bounds = nk_widget_bounds(ctx);
            nk_label(ctx, "Right click me for menu", NK_TEXT_LEFT);

            if (nk_contextual_begin(ctx, 0, nk_vec2(100, 300), bounds)) {
                static size_t prog = 40;
                static int slider = 10;

                nk_layout_row_dynamic(ctx, 25, 1);
                nk_checkbox_label(ctx, "Menu", &show_menu);
                nk_progress(ctx, &prog, 100, NK_MODIFIABLE);
                nk_slider_int(ctx, 0, &slider, 16, 1);
                if (nk_contextual_item_label(ctx, "About", NK_TEXT_CENTERED))
                    show_app_about = nk_true;
                nk_selectable_label(ctx, select[0]?"Unselect":"Select", NK_TEXT_LEFT, &select[0]);
                nk_selectable_label(ctx, select[1]?"Unselect":"Select", NK_TEXT_LEFT, &select[1]);
                nk_selectable_label(ctx, select[2]?"Unselect":"Select", NK_TEXT_LEFT, &select[2]);
                nk_selectable_label(ctx, select[3]?"Unselect":"Select", NK_TEXT_LEFT, &select[3]);
                nk_contextual_end(ctx);
            }

            /* color contextual */
            nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
            nk_layout_row_push(ctx, 100);
            nk_label(ctx, "Right Click here:", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, 50);
            bounds = nk_widget_bounds(ctx);
            nk_button_color(ctx, color);
            nk_layout_row_end(ctx);

            if (nk_contextual_begin(ctx, 0, nk_vec2(350, 60), bounds)) {
                nk_layout_row_dynamic(ctx, 30, 4);
                color.r = (nk_byte)nk_propertyi(ctx, "#r", 0, color.r, 255, 1, 1);
                color.g = (nk_byte)nk_propertyi(ctx, "#g", 0, color.g, 255, 1, 1);
                color.b = (nk_byte)nk_propertyi(ctx, "#b", 0, color.b, 255, 1, 1);
                color.a = (nk_byte)nk_propertyi(ctx, "#a", 0, color.a, 255, 1, 1);
                nk_contextual_end(ctx);
            }

            /* popup */
            nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
            nk_layout_row_push(ctx, 100);
            nk_label(ctx, "Popup:", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, 50);
            if (nk_button_label(ctx, "Popup"))
                popup_active = 1;
            nk_layout_row_end(ctx);

            if (popup_active)
            {
                static struct nk_rect s = {20, 100, 220, 90};
                if (nk_popup_begin(ctx, NK_POPUP_STATIC, "Error", 0, s))
                {
                    nk_layout_row_dynamic(ctx, 25, 1);
                    nk_label(ctx, "A terrible error as occured", NK_TEXT_LEFT);
                    nk_layout_row_dynamic(ctx, 25, 2);
                    if (nk_button_label(ctx, "OK")) {
                        popup_active = 0;
                        nk_popup_close(ctx);
                    }
                    if (nk_button_label(ctx, "Cancel")) {
                        popup_active = 0;
                        nk_popup_close(ctx);
                    }
                    nk_popup_end(ctx);
                } else popup_active = nk_false;
            }

            /* tooltip */
            nk_layout_row_static(ctx, 30, 150, 1);
            bounds = nk_widget_bounds(ctx);
            nk_label(ctx, "Hover me for tooltip", NK_TEXT_LEFT);
            if (nk_input_is_mouse_hovering_rect(in, bounds))
                nk_tooltip(ctx, "This is a tooltip");

            nk_tree_pop(ctx);
        }

        if (nk_tree_push(ctx, NK_TREE_TAB, "Layout", NK_MINIMIZED))
        {
            if (nk_tree_push(ctx, NK_TREE_NODE, "Widget", NK_MINIMIZED))
            {
                float ratio_two[] = {0.2f, 0.6f, 0.2f};
                float width_two[] = {100, 200, 50};

                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "Dynamic fixed column layout with generated position and size:", NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 30, 3);
                nk_button_label(ctx, "button");
                nk_button_label(ctx, "button");
                nk_button_label(ctx, "button");

                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "static fixed column layout with generated position and size:", NK_TEXT_LEFT);
                nk_layout_row_static(ctx, 30, 100, 3);
                nk_button_label(ctx, "button");
                nk_button_label(ctx, "button");
                nk_button_label(ctx, "button");

                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "Dynamic array-based custom column layout with generated position and custom size:",NK_TEXT_LEFT);
                nk_layout_row(ctx, NK_DYNAMIC, 30, 3, ratio_two);
                nk_button_label(ctx, "button");
                nk_button_label(ctx, "button");
                nk_button_label(ctx, "button");

                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "Static array-based custom column layout with generated position and custom size:",NK_TEXT_LEFT );
                nk_layout_row(ctx, NK_STATIC, 30, 3, width_two);
                nk_button_label(ctx, "button");
                nk_button_label(ctx, "button");
                nk_button_label(ctx, "button");

                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "Dynamic immediate mode custom column layout with generated position and custom size:",NK_TEXT_LEFT);
                nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 3);
                nk_layout_row_push(ctx, 0.2f);
                nk_button_label(ctx, "button");
                nk_layout_row_push(ctx, 0.6f);
                nk_button_label(ctx, "button");
                nk_layout_row_push(ctx, 0.2f);
                nk_button_label(ctx, "button");
                nk_layout_row_end(ctx);

                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "Static immediate mode custom column layout with generated position and custom size:", NK_TEXT_LEFT);
                nk_layout_row_begin(ctx, NK_STATIC, 30, 3);
                nk_layout_row_push(ctx, 100);
                nk_button_label(ctx, "button");
                nk_layout_row_push(ctx, 200);
                nk_button_label(ctx, "button");
                nk_layout_row_push(ctx, 50);
                nk_button_label(ctx, "button");
                nk_layout_row_end(ctx);

                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "Static free space with custom position and custom size:", NK_TEXT_LEFT);
                nk_layout_space_begin(ctx, NK_STATIC, 60, 4);
                nk_layout_space_push(ctx, nk_rect(100, 0, 100, 30));
                nk_button_label(ctx, "button");
                nk_layout_space_push(ctx, nk_rect(0, 15, 100, 30));
                nk_button_label(ctx, "button");
                nk_layout_space_push(ctx, nk_rect(200, 15, 100, 30));
                nk_button_label(ctx, "button");
                nk_layout_space_push(ctx, nk_rect(100, 30, 100, 30));
                nk_button_label(ctx, "button");
                nk_layout_space_end(ctx);

                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "Row template:", NK_TEXT_LEFT);
                nk_layout_row_template_begin(ctx, 30);
                nk_layout_row_template_push_dynamic(ctx);
                nk_layout_row_template_push_variable(ctx, 80);
                nk_layout_row_template_push_static(ctx, 80);
                nk_layout_row_template_end(ctx);
                nk_button_label(ctx, "button");
                nk_button_label(ctx, "button");
                nk_button_label(ctx, "button");

                nk_tree_pop(ctx);
            }

            if (nk_tree_push(ctx, NK_TREE_NODE, "Group", NK_MINIMIZED))
            {
                static int group_titlebar = nk_false;
                static int group_border = nk_true;
                static int group_no_scrollbar = nk_false;
                static int group_width = 320;
                static int group_height = 200;

                nk_flags group_flags = 0;
                if (group_border) group_flags |= NK_WINDOW_BORDER;
                if (group_no_scrollbar) group_flags |= NK_WINDOW_NO_SCROLLBAR;
                if (group_titlebar) group_flags |= NK_WINDOW_TITLE;

                nk_layout_row_dynamic(ctx, 30, 3);
                nk_checkbox_label(ctx, "Titlebar", &group_titlebar);
                nk_checkbox_label(ctx, "Border", &group_border);
                nk_checkbox_label(ctx, "No Scrollbar", &group_no_scrollbar);

                nk_layout_row_begin(ctx, NK_STATIC, 22, 3);
                nk_layout_row_push(ctx, 50);
                nk_label(ctx, "size:", NK_TEXT_LEFT);
                nk_layout_row_push(ctx, 130);
                nk_property_int(ctx, "#Width:", 100, &group_width, 500, 10, 1);
                nk_layout_row_push(ctx, 130);
                nk_property_int(ctx, "#Height:", 100, &group_height, 500, 10, 1);
                nk_layout_row_end(ctx);

                nk_layout_row_static(ctx, (float)group_height, group_width, 2);
                if (nk_group_begin(ctx, "Group", group_flags)) {
                    int i = 0;
                    static int selected[16];
                    nk_layout_row_static(ctx, 18, 100, 1);
                    for (i = 0; i < 16; ++i)
                        nk_selectable_label(ctx, (selected[i]) ? "Selected": "Unselected", NK_TEXT_CENTERED, &selected[i]);
                    nk_group_end(ctx);
                }
                nk_tree_pop(ctx);
            }

            if (nk_tree_push(ctx, NK_TREE_NODE, "Notebook", NK_MINIMIZED))
            {
                static int current_tab = 0;
                struct nk_vec2 item_padding;
                struct nk_rect bounds;
                float step = (2*3.141592654f) / 32;
                enum chart_type {CHART_LINE, CHART_HISTO, CHART_MIXED};
                const char *names[] = {"Lines", "Columns", "Mixed"};
                float id = 0;
                int i;

                /* Header */
                nk_style_push_vec2(ctx, &ctx->style.window.spacing, nk_vec2(0,0));
                nk_style_push_float(ctx, &ctx->style.button.rounding, 0);
                nk_layout_row_begin(ctx, NK_STATIC, 20, 3);
                for (i = 0; i < 3; ++i) {
                    /* make sure button perfectly fits text */
                    const struct nk_user_font *f = ctx->style.font;
                    float text_width = f->width(f->userdata, f->height, names[i], nk_strlen(names[i]));
                    float widget_width = text_width + 3 * ctx->style.button.padding.x;
                    nk_layout_row_push(ctx, widget_width);
                    if (current_tab == i) {
                        /* active tab gets highlighted */
                        struct nk_style_item button_color = ctx->style.button.normal;
                        ctx->style.button.normal = ctx->style.button.active;
                        current_tab = nk_button_label(ctx, names[i]) ? i: current_tab;
                        ctx->style.button.normal = button_color;
                    } else current_tab = nk_button_label(ctx, names[i]) ? i: current_tab;
                }
                nk_style_pop_float(ctx);

                /* Body */
                nk_layout_row_dynamic(ctx, 140, 1);
                if (nk_group_begin(ctx, "Notebook", NK_WINDOW_BORDER))
                {
                    nk_style_pop_vec2(ctx);
                    switch (current_tab) {
                    case CHART_LINE:
                        nk_layout_row_dynamic(ctx, 100, 1);
                        bounds = nk_widget_bounds(ctx);
                        if (nk_chart_begin_colored(ctx, NK_CHART_LINES, nk_rgb(255,0,0), nk_rgb(150,0,0), 32, 0.0f, 1.0f)) {
                            nk_chart_add_slot_colored(ctx, NK_CHART_LINES, nk_rgb(0,0,255), nk_rgb(0,0,150),32, -1.0f, 1.0f);
                            for (i = 0, id = 0; i < 32; ++i) {
                                nk_chart_push_slot(ctx, (float)fabs(sin(id)), 0);
                                nk_chart_push_slot(ctx, (float)cos(id), 1);
                                id += step;
                            }
                        }
                        nk_chart_end(ctx);
                        break;
                    case CHART_HISTO:
                        nk_layout_row_dynamic(ctx, 100, 1);
                        bounds = nk_widget_bounds(ctx);
                        if (nk_chart_begin_colored(ctx, NK_CHART_COLUMN, nk_rgb(255,0,0), nk_rgb(150,0,0), 32, 0.0f, 1.0f)) {
                            for (i = 0, id = 0; i < 32; ++i) {
                                nk_chart_push_slot(ctx, (float)fabs(sin(id)), 0);
                                id += step;
                            }
                        }
                        nk_chart_end(ctx);
                        break;
                    case CHART_MIXED:
                        nk_layout_row_dynamic(ctx, 100, 1);
                        bounds = nk_widget_bounds(ctx);
                        if (nk_chart_begin_colored(ctx, NK_CHART_LINES, nk_rgb(255,0,0), nk_rgb(150,0,0), 32, 0.0f, 1.0f)) {
                            nk_chart_add_slot_colored(ctx, NK_CHART_LINES, nk_rgb(0,0,255), nk_rgb(0,0,150),32, -1.0f, 1.0f);
                            nk_chart_add_slot_colored(ctx, NK_CHART_COLUMN, nk_rgb(0,255,0), nk_rgb(0,150,0), 32, 0.0f, 1.0f);
                            for (i = 0, id = 0; i < 32; ++i) {
                                nk_chart_push_slot(ctx, (float)fabs(sin(id)), 0);
                                nk_chart_push_slot(ctx, (float)fabs(cos(id)), 1);
                                nk_chart_push_slot(ctx, (float)fabs(sin(id)), 2);
                                id += step;
                            }
                        }
                        nk_chart_end(ctx);
                        break;
                    }
                    nk_group_end(ctx);
                } else nk_style_pop_vec2(ctx);
                nk_tree_pop(ctx);
            }

            if (nk_tree_push(ctx, NK_TREE_NODE, "Simple", NK_MINIMIZED))
            {
                nk_layout_row_dynamic(ctx, 300, 2);
                if (nk_group_begin(ctx, "Group_Without_Border", 0)) {
                    int i = 0;
                    char buffer[64];
                    nk_layout_row_static(ctx, 18, 150, 1);
                    for (i = 0; i < 64; ++i) {
                        sprintf(buffer, "0x%02x", i);
                        nk_labelf(ctx, NK_TEXT_LEFT, "%s: scrollable region", buffer);
                    }
                    nk_group_end(ctx);
                }
                if (nk_group_begin(ctx, "Group_With_Border", NK_WINDOW_BORDER)) {
                    int i = 0;
                    char buffer[64];
                    nk_layout_row_dynamic(ctx, 25, 2);
                    for (i = 0; i < 64; ++i) {
                        sprintf(buffer, "%08d", ((((i%7)*10)^32))+(64+(i%2)*2));
                        nk_button_label(ctx, buffer);
                    }
                    nk_group_end(ctx);
                }
                nk_tree_pop(ctx);
            }

            if (nk_tree_push(ctx, NK_TREE_NODE, "Complex", NK_MINIMIZED))
            {
                int i;
                nk_layout_space_begin(ctx, NK_STATIC, 500, 64);
                nk_layout_space_push(ctx, nk_rect(0,0,150,500));
                if (nk_group_begin(ctx, "Group_left", NK_WINDOW_BORDER)) {
                    static int selected[32];
                    nk_layout_row_static(ctx, 18, 100, 1);
                    for (i = 0; i < 32; ++i)
                        nk_selectable_label(ctx, (selected[i]) ? "Selected": "Unselected", NK_TEXT_CENTERED, &selected[i]);
                    nk_group_end(ctx);
                }

                nk_layout_space_push(ctx, nk_rect(160,0,150,240));
                if (nk_group_begin(ctx, "Group_top", NK_WINDOW_BORDER)) {
                    nk_layout_row_dynamic(ctx, 25, 1);
                    nk_button_label(ctx, "#FFAA");
                    nk_button_label(ctx, "#FFBB");
                    nk_button_label(ctx, "#FFCC");
                    nk_button_label(ctx, "#FFDD");
                    nk_button_label(ctx, "#FFEE");
                    nk_button_label(ctx, "#FFFF");
                    nk_group_end(ctx);
                }

                nk_layout_space_push(ctx, nk_rect(160,250,150,250));
                if (nk_group_begin(ctx, "Group_buttom", NK_WINDOW_BORDER)) {
                    nk_layout_row_dynamic(ctx, 25, 1);
                    nk_button_label(ctx, "#FFAA");
                    nk_button_label(ctx, "#FFBB");
                    nk_button_label(ctx, "#FFCC");
                    nk_button_label(ctx, "#FFDD");
                    nk_button_label(ctx, "#FFEE");
                    nk_button_label(ctx, "#FFFF");
                    nk_group_end(ctx);
                }

                nk_layout_space_push(ctx, nk_rect(320,0,150,150));
                if (nk_group_begin(ctx, "Group_right_top", NK_WINDOW_BORDER)) {
                    static int selected[4];
                    nk_layout_row_static(ctx, 18, 100, 1);
                    for (i = 0; i < 4; ++i)
                        nk_selectable_label(ctx, (selected[i]) ? "Selected": "Unselected", NK_TEXT_CENTERED, &selected[i]);
                    nk_group_end(ctx);
                }

                nk_layout_space_push(ctx, nk_rect(320,160,150,150));
                if (nk_group_begin(ctx, "Group_right_center", NK_WINDOW_BORDER)) {
                    static int selected[4];
                    nk_layout_row_static(ctx, 18, 100, 1);
                    for (i = 0; i < 4; ++i)
                        nk_selectable_label(ctx, (selected[i]) ? "Selected": "Unselected", NK_TEXT_CENTERED, &selected[i]);
                    nk_group_end(ctx);
                }

                nk_layout_space_push(ctx, nk_rect(320,320,150,150));
                if (nk_group_begin(ctx, "Group_right_bottom", NK_WINDOW_BORDER)) {
                    static int selected[4];
                    nk_layout_row_static(ctx, 18, 100, 1);
                    for (i = 0; i < 4; ++i)
                        nk_selectable_label(ctx, (selected[i]) ? "Selected": "Unselected", NK_TEXT_CENTERED, &selected[i]);
                    nk_group_end(ctx);
                }
                nk_layout_space_end(ctx);
                nk_tree_pop(ctx);
            }

            if (nk_tree_push(ctx, NK_TREE_NODE, "Splitter", NK_MINIMIZED))
            {
                const struct nk_input *in = &ctx->input;
                nk_layout_row_static(ctx, 20, 320, 1);
                nk_label(ctx, "Use slider and spinner to change tile size", NK_TEXT_LEFT);
                nk_label(ctx, "Drag the space between tiles to change tile ratio", NK_TEXT_LEFT);

                if (nk_tree_push(ctx, NK_TREE_NODE, "Vertical", NK_MINIMIZED))
                {
                    static float a = 100, b = 100, c = 100;
                    struct nk_rect bounds;

                    float row_layout[5];
                    row_layout[0] = a;
                    row_layout[1] = 8;
                    row_layout[2] = b;
                    row_layout[3] = 8;
                    row_layout[4] = c;

                    /* header */
                    nk_layout_row_static(ctx, 30, 100, 2);
                    nk_label(ctx, "left:", NK_TEXT_LEFT);
                    nk_slider_float(ctx, 10.0f, &a, 200.0f, 10.0f);

                    nk_label(ctx, "middle:", NK_TEXT_LEFT);
                    nk_slider_float(ctx, 10.0f, &b, 200.0f, 10.0f);

                    nk_label(ctx, "right:", NK_TEXT_LEFT);
                    nk_slider_float(ctx, 10.0f, &c, 200.0f, 10.0f);

                    /* tiles */
                    nk_layout_row(ctx, NK_STATIC, 200, 5, row_layout);

                    /* left space */
                    if (nk_group_begin(ctx, "left", NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
                        nk_layout_row_dynamic(ctx, 25, 1);
                        nk_button_label(ctx, "#FFAA");
                        nk_button_label(ctx, "#FFBB");
                        nk_button_label(ctx, "#FFCC");
                        nk_button_label(ctx, "#FFDD");
                        nk_button_label(ctx, "#FFEE");
                        nk_button_label(ctx, "#FFFF");
                        nk_group_end(ctx);
                    }

                    /* scaler */
                    bounds = nk_widget_bounds(ctx);
                    nk_spacing(ctx, 1);
                    if ((nk_input_is_mouse_hovering_rect(in, bounds) ||
						 nk_input_is_mouse_prev_hovering_rect(in, bounds)) &&
                        nk_input_is_mouse_down(in, NK_BUTTON_LEFT))
                    {
                        a = row_layout[0] + in->mouse.delta.x;
                        b = row_layout[2] - in->mouse.delta.x;
                    }

                    /* middle space */
                    if (nk_group_begin(ctx, "center", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
                        nk_layout_row_dynamic(ctx, 25, 1);
                        nk_button_label(ctx, "#FFAA");
                        nk_button_label(ctx, "#FFBB");
                        nk_button_label(ctx, "#FFCC");
                        nk_button_label(ctx, "#FFDD");
                        nk_button_label(ctx, "#FFEE");
                        nk_button_label(ctx, "#FFFF");
                        nk_group_end(ctx);
                    }

                    /* scaler */
                    bounds = nk_widget_bounds(ctx);
                    nk_spacing(ctx, 1);
                    if ((nk_input_is_mouse_hovering_rect(in, bounds) ||
						 nk_input_is_mouse_prev_hovering_rect(in, bounds)) &&
                        nk_input_is_mouse_down(in, NK_BUTTON_LEFT))
                    {
                        b = (row_layout[2] + in->mouse.delta.x);
                        c = (row_layout[4] - in->mouse.delta.x);
                    }

                    /* right space */
                    if (nk_group_begin(ctx, "right", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
                        nk_layout_row_dynamic(ctx, 25, 1);
                        nk_button_label(ctx, "#FFAA");
                        nk_button_label(ctx, "#FFBB");
                        nk_button_label(ctx, "#FFCC");
                        nk_button_label(ctx, "#FFDD");
                        nk_button_label(ctx, "#FFEE");
                        nk_button_label(ctx, "#FFFF");
                        nk_group_end(ctx);
                    }

                    nk_tree_pop(ctx);
                }

                if (nk_tree_push(ctx, NK_TREE_NODE, "Horizontal", NK_MINIMIZED))
                {
                    static float a = 100, b = 100, c = 100;
                    struct nk_rect bounds;

                    /* header */
                    nk_layout_row_static(ctx, 30, 100, 2);
                    nk_label(ctx, "top:", NK_TEXT_LEFT);
                    nk_slider_float(ctx, 10.0f, &a, 200.0f, 10.0f);

                    nk_label(ctx, "middle:", NK_TEXT_LEFT);
                    nk_slider_float(ctx, 10.0f, &b, 200.0f, 10.0f);

                    nk_label(ctx, "bottom:", NK_TEXT_LEFT);
                    nk_slider_float(ctx, 10.0f, &c, 200.0f, 10.0f);

                    /* top space */
                    nk_layout_row_dynamic(ctx, a, 1);
                    if (nk_group_begin(ctx, "top", NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_BORDER)) {
                        nk_layout_row_dynamic(ctx, 25, 3);
                        nk_button_label(ctx, "#FFAA");
                        nk_button_label(ctx, "#FFBB");
                        nk_button_label(ctx, "#FFCC");
                        nk_button_label(ctx, "#FFDD");
                        nk_button_label(ctx, "#FFEE");
                        nk_button_label(ctx, "#FFFF");
                        nk_group_end(ctx);
                    }

                    /* scaler */
                    nk_layout_row_dynamic(ctx, 8, 1);
                    bounds = nk_widget_bounds(ctx);
                    nk_spacing(ctx, 1);
                    if ((nk_input_is_mouse_hovering_rect(in, bounds) ||
						 nk_input_is_mouse_prev_hovering_rect(in, bounds)) &&
                        nk_input_is_mouse_down(in, NK_BUTTON_LEFT))
                    {
                        a = a + in->mouse.delta.y;
                        b = b - in->mouse.delta.y;
                    }

                    /* middle space */
                    nk_layout_row_dynamic(ctx, b, 1);
                    if (nk_group_begin(ctx, "middle", NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_BORDER)) {
                        nk_layout_row_dynamic(ctx, 25, 3);
                        nk_button_label(ctx, "#FFAA");
                        nk_button_label(ctx, "#FFBB");
                        nk_button_label(ctx, "#FFCC");
                        nk_button_label(ctx, "#FFDD");
                        nk_button_label(ctx, "#FFEE");
                        nk_button_label(ctx, "#FFFF");
                        nk_group_end(ctx);
                    }

                    {
                        /* scaler */
                        nk_layout_row_dynamic(ctx, 8, 1);
                        bounds = nk_widget_bounds(ctx);
                        if ((nk_input_is_mouse_hovering_rect(in, bounds) ||
							 nk_input_is_mouse_prev_hovering_rect(in, bounds)) &&
                            nk_input_is_mouse_down(in, NK_BUTTON_LEFT))
                        {
                            b = b + in->mouse.delta.y;
                            c = c - in->mouse.delta.y;
                        }
                    }

                    /* bottom space */
                    nk_layout_row_dynamic(ctx, c, 1);
                    if (nk_group_begin(ctx, "bottom", NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_BORDER)) {
                        nk_layout_row_dynamic(ctx, 25, 3);
                        nk_button_label(ctx, "#FFAA");
                        nk_button_label(ctx, "#FFBB");
                        nk_button_label(ctx, "#FFCC");
                        nk_button_label(ctx, "#FFDD");
                        nk_button_label(ctx, "#FFEE");
                        nk_button_label(ctx, "#FFFF");
                        nk_group_end(ctx);
                    }
                    nk_tree_pop(ctx);
                }
                nk_tree_pop(ctx);
            }
            nk_tree_pop(ctx);
        }
    }
    nk_end(ctx);
    return !nk_window_is_closed(ctx, "Overview");
}


void render(void)
{
	renderer_draw();
}

void game_cleanup(void)
{
	editor_cleanup();
	scene_cleanup();
	entity_cleanup();
	model_cleanup();
	material_cleanup();
	geom_cleanup();
	light_cleanup();
	transform_cleanup();
	camera_cleanup();
	input_cleanup();
	renderer_cleanup();
	io_file_cleanup();
	framebuffer_cleanup();
	texture_cleanup();
	shader_cleanup();
	window_destroy(game_state->window);
	gl_cleanup();
	window_cleanup();
	platform_cleanup();
	free(game_state);
}

struct Game_State* game_state_get(void)
{
	return game_state;
}
