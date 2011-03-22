//
//  ExtOpenGLView.h
//  OutofrangePeel
//

#import <Cocoa/Cocoa.h>


@interface ExtOpenGLView : NSOpenGLView {
	IBOutlet NSTextField *statusText;
	
	NSTrackingRectTag tag;
}

@property (nonatomic, retain) NSTextField *statusText;

- (void) onTimer:(NSTimer*)timer;


@end
