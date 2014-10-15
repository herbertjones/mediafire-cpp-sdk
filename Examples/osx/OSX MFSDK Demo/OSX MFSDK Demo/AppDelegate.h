//
//  AppDelegate.h
//  OSX MFSDK Demo
//
//  Created by Zachary Marquez on 10/12/14.
//  Copyright (c) 2014 MediaFire. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "MFMainViewController.h"

@interface AppDelegate : NSObject <NSApplicationDelegate>

@property (assign) IBOutlet NSWindow *window;
@property (assign) IBOutlet MFMainViewController *mainViewController;

@end
