//
//  GETFrameTests.m
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/13/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "GETFrame.h"

@interface GETFrameTests : XCTestCase

@end

@implementation GETFrameTests

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

- (void)testExtractAgetId
{
    uint32_t testData;
    for (int i = 0; i < 4; i++) {
        testData = i << 30;
        XCTAssert(GETFrame::ExtractAgetId(testData) == i, @"Extracting AGET failed for value %d",i);
    }
}

-(void)testExtractChannel
{
    uint32_t testData;
    for (int i = 0; i < 68; i++) {
        testData = i << 23;
        XCTAssert(GETFrame::ExtractChannel(testData) == i, @"Extracting Channel failed for value %d",i);
    }
}

-(void)testExtractTBid
{
    uint32_t testData;
    for (int i = 0; i < 512; i++) {
        testData = i << 14;
        XCTAssert(GETFrame::ExtractTBid(testData) == i, @"Extracting TBid failed for value %d",i);
    }
}

-(void)testExtractSample
{
    uint32_t testData;
    for (int i = 0; i < 4095; i++) {
        testData = i;
        XCTAssert(GETFrame::ExtractSample(testData) == i, @"Extracting Sample failed for value %d",i);
    }
}

-(void)testDataExtractionCombined
{
    uint32_t testData;
    for (uint8_t aget = 0; aget < 4; aget++) {
        for (uint8_t ch = 0; ch < 68; ch++) {
            for (uint16_t tb = 0; tb < 512; tb++) {
                for (int16_t sample = 0; sample < 4095; sample++) {
                    testData = (aget << 30) | (ch << 23) | (tb << 14) | sample;
                    uint8_t aget_res = GETFrame::ExtractAgetId(testData);
                    uint8_t ch_res = GETFrame::ExtractChannel(testData);
                    uint16_t tb_res = GETFrame::ExtractTBid(testData);
                    int16_t sample_res = GETFrame::ExtractSample(testData);
                    XCTAssertEqual(aget, aget_res, @"AGET failed for %d", aget);
                    XCTAssertEqual(ch, ch_res, @"Channel failed for %d", ch);
                    XCTAssertEqual(tb, tb_res, @"TB failed for %d", tb);
                    XCTAssertEqual(sample, sample_res, @"Sample failed for %d", sample);
                }
            }
        }
    }
}

@end
