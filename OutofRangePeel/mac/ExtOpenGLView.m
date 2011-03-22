//
//  ExtOpenGLView.m
//  OutofrangePeel
//

#import "ExtOpenGLView.h"
#import <OpenGL/OpenGL.h>
#import <GLUT/GLUT.h>

#import "../OutofrangePeel/MacMain.h"

@interface NSEvent (DeviceDelta)
- (float)deviceDeltaX;
- (float)deviceDeltaY;
@end

@implementation ExtOpenGLView

@synthesize statusText;

- (id)initWithFrame:(NSRect)frame {
    NSOpenGLPixelFormatAttribute attrs[] = {
        NSOpenGLPFAWindow,
        NSOpenGLPFAAccelerated,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAColorSize, (NSOpenGLPixelFormatAttribute)32,
        NSOpenGLPFAAlphaSize, (NSOpenGLPixelFormatAttribute)8,
        NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)16,
        (NSOpenGLPixelFormatAttribute)0
    };
    NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    if (!pixelFormat) {
        NSLog(@"Failed to create a pixel format object.");
        [self release];
        return nil;
    }
    self = [super initWithFrame:frame pixelFormat:pixelFormat];
    [pixelFormat release];
    if (self) {
        // 通常の NSView クラスのサブクラスであればここで初期化を行う訳ですが、
        // NSOpenGLView クラスのサブクラスの場合は prepareOpenGL メソッドの中で
        // 行う方がいろいろ便利です。
        // ひとつの理由としては、IB で OpenGL View を貼り付けた場合には initWithFrame:
        // メソッドが呼ばれないため、prepareOpenGL メソッドで初期化を行う方がコードの共通化が
        // 図れるためです。
        // もうひとつの理由としては、prepareOpenGL メソッドが呼ばれる時点では必ず OpenGL の
        // コンテキストが作成されているため、コンテキストが必要な操作も同時に処理できるためです。
    }
	
	// timer
	[NSTimer scheduledTimerWithTimeInterval:0.016
									 target:self
								   selector:@selector(onTimer:)
								   userInfo:NULL
									repeats:YES];
	
    return self;
}

- (void) onTimer:(NSTimer*)timer
{
	//NSLog(@"ontimer: %@", timer);
	
	core_callback_animate();
	[self setNeedsDisplay:YES];

}


- (void)prepareOpenGL
{
	core_initialize();
}

- (void)drawRect:(NSRect)dirtyRect
{    
	core_callback_disp();

    // flip double buffer
	[[self openGLContext] flushBuffer];
}

- (void)reshape
{
    NSRect frame = [self frame];
	int w=(GLsizei)frame.size.width;
	int h=(GLsizei)frame.size.height;

	char str[1024];
	sprintf(str,"reshape %d %d", w, h);
	[statusText setStringValue:[NSString stringWithUTF8String:str]];

	core_callback_resize(w, h);
}

-(void) rightMouseDown:(NSEvent *)theEvent
{
	NSRect r=[self frame];
	NSPoint p=[theEvent locationInWindow];
	int x=p.x;
	int y=r.size.height+r.origin.y-p.y;
	
	char str[1024];
	sprintf(str,"right mouse down %d %d", x, y);
	[statusText setStringValue:[NSString stringWithUTF8String:str]];	
	
	core_callback_mouse(GLUT_RIGHT_BUTTON, GLUT_DOWN, x, y);
}

-(void) rightMouseUp:(NSEvent *)theEvent
{
	NSRect r=[self frame];
	NSPoint p=[theEvent locationInWindow];
	int x=p.x;
	int y=r.size.height+r.origin.y-p.y;
	
	char str[1024];
	sprintf(str,"right mouse up %d %d", x, y);
	[statusText setStringValue:[NSString stringWithUTF8String:str]];	
	
	core_callback_mouse(GLUT_RIGHT_BUTTON, GLUT_UP, x, y);
}

-(void) mouseDown:(NSEvent *)theEvent
{
	NSRect r=[self frame];
	NSPoint p=[theEvent locationInWindow];
	int x=p.x;
	int y=r.size.height+r.origin.y-p.y;
	
	char str[1024];
	sprintf(str,"mouse down %d %d", x, y);
	[statusText setStringValue:[NSString stringWithUTF8String:str]];	
	
	core_callback_mouse(GLUT_LEFT_BUTTON, GLUT_DOWN, x, y);
}

-(void) mouseUp:(NSEvent *)theEvent
{
	NSRect r=[self frame];
	NSPoint p=[theEvent locationInWindow];
	int x=p.x;
	int y=r.size.height+r.origin.y-p.y;
	
	char str[1024];
	sprintf(str,"mouse up %d %d", x, y);
	[statusText setStringValue:[NSString stringWithUTF8String:str]];	

	core_callback_mouse(GLUT_LEFT_BUTTON, GLUT_UP, x, y);
}

-(void) mouseDragged:(NSEvent *)theEvent
{
	NSRect r=[self frame];
	NSPoint p=[theEvent locationInWindow];
	int x=p.x;
	int y=r.size.height+r.origin.y-p.y;
	
	char str[1024];
	sprintf(str,"mouse dragged %d %d", x, y);
	[statusText setStringValue:[NSString stringWithUTF8String:str]];
	
	core_callback_mousemove(x, y);
}
-(void) rightMouseDragged:(NSEvent *)theEvent
{
	NSRect r=[self frame];
	NSPoint p=[theEvent locationInWindow];
	int x=p.x;
	int y=r.size.height+r.origin.y-p.y;
	
	char str[1024];
	sprintf(str,"right mouse dragged %d %d", x, y);
	[statusText setStringValue:[NSString stringWithUTF8String:str]];
	
	core_callback_mousemove(x, y);
}

-(void) mouseMoved:(NSEvent *)theEvent
{
	NSRect r=[self frame];
	NSPoint p=[theEvent locationInWindow];
	int x=p.x;
	int y=r.size.height+r.origin.y-p.y;
	
	char str[1024];
	sprintf(str,"mouse moved %d %d", x, y);
	[statusText setStringValue:[NSString stringWithUTF8String:str]];	

	core_callback_mousepassivemove(x, y);
}

- (void)viewDidMoveToWindow {
	tag = [self addTrackingRect:[self bounds] owner:self userData:NULL assumeInside:NO];
}
- (void)setFrame:(NSRect)frame
{
	[super setFrame:frame];
	[self removeTrackingRect:tag];
	tag = [self addTrackingRect:[self bounds] owner:self userData:NULL assumeInside:NO];
}
- (void)setBounds:(NSRect)bounds
{
	[super setBounds:bounds];
	[self removeTrackingRect:tag];
	tag = [self addTrackingRect:[self bounds] owner:self userData:NULL assumeInside:NO];
}
- (void)mouseEntered:(NSEvent *)theEvent
{
//	NSLog(@"entered: %@", theEvent);
	[[self window] setAcceptsMouseMovedEvents:YES];
	[[self window] makeFirstResponder:self];
	[self setNeedsDisplay:YES];
}
- (void)mouseExited:(NSEvent *)theEvent
{
//	NSLog(@"exited: %@", theEvent);
	[[self window] setAcceptsMouseMovedEvents:NO];
	[self setNeedsDisplay:YES];
}
- (void)awakeFromNib
{
//	NSLog(@"awakeFromNib");
	[[self window] setAcceptsMouseMovedEvents:YES];
	[[self window] makeFirstResponder:self];
}


-(void) scrollWheel:(NSEvent *)theEvent
{
	NSRect r = [self frame];
	NSPoint p = [theEvent locationInWindow];
	int x = p.x;
	int y = r.size.height+r.origin.y-p.y;	
	
	float dx = [theEvent deltaX] * r.size.height * 0.005;
	float dy = [theEvent deltaY] * r.size.height * 0.005;
	
	core_callback_mouse(GLUT_MIDDLE_BUTTON, GLUT_DOWN, x, y);
	core_callback_mousemove(x-dx, y-dy);
	core_callback_mouse(GLUT_MIDDLE_BUTTON, GLUT_UP, x, y);
}

-(void) swipeWithEvent:(NSEvent *)event
{
	NSLog(@"swipe: %@", event);
}
-(void) magnifyWithEvent:(NSEvent *)event
{
	NSLog(@"magnify: %@", event);
	
	NSRect r = [self frame];
	NSPoint p = [event locationInWindow];
	int x = p.x;
	int y = r.size.height+r.origin.y-p.y;	

	float mag=[event magnification]*10.0;
	
	core_callback_mousewheel(GLUT_MIDDLE_BUTTON, mag, x, y);
}
-(void) rotateWithEvent:(NSEvent *)event
{
	NSLog(@"rotate: %@", event);
}



@end
