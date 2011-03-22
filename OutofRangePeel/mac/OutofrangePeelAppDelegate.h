//
//  OutofrangePeelAppDelegate.h
//  OutofrangePeel
//

#import <Cocoa/Cocoa.h>

@interface OutofrangePeelAppDelegate : NSObject <NSApplicationDelegate> {
    NSWindow *window;
	
	IBOutlet NSTextField *stateText;
}

@property (assign) IBOutlet NSWindow *window;
-(void) awakeFromNib;

@end
