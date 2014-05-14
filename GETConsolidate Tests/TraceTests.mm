//
//  TraceTests.m
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/13/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "Trace.h"

@interface TraceTests : XCTestCase

@end

@implementation TraceTests

- (void)setUp
{
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown
{
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testCompactSample
{
    for (uint16_t tb = 0; tb < 512; tb++) {
        for (int16_t val = 0; val < 0xFFF; val++) {
            uint32_t res = Trace::CompactSample(tb, val);
            uint16_t res_tb = (res & 0xFF8000) >> 15;
            int16_t res_val = res & 0xFFF;
            XCTAssertEqual(tb, res_tb, @"Bad TB for %d",tb);
            XCTAssertEqual(val, res_val, @"Bad val for %d",val);
        }
    }
}

-(void)testAddition
{
    Trace *t1, *t2;
    
    for (int a = 0; a < 200; a++) {
        for (int b = 0; b < 200; b++) {
            t1 = new Trace ();
            t2 = new Trace ();
            for (int i = 0; i < 512; i++) {
                t1->AppendSample(i, a);
                t2->AppendSample(i, b);
            }
            *t1 += *t2;
            
            for (int i = 0; i < 512; i++) {
                int16_t val = t1->GetSample(i);
                XCTAssertEqual(a+b, val, "Addition failed.");
            }
            delete t1;
            delete t2;
        }
    }
}

@end
