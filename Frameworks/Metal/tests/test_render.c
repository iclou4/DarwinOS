/*
 * test_render.c - host test harness for the ravynOS Metal software backend.
 *
 * No VM, no driver, no window server: create a device, render frames the way
 * an application would (command queue -> command buffer -> render encoder ->
 * draw -> present) and write the result out as images.  That is exactly the
 * code path the guest will run, minus the framebuffer syscall at the end.
 *
 * Build:  make
 * Run:    ./build/test_render build
 */

#include <Metal/MTLSoftware.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define FRAME_WIDTH  1280
#define FRAME_HEIGHT 800

/* ------------------------------------------------------------------------- */
/* shaders                                                                   */
/* ------------------------------------------------------------------------- */

/* vertex buffer 0: vec4 clip space position (stride 16)
 * vertex buffer 1: vec4 colour                 (stride 16) */
static void
vs_colored(const mtl_vertex_in *in, mtl_vertex_out *out)
{
	const float *position = (const float *)mtl_vertex_pointer(in, 0, 16);
	const float *color = (const float *)mtl_vertex_pointer(in, 1, 16);

	out->position = mtl_make_float4(position[0], position[1], position[2],
	    position[3]);
	out->varying[0] = mtl_make_float4(color[0], color[1], color[2], color[3]);
}

/* vertex buffer 0: vec4 clip space position (stride 16), no varyings */
static void
vs_position(const mtl_vertex_in *in, mtl_vertex_out *out)
{
	const float *position = (const float *)mtl_vertex_pointer(in, 0, 16);

	out->position = mtl_make_float4(position[0], position[1], position[2],
	    position[3]);
}

/* same, but asks for larger points */
static void
vs_point(const mtl_vertex_in *in, mtl_vertex_out *out)
{
	const float *position = (const float *)mtl_vertex_pointer(in, 0, 16);

	out->position = mtl_make_float4(position[0], position[1], position[2],
	    position[3]);
	out->point_size = 11.0f;
}

/* vertex buffer 0: vec4 clip space position (stride 16)
 * vertex buffer 1: vec2 texture coordinate    (stride 8) */
static void
vs_textured(const mtl_vertex_in *in, mtl_vertex_out *out)
{
	const float *position = (const float *)mtl_vertex_pointer(in, 0, 16);
	const float *uv = (const float *)mtl_vertex_pointer(in, 1, 8);

	out->position = mtl_make_float4(position[0], position[1], position[2],
	    position[3]);
	out->varying[0] = mtl_make_float4(uv[0], uv[1], 0.0f, 0.0f);
}

static void
fs_varying(const mtl_fragment_in *in, mtl_fragment_out *out)
{
	out->color = in->varying[0];
}

static void
fs_solid(const mtl_fragment_in *in, mtl_fragment_out *out)
{
	const mtl_float4 *color = (const mtl_float4 *)in->slots[0];

	out->color = *color;
}

static void
fs_textured(const mtl_fragment_in *in, mtl_fragment_out *out)
{
	const mtl_float4 *tint = (const mtl_float4 *)in->slots[0];
	mtl_float4 texel = mtl_sample_texture(in, 0,
	    mtl_make_float2(in->varying[0].x, in->varying[0].y));

	out->color = mtl_f4_mul(texel, *tint);
}

/* ------------------------------------------------------------------------- */
/* output                                                                    */
/* ------------------------------------------------------------------------- */

static void
write_ppm(const mtl_texture_t *texture, const char *path)
{
	const unsigned char *bytes =
	    (const unsigned char *)mtl_texture_contents((mtl_texture_t *)texture);
	FILE *file;
	long x, y;

	file = fopen(path, "wb");
	if (file == NULL) {
		fprintf(stderr, "cannot write %s\n", path);
		return;
	}

	fprintf(file, "P6\n%u %u\n255\n", (unsigned)FRAME_WIDTH,
	    (unsigned)FRAME_HEIGHT);
	for (y = 0; y < FRAME_HEIGHT; y++) {
		for (x = 0; x < FRAME_WIDTH; x++) {
			const unsigned char *texel = bytes + (y * FRAME_WIDTH + x) * 4;

			/* memory order is B,G,R,A */
			fputc(texel[2], file);
			fputc(texel[1], file);
			fputc(texel[0], file);
		}
	}
	fclose(file);
	printf("wrote %s\n", path);
}

typedef struct {
	const char *directory;
	int         frame;
} present_context;

static void
on_present(mtl_texture_t *drawable, void *context)
{
	present_context *present = (present_context *)context;
	char path[512];

	if (present == NULL) {
		return;
	}
	/* In the guest this is exactly where fb_present() hands the drawable to
	 * the kernel: identical buffer layout, one syscall. */
	snprintf(path, sizeof(path), "%s/frame%d.ppm", present->directory,
	    present->frame);
	write_ppm(drawable, path);
}

/* ------------------------------------------------------------------------- */
/* textures                                                                  */
/* ------------------------------------------------------------------------- */

/* a checkerboard with a horizontal alpha ramp, in RGBA8: exercises the
 * RGBA -> BGRA conversion in mtl_texture_replace_region */
static mtl_texture_t *
make_checkerboard(mtl_device_t *device)
{
	mtl_texture_descriptor descriptor;
	unsigned char texels[64 * 64 * 4];
	unsigned int x, y;
	mtl_texture_t *texture;

	descriptor.pixel_format = MTLPixelFormatRGBA8Unorm;
	descriptor.width = 64;
	descriptor.height = 64;
	descriptor.depth = 1;
	descriptor.usage = MTLTextureUsageShaderRead;
	descriptor.storage_mode = MTLResourceStorageModeShared;

	for (y = 0; y < 64; y++) {
		for (x = 0; x < 64; x++) {
			unsigned char *texel = texels + (y * 64 + x) * 4;
			int cell = ((x / 8) + (y / 8)) & 1;
			int border = (x < 3 || y < 3 || x > 60 || y > 60);
			float alpha = 0.25f + 0.7f * ((float)x / 63.0f);

			if (border) {
				texel[0] = 255;
				texel[1] = 160;
				texel[2] = 60;
				texel[3] = 255;
			} else if (cell) {
				texel[0] = 235;
				texel[1] = 235;
				texel[2] = 240;
				texel[3] = (unsigned char)(alpha * 255.0f);
			} else {
				texel[0] = 130;
				texel[1] = 150;
				texel[2] = 190;
				texel[3] = (unsigned char)(alpha * 255.0f);
			}
		}
	}

	texture = mtl_device_new_texture(device, &descriptor);
	mtl_texture_replace_region(texture, MTLRegionMake2D(0, 0, 64, 64), texels,
	    64 * 4);
	return texture;
}

/* a soft radial dot with alpha, 32x32, in BGRA8 */
static mtl_texture_t *
make_dot(mtl_device_t *device)
{
	mtl_texture_descriptor descriptor;
	uint32_t texels[32 * 32];
	unsigned int x, y;
	mtl_texture_t *texture;

	descriptor.pixel_format = MTLPixelFormatBGRA8Unorm;
	descriptor.width = 32;
	descriptor.height = 32;
	descriptor.depth = 1;
	descriptor.usage = MTLTextureUsageShaderRead;
	descriptor.storage_mode = MTLResourceStorageModeShared;

	for (y = 0; y < 32; y++) {
		for (x = 0; x < 32; x++) {
			float dx = ((float)x + 0.5f) / 32.0f - 0.5f;
			float dy = ((float)y + 0.5f) / 32.0f - 0.5f;
			float distance = sqrtf(dx * dx + dy * dy) * 2.0f;
			float alpha = distance >= 1.0f ? 0.0f :
			    (1.0f - distance) * (1.0f - distance);
			unsigned int a = (unsigned int)(alpha * 255.0f);
			unsigned int r = 255;
			unsigned int g = (unsigned int)(235.0f - distance * 60.0f);
			unsigned int b = (unsigned int)(120.0f - distance * 40.0f);

			texels[y * 32 + x] = (a << 24) | (r << 16) | (g << 8) | b;
		}
	}

	texture = mtl_device_new_texture(device, &descriptor);
	mtl_texture_replace_region(texture, MTLRegionMake2D(0, 0, 32, 32), texels,
	    32 * 4);
	return texture;
}

/* ------------------------------------------------------------------------- */
/* geometry helpers                                                          */
/* ------------------------------------------------------------------------- */

/* a quad as a triangle strip: vec4 positions at offset 0, vec4 colours after */
static void
fill_colored_quad(mtl_buffer_t *buffer, const float corners[4][2],
    const float positions[4][4], const float colors[4][4])
{
	unsigned char *bytes = (unsigned char *)mtl_buffer_contents(buffer);

	(void)corners;
	memcpy(bytes, positions, 4 * 16);
	memcpy(bytes + 4 * 16, colors, 4 * 16);
}

static void
fill_rotated_quad(float positions[4][4], float centre_x, float centre_y,
    float half_size, float depth, float angle)
{
	float corner_x[4] = { -1.0f, 1.0f, -1.0f, 1.0f };
	float corner_y[4] = { -1.0f, -1.0f, 1.0f, 1.0f };
	float cosine = cosf(angle);
	float sine = sinf(angle);
	int i;

	for (i = 0; i < 4; i++) {
		float local_x = corner_x[i] * half_size;
		float local_y = corner_y[i] * half_size;

		positions[i][0] = centre_x + local_x * cosine - local_y * sine;
		positions[i][1] = centre_y + local_x * sine + local_y * cosine;
		positions[i][2] = depth;
		positions[i][3] = 1.0f;
	}
}

/* ------------------------------------------------------------------------- */
/* scene                                                                     */
/* ------------------------------------------------------------------------- */

typedef struct {
	mtl_texture_t         *drawable;
	mtl_texture_t         *depth;
	mtl_render_pipeline_t *pipeline_varying;
	mtl_render_pipeline_t *pipeline_solid;
	mtl_render_pipeline_t *pipeline_point;
	mtl_render_pipeline_t *pipeline_blend;
	mtl_render_pipeline_t *pipeline_textured;
	mtl_sampler_t         *sampler_nearest;
	mtl_sampler_t         *sampler_linear;
	mtl_texture_t         *checker;
	mtl_texture_t         *dot;
	mtl_buffer_t          *background;
	mtl_buffer_t          *triangle;
	mtl_buffer_t          *quad_positions;
	mtl_buffer_t          *quad_positions_b;
	mtl_buffer_t          *quad_uvs;
	mtl_buffer_t          *depth_far;
	mtl_buffer_t          *depth_near;
	mtl_buffer_t          *wireframe;
	mtl_buffer_t          *points;
} scene;

static void
encode_frame(scene *s, mtl_command_queue_t *queue, float angle)
{
	mtl_command_buffer_t *command_buffer = mtl_command_queue_command_buffer(queue);
	mtl_render_pass_descriptor pass;
	mtl_render_encoder_t *encoder;
	mtl_float4 white = { 1.0f, 1.0f, 1.0f, 1.0f };
	mtl_float4 tint_alpha = { 1.0f, 1.0f, 1.0f, 0.85f };
	mtl_float4 wire_color = { 1.0f, 1.0f, 1.0f, 0.6f };
	mtl_float4 near_color = { 0.16f, 0.32f, 0.78f, 1.0f };
	mtl_float4 far_color = { 1.00f, 0.55f, 0.10f, 1.0f };
	float positions[4][4];
	(void)white;

	memset(&pass, 0, sizeof(pass));
	pass.color_attachment = s->drawable;
	pass.depth_attachment = s->depth;
	pass.color_load_action = MTLLoadActionClear;
	pass.depth_load_action = MTLLoadActionClear;
	pass.color_store_action = MTLStoreActionStore;
	pass.clear_color = mtl_make_float4(0.0f, 0.0f, 0.0f, 1.0f);
	pass.clear_depth = 1.0f;

	encoder = mtl_command_buffer_render_encoder(command_buffer, &pass);
	if (encoder == NULL) {
		fprintf(stderr, "cannot create render encoder\n");
		return;
	}

	/* background: full screen, dark and graded, at the far plane */
	mtl_render_encoder_set_pipeline(encoder, s->pipeline_varying);
	mtl_render_encoder_set_vertex_buffer(encoder, 0, s->background, 0);
	mtl_render_encoder_set_vertex_buffer(encoder, 1, s->background, 4 * 16);
	mtl_render_encoder_draw_primitives(encoder, MTLPrimitiveTypeTriangleStrip,
	    0, 4);

	/* the big Gouraud triangle */
	mtl_render_encoder_set_vertex_buffer(encoder, 0, s->triangle, 0);
	mtl_render_encoder_set_vertex_buffer(encoder, 1, s->triangle, 3 * 16);
	mtl_render_encoder_draw_primitives(encoder, MTLPrimitiveTypeTriangle, 0, 3);

	/* depth test: the far triangle is drawn *after* the near one, so it only
	 * survives where the near one does not cover it */
	mtl_render_encoder_set_pipeline(encoder, s->pipeline_solid);
	mtl_render_encoder_set_vertex_buffer(encoder, 0, s->depth_near, 0);
	mtl_render_encoder_set_fragment_bytes(encoder, 0, &near_color,
	    sizeof(near_color));
	mtl_render_encoder_draw_primitives(encoder, MTLPrimitiveTypeTriangle, 0, 3);

	mtl_render_encoder_set_vertex_buffer(encoder, 0, s->depth_far, 0);
	mtl_render_encoder_set_fragment_bytes(encoder, 0, &far_color,
	    sizeof(far_color));
	mtl_render_encoder_draw_primitives(encoder, MTLPrimitiveTypeTriangle, 0, 3);

	/* textured quads: nearest sampling on the left, linear on the right */
	mtl_render_encoder_set_pipeline(encoder, s->pipeline_textured);
	mtl_render_encoder_set_vertex_buffer(encoder, 1, s->quad_uvs, 0);
	mtl_render_encoder_set_depth_state(encoder, MTLCompareFunctionLess, 0);

	fill_rotated_quad(positions, -0.62f, -0.52f, 0.30f, 0.40f, angle);
	memcpy(mtl_buffer_contents(s->quad_positions), positions, sizeof(positions));
	mtl_render_encoder_set_vertex_buffer(encoder, 0, s->quad_positions, 0);
	mtl_render_encoder_set_fragment_texture(encoder, 0, s->checker);
	mtl_render_encoder_set_fragment_sampler_state(encoder, 0, s->sampler_nearest);
	mtl_render_encoder_set_fragment_bytes(encoder, 0, &tint_alpha,
	    sizeof(tint_alpha));
	mtl_render_encoder_draw_primitives(encoder, MTLPrimitiveTypeTriangleStrip,
	    0, 4);

	/* separate buffers: contents are read when the recording runs, exactly
	 * as they would be on a GPU, so the two quads must not share one */
	fill_rotated_quad(positions, 0.62f, -0.52f, 0.30f, 0.40f, angle);
	memcpy(mtl_buffer_contents(s->quad_positions_b), positions,
	    sizeof(positions));
	mtl_render_encoder_set_vertex_buffer(encoder, 0, s->quad_positions_b, 0);
	mtl_render_encoder_set_fragment_texture(encoder, 0, s->dot);
	mtl_render_encoder_set_fragment_sampler_state(encoder, 0, s->sampler_linear);
	{
		mtl_float4 solid = { 1.0f, 1.0f, 1.0f, 1.0f };

		mtl_render_encoder_set_fragment_bytes(encoder, 0, &solid, sizeof(solid));
	}
	mtl_render_encoder_draw_primitives(encoder, MTLPrimitiveTypeTriangleStrip,
	    0, 4);

	mtl_render_encoder_set_depth_state(encoder, MTLCompareFunctionLess, 1);

	/* wireframe outline of the big triangle, alpha blended */
	mtl_render_encoder_set_pipeline(encoder, s->pipeline_blend);
	mtl_render_encoder_set_vertex_buffer(encoder, 0, s->wireframe, 0);
	mtl_render_encoder_set_fragment_bytes(encoder, 0, &wire_color,
	    sizeof(wire_color));
	mtl_render_encoder_draw_primitives(encoder, MTLPrimitiveTypeLineStrip, 0, 4);

	/* fat points along the top */
	mtl_render_encoder_set_pipeline(encoder, s->pipeline_point);
	mtl_render_encoder_set_vertex_buffer(encoder, 0, s->points, 0);
	{
		mtl_float4 point_color = { 0.55f, 1.0f, 0.75f, 1.0f };

		mtl_render_encoder_set_fragment_bytes(encoder, 0, &point_color,
		    sizeof(point_color));
	}
	mtl_render_encoder_draw_primitives(encoder, MTLPrimitiveTypePoint, 0, 3);

	mtl_render_encoder_end_encoding(encoder);
	mtl_command_buffer_present(command_buffer, s->drawable);
	mtl_command_buffer_commit(command_buffer);
	mtl_command_buffer_wait_until_completed(command_buffer);
}

/* ------------------------------------------------------------------------- */
/* main                                                                      */
/* ------------------------------------------------------------------------- */

int
main(int argc, char **argv)
{
	const char *directory = argc > 1 ? argv[1] : "build";
	mtl_device_t *device;
	mtl_texture_descriptor color_descriptor, depth_descriptor;
	mtl_library_t *library;
	mtl_render_pipeline_descriptor pipeline_descriptor;
	mtl_sampler_descriptor sampler_descriptor;
	mtl_command_queue_t *queue;
	scene s;
	present_context present;
	mtl_device_statistics statistics;
	int frame;

	memset(&s, 0, sizeof(s));

	device = MTLCreateSystemDefaultDevice();
	if (device == NULL) {
		fprintf(stderr, "no device\n");
		return 1;
	}
	printf("device: %s\n", mtl_device_name(device));

	/* the drawable: BGRA8, the scanout layout, so presenting is a row copy */
	color_descriptor.pixel_format = MTLPixelFormatBGRA8Unorm;
	color_descriptor.width = FRAME_WIDTH;
	color_descriptor.height = FRAME_HEIGHT;
	color_descriptor.depth = 1;
	color_descriptor.usage = MTLTextureUsageRenderTarget |
	    MTLTextureUsageShaderRead;
	color_descriptor.storage_mode = MTLResourceStorageModeShared;
	s.drawable = mtl_device_new_texture(device, &color_descriptor);

	depth_descriptor = color_descriptor;
	depth_descriptor.pixel_format = MTLPixelFormatDepth32Float;
	depth_descriptor.usage = MTLTextureUsageRenderTarget;
	s.depth = mtl_device_new_texture(device, &depth_descriptor);

	s.checker = make_checkerboard(device);
	s.dot = make_dot(device);

	library = mtl_device_new_library(device);
	mtl_library_add_function(library, "vs_colored", vs_colored);
	mtl_library_add_function(library, "vs_position", vs_position);
	mtl_library_add_function(library, "vs_point", vs_point);
	mtl_library_add_function(library, "vs_textured", vs_textured);
	mtl_library_add_fragment_function(library, "fs_varying", fs_varying);
	mtl_library_add_fragment_function(library, "fs_solid", fs_solid);
	mtl_library_add_fragment_function(library, "fs_textured", fs_textured);

	memset(&pipeline_descriptor, 0, sizeof(pipeline_descriptor));
	pipeline_descriptor.color_attachment_format = MTLPixelFormatBGRA8Unorm;
	pipeline_descriptor.depth_attachment_format = MTLPixelFormatDepth32Float;
	pipeline_descriptor.color_attachment.write_mask = MTLColorWriteMaskAll;

	pipeline_descriptor.vertex_function = mtl_library_vertex_function(library,
	    "vs_colored");
	pipeline_descriptor.fragment_function = mtl_library_fragment_function(library,
	    "fs_varying");
	s.pipeline_varying = mtl_device_new_render_pipeline(device,
	    &pipeline_descriptor);

	pipeline_descriptor.vertex_function = mtl_library_vertex_function(library,
	    "vs_position");
	pipeline_descriptor.fragment_function = mtl_library_fragment_function(library,
	    "fs_solid");
	s.pipeline_solid = mtl_device_new_render_pipeline(device,
	    &pipeline_descriptor);

	pipeline_descriptor.vertex_function = mtl_library_vertex_function(library,
	    "vs_point");
	s.pipeline_point = mtl_device_new_render_pipeline(device,
	    &pipeline_descriptor);

	pipeline_descriptor.color_attachment.blending_enabled = 1;
	pipeline_descriptor.color_attachment.source_rgb = MTLBlendFactorSourceAlpha;
	pipeline_descriptor.color_attachment.destination_rgb =
	    MTLBlendFactorOneMinusSourceAlpha;
	pipeline_descriptor.color_attachment.operation_rgb = MTLBlendOperationAdd;
	pipeline_descriptor.color_attachment.source_alpha = MTLBlendFactorOne;
	pipeline_descriptor.color_attachment.destination_alpha =
	    MTLBlendFactorOneMinusSourceAlpha;
	pipeline_descriptor.color_attachment.operation_alpha = MTLBlendOperationAdd;
	s.pipeline_blend = mtl_device_new_render_pipeline(device,
	    &pipeline_descriptor);

	pipeline_descriptor.vertex_function = mtl_library_vertex_function(library,
	    "vs_textured");
	pipeline_descriptor.fragment_function = mtl_library_fragment_function(library,
	    "fs_textured");
	s.pipeline_textured = mtl_device_new_render_pipeline(device,
	    &pipeline_descriptor);

	sampler_descriptor.min_filter = MTLSamplerMinMagFilterNearest;
	sampler_descriptor.mag_filter = MTLSamplerMinMagFilterNearest;
	sampler_descriptor.address_s = MTLSamplerAddressModeClampToEdge;
	sampler_descriptor.address_t = MTLSamplerAddressModeClampToEdge;
	s.sampler_nearest = mtl_device_new_sampler(device, &sampler_descriptor);

	sampler_descriptor.mag_filter = MTLSamplerMinMagFilterLinear;
	s.sampler_linear = mtl_device_new_sampler(device, &sampler_descriptor);

	/* geometry */
	s.background = mtl_device_new_buffer(device, 8 * 16);
	s.triangle = mtl_device_new_buffer(device, 6 * 16);
	s.quad_positions = mtl_device_new_buffer(device, 4 * 16);
	s.quad_positions_b = mtl_device_new_buffer(device, 4 * 16);
	s.quad_uvs = mtl_device_new_buffer(device, 4 * 8);
	s.depth_far = mtl_device_new_buffer(device, 3 * 16);
	s.depth_near = mtl_device_new_buffer(device, 3 * 16);
	s.wireframe = mtl_device_new_buffer(device, 4 * 16);
	s.points = mtl_device_new_buffer(device, 3 * 16);

	{
		/* texture coordinates for the quad strip corners, matching
		 * fill_rotated_quad's corner order (bottom-left, bottom-right,
		 * top-left, top-right) */
		float uv_values[4][2] = {
			{ 0.0f, 1.0f },
			{ 1.0f, 1.0f },
			{ 0.0f, 0.0f },
			{ 1.0f, 0.0f },
		};

		memcpy(mtl_buffer_contents(s.quad_uvs), uv_values,
		    sizeof(uv_values));
	}

	{
		float positions[4][4] = {
			{ -1.0f, -1.0f, 0.95f, 1.0f },
			{  1.0f, -1.0f, 0.95f, 1.0f },
			{ -1.0f,  1.0f, 0.95f, 1.0f },
			{  1.0f,  1.0f, 0.95f, 1.0f },
		};
		float colors[4][4] = {
			{ 0.06f, 0.06f, 0.08f, 1.0f },
			{ 0.06f, 0.06f, 0.08f, 1.0f },
			{ 0.21f, 0.21f, 0.24f, 1.0f },
			{ 0.21f, 0.21f, 0.24f, 1.0f },
		};
		float corners[4][2] = { { 0 } };

		fill_colored_quad(s.background, corners, positions, colors);
	}

	{
		float positions[3][4] = {
			{  0.05f,  0.82f, 0.50f, 1.0f },
			{ -0.98f, -0.50f, 0.50f, 1.0f },
			{  1.02f, -0.50f, 0.50f, 1.0f },
		};
		float colors[3][4] = {
			{ 0.30f, 0.85f, 1.00f, 1.0f },
			{ 1.00f, 0.25f, 0.55f, 1.0f },
			{ 1.00f, 0.85f, 0.20f, 1.0f },
		};
		unsigned char *bytes = (unsigned char *)mtl_buffer_contents(s.triangle);

		memcpy(bytes, positions, sizeof(positions));
		memcpy(bytes + 3 * 16, colors, sizeof(colors));
	}

	{
		float far_positions[3][4] = {
			{ -0.30f, -0.28f, 0.35f, 1.0f },
			{  0.34f, -0.28f, 0.35f, 1.0f },
			{  0.02f,  0.36f, 0.35f, 1.0f },
		};
		float near_positions[3][4] = {
			{ -0.38f, -0.38f, 0.25f, 1.0f },
			{  0.26f, -0.38f, 0.25f, 1.0f },
			{ -0.06f,  0.26f, 0.25f, 1.0f },
		};

		memcpy(mtl_buffer_contents(s.depth_far), far_positions,
		    sizeof(far_positions));
		memcpy(mtl_buffer_contents(s.depth_near), near_positions,
		    sizeof(near_positions));
	}

	{
		float positions[4][4] = {
			{  0.05f,  0.82f, 0.05f, 1.0f },
			{ -0.98f, -0.50f, 0.05f, 1.0f },
			{  1.02f, -0.50f, 0.05f, 1.0f },
			{  0.05f,  0.82f, 0.05f, 1.0f },
		};
		memcpy(mtl_buffer_contents(s.wireframe), positions, sizeof(positions));
	}

	{
		float positions[3][4] = {
			{ -0.45f, 0.20f, 0.02f, 1.0f },
			{  0.00f, 0.34f, 0.02f, 1.0f },
			{  0.45f, 0.20f, 0.02f, 1.0f },
		};
		memcpy(mtl_buffer_contents(s.points), positions, sizeof(positions));
	}

	queue = mtl_device_new_command_queue(device);

	present.directory = directory;
	present.frame = 0;
	mtl_set_present_handler(on_present, &present);

	for (frame = 0; frame < 2; frame++) {
		present.frame = frame;
		encode_frame(&s, queue, frame == 0 ? 0.0f : 0.30f);
	}

	mtl_device_get_statistics(device, &statistics);

	printf("draw calls %lu, triangles %lu, fragments %lu\n",
	    statistics.draw_calls, statistics.triangles, statistics.fragments);
	printf("frames presented %lu\n", statistics.frames_presented);
	return 0;
}
