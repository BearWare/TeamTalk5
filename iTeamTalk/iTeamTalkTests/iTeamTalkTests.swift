//
//  iTeamTalkTests.swift
//  iTeamTalkTests
//
//  Created by Bjoern Rasmussen on 3-09-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit
import XCTest
import iTeamTalk

class iTeamTalkTests: XCTestCase {
    
    func newClient() -> UnsafeMutablePointer<Void> {
        let ttInst = TT_InitTeamTalkPoll()
        
        return ttInst
    }
    
    override func setUp() {
        super.setUp()
        // Put setup code here. This method is called before the invocation of each test method in the class.
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func testExample() {
        // This is an example of a functional test case.
        XCTAssert(true, "Pass")
        
        let ttInst = newClient()
        XCTAssert(TT_CloseTeamTalk(ttInst) != 0, "Closed TT instance")
    }
    
    func testPerformanceExample() {
        // This is an example of a performance test case.
        self.measureBlock() {
            // Put the code you want to measure the time of here.
        }
    }
    
}
