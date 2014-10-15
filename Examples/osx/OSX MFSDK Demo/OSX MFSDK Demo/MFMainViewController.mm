//
//  MFMainViewController.m
//  OSX MFSDK Demo
//
//  Created by Zachary Marquez on 10/12/14.
//  Copyright (c) 2014 MediaFire. All rights reserved.
//

#import "MFMainViewController.h"

#include "MFApiBridge.hpp"

@interface MFMainViewController () {
    // The API is not directly usable from Objective-C++ code, so a bridge
    // is created.
    MfApiBridge::Pointer apiBridge_;
}

@property NSMutableArray* contentResults;

@end

@implementation MFMainViewController

// This is called when we are loaded with a nib as would conventionally be
// done.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        [self initialize];
    }
    return self;
}

// This is called when we are loaded directly from another nib file as is the
// case in this demo.
- (id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if (self) {
        [self initialize];
    }
    return self;
}

- (void)initialize
{
    _contentResults = [[NSMutableArray alloc] init];
}

- (void)askForCredentials
{
    [self askForCredentials:@"Enter email and password."];
}

- (void)askForCredentials:(NSString*)detailsText
{
    self.loginStatus.stringValue = @"Waiting for credentials...";

    NSAlert *loginAlert = [[NSAlert alloc] init];
    [loginAlert setMessageText:@"Please log in"];
    [loginAlert setInformativeText:detailsText];
    [loginAlert addButtonWithTitle:@"OK"];
    [loginAlert addButtonWithTitle:@"Cancel"];
    [loginAlert setAccessoryView:[self credentialsForm]];
    [loginAlert beginSheetModalForWindow:self.view.window completionHandler:^(NSModalResponse returnCode) {
            if ( returnCode == NSAlertFirstButtonReturn )
            {
                [self loginWithEmail:self.emailField.stringValue Password:self.passwordField.stringValue];
            }
            else
            {
                self.loginStatus.stringValue = @"User canceled login.";
                [self alertNeedCredentials];
            }
        }];
}

- (void)alertNeedCredentials
{
    NSAlert *credentialsAlert = [[NSAlert alloc] init];
    [credentialsAlert setMessageText:@"Missing credentials"];
    [credentialsAlert setInformativeText:@"This application cannot continue without credentials."];
    [credentialsAlert beginSheetModalForWindow:self.view.window completionHandler:^(NSModalResponse returnCode) {
            [NSApp terminate:self];
        }];
}

- (void)handleLoginFailure
{
    [self.loginSpinner stopAnimation:self];
    [self askForCredentials:@"Login attempt failed."];
}

- (void)handleLoginSuccess
{
    [self.loginSpinner stopAnimation:self];
    [self.tabView selectTabViewItem:self.loggedInTab];

    NSLog(@"Debug: Requesting user info...");
    apiBridge_->GetUserInfo(
            ^(NSDictionary *userInfo) {
                NSString* fullName = [NSString stringWithFormat:@"%@ %@",
                    [userInfo objectForKey:skDemoUserFirstNameKey],
                    [userInfo objectForKey:skDemoUserLastNameKey]];
                NSLog(@"Debug: Received user info for %@.", fullName);
                self.nameField.stringValue = fullName;
            },
            ^(int code, NSString* msg) {
                NSLog(@"Failed to get user info: %d: %@", code, msg);
            }
        );

    NSLog(@"Debug: Requesting folders...");
    [self requestFolderChunk:1];
}

- (void)requestFolderChunk:(NSInteger)chunk
{
    apiBridge_->GetFolderContent(
            @"myfiles",
            MfApiBridge::ContentType::Folders,
            chunk,
            ^(NSArray *folders, NSInteger chunk, BOOL moreChunks) {
                NSLog(@"Debug: Received folder chunk %lu.",chunk);
                [self.contentResults addObjectsFromArray:folders];
                [self.fileTable noteNumberOfRowsChanged];
                if ( moreChunks )
                {
                    [self requestFolderChunk:(moreChunks+1)];
                }
                else
                {
                    NSLog(@"Debug: Requesting files...");
                    [self requestFileChunk:1];
                }
            },
            ^(int code, NSString* msg) {
                NSLog(@"Failed to get myfiles content: %d: %@", code, msg);
            }
        );
}

- (void)requestFileChunk:(NSInteger)chunk
{
    apiBridge_->GetFolderContent(
            @"myfiles",
            MfApiBridge::ContentType::Files,
            chunk,
            ^(NSArray *files, NSInteger chunk, BOOL moreChunks) {
                NSLog(@"Debug: Received file chunk %lu.",chunk);
                [self.contentResults addObjectsFromArray:files];
                [self.fileTable noteNumberOfRowsChanged];
                if ( moreChunks )
                    [self requestFileChunk:(moreChunks+1)];
            },
            ^(int code, NSString* msg) {
                NSLog(@"Failed to get myfiles content: %d: %@", code, msg);
            }
        );
}

- (void)loginWithEmail:(NSString*)email Password:(NSString*)password
{
    self.loginStatus.stringValue = @"Logging in...";
    [self.loginStatus display];
    [self.loginSpinner startAnimation:self];

    MfApiBridge::LogIn(
            email,
            password,
            ^(MfApiBridge::Pointer newBridge) {
                apiBridge_ = newBridge;
                [self handleLoginSuccess];
            },
            ^(int code, NSString* msg) {
                NSLog(@"Failed to login: %d: %@", code, msg);
                [self handleLoginFailure];
            }
        );
}

//============================================================================
// Table View Data Source
//============================================================================
//------------------------------------------------------------------------------
- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return [self.contentResults count];
}

//------------------------------------------------------------------------------
- (NSTableCellView*)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSDictionary* content = [self.contentResults objectAtIndex:row];
    
    NSTableCellView *cell = [tableView makeViewWithIdentifier:tableColumn.identifier owner:self];
    NSAssert( cell != nil, @"COULD NOT FETCH CELL VIEW.");

    if ([tableColumn.identifier isEqualToString:@"MyFilesName"]) {
        cell.textField.stringValue = content[skDemoFolderContentNameKey];
    } else if ([tableColumn.identifier isEqualToString:@"MyFilesType"]) {
        cell.textField.stringValue = content[skDemoFolderContentTypeKey];
    } else if ([tableColumn.identifier isEqualToString:@"MyFilesKey"]) {
        cell.textField.stringValue = content[skDemoFolderContentKeyKey];
    } else if ([tableColumn.identifier isEqualToString:@"MyFilesDate"]) {
        cell.textField.stringValue = content[skDemoFolderContentDateKey];
    } else if ([tableColumn.identifier isEqualToString:@"MyFilesPrivacy"]) {
        cell.textField.stringValue = content[skDemoFolderContentPrivacyKey];
    } else {
        NSAssert(NO, @"Unknown table column: %@",tableColumn.identifier);
    }

    return cell;
}


@end
