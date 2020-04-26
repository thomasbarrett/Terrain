#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>
#import <Metal/Metal.h>
#import "ShaderTypes.h"
#include "GameEngine.h"
#include "linalg.h"


@interface Renderer: NSResponder <MTKViewDelegate> {
@public 
    PlayerCamera playerCamera;
}

@property (nonatomic, strong) MTKView *view;
@property (nonatomic, strong) id<MTLDevice> device;
@property (nonatomic, strong) id<MTLRenderPipelineState> pipelineState;
@property (nonatomic, strong) id<MTLCommandQueue> commandQueue;
@property (nonatomic, strong) id<MTLTexture> texture;

@property (nonatomic) simd::uint2 viewportSize;
@property (nonatomic) unsigned long tick;

- (instancetype) initWithView: (MTKView *) view device: (id<MTLDevice>) device;
- (void) mtkView: (MTKView *) view drawableSizeWillChange: (CGSize) size;
- (void) drawInMTKView: (MTKView *) view;

@end

@implementation Renderer

id<MTLDepthStencilState> _depthState;

- (instancetype)initWithView:(MTKView *)view device:(id<MTLDevice>)device {
    self = [super init];
    if (self) {
        self.view = view;
        self.device = device;
        self.tick = 0;
        self->playerCamera.moveUp(Biome::SEA_LEVEL + 2);
        NSBundle* mainBundle = [NSBundle mainBundle];
        NSURL* blocksTextureURL = [mainBundle URLForImageResource:@"blocks.png"];

        // Load the default texture
        MTKTextureLoader *loader = [[MTKTextureLoader alloc] initWithDevice: device];
        self.texture  = [loader newTextureWithContentsOfURL: blocksTextureURL options: @{
            MTKTextureLoaderOptionGenerateMipmaps : @true,
        }  error:nil];

        if(!_texture) {
            NSLog(@"Failed to create the texture from %@", blocksTextureURL.absoluteString);
            return nil;
        }

        _viewportSize.x = _view.frame.size.width;
        _viewportSize.y = _view.frame.size.height;
        NSError *error = NULL;


        self.view.clearColor = MTLClearColorMake(1, 1, 1, 1);
        self.view.depthStencilPixelFormat = MTLPixelFormatDepth32Float;

        // Load all the shader files with a .metal file extension in the project.
        id<MTLLibrary> defaultLibrary = [_device newDefaultLibrary];
        id<MTLFunction> vertexFunction = [defaultLibrary newFunctionWithName:@"vertexShader"];
        id<MTLFunction> fragmentFunction = [defaultLibrary newFunctionWithName:@"fragmentShader"];

        // Configure a pipeline descriptor that is used to create a pipeline state.
        MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipelineStateDescriptor.label = @"Render Pipeline";
        pipelineStateDescriptor.sampleCount = self.view.sampleCount;
        pipelineStateDescriptor.vertexFunction = vertexFunction;
        pipelineStateDescriptor.fragmentFunction = fragmentFunction;
        pipelineStateDescriptor.colorAttachments[0].pixelFormat = self.view.colorPixelFormat;
        pipelineStateDescriptor.depthAttachmentPixelFormat = self.view.depthStencilPixelFormat;
        pipelineStateDescriptor.vertexBuffers[0].mutability = MTLMutabilityImmutable;
        
        _pipelineState = [_device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
        NSAssert(_pipelineState, @"Failed to create pipeline state: %@", error);

        MTLDepthStencilDescriptor *depthDescriptor = [MTLDepthStencilDescriptor new];
        depthDescriptor.depthCompareFunction = MTLCompareFunctionLess;
        depthDescriptor.depthWriteEnabled = YES;
        _depthState = [_device newDepthStencilStateWithDescriptor:depthDescriptor];
        
        _commandQueue = [_device newCommandQueue];
    }
    return self;
}

- (void) mtkView: (MTKView *) view drawableSizeWillChange: (CGSize) size {
    _viewportSize.x = size.width;
    _viewportSize.y = size.height;
}


- (void)mouseMoved:(NSEvent *)theEvent {
    self->playerCamera.rotateTheta(theEvent.deltaX);
}

- (void)drawInMTKView: (MTKView *) view {

    static GameEngine engine(_device, [=](const std::vector<NativeBuffer> &buffers) {
        self.tick += 1;
        id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
        MTLRenderPassDescriptor *renderPassDescriptor = view.currentRenderPassDescriptor;

        float aspect = _viewportSize.x / _viewportSize.y;
        float fov = (2 * M_PI) / 5;
        float near = 0.01;
        float far = 1000;

        matrix_float4x4 cameraPerspective = matrix_projection(fov, aspect, near, far);
        matrix_float4x4 cameraTranslation = matrix_from_translation(-self->playerCamera.x(), -self->playerCamera.z(), -self->playerCamera.y());
        matrix_float4x4 cameraRotation = matrix_from_rotation(-self->playerCamera.theta() * M_PI / 180, 0, 1, 0);
        matrix_float4x4 camera;

        camera = simd_mul(cameraRotation, cameraTranslation);
        camera = simd_mul(cameraPerspective, camera);

        matrix_float4x4 mvpMatrix = matrix_from_translation(0, 0, 0);

        if(renderPassDescriptor != nil) {
            
            id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];

            for (const NativeBuffer &buffer: buffers) {
                [renderEncoder setViewport:(MTLViewport){0.0, 0.0, (double) _viewportSize.x, (double) _viewportSize.y, 0.0, 1.0 }];
                [renderEncoder setRenderPipelineState: _pipelineState];
                [renderEncoder setCullMode: MTLCullModeBack];
                [renderEncoder setFragmentTexture:_texture atIndex:0];
                [renderEncoder setDepthStencilState: _depthState];

                [renderEncoder 
                    setVertexBuffer: buffer.data()
                    offset: 0
                    atIndex: 0
                ];
                
                [renderEncoder
                    setVertexBytes:&mvpMatrix
                    length:sizeof(matrix_float4x4)
                    atIndex: 2
                ];

                [renderEncoder
                    setVertexBytes:&camera
                    length:sizeof(matrix_float4x4)
                    atIndex: 3
                ];
                
                [renderEncoder drawPrimitives: MTLPrimitiveTypeTriangle vertexStart:0 vertexCount: buffer.size()];
                
            }
            
            [renderEncoder endEncoding];
            [commandBuffer presentDrawable:view.currentDrawable];
        }
        [commandBuffer commit];
    });

    engine.render();  
}

@end 


@interface ViewController: NSViewController 

@property (nonatomic, strong) Renderer *renderer;

@end

@implementation ViewController 

- (void)mouseMoved:(NSEvent *)theEvent {
    [self.renderer mouseMoved: theEvent];
}

- (void)mouseDown:(NSEvent *)theEvent {
    [NSCursor hide];
    self.renderer.view.paused = NO;
}

- (void) keyDown:(NSEvent *)theEvent {
    NSLog(@"Key Pressed");
    NSString *characters = [theEvent characters];
    NSString *firstLetter = [characters substringToIndex:1];

    if ([firstLetter isEqualToString: @"w"]) {
        self.renderer->playerCamera.moveForwards(1.0);
    } else if ([firstLetter isEqualToString: @"a"]) {
        self.renderer->playerCamera.moveLeft(1.0);
    } else if ([firstLetter isEqualToString: @"s"]) {
        self.renderer->playerCamera.moveBackwards(1.0);
    } else if ([firstLetter isEqualToString: @"d"]) {
        self.renderer->playerCamera.moveRight(1.0);
    } else if ([firstLetter isEqualToString: @"z"]) {
        self.renderer->playerCamera.moveUp(1.0);
    } else if ([firstLetter isEqualToString: @"x"]) {
        self.renderer->playerCamera.moveDown(1.0);
    } else {
        [super keyDown:theEvent];
    }
}
- (void)cancelOperation:(id)sender {
    [NSCursor unhide];
    self.renderer.view.paused = YES;
}

- (void)viewDidLoad {
    [NSCursor hide];
}

- (void)loadView {
    NSRect bounds = NSMakeRect(0.0, 0.0, 800.0, 600.0);
    MTKView *view;
    view = [[MTKView alloc] init];
    NSRect bounds2 = [view convertRectToBacking: bounds];
    view.frame = bounds2;
    id<MTLDevice> device =  MTLCreateSystemDefaultDevice();
    self.renderer = [[Renderer alloc] initWithView: view device: device];
    view.delegate = _renderer;
    view.device = device;
    self.view = view;
}
@end

@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate> {
    NSWindow *window;
}

@end

@implementation AppDelegate


- (id) init {
    if (self = [super init]) {
        NSRect graphicsRect = NSMakeRect(0.0, 0.0, 800.0, 600.0);
       
        window = [ [NSWindow alloc]
                initWithContentRect: graphicsRect
                            styleMask:NSWindowStyleMaskTitled 
                                        |NSWindowStyleMaskClosable 
                                        |NSWindowStyleMaskMiniaturizable
                                        |NSWindowStyleMaskResizable
                            backing:NSBackingStoreBuffered
                                defer:NO ];

        [window setTitle:@"Tiny Application Window"];
        [window setAcceptsMouseMovedEvents:YES];

        ViewController *rootViewController = [[ViewController alloc] init];
        window.contentViewController = rootViewController;
        [window makeFirstResponder: rootViewController];
    }
    return self;
}

- (void)applicationWillFinishLaunching:(NSNotification *)notification {
    [window makeKeyAndOrderFront:self];
}


- (void)dealloc {
    [window release];
    [super dealloc];
}

@end

int main() {
    @autoreleasepool {
        NSApp = [NSApplication sharedApplication];
        AppDelegate *appDelegate = [[AppDelegate alloc] init];
        [NSApp setDelegate:appDelegate];
        id applicationMenuBar = [NSMenu new];
        id appMenuItem        = [NSMenuItem new];
        [applicationMenuBar addItem:appMenuItem];
        [NSApp setMainMenu: applicationMenuBar];
        [NSApp run];
    }
    return 0;
}