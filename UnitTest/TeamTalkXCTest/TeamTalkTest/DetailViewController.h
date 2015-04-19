//
//  DetailViewController.h
//  TeamTalkTest
//
//  Created by Bjoern Rasmussen on 4-01-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface DetailViewController : UIViewController

@property (strong, nonatomic) id detailItem;
@property (weak, nonatomic) IBOutlet UILabel *detailDescriptionLabel;

@end

