/*
 * mtl_core.c - ravynOS Metal: devices, resources, samplers, pipelines.
 *
 * The software device owns plain blocks of memory.  Nothing here knows about
 * pixels beyond the texel store layout, which is BGRA8 - the same layout the
 * scanout framebuffer uses, so a rendered drawable can be presented with a row
 * copy.
 */

#include "mtl_internal.h"

/* ------------------------------------------------------------------------- */
/* helpers                                                                   */
/* ------------------------------------------------------------------------- */

static char *
mtl_strdup(const char *text)
{
	size_t length;
	char *copy;

	if (text == NULL) {
		return NULL;
	}
	length = strlen(text) + 1;
	copy = (char *)malloc(length);
	if (copy != NULL) {
		memcpy(copy, text, length);
	}
	return copy;
}

/* ------------------------------------------------------------------------- */
/* device                                                                    */
/* ------------------------------------------------------------------------- */

mtl_device_t *
mtl_device_create(void)
{
	mtl_device_t *device = (mtl_device_t *)calloc(1, sizeof(*device));

	if (device == NULL) {
		return NULL;
	}

	/* Everything the device needs is a linear framebuffer somewhere, which is
	 * what makes this backend work on every video card. */
	snprintf(device->name, sizeof(device->name),
	    "ravynOS Software Metal Device");
	return device;
}

const char *
mtl_device_name(mtl_device_t *device)
{
	return device != NULL ? device->name : "";
}

void
mtl_device_release(mtl_device_t *device)
{
	free(device);
}

void
mtl_device_get_statistics(mtl_device_t *device, mtl_device_statistics *statistics)
{
	if (statistics == NULL) {
		return;
	}
	if (device == NULL) {
		memset(statistics, 0, sizeof(*statistics));
		return;
	}
	statistics->draw_calls = device->draw_calls;
	statistics->triangles = device->triangles;
	statistics->fragments = device->fragments;
	statistics->frames_presented = device->frames_presented;
}

/* ------------------------------------------------------------------------- */
/* buffers                                                                   */
/* ------------------------------------------------------------------------- */

mtl_buffer_t *
mtl_device_new_buffer(mtl_device_t *device, mtl_size_t length)
{
	mtl_buffer_t *buffer;

	(void)device;

	if (length == 0) {
		length = 1;
	}

	buffer = (mtl_buffer_t *)calloc(1, sizeof(*buffer));
	if (buffer == NULL) {
		return NULL;
	}
	buffer->length = length;
	buffer->bytes = (unsigned char *)calloc(1, (size_t)length);
	if (buffer->bytes == NULL) {
		free(buffer);
		return NULL;
	}
	return buffer;
}

void *
mtl_buffer_contents(mtl_buffer_t *buffer)
{
	return buffer != NULL ? buffer->bytes : NULL;
}

mtl_size_t
mtl_buffer_length(mtl_buffer_t *buffer)
{
	return buffer != NULL ? buffer->length : 0;
}

/* ------------------------------------------------------------------------- */
/* textures                                                                  */
/* ------------------------------------------------------------------------- */

static uint32_t
pack_rgba8_to_bgra8(const unsigned char *bytes)
{
	return ((uint32_t)bytes[3] << 24) | ((uint32_t)bytes[0] << 16) |
	    ((uint32_t)bytes[1] << 8) | (uint32_t)bytes[2];
}

static void
unpack_bgra8_to_rgba8(uint32_t texel_value, unsigned char *bytes)
{
	bytes[0] = (unsigned char)((texel_value >> 16) & 0xff);
	bytes[1] = (unsigned char)((texel_value >> 8) & 0xff);
	bytes[2] = (unsigned char)(texel_value & 0xff);
	bytes[3] = (unsigned char)((texel_value >> 24) & 0xff);
}

mtl_texture_t *
mtl_device_new_texture(mtl_device_t *device, const mtl_texture_descriptor *descriptor)
{
	mtl_texture_t *texture;

	(void)device;

	if (descriptor == NULL || descriptor->width == 0 || descriptor->height == 0) {
		return NULL;
	}

	texture = (mtl_texture_t *)calloc(1, sizeof(*texture));
	if (texture == NULL) {
		return NULL;
	}

	texture->format = descriptor->pixel_format == MTLPixelFormatInvalid ?
	    MTLPixelFormatBGRA8Unorm : descriptor->pixel_format;
	texture->width = descriptor->width;
	texture->height = descriptor->height;
	texture->depth = descriptor->depth == 0 ? 1 : descriptor->depth;
	texture->usage = descriptor->usage;

	if (texture->format == MTLPixelFormatDepth32Float) {
		texture->depth_values = (float *)calloc(
		    (size_t)texture->width * (size_t)texture->height, sizeof(float));
		if (texture->depth_values == NULL) {
			free(texture);
			return NULL;
		}
	} else {
		texture->pixels = (uint32_t *)calloc(
		    (size_t)texture->width * (size_t)texture->height, sizeof(uint32_t));
		if (texture->pixels == NULL) {
			free(texture);
			return NULL;
		}
	}

	return texture;
}

void *
mtl_texture_contents(mtl_texture_t *texture)
{
	if (texture == NULL) {
		return NULL;
	}
	return texture->pixels != NULL ? (void *)texture->pixels :
	    (void *)texture->depth_values;
}

void
mtl_texture_set_backing(mtl_texture_t *texture, void *pixels)
{
	if (texture != NULL) {
		if (texture->pixels == (uint32_t *)pixels) {
			return;
		}
		if (texture->pixels != NULL && !texture->is_external) {
			free(texture->pixels);
		}
		texture->pixels = (uint32_t *)pixels;
		texture->is_external = 1;
	}
}

MTLPixelFormat
mtl_texture_pixel_format(mtl_texture_t *texture)
{
	return texture != NULL ? texture->format : MTLPixelFormatInvalid;
}

void
mtl_texture_replace_region(mtl_texture_t *texture, MTLRegion region,
    const void *bytes, mtl_size_t bytes_per_row)
{
	const unsigned char *source = (const unsigned char *)bytes;
	mtl_size_t row, column;

	if (texture == NULL || texture->pixels == NULL || source == NULL) {
		return;
	}

	for (row = 0; row < region.size.height; row++) {
		mtl_size_t destination_y = region.origin.y + row;

		if (destination_y >= texture->height) {
			break;
		}
		for (column = 0; column < region.size.width; column++) {
			mtl_size_t destination_x = region.origin.x + column;
			const unsigned char *texel_bytes =
			    source + row * bytes_per_row + column * 4;
			uint32_t packed;

			if (destination_x >= texture->width) {
				break;
			}

			switch (texture->format) {
			case MTLPixelFormatRGBA8Unorm:
			case MTLPixelFormatRGBA8Unorm_sRGB:
				packed = pack_rgba8_to_bgra8(texel_bytes);
				break;
			case MTLPixelFormatR8Unorm:
			case MTLPixelFormatA8Unorm:
				packed = ((uint32_t)255 << 24) |
				    ((uint32_t)texel_bytes[0] << 16) |
				    ((uint32_t)texel_bytes[0] << 8) |
				    (uint32_t)texel_bytes[0];
				break;
			case MTLPixelFormatBGRA8Unorm:
			case MTLPixelFormatBGRA8Unorm_sRGB:
			default:
				packed = (uint32_t)texel_bytes[0] |
				    ((uint32_t)texel_bytes[1] << 8) |
				    ((uint32_t)texel_bytes[2] << 16) |
				    ((uint32_t)texel_bytes[3] << 24);
				break;
			}

			texture->pixels[destination_y * texture->width + destination_x] =
			    packed;
		}
	}
}

void
mtl_texture_get_bytes(mtl_texture_t *texture, MTLRegion region, void *bytes,
    mtl_size_t bytes_per_row)
{
	unsigned char *destination = (unsigned char *)bytes;
	mtl_size_t row, column;

	if (texture == NULL || texture->pixels == NULL || destination == NULL) {
		return;
	}

	for (row = 0; row < region.size.height; row++) {
		mtl_size_t source_y = region.origin.y + row;

		if (source_y >= texture->height) {
			break;
		}
		for (column = 0; column < region.size.width; column++) {
			mtl_size_t source_x = region.origin.x + column;
			unsigned char *texel_bytes =
			    destination + row * bytes_per_row + column * 4;
			uint32_t packed;

			if (source_x >= texture->width) {
				break;
			}

			packed = texture->pixels[source_y * texture->width + source_x];

			switch (texture->format) {
			case MTLPixelFormatRGBA8Unorm:
			case MTLPixelFormatRGBA8Unorm_sRGB:
				unpack_bgra8_to_rgba8(packed, texel_bytes);
				break;
			case MTLPixelFormatR8Unorm:
			case MTLPixelFormatA8Unorm:
				texel_bytes[0] = (unsigned char)((packed >> 16) & 0xff);
				break;
			default:
				texel_bytes[0] = (unsigned char)(packed & 0xff);
				texel_bytes[1] = (unsigned char)((packed >> 8) & 0xff);
				texel_bytes[2] = (unsigned char)((packed >> 16) & 0xff);
				texel_bytes[3] = (unsigned char)((packed >> 24) & 0xff);
				break;
			}
		}
	}
}

/* ------------------------------------------------------------------------- */
/* samplers                                                                  */
/* ------------------------------------------------------------------------- */

mtl_sampler_t *
mtl_device_new_sampler(mtl_device_t *device, const mtl_sampler_descriptor *descriptor)
{
	mtl_sampler_t *sampler = (mtl_sampler_t *)calloc(1, sizeof(*sampler));

	(void)device;

	if (sampler == NULL) {
		return NULL;
	}

	if (descriptor != NULL) {
		sampler->min_filter = descriptor->min_filter;
		sampler->mag_filter = descriptor->mag_filter;
		sampler->address_s = descriptor->address_s;
		sampler->address_t = descriptor->address_t;
	} else {
		sampler->min_filter = MTLSamplerMinMagFilterNearest;
		sampler->mag_filter = MTLSamplerMinMagFilterNearest;
		sampler->address_s = MTLSamplerAddressModeClampToEdge;
		sampler->address_t = MTLSamplerAddressModeClampToEdge;
	}
	return sampler;
}

/* ------------------------------------------------------------------------- */
/* libraries                                                                 */
/* ------------------------------------------------------------------------- */

mtl_library_t *
mtl_device_new_library(mtl_device_t *device)
{
	(void)device;
	return (mtl_library_t *)calloc(1, sizeof(mtl_library_t));
}

static void
library_add(mtl_library_t *library, const char *name, mtl_vertex_function vertex,
    mtl_fragment_function fragment)
{
	if (library == NULL || name == NULL ||
	    library->count >= MTL_LIBRARY_MAX_ENTRIES) {
		return;
	}
	library->entries[library->count].name = mtl_strdup(name);
	library->entries[library->count].vertex = vertex;
	library->entries[library->count].fragment = fragment;
	library->count++;
}

void
mtl_library_add_function(mtl_library_t *library, const char *name,
    mtl_vertex_function function)
{
	library_add(library, name, function, NULL);
}

void
mtl_library_add_fragment_function(mtl_library_t *library, const char *name,
    mtl_fragment_function function)
{
	library_add(library, name, NULL, function);
}

static int
library_find(mtl_library_t *library, const char *name)
{
	unsigned int i;

	if (library == NULL || name == NULL) {
		return -1;
	}
	for (i = 0; i < library->count; i++) {
		if (library->entries[i].name != NULL &&
		    strcmp(library->entries[i].name, name) == 0) {
			return (int)i;
		}
	}
	return -1;
}

mtl_vertex_function
mtl_library_vertex_function(mtl_library_t *library, const char *name)
{
	int index = library_find(library, name);

	if (index < 0) {
		return NULL;
	}
	return library->entries[index].vertex;
}

mtl_fragment_function
mtl_library_fragment_function(mtl_library_t *library, const char *name)
{
	int index = library_find(library, name);

	if (index < 0) {
		return NULL;
	}
	return library->entries[index].fragment;
}

/* ------------------------------------------------------------------------- */
/* render pipelines                                                          */
/* ------------------------------------------------------------------------- */

mtl_render_pipeline_t *
mtl_device_new_render_pipeline(mtl_device_t *device,
    const mtl_render_pipeline_descriptor *descriptor)
{
	mtl_render_pipeline_t *pipeline;

	(void)device;

	if (descriptor == NULL) {
		return NULL;
	}

	pipeline = (mtl_render_pipeline_t *)calloc(1, sizeof(*pipeline));
	if (pipeline == NULL) {
		return NULL;
	}

	pipeline->vertex_function = descriptor->vertex_function;
	pipeline->fragment_function = descriptor->fragment_function;
	pipeline->color_format = descriptor->color_attachment_format ==
	    MTLPixelFormatInvalid ? MTLPixelFormatBGRA8Unorm :
	    descriptor->color_attachment_format;
	pipeline->depth_format = descriptor->depth_attachment_format;

	pipeline->blending_enabled = descriptor->color_attachment.blending_enabled;
	if (pipeline->blending_enabled) {
		pipeline->source_rgb = descriptor->color_attachment.source_rgb;
		pipeline->destination_rgb = descriptor->color_attachment.destination_rgb;
		pipeline->operation_rgb = descriptor->color_attachment.operation_rgb;
		pipeline->source_alpha = descriptor->color_attachment.source_alpha;
		pipeline->destination_alpha = descriptor->color_attachment.destination_alpha;
		pipeline->operation_alpha = descriptor->color_attachment.operation_alpha;
	}

	/* A zeroed descriptor means "no masking", which is what ported code
	 * written against the C core will have produced. */
	pipeline->write_mask = descriptor->color_attachment.write_mask == 0 ?
	    MTLColorWriteMaskAll : descriptor->color_attachment.write_mask;

	return pipeline;
}

void
mtl_render_pipeline_release(mtl_render_pipeline_t *pipeline)
{
	free(pipeline);
}

void
mtl_buffer_release(mtl_buffer_t *buffer)
{
	if (buffer == NULL) return;
	free(buffer->bytes);
	free(buffer);
}

void
mtl_texture_release(mtl_texture_t *texture)
{
	if (texture == NULL) return;
	if (texture->pixels != NULL && !texture->is_external) {
		free(texture->pixels);
	}
	free(texture->depth_values);
	free(texture);
}

void
mtl_sampler_release(mtl_sampler_t *sampler)
{
	if (sampler == NULL) return;
	free(sampler);
}

void
mtl_library_release(mtl_library_t *library)
{
	if (library == NULL) return;
	for (unsigned int i = 0; i < library->count; i++) {
		free(library->entries[i].name);
	}
	free(library);
}

void
mtl_command_queue_release(mtl_command_queue_t *queue)
{
	if (queue == NULL) return;
	free(queue);
}

void
mtl_command_buffer_release(mtl_command_buffer_t *command_buffer)
{
	if (command_buffer == NULL) return;
	extern void release_commands(mtl_command_buffer_t *command_buffer);
	release_commands(command_buffer);
	free(command_buffer);
}
