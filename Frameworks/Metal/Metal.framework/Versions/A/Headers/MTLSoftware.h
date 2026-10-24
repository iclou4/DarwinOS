/*
 * MTLSoftware.h - ravynOS Metal: the C core of the API.
 *
 * This is the software backend: vendor independent by construction.  It needs
 * a linear framebuffer and nothing else, so it works on every video card -
 * UEFI GOP today, an accelerated driver later behind the same interface.
 *
 * The Objective-C veneer (MTLDevice, MTLCommandQueue, MTLRenderPipelineState,
 * ...) that applications actually message wraps these functions one to one,
 * which is why the naming here is mechanical.
 *
 * Threading model: a command buffer is a recording of commands (device,
 * resource and encoder calls do not touch pixels).  Executing them happens in
 * `mtl_command_buffer_commit`, the same split Metal has, so a future GPU
 * backend can hand the same recording to hardware.
 */

#ifndef RAVYNOS_MTL_SOFTWARE_H
#define RAVYNOS_MTL_SOFTWARE_H

#include <Metal/MTLDefines.h>
#include <Metal/MTLTypes.h>
#include <Metal/MTLPixelFormat.h>
#include <Metal/MTLResource.h>
#include <Metal/MTLTexture.h>
#include <Metal/MTLSampler.h>
#include <Metal/MTLDepthStencil.h>
#include <Metal/MTLRenderPass.h>
#include <Metal/MTLRenderPipeline.h>
#include <Metal/MTLRenderCommandEncoder.h>
#include <Metal/MTLShaders.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mtl_device          mtl_device_t;
typedef struct mtl_buffer          mtl_buffer_t;
typedef struct mtl_texture         mtl_texture_t;
typedef struct mtl_sampler         mtl_sampler_t;
typedef struct mtl_library         mtl_library_t;
typedef struct mtl_render_pipeline mtl_render_pipeline_t;
typedef struct mtl_command_queue   mtl_command_queue_t;
typedef struct mtl_command_buffer  mtl_command_buffer_t;
typedef struct mtl_render_encoder  mtl_render_encoder_t;

/* ------------------------------------------------------------------------- */
/* device                                                                    */
/* ------------------------------------------------------------------------- */

/* The CPU rasterizer device.  Always present, on any video card. */
mtl_device_t *mtl_device_create(void);
const char   *mtl_device_name(mtl_device_t *device);
void          mtl_device_release(mtl_device_t *device);

/* Accounting: proves work actually happened and is the seed of the GPU
 * counters a driver backed device would expose. */
typedef struct {
	unsigned long draw_calls;
	unsigned long triangles;
	unsigned long fragments;
	unsigned long frames_presented;
} mtl_device_statistics;

void mtl_device_get_statistics(mtl_device_t *device,
    mtl_device_statistics *statistics);

/* ------------------------------------------------------------------------- */
/* buffers                                                                   */
/* ------------------------------------------------------------------------- */

mtl_buffer_t *mtl_device_new_buffer(mtl_device_t *device, mtl_size_t length);
void         *mtl_buffer_contents(mtl_buffer_t *buffer);
mtl_size_t    mtl_buffer_length(mtl_buffer_t *buffer);
void          mtl_buffer_release(mtl_buffer_t *buffer);

/* ------------------------------------------------------------------------- */
/* textures                                                                  */
/* ------------------------------------------------------------------------- */

typedef struct {
	MTLPixelFormat pixel_format;
	mtl_size_t     width;
	mtl_size_t     height;
	mtl_size_t     depth;              /* 1 for 2D */
	unsigned int   usage;              /* MTLTextureUsage bits */
	unsigned int   storage_mode;
} mtl_texture_descriptor;

mtl_texture_t *mtl_device_new_texture(mtl_device_t *device,
    const mtl_texture_descriptor *descriptor);
void  mtl_texture_replace_region(mtl_texture_t *texture, MTLRegion region,
    const void *bytes, mtl_size_t bytes_per_row);
void  mtl_texture_get_bytes(mtl_texture_t *texture, MTLRegion region,
    void *bytes, mtl_size_t bytes_per_row);

/* Direct access to the texel store.  The layout is BGRA8 in memory order
 * (0xAARRGGBB in a little endian word), which is exactly what the scanout
 * framebuffer uses, so presenting is a row copy and nothing else. */
void *mtl_texture_contents(mtl_texture_t *texture);
void  mtl_texture_set_backing(mtl_texture_t *texture, void *pixels);
MTLPixelFormat mtl_texture_pixel_format(mtl_texture_t *texture);
void  mtl_texture_release(mtl_texture_t *texture);

/* ------------------------------------------------------------------------- */
/* samplers                                                                  */
/* ------------------------------------------------------------------------- */

typedef struct {
	MTLSamplerMinMagFilter min_filter;
	MTLSamplerMinMagFilter mag_filter;
	MTLSamplerAddressMode  address_s;
	MTLSamplerAddressMode  address_t;
} mtl_sampler_descriptor;

mtl_sampler_t *mtl_device_new_sampler(mtl_device_t *device,
    const mtl_sampler_descriptor *descriptor);
void           mtl_sampler_release(mtl_sampler_t *sampler);

/* ------------------------------------------------------------------------- */
/* libraries and render pipelines                                            */
/* ------------------------------------------------------------------------- */

/* A "function" is a C shader: name -> function pointer. */
void            mtl_library_add_function(mtl_library_t *library,
    const char *name, mtl_vertex_function function);
void            mtl_library_add_fragment_function(mtl_library_t *library,
    const char *name, mtl_fragment_function function);
mtl_library_t  *mtl_device_new_library(mtl_device_t *device);
mtl_vertex_function   mtl_library_vertex_function(mtl_library_t *library,
    const char *name);
mtl_fragment_function mtl_library_fragment_function(mtl_library_t *library,
    const char *name);
void                  mtl_library_release(mtl_library_t *library);

typedef struct {
	MTLBlendFactor    source_rgb;
	MTLBlendFactor    destination_rgb;
	MTLBlendOperation operation_rgb;
	MTLBlendFactor    source_alpha;
	MTLBlendFactor    destination_alpha;
	MTLBlendOperation operation_alpha;
	unsigned int      write_mask;          /* MTLColorWriteMask */
	int               blending_enabled;
} mtl_color_attachment_descriptor;

typedef struct {
	mtl_vertex_function   vertex_function;
	mtl_fragment_function fragment_function;
	MTLPixelFormat        color_attachment_format;
	MTLPixelFormat        depth_attachment_format;  /* Invalid if unused */
	mtl_color_attachment_descriptor color_attachment;
} mtl_render_pipeline_descriptor;

mtl_render_pipeline_t *mtl_device_new_render_pipeline(mtl_device_t *device,
    const mtl_render_pipeline_descriptor *descriptor);
void                   mtl_render_pipeline_release(mtl_render_pipeline_t *pipeline);

/* ------------------------------------------------------------------------- */
/* command buffers                                                           */
/* ------------------------------------------------------------------------- */

typedef struct {
	mtl_texture_t *color_attachment;   /* the drawable or an offscreen target */
	mtl_texture_t *depth_attachment;   /* optional */
	MTLLoadAction  color_load_action;
	MTLLoadAction  depth_load_action;
	MTLStoreAction color_store_action;
	MTLStoreAction depth_store_action;
	mtl_float4     clear_color;
	float          clear_depth;
} mtl_render_pass_descriptor;

mtl_command_queue_t  *mtl_device_new_command_queue(mtl_device_t *device);
void                  mtl_command_queue_release(mtl_command_queue_t *queue);
mtl_command_buffer_t *mtl_command_queue_command_buffer(mtl_command_queue_t *queue);
void                  mtl_command_buffer_release(mtl_command_buffer_t *command_buffer);

mtl_render_encoder_t *mtl_command_buffer_render_encoder(
    mtl_command_buffer_t *command_buffer,
    const mtl_render_pass_descriptor *descriptor);

/* encoder state */
void mtl_render_encoder_set_pipeline(mtl_render_encoder_t *encoder,
    mtl_render_pipeline_t *pipeline);
void mtl_render_encoder_set_vertex_buffer(mtl_render_encoder_t *encoder,
    unsigned int index, mtl_buffer_t *buffer, mtl_size_t offset);
void mtl_render_encoder_set_vertex_bytes(mtl_render_encoder_t *encoder,
    unsigned int index, const void *bytes, mtl_size_t length);
void mtl_render_encoder_set_fragment_buffer(mtl_render_encoder_t *encoder,
    unsigned int index, mtl_buffer_t *buffer, mtl_size_t offset);
void mtl_render_encoder_set_fragment_bytes(mtl_render_encoder_t *encoder,
    unsigned int index, const void *bytes, mtl_size_t length);
void mtl_render_encoder_set_fragment_texture(mtl_render_encoder_t *encoder,
    unsigned int index, mtl_texture_t *texture);
void mtl_render_encoder_set_fragment_sampler_state(mtl_render_encoder_t *encoder,
    unsigned int index, mtl_sampler_t *sampler);
void mtl_render_encoder_set_viewport(mtl_render_encoder_t *encoder,
    float x, float y, float width, float height, float near, float far);
void mtl_render_encoder_set_scissor_rect(mtl_render_encoder_t *encoder,
    long x, long y, long width, long height);
void mtl_render_encoder_set_cull_mode(mtl_render_encoder_t *encoder,
    MTLCullMode mode);
void mtl_render_encoder_set_front_facing_winding(mtl_render_encoder_t *encoder,
    MTLWinding winding);
void mtl_render_encoder_set_depth_state(mtl_render_encoder_t *encoder,
    MTLCompareFunction compare, int write_enabled);

/* drawing */
void mtl_render_encoder_draw_primitives(mtl_render_encoder_t *encoder,
    MTLPrimitiveType primitive_type, mtl_size_t vertex_start,
    mtl_size_t vertex_count);
void mtl_render_encoder_draw_primitives_instanced(mtl_render_encoder_t *encoder,
    MTLPrimitiveType primitive_type, mtl_size_t vertex_start,
    mtl_size_t vertex_count, mtl_size_t instance_count);
void mtl_render_encoder_end_encoding(mtl_render_encoder_t *encoder);

/* presentation: the texture becomes the screen contents (fb_present in the
 * guest; the test harness just records it) */
void mtl_command_buffer_present(mtl_command_buffer_t *command_buffer,
    mtl_texture_t *drawable);

void mtl_command_buffer_commit(mtl_command_buffer_t *command_buffer);
void mtl_command_buffer_wait_until_completed(mtl_command_buffer_t *command_buffer);

/* Presentation hook: the guest installs one that calls the framebuffer
 * syscall; a host test installs one that writes a file. */
typedef void (*mtl_present_handler)(mtl_texture_t *drawable, void *context);
void mtl_set_present_handler(mtl_present_handler handler, void *context);

#ifdef __cplusplus
}
#endif

#endif /* RAVYNOS_MTL_SOFTWARE_H */
