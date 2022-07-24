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

#import "IJKDemoLocalFolderViewController.h"
#import "IJKMoviePlayerViewController.h"

@interface IJKDemoLocalFolderViewController ()

@end

@implementation IJKDemoLocalFolderViewController {
    NSString *_folderPath;
    NSMutableArray *_subpaths;
    NSMutableArray *_files;
}

- (instancetype)initWithFolderPath:(NSString *)folderPath {
    self = [super init];
    if (self) {
        folderPath = [folderPath stringByStandardizingPath];
        self.title = [folderPath lastPathComponent];
        
        _folderPath = folderPath;
        _subpaths = [NSMutableArray array];
        _files = [NSMutableArray array];
    }
    return self;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    NSError *error = nil;
    BOOL isDirectory = NO;
    NSArray *files = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:_folderPath error:&error];

    [_subpaths addObject:@".."];

    for (NSString *fileName in files) {
        NSString *fullFileName = [_folderPath stringByAppendingPathComponent:fileName];
        
        [[NSFileManager defaultManager] fileExistsAtPath:fullFileName isDirectory:&isDirectory];
        if (isDirectory) {
            [_subpaths addObject:fileName];
        } else {
            [_files addObject:fileName];
        }
    }
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 2;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    switch (section) {
        case 0:
            return _subpaths.count;
            
        case 1:
            return _files.count;
            
        default:
            break;
    }

    return 0;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"abc"];
    if (nil == cell) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"abc"];
        cell.textLabel.lineBreakMode = NSLineBreakByTruncatingMiddle;
    }
    
    switch (indexPath.section) {
        case 0: {
            cell.textLabel.text = [NSString stringWithFormat:@"[%@]", _subpaths[indexPath.row]];
            cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
        } break;
        case 1: {
            cell.textLabel.text = _files[indexPath.row];
            cell.accessoryType = UITableViewCellAccessoryNone;
        } break;
        default:
            break;
    }
    
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
    
    switch (indexPath.section) {
        case 0: {
            NSString *fileName = [_folderPath stringByAppendingPathComponent:_subpaths[indexPath.row]];

            IJKDemoLocalFolderViewController *viewController = [[IJKDemoLocalFolderViewController alloc] initWithFolderPath:fileName];
            
            [self.navigationController pushViewController:viewController animated:YES];
        } break;
        case 1: {
            NSString *fileName = [_folderPath stringByAppendingPathComponent:_files[indexPath.row]];

            fileName = [fileName stringByStandardizingPath];
            
            [IJKVideoViewController presentFromViewController:self withTitle:[NSString stringWithFormat:@"File: %@", fileName] URL:[NSURL fileURLWithPath:fileName] completion:^{
            }];
            
        } break;
        default:
            break;
    }
}

/*
// Override to support conditional editing of the table view.
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the specified item to be editable.
    return YES;
}
*/

/*
// Override to support editing the table view.
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source
        [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationFade];
    } else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view
    }   
}
*/

/*
// Override to support rearranging the table view.
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath {
}
*/

/*
// Override to support conditional rearranging of the table view.
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the item to be re-orderable.
    return YES;
}
*/

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
