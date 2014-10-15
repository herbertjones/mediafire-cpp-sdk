//
//  MFMainViewController.h
//  OSX MFSDK Demo
//
//  Created by Zachary Marquez on 10/12/14.
//  Copyright (c) 2014 MediaFire. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface MFMainViewController : NSViewController <NSTableViewDataSource>

// Credentials Form
@property (assign) IBOutlet NSView *credentialsForm;
@property (assign) IBOutlet NSTextField *emailField;
@property (assign) IBOutlet NSSecureTextField *passwordField;

// Main tab view
@property (assign) IBOutlet NSTabView *tabView;

// Start tab
@property (assign) IBOutlet NSTabViewItem *startTab;
@property (assign) IBOutlet NSTextField *loginStatus;
@property (assign) IBOutlet NSProgressIndicator *loginSpinner;

// LoggedIn tab
@property (assign) IBOutlet NSTabViewItem *loggedInTab;
@property (assign) IBOutlet NSTextField *nameField;
@property (assign) IBOutlet NSTableView *fileTable;

// Ask the user to input credentials.
// Upon receiving valid credentials, the whole dialog automatically will
// retrive and display the user's MyFiles contents.
- (void)askForCredentials;

@end
