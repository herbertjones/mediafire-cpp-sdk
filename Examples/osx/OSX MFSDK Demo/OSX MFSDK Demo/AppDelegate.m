//
//  AppDelegate.m
//  OSX MFSDK Demo
//
//  Created by Zachary Marquez on 10/12/14.
//  Copyright (c) 2014 MediaFire. All rights reserved.
//

#import "AppDelegate.h"

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Replace the content view with the mainViewController's view and start
    // the login process
    [self.window setContentView:self.mainViewController.view];
    [self.mainViewController askForCredentials];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}

@end
