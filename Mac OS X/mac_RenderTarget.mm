// ------------------------------------------------------------------
// mac_RenderTarget - stardazed
// (c) 2014 by Arthur Langereis
// ------------------------------------------------------------------

#include "../include/RenderTarget.h"
#include "../include/Application.h"
#import <AppKit/AppKit.h>
#import <OpenGL/gl3.h>
#include <vector>
#include <cassert>


@interface SDWindow : NSWindow {}
@end
@implementation SDWindow
- (BOOL)canBecomeKeyWindow {
	return YES;
}
- (BOOL)canBecomeMainWindow {
	return YES;
}
@end


@interface SDWindowDelegate : NSObject<NSWindowDelegate> {}
@end

@implementation SDWindowDelegate
- (BOOL)windowShouldClose:(id)sender {
	stardazed::quitNow();
	return NO;
}
@end


@interface SDOpenGLView : NSOpenGLView {}
@end
@implementation SDOpenGLView
@end



static NSOpenGLPixelFormat* pixelFormatForRenderOptions(const stardazed::RenderTargetOptions &options) {
	using namespace stardazed;
	
	std::vector<NSOpenGLPixelFormatAttribute> attrs = {
		NSOpenGLPFAAccelerated,
		NSOpenGLPFANoRecovery,
		NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
	};

	auto boolAttr = [&attrs](NSOpenGLPixelFormatAttribute name) {
		attrs.push_back(name);
	};
	auto valueAttr = [&attrs](NSOpenGLPixelFormatAttribute name, NSOpenGLPixelFormatAttribute value) {
		attrs.push_back(name);
		attrs.push_back(value);
	};

	// double or triple buffering
	if (options.bufferMethod == BufferingMethod::DoubleBuffer)
		boolAttr(NSOpenGLPFADoubleBuffer);
	else
		boolAttr(NSOpenGLPFATripleBuffer);

	// FSAA method
	if (options.fsaa != FullscreenAntiAliasMethod::None) {
		if (options.fsaa == FullscreenAntiAliasMethod::SSAA)
			boolAttr(NSOpenGLPFASupersample);
		else
			boolAttr(NSOpenGLPFAMultisample);
		valueAttr(NSOpenGLPFASampleBuffers, 1);
		valueAttr(NSOpenGLPFASamples, options.antiAliasSamples);
	}
	
	// buffer depths
	valueAttr(NSOpenGLPFAColorSize, 24);
	valueAttr(NSOpenGLPFAAlphaSize, 8);
	if (options.depthBits > 0)
		valueAttr(NSOpenGLPFADepthSize, options.depthBits);
	if (options.stencilBits > 0)
		valueAttr(NSOpenGLPFAStencilSize, options.stencilBits);
	if (options.accumulatorBits > 0)
		valueAttr(NSOpenGLPFAAccumSize, options.accumulatorBits);
	if (options.auxBufferCount > 0)
		valueAttr(NSOpenGLPFAAuxBuffers, options.auxBufferCount);

	attrs.push_back(0);
	return [[NSOpenGLPixelFormat alloc] initWithAttributes: attrs.data()];
}


static SDOpenGLView* createOpenGLView(const NSRect frame, const stardazed::RenderTargetOptions &options) {
	NSOpenGLPixelFormat* pixelFormat = pixelFormatForRenderOptions(options);
	SDOpenGLView *oglView = [[SDOpenGLView alloc] initWithFrame:frame pixelFormat: pixelFormat];
	
	GLint sync = options.verticalSync ? 1 : 0;
	[[oglView openGLContext] setValues: &sync forParameter: NSOpenGLCPSwapInterval];

	return oglView;
}


static NSWindow* createRenderWindow(const stardazed::RenderTargetOptions &options) {
	NSRect frame;
	NSUInteger styleOptions;
	if (options.fullscreen) {
		// In fullscreen mode, the width and height are used only for
		// the backbuffer of the OpenGL context. The window is plain.
		frame = [[NSScreen mainScreen] frame];
		styleOptions = NSBorderlessWindowMask;
	}
	else {
		// In windowed mode the window content is sized equal to
		// the viewport and looks like a normal window with titlebar
		// and close control.
		frame = NSMakeRect(0, 0, (CGFloat)options.width, (CGFloat)options.height);
		styleOptions = NSTitledWindowMask | NSClosableWindowMask;
	}
	
	SDWindow *window = [[SDWindow alloc]
		initWithContentRect: frame
				  styleMask: styleOptions
					backing: NSBackingStoreBuffered
					  defer: NO
	];
	[window setTitle: @"Stardazed"];
	[window setAcceptsMouseMovedEvents: YES];
	[window setOpaque: YES];
	[window center];
	
	// In fullscreen mode we need to hover above all other windows
	// and be nice and hide ourselves when we're moved to the bg.
	if (options.fullscreen) {
		[window setLevel: NSMainMenuWindowLevel + 1];
		[window setHidesOnDeactivate: YES];
	}

	// Add main content view (a subclass of an NSOpenGLView)
	SDOpenGLView *contentView = createOpenGLView(frame, options);
	[window setContentView: contentView];
	[[contentView openGLContext] makeCurrentContext];

	return window;
}


static void setupGL(const stardazed::RenderTargetOptions& rto) {
	glViewport(0, 0, rto.width, rto.height);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearColor(0, 0, 0, 0);
}


namespace stardazed {

class RenderTarget::Impl {
public:
	NSWindow* coverWindow;
	id windowDelegate;
	NSOpenGLContext* glContext;
};


RenderTarget::RenderTarget(RenderTargetOptions rto)
: options(rto)
{
	pimpl = std::make_unique<Impl>();
	NSWindow *window = createRenderWindow(options);

	id delegate = [[SDWindowDelegate alloc] init];
	[window setDelegate: delegate];
	
	[window makeKeyAndOrderFront: nil];
	
	pimpl->coverWindow = window;
	pimpl->windowDelegate = delegate;
	pimpl->glContext = [[pimpl->coverWindow contentView] openGLContext];
	
	setupGL(rto);
}


RenderTarget::~RenderTarget() {
}


void RenderTarget::swap() {
	[pimpl->glContext flushBuffer];
}

} // stardazed namespace