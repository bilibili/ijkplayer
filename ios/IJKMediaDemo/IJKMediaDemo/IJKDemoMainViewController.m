/*
 * Copyright (C) 2015 Gdier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#import "IJKDemoMainViewController.h"
#import "IJKDemoInputURLViewController.h"
#import "IJKQRCodeScanViewController.h"
#import "IJKCommon.h"
#import "IJKDemoHistory.h"
#import "IJKMoviePlayerViewController.h"
#import "IJKDemoLocalFolderViewController.h"
#import "IJKDemoSampleViewController.h"

@interface IJKDemoMainViewController () <UITableViewDataSource, UITableViewDelegate>

@property(nonatomic,strong) IBOutlet UITableView *tableView;
@property(nonatomic,strong) NSArray *tableViewCellTitles;
@property(nonatomic,strong) NSArray *historyList;

@end

@implementation IJKDemoMainViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.title = @"Main";
    
    self.tableViewCellTitles = @[
                                 @"Local Folder",
                                 @"Input URL",
                                 @"Scan QRCode",
                                 @"Online Samples",
                                 ];
    
    NSURL *documentsUrl = [[[NSFileManager defaultManager] URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask] firstObject];
    
    NSError *error = nil;
    
    [documentsUrl setResourceValue:[NSNumber numberWithBool:YES]
                            forKey:NSURLIsExcludedFromBackupKey
                             error:&error];
}

- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
    
    self.historyList = [[IJKDemoHistory instance] list];

    [self.tableView reloadData];
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 2;
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section {
    switch (section) {
        case 0:
            return @"Open from";
            
        case 1:
            return @"History";
            
        default:
            return nil;
    }
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    switch (section) {
        case 0:
            if (IOS_NEWER_OR_EQUAL_TO_7) {
                return self.tableViewCellTitles.count;
            } else {
                return self.tableViewCellTitles.count - 1;
            }
            
        case 1:
            return self.historyList.count;
            
        default:
            return 0;
    }
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"abc"];
    if (nil == cell) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"abc"];
        cell.textLabel.lineBreakMode = NSLineBreakByTruncatingMiddle;
    }
    
    switch (indexPath.section) {
        case 0:
            cell.textLabel.text = self.tableViewCellTitles[indexPath.row];
            break;
            
        case 1:
            cell.textLabel.text = [self.historyList[indexPath.row] title];
            break;
            
        default:
            break;
    }
    
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
    
    switch (indexPath.section) {
        case 0: {
            switch (indexPath.row) {
                case 0: {
                    NSString *documentsPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject];

                    IJKDemoLocalFolderViewController *viewController = [[IJKDemoLocalFolderViewController alloc] initWithFolderPath:documentsPath];
                    
                    [self.navigationController pushViewController:viewController animated:YES];
                } break;
                    
                case 1:
                    [self.navigationController pushViewController:[[IJKDemoInputURLViewController alloc] init] animated:YES];
                    break;
                    
                case 2:
                    [self.navigationController pushViewController:[[IJKQRCodeScanViewController alloc] init] animated:YES];
                    break;

                case 3:
                    [self.navigationController pushViewController:[[IJKDemoSampleViewController alloc] init] animated:YES];
                    break;

                default:
                    break;
            }
        } break;
            
        case 1: {
            IJKDemoHistoryItem *historyItem = self.historyList[indexPath.row];
            
            [IJKVideoViewController presentFromViewController:self withTitle:historyItem.title URL:historyItem.url completion:^{
                [self.navigationController popViewControllerAnimated:NO];
            }];
        } break;
            
        default:
            break;
    }
}

- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    return (indexPath.section == 1);
}

- (UITableViewCellEditingStyle)tableView:(UITableView *)tableView editingStyleForRowAtIndexPath:(NSIndexPath *)indexPath {
    if (indexPath.section == 1) {
        return UITableViewCellEditingStyleDelete;
    }
    
    return UITableViewCellEditingStyleNone;
}

- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    if (indexPath.section == 1 && editingStyle == UITableViewCellEditingStyleDelete) {
        [[IJKDemoHistory instance] removeAtIndex:indexPath.row];
        self.historyList = [[IJKDemoHistory instance] list];
        
        [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
    }
}

@end
