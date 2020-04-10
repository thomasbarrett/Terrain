#import <Cocoa/Cocoa.h>

@interface MainView : NSView 
- (void) drawRect: (NSRect) rect; 
@end

@implementation MainView 
- (void) drawRect: (NSRect) rect {
    [[NSColor blackColor] set];
    NSRectFill([self bounds]);
}
@end

int main(void) {
    @autoreleasepool {
        NSApp = [NSApplication sharedApplication];
        NSRect frameRect = NSMakeRect(100.0, 350.0, 800.0, 600.0);
        NSView *mainView = [[MainView alloc] initWithFrame: frameRect];
        NSWindow *window = [[NSWindow alloc]
            initWithContentRect: frameRect
            styleMask: NSWindowStyleMaskTitled 
                | NSWindowStyleMaskClosable 
                | NSWindowStyleMaskMiniaturizable
                | NSWindowStyleMaskResizable
            backing: NSBackingStoreBuffered
            defer: NO
        ];
        [window setTitle: @"Example Application"];
        [window setContentView: mainView];
        [window makeKeyAndOrderFront: nil];
        [NSApp run];
    }
    return 0;
}