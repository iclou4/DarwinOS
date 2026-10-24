/*
 * mtl_command.c - ravynOS Metal: command queues, command buffers, encoders.
 *
 * Metal separates recording from execution: encoders describe work, commit()
 * runs it.  This file keeps that split, so a future GPU backend can replay the
 * exact same recording on hardware instead of the rasterizer.
 */

#include "mtl_internal.h"

static mtl_present_handler g_present_handler;
static void           *g_present_context;

void
mtl_set_present_handler(mtl_present_handler handler, void *context)
{
	g_present_handler = handler;
	g_present_context = context;
}

/* ------------------------------------------------------------------------- */
/* command storage                                                           */
/* ------------------------------------------------------------------------- */

static mtl_command *
push_command(mtl_command_buffer_t *command_buffer, mtl_command_kind kind)
{
	mtl_command *command;

	if (command_buffer->count == command_buffer->capacity) {
		size_t capacity = command_buffer->capacity == 0 ? 64 :
		    command_buffer->capacity * 2;
		mtl_command *commands = (mtl_command *)realloc(command_buffer->commands,
		    capacity * sizeof(mtl_command));

		if (commands == NULL) {
			return NULL;
		}
		command_buffer->commands = commands;
		command_buffer->capacity = capacity;
	}

	command = &command_buffer->commands[command_buffer->count++];
	memset(command, 0, sizeof(*command));
	command->kind = kind;
	return command;
}

static void *
own_bytes(mtl_command_buffer_t *command_buffer, const void *bytes,
    mtl_size_t length)
{
	void *copy;
	size_t count = length == 0 ? 1 : (size_t)length;

	copy = malloc(count);
	if (copy == NULL) {
		return NULL;
	}
	if (bytes != NULL) {
		memcpy(copy, bytes, (size_t)length);
	} else {
		memset(copy, 0, count);
	}

	if (command_buffer->owned_count == command_buffer->owned_capacity) {
		size_t capacity = command_buffer->owned_capacity == 0 ? 16 :
		    command_buffer->owned_capacity * 2;
		void **owned = (void **)realloc(command_buffer->owned,
		    capacity * sizeof(void *));

		if (owned == NULL) {
			free(copy);
			return NULL;
		}
		command_buffer->owned = owned;
		command_buffer->owned_capacity = capacity;
	}

	command_buffer->owned[command_buffer->owned_count++] = copy;
	return copy;
}

void
release_commands(mtl_command_buffer_t *command_buffer)
{
	size_t i;

	for (i = 0; i < command_buffer->owned_count; i++) {
		free(command_buffer->owned[i]);
	}
	free(command_buffer->owned);
	command_buffer->owned = NULL;
	command_buffer->owned_count = 0;
	command_buffer->owned_capacity = 0;

	free(command_buffer->commands);
	command_buffer->commands = NULL;
	command_buffer->count = 0;
	command_buffer->capacity = 0;
}

/* ------------------------------------------------------------------------- */
/* queue and command buffer                                                  */
/* ------------------------------------------------------------------------- */

mtl_command_queue_t *
mtl_device_new_command_queue(mtl_device_t *device)
{
	mtl_command_queue_t *queue = (mtl_command_queue_t *)calloc(1, sizeof(*queue));

	if (queue != NULL) {
		queue->device = device;
	}
	return queue;
}

mtl_command_buffer_t *
mtl_command_queue_command_buffer(mtl_command_queue_t *queue)
{
	mtl_command_buffer_t *command_buffer =
	    (mtl_command_buffer_t *)calloc(1, sizeof(*command_buffer));

	if (command_buffer == NULL) {
		return NULL;
	}
	command_buffer->device = queue != NULL ? queue->device : NULL;
	command_buffer->completed = 0;
	return command_buffer;
}

/* ------------------------------------------------------------------------- */
/* encoder                                                                   */
/* ------------------------------------------------------------------------- */

mtl_render_encoder_t *
mtl_command_buffer_render_encoder(mtl_command_buffer_t *command_buffer,
    const mtl_render_pass_descriptor *descriptor)
{
	mtl_render_encoder_t *encoder;
	mtl_command *command;

	if (command_buffer == NULL || descriptor == NULL ||
	    descriptor->color_attachment == NULL) {
		return NULL;
	}
	if (descriptor->color_attachment->pixels == NULL &&
	    descriptor->color_attachment->depth_values == NULL) {
		return NULL;
	}

	command = push_command(command_buffer, MTL_CMD_BEGIN_PASS);
	if (command == NULL) {
		return NULL;
	}
	command->u.begin_pass.descriptor = *descriptor;

	encoder = (mtl_render_encoder_t *)calloc(1, sizeof(*encoder));
	if (encoder == NULL) {
		return NULL;
	}
	encoder->command_buffer = command_buffer;
	encoder->pass = *descriptor;
	return encoder;
}

static int
encoder_usable(mtl_render_encoder_t *encoder)
{
	return encoder != NULL && !encoder->ended &&
	    encoder->command_buffer != NULL &&
	    !encoder->command_buffer->committed;
}

void
mtl_render_encoder_set_pipeline(mtl_render_encoder_t *encoder,
    mtl_render_pipeline_t *pipeline)
{
	mtl_command *command;

	if (!encoder_usable(encoder)) {
		return;
	}
	command = push_command(encoder->command_buffer, MTL_CMD_SET_PIPELINE);
	if (command != NULL) {
		command->u.set_pipeline.pipeline = pipeline;
	}
}

static void
set_buffer(mtl_render_encoder_t *encoder, unsigned int index, mtl_command_kind kind,
    mtl_buffer_t *buffer, const void *bytes, mtl_size_t offset, mtl_size_t length)
{
	mtl_command *command;

	if (!encoder_usable(encoder) || index >= MTL_MAX_SLOTS) {
		return;
	}
	command = push_command(encoder->command_buffer, kind);
	if (command == NULL) {
		return;
	}
	command->u.set_buffer.index = index;
	command->u.set_buffer.offset = offset;
	command->u.set_buffer.length = length;
	if (kind == MTL_CMD_SET_VERTEX_BYTES || kind == MTL_CMD_SET_FRAGMENT_BYTES) {
		command->u.set_buffer.bytes = own_bytes(encoder->command_buffer, bytes,
		    length);
	} else {
		command->u.set_buffer.buffer = buffer;
	}
}

void
mtl_render_encoder_set_vertex_buffer(mtl_render_encoder_t *encoder,
    unsigned int index, mtl_buffer_t *buffer, mtl_size_t offset)
{
	set_buffer(encoder, index, MTL_CMD_SET_VERTEX_BUFFER, buffer, NULL, offset, 0);
}

void
mtl_render_encoder_set_vertex_bytes(mtl_render_encoder_t *encoder,
    unsigned int index, const void *bytes, mtl_size_t length)
{
	set_buffer(encoder, index, MTL_CMD_SET_VERTEX_BYTES, NULL, bytes, 0, length);
}

void
mtl_render_encoder_set_fragment_buffer(mtl_render_encoder_t *encoder,
    unsigned int index, mtl_buffer_t *buffer, mtl_size_t offset)
{
	set_buffer(encoder, index, MTL_CMD_SET_FRAGMENT_BUFFER, buffer, NULL, offset, 0);
}

void
mtl_render_encoder_set_fragment_bytes(mtl_render_encoder_t *encoder,
    unsigned int index, const void *bytes, mtl_size_t length)
{
	set_buffer(encoder, index, MTL_CMD_SET_FRAGMENT_BYTES, NULL, bytes, 0, length);
}

void
mtl_render_encoder_set_fragment_texture(mtl_render_encoder_t *encoder,
    unsigned int index, mtl_texture_t *texture)
{
	mtl_command *command;

	if (!encoder_usable(encoder) || index >= MTL_MAX_TEXTURES) {
		return;
	}
	command = push_command(encoder->command_buffer, MTL_CMD_SET_FRAGMENT_TEXTURE);
	if (command != NULL) {
		command->u.set_texture.index = index;
		command->u.set_texture.texture = texture;
	}
}

void
mtl_render_encoder_set_fragment_sampler_state(mtl_render_encoder_t *encoder,
    unsigned int index, mtl_sampler_t *sampler)
{
	mtl_command *command;

	if (!encoder_usable(encoder) || index >= MTL_MAX_TEXTURES) {
		return;
	}
	command = push_command(encoder->command_buffer, MTL_CMD_SET_FRAGMENT_SAMPLER);
	if (command != NULL) {
		command->u.set_sampler.index = index;
		command->u.set_sampler.sampler = sampler;
	}
}

void
mtl_render_encoder_set_viewport(mtl_render_encoder_t *encoder, float x, float y,
    float width, float height, float near, float far)
{
	mtl_command *command;

	if (!encoder_usable(encoder)) {
		return;
	}
	command = push_command(encoder->command_buffer, MTL_CMD_SET_VIEWPORT);
	if (command != NULL) {
		command->u.set_viewport.x = x;
		command->u.set_viewport.y = y;
		command->u.set_viewport.width = width;
		command->u.set_viewport.height = height;
		command->u.set_viewport.near = near;
		command->u.set_viewport.far = far;
	}
}

void
mtl_render_encoder_set_scissor_rect(mtl_render_encoder_t *encoder, long x,
    long y, long width, long height)
{
	mtl_command *command;

	if (!encoder_usable(encoder)) {
		return;
	}
	command = push_command(encoder->command_buffer, MTL_CMD_SET_SCISSOR);
	if (command != NULL) {
		command->u.set_scissor.x = x;
		command->u.set_scissor.y = y;
		command->u.set_scissor.width = width;
		command->u.set_scissor.height = height;
	}
}

void
mtl_render_encoder_set_cull_mode(mtl_render_encoder_t *encoder, MTLCullMode mode)
{
	mtl_command *command;

	if (!encoder_usable(encoder)) {
		return;
	}
	command = push_command(encoder->command_buffer, MTL_CMD_SET_CULL);
	if (command != NULL) {
		command->u.set_cull.mode = mode;
	}
}

void
mtl_render_encoder_set_front_facing_winding(mtl_render_encoder_t *encoder,
    MTLWinding winding)
{
	mtl_command *command;

	if (!encoder_usable(encoder)) {
		return;
	}
	command = push_command(encoder->command_buffer, MTL_CMD_SET_WINDING);
	if (command != NULL) {
		command->u.set_winding.winding = winding;
	}
}

void
mtl_render_encoder_set_depth_state(mtl_render_encoder_t *encoder,
    MTLCompareFunction compare, int write_enabled)
{
	mtl_command *command;

	if (!encoder_usable(encoder)) {
		return;
	}
	command = push_command(encoder->command_buffer, MTL_CMD_SET_DEPTH);
	if (command != NULL) {
		command->u.set_depth.compare = compare;
		command->u.set_depth.write_enabled = write_enabled;
	}
}

static void
draw(mtl_render_encoder_t *encoder, MTLPrimitiveType primitive_type,
    mtl_size_t vertex_start, mtl_size_t vertex_count, mtl_size_t instance_count)
{
	mtl_command *command;

	if (!encoder_usable(encoder)) {
		return;
	}
	command = push_command(encoder->command_buffer, MTL_CMD_DRAW);
	if (command != NULL) {
		command->u.draw.primitive_type = primitive_type;
		command->u.draw.vertex_start = vertex_start;
		command->u.draw.vertex_count = vertex_count;
		command->u.draw.instance_count = instance_count;
	}
}

void
mtl_render_encoder_draw_primitives(mtl_render_encoder_t *encoder,
    MTLPrimitiveType primitive_type, mtl_size_t vertex_start,
    mtl_size_t vertex_count)
{
	draw(encoder, primitive_type, vertex_start, vertex_count, 1);
}

void
mtl_render_encoder_draw_primitives_instanced(mtl_render_encoder_t *encoder,
    MTLPrimitiveType primitive_type, mtl_size_t vertex_start,
    mtl_size_t vertex_count, mtl_size_t instance_count)
{
	draw(encoder, primitive_type, vertex_start, vertex_count, instance_count);
}

void
mtl_render_encoder_end_encoding(mtl_render_encoder_t *encoder)
{
	if (!encoder_usable(encoder)) {
		return;
	}
	encoder->ended = 1;

	if (push_command(encoder->command_buffer, MTL_CMD_END_PASS) == NULL) {
		return;
	}
}

void
mtl_command_buffer_present(mtl_command_buffer_t *command_buffer,
    mtl_texture_t *drawable)
{
	mtl_command *command;

	if (command_buffer == NULL || command_buffer->committed) {
		return;
	}
	command = push_command(command_buffer, MTL_CMD_PRESENT);
	if (command != NULL) {
		command->u.present.drawable = drawable;
	}
}

/* ------------------------------------------------------------------------- */
/* execution                                                                 */
/* ------------------------------------------------------------------------- */

static void
begin_pass(mtl_render_state *state, const mtl_render_pass_descriptor *descriptor)
{
	memset(state, 0, sizeof(*state));

	state->color = descriptor->color_attachment;
	state->depth = descriptor->depth_attachment;

	state->viewport_x = 0.0f;
	state->viewport_y = 0.0f;
	state->viewport_width = state->color != NULL ?
	    (float)state->color->width : 0.0f;
	state->viewport_height = state->color != NULL ?
	    (float)state->color->height : 0.0f;
	state->viewport_near = 0.0f;
	state->viewport_far = 1.0f;
	state->scissor_set = 0;

	state->cull_mode = MTLCullModeNone;
	state->winding = MTLWindingClockwise;
	state->depth_compare = MTLCompareFunctionLess;
	state->depth_write = 1;

	if (state->color != NULL && descriptor->color_load_action ==
	    MTLLoadActionClear) {
		mtl_raster_clear_color(state->color, descriptor->clear_color);
	}
	if (state->depth != NULL && descriptor->depth_load_action ==
	    MTLLoadActionClear) {
		mtl_raster_clear_depth(state->depth, descriptor->clear_depth);
	}
}

static void
execute(mtl_command_buffer_t *command_buffer)
{
	mtl_render_state state;

	memset(&state, 0, sizeof(state));

	if (command_buffer->device != NULL) {
		for (size_t i = 0; i < command_buffer->count; i++) {
			mtl_command *command = &command_buffer->commands[i];

			switch (command->kind) {
			case MTL_CMD_BEGIN_PASS:
				begin_pass(&state, &command->u.begin_pass.descriptor);
				break;
			case MTL_CMD_END_PASS:
				break;
			case MTL_CMD_SET_PIPELINE:
				state.pipeline = command->u.set_pipeline.pipeline;
				break;
			case MTL_CMD_SET_VERTEX_BUFFER:
				state.vertex_slots[command->u.set_buffer.index] =
				    command->u.set_buffer.buffer != NULL ?
				    (const void *)(command->u.set_buffer.buffer->bytes +
				    command->u.set_buffer.offset) : NULL;
				break;
			case MTL_CMD_SET_VERTEX_BYTES:
				state.vertex_slots[command->u.set_buffer.index] =
				    command->u.set_buffer.bytes;
				break;
			case MTL_CMD_SET_FRAGMENT_BUFFER:
				state.fragment_slots[command->u.set_buffer.index] =
				    command->u.set_buffer.buffer != NULL ?
				    (const void *)(command->u.set_buffer.buffer->bytes +
				    command->u.set_buffer.offset) : NULL;
				break;
			case MTL_CMD_SET_FRAGMENT_BYTES:
				state.fragment_slots[command->u.set_buffer.index] =
				    command->u.set_buffer.bytes;
				break;
			case MTL_CMD_SET_FRAGMENT_TEXTURE:
				state.fragment_textures[command->u.set_texture.index] =
				    command->u.set_texture.texture;
				break;
			case MTL_CMD_SET_FRAGMENT_SAMPLER:
				state.fragment_samplers[command->u.set_sampler.index] =
				    command->u.set_sampler.sampler;
				break;
			case MTL_CMD_SET_VIEWPORT:
				state.viewport_x = command->u.set_viewport.x;
				state.viewport_y = command->u.set_viewport.y;
				state.viewport_width = command->u.set_viewport.width;
				state.viewport_height = command->u.set_viewport.height;
				state.viewport_near = command->u.set_viewport.near;
				state.viewport_far = command->u.set_viewport.far;
				break;
			case MTL_CMD_SET_SCISSOR:
				state.scissor_x = command->u.set_scissor.x;
				state.scissor_y = command->u.set_scissor.y;
				state.scissor_width = command->u.set_scissor.width;
				state.scissor_height = command->u.set_scissor.height;
				state.scissor_set = 1;
				break;
			case MTL_CMD_SET_CULL:
				state.cull_mode = command->u.set_cull.mode;
				break;
			case MTL_CMD_SET_WINDING:
				state.winding = command->u.set_winding.winding;
				break;
			case MTL_CMD_SET_DEPTH:
				state.depth_compare = command->u.set_depth.compare;
				state.depth_write = command->u.set_depth.write_enabled;
				break;
			case MTL_CMD_DRAW:
				mtl_raster_draw(&state, command->u.draw.primitive_type,
				    command->u.draw.vertex_start,
				    command->u.draw.vertex_count,
				    command->u.draw.instance_count);
				command_buffer->device->draw_calls++;
				command_buffer->device->triangles += state.triangles;
				command_buffer->device->fragments += state.fragments;
				state.triangles = 0;
				state.fragments = 0;
				break;
			case MTL_CMD_PRESENT:
				if (command->u.present.drawable != NULL) {
					if (g_present_handler != NULL) {
						g_present_handler(
						    command->u.present.drawable,
						    g_present_context);
					}
					command_buffer->device->frames_presented++;
				}
				break;
			}
		}
	}

	command_buffer->completed = 1;
	release_commands(command_buffer);
}

void
mtl_command_buffer_commit(mtl_command_buffer_t *command_buffer)
{
	if (command_buffer == NULL || command_buffer->committed) {
		return;
	}
	command_buffer->committed = 1;
	execute(command_buffer);
}

void
mtl_command_buffer_wait_until_completed(mtl_command_buffer_t *command_buffer)
{
	/* the software backend executes during commit() */
	if (command_buffer != NULL) {
		command_buffer->completed = 1;
	}
}
