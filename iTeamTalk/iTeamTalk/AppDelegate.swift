//
//  AppDelegate.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 3-09-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate {

    var window: UIWindow?

    func application(application: UIApplication, didFinishLaunchingWithOptions launchOptions: [NSObject: AnyObject]?) -> Bool {
        // Override point for customization after application launch.

        // Default values are not set in Settings bundle, so we need to load them manually
        let defaults = NSUserDefaults.standardUserDefaults()
        defaults.synchronize()
        
        var bundleDefaults = [NSObject : AnyObject]()
        
        
        if let settingsBundle = NSBundle.mainBundle().pathForResource("Settings", ofType: "bundle") {
            
            let path = settingsBundle.stringByAppendingPathComponent("Root.plist")
            let settings = NSDictionary(contentsOfFile: path)
            let preferences = settings!.objectForKey("PreferenceSpecifiers") as! NSArray

            for prefSpecification in preferences {
                if let key: AnyObject = prefSpecification.objectForKey("Key") {
                    if defaults.objectForKey(key as! String) == nil {
                        let defValue: AnyObject? = prefSpecification.objectForKey("DefaultValue")
                        bundleDefaults[key as! String] = defValue
//                        println("Pref: " + (key as! String) + "=" + defValue)
                    }
                }
            }
            
        }

        if !bundleDefaults.isEmpty {
            defaults.registerDefaults(bundleDefaults)
            defaults.synchronize()
        }

//        var s = Server();
//        
//        s.name = "Foo"
//        
//        let data = NSKeyedArchiver.archivedDataWithRootObject(s)
//        
////        let s_arr = NSMutableArray()
////        s_arr.addObject(data)
//        
//        var s_arr = [NSData]()
//        s_arr.append(data)
//        
//        defaults.setObject(s_arr, forKey: "ServerTest")
//        
//        //defaults.synchronize()
//        //let foo  = NSKeyedUnarchiver.unarchiveObjectWithData(data) as! Server
//        
//        if let s_a = defaults.arrayForKey("ServerTest") {
//            let vv = s_a[0] as! NSData
//            
//            let foo = NSKeyedUnarchiver.unarchiveObjectWithData(vv) as! Server
//            let x = 3
//            
//            let y = x+0
//        }

        return true
    }

    func applicationWillResignActive(application: UIApplication) {
        // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
        // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
    }

    func applicationDidEnterBackground(application: UIApplication) {
        // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
        // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
    }

    func applicationWillEnterForeground(application: UIApplication) {
        // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
    }

    func applicationDidBecomeActive(application: UIApplication) {
        // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
    }

    func applicationWillTerminate(application: UIApplication) {
        // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
    }


}

