/*
 * test_metal_cametal.m - End-to-end verification of Metal + QuartzCore + MSL
 */

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <QuartzCore/CATransaction.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct Vertex {
    float position[4];
    float color[4];
};

static void write_ppm(const char *filename, const uint32_t *pixels, int width, int height) {
    FILE *f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Could not open %s for writing\n", filename);
        return;
    }
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint32_t p = pixels[y * width + x];
            uint8_t b = (p >> 0) & 0xFF;
            uint8_t g = (p >> 8) & 0xFF;
            uint8_t r = (p >> 16) & 0xFF;
            fputc(r, f);
            fputc(g, f);
            fputc(b, f);
        }
    }
    fclose(f);
}

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        NSLog(@"=== Testing Apple 1:1 Metal & QuartzCore ===");

        // 1. Get default device (IOKit MTLIOAccelDevice / MTLSoftwareDevice)
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        if (!device) {
            NSLog(@"Failed to get default Metal device!");
            return 1;
        }
        NSLog(@"Device: %@", [device name]);
        NSLog(@"RegistryID: 0x%llx", [device registryID]);
        NSLog(@"Unified Memory: %d", [device hasUnifiedMemory]);

        // 2. Setup QuartzCore CAMetalLayer
        CAMetalLayer *metalLayer = [CAMetalLayer layer];
        [metalLayer setDevice:device];
        [metalLayer setPixelFormat:MTLPixelFormatBGRA8Unorm];
        [metalLayer setDrawableSize:CGSizeMake(1280, 800)];
        NSLog(@"Created CAMetalLayer with drawableSize 1280x800");

        // 3. Acquire next drawable from QuartzCore layer
        id<CAMetalDrawable> drawable = [metalLayer nextDrawable];
        if (!drawable) {
            NSLog(@"Failed to get next drawable from CAMetalLayer!");
            return 1;
        }
        NSLog(@"Acquired CAMetalDrawable ID: %lu, texture: %@", (unsigned long)[drawable drawableID], [drawable texture]);

        // 4. Create command queue
        id<MTLCommandQueue> commandQueue = [device newCommandQueue];
        if (!commandQueue) {
            NSLog(@"Failed to create command queue!");
            return 1;
        }

        // 5. MSL Shading Language shader source
        NSString *mslShader = @""
            "#include <metal_stdlib>\n"
            "using namespace metal;\n"
            "\n"
            "struct VertexInput {\n"
            "    float4 position;\n"
            "    float4 color;\n"
            "};\n"
            "\n"
            "struct VertexOutput {\n"
            "    float4 position [[position]];\n"
            "    float4 color;\n"
            "};\n"
            "\n"
            "vertex VertexOutput vertex_main(uint vid [[vertex_id]], constant VertexInput *vertices [[buffer(0)]]) {\n"
            "    VertexOutput out;\n"
            "    out.position = vertices[vid].position;\n"
            "    out.color = vertices[vid].color;\n"
            "    return out;\n"
            "}\n"
            "\n"
            "fragment float4 fragment_main(VertexOutput in [[stage_in]]) {\n"
            "    return in.color;\n"
            "}\n";

        NSError *error = nil;
        id<MTLLibrary> library = [device newLibraryWithSource:mslShader options:nil error:&error];
        if (!library) {
            NSLog(@"MSL compilation error: %@", error);
        } else {
            NSLog(@"MSL compilation successful! Available functions: %@", [library functionNames]);
        }

        // 6. Setup Vertex Buffer (smooth RGB triangle)
        struct Vertex triangleVertices[3] = {
            { {  0.0f,  0.65f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } }, // Top (Red)
            { { -0.65f, -0.65f, 0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } }, // Bottom-Left (Green)
            { {  0.65f, -0.65f, 0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }  // Bottom-Right (Blue)
        };
        id<MTLBuffer> vertexBuffer = [device newBufferWithBytes:triangleVertices length:sizeof(triangleVertices) options:MTLResourceStorageModeShared];

        // 7. Setup Pipeline State
        MTLRenderPipelineDescriptor *pipeDesc = [MTLRenderPipelineDescriptor new];
        pipeDesc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
        if (library) {
            pipeDesc.vertexFunction = [library newFunctionWithName:@"vertex_main"];
            pipeDesc.fragmentFunction = [library newFunctionWithName:@"fragment_main"];
        }
        id<MTLRenderPipelineState> pipelineState = [device newRenderPipelineStateWithDescriptor:pipeDesc error:&error];

        // 8. Render Pass Descriptor targeting CAMetalDrawable texture
        MTLRenderPassDescriptor *passDesc = [MTLRenderPassDescriptor renderPassDescriptor];
        passDesc.colorAttachments[0].texture = [drawable texture];
        passDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
        passDesc.colorAttachments[0].storeAction = MTLStoreActionStore;
        passDesc.colorAttachments[0].clearColor = MTLClearColorMake(0.08, 0.08, 0.12, 1.0); // Slate dark blue

        // 9. Command Buffer & Render Command Encoder
        id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
        id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:passDesc];

        [encoder setRenderPipelineState:pipelineState];
        [encoder setVertexBuffer:vertexBuffer offset:0 atIndex:0];
        [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
        [encoder endEncoding];

        // 10. Present via QuartzCore CAMetalDrawable & Commit
        [commandBuffer presentDrawable:drawable];
        [commandBuffer commit];
        [commandBuffer waitUntilCompleted];

        NSLog(@"Render & Presentation completed!");

        // 11. Read back pixels from drawable's texture to save verification image
        void *pixels = NULL;
        if ([[drawable texture] respondsToSelector:@selector(contents)]) {
            pixels = [((id)[drawable texture]) contents];
        }
        if (pixels) {
            write_ppm("/tmp/metal_cametal_out.ppm", (const uint32_t *)pixels, 1280, 800);
            NSLog(@"Saved /tmp/metal_cametal_out.ppm");
        } else {
            NSLog(@"Warning: drawable texture contents not accessible directly");
        }
    }
    return 0;
}
