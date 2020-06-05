#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>
#import <Metal/Metal.h>
#import <CoreVideo/CoreVideo.h>

#import "ShaderTypes.h"
#include "GameEngine.h"
#include "linalg.h"
#include <cairo/cairo.h>
#include <cairo/cairo-quartz.h>
#include <pango/pangocairo.h>

#include "gui.h"

@interface TestView : NSView {
    CVDisplayLinkRef displayLink;
}
@end

CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext);

@implementation TestView

- (id)initWithFrame:(NSRect)frameRect
{
	if ((self = [super initWithFrame:frameRect]) != nil)
	{
        self.layerContentsRedrawPolicy = NSViewLayerContentsRedrawOnSetNeedsDisplay;
		CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
        CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, self);
        CVDisplayLinkStart(displayLink);
	}
	return self;
}

- (BOOL) isOpaque {
	return NO;
}

- (void) drawRect: (NSRect) rect {

    static int x = 0;

	NSRect bounds = [self bounds];
	int width = bounds.size.width;
	int height = bounds.size.height;

    /*
	CGContextRef ctx = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
    cairo_surface_t *surface = cairo_quartz_surface_create_for_cg_context(ctx, width, height);
    cairo_t *cr = cairo_create(surface);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_translate(cr, 0, height);
    cairo_scale(cr, 1, -1.0);

    
    gui::Element::cr = cr;

    // gui::loading_bar(cr, width / 2, height / 2, 25.0, x / 180.0 * M_PI);
    gui::Element root(width, height);
    for (int i = 0; i < 5; i++) {
            gui::FlexLayout *child = new gui::FlexLayout();
            if (i % 2 == 0) {
                child->setPangoLayout("There once was a boy named Harry, destined to be a star. His parents were killed by voldemort, who gave him a lightning scar.");
            } else {
                for (int i = 0; i < 4; i++) {
                    gui::Element *child2 = new gui::Element(50, 50);
                    child2->style().padding = 5.0;
                    child2->style().borderWidth = 1.0;
                    child->style().color = {0.0, 0.0, 0.0, 1.0};
                    child2->style().borderColor = {235.0 / 255.0, 235.0 / 255.0, 235.0 / 255.0, 1.0};
                    child2->style().backgroundColor = {250.0 / 255.0, 100.0 / 255.0, 95.0 / 255.0, 1.0};
                    child->addChild(child2);
                }
                if (i == 0) child->setJustifyContent(gui::FlexLayout::JustifyContent::FLEX_START);
                if (i == 1) child->setJustifyContent(gui::FlexLayout::JustifyContent::FLEX_END);
                if (i == 2) child->setJustifyContent(gui::FlexLayout::JustifyContent::CENTER);
                if (i == 3) child->setJustifyContent(gui::FlexLayout::JustifyContent::SPACE_BETWEEN);
                if (i == 4) child->setJustifyContent(gui::FlexLayout::JustifyContent::SPACE_AROUND);
                if (i == 5) child->setJustifyContent(gui::FlexLayout::JustifyContent::SPACE_EVENLY);
                
            }
            
            child->style().color = {0.0, 0.0, 0.0, 1.0};
            child->style().backgroundColor = {1.0, 1.0, 1.0, 1.0};
            child->style().padding = 10;
            child->style().borderWidth = 1.0;
            child->style().borderColor = {235.0 / 255.0, 235.0 / 255.0, 235.0 / 255.0, 1.0};
            root.addChild(child);
        
    }
    root.x = 0;
    root.w = width;
    root.h = height;
    root.style().padding = 5.0;
    root.layoutChildren();
    root.draw(cr, 0, 0);
    

    */

    x += 2;
}

@end

CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext) {
    dispatch_async(dispatch_get_main_queue(), ^{
        TestView *view = ((TestView*) displayLinkContext);
        [view setNeedsDisplay: YES];
    });
    return kCVReturnSuccess;
}

@interface Renderer: NSResponder <MTKViewDelegate> {
@public 
    GameEngine gameEngine;
}

@property (nonatomic, strong) MTKView *view;
@property (nonatomic, strong) id<MTLDevice> device;
@property (nonatomic, strong) id<MTLRenderPipelineState> pipelineState;
@property (nonatomic, strong) id<MTLCommandQueue> commandQueue;
@property (nonatomic, strong) id<MTLTexture> texture;
@property (nonatomic, strong) TestView *overlay;

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
        self.overlay = [[TestView alloc] initWithFrame: self.view.frame];
        [self.view addSubview: self.overlay];
        self.device = device;
        self.tick = 0;
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


        self.view.clearColor = MTLClearColorMake(0.707, 0.8125, 0.957, 1);
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
        pipelineStateDescriptor.colorAttachments[0].blendingEnabled = YES;
        pipelineStateDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
        pipelineStateDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
        pipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
        pipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
        pipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        pipelineStateDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
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
    self.overlay.frame = self.view.frame;
}


- (void)mouseMoved:(NSEvent *)theEvent {
    self->gameEngine.playerCamera().rotateTheta(theEvent.deltaX);
}

- (void)drawInMTKView: (MTKView *) view {

    self->gameEngine.setDevice(_device);
    self->gameEngine.setDrawFunction([=](const std::vector<NativeBuffer> &buffers) {
        self.tick += 1;
        id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
        MTLRenderPassDescriptor *renderPassDescriptor = view.currentRenderPassDescriptor;

        float aspect = _viewportSize.x / _viewportSize.y;
        float fov = (2 * M_PI) / 5;
        float near = 0.01;
        float far = 1000;

        matrix_float4x4 cameraPerspective = matrix_projection(fov, aspect, near, far);
        matrix_float4x4 cameraTranslation = matrix_from_translation(-self->gameEngine.playerCamera().x(), -self->gameEngine.playerCamera().z(), -self->gameEngine.playerCamera().y());
        matrix_float4x4 cameraRotation = matrix_from_rotation(-self->gameEngine.playerCamera().theta() * M_PI / 180, 0, 1, 0);
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

                float f = buffer.secondsSinceFirstLoaded();
                [renderEncoder
                    setVertexBytes:&f
                    length:sizeof(float)
                    atIndex: 4
                ];

                [renderEncoder drawPrimitives: MTLPrimitiveTypeTriangle vertexStart:0 vertexCount: buffer.size()];
                
            }
            
            [renderEncoder endEncoding];
            [commandBuffer presentDrawable:view.currentDrawable];
        }
        [commandBuffer commit];
    });

    gameEngine.render();  
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
    NSString *characters = [theEvent characters];
    if (!self.renderer->gameEngine.onKeyPress([characters characterAtIndex:0])) {
        [super keyDown:theEvent];
    }

}


- (void) keyUp:(NSEvent *)theEvent {
    NSString *characters = [theEvent characters];
    if (!self.renderer->gameEngine.onKeyRelease([characters characterAtIndex:0])) {
        [super keyUp:theEvent];
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