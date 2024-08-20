#define SOKOL_IMPL
#define SOKOL_GLCORE
#include "libs/sokol_gfx.h"
#include "libs/sokol_app.h"
#include "libs/sokol_glue.h"
#include "libs/sokol_time.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#include "libs/nuklear.h"

#define SOKOL_NUKLEAR_IMPL
#include "libs/sokol_nuklear.h"

static struct {
    struct nk_context* ctx;
    uint64_t last_time;
} state;

static void init(void) {
    sg_setup(&(sg_desc){
	.environment = sglue_environment(),
    });

    stm_setup();

    snk_setup(&(snk_desc_t){
        .dpi_scale = sapp_dpi_scale()
    });
    state.last_time = stm_now();
}

static void frame(void) {
    uint64_t current_time = stm_now();
    double delta_time = stm_sec(current_time - state.last_time);
    state.last_time = current_time;

    state.ctx = snk_new_frame();

    if (nk_begin(state.ctx, "Demo", nk_rect(50, 50, 220, 220),
        NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE)) {
        if (nk_button_label(state.ctx, "Button")) {
            // Button was pressed
        }
    }
    nk_end(state.ctx);

    snk_render(sapp_width(), sapp_height());
    sg_end_pass();
    sg_commit();
}

static void cleanup(void) {
    snk_shutdown();
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .width = 800,
        .height = 600,
        .window_title = "Nuklear + Sokol Example",
    };
}

