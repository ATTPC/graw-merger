//
//  GETDataFileTests.m
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/9/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#import <XCTest/XCTest.h>
#include "GETDataFile.h"
#include "GETFrame.h"
#include <boost/filesystem.hpp>

@interface GETDataFileTests : XCTestCase {
    @private
    GETDataFile* dataFile;
    GETFrame* frame;
    boost::filesystem::path* validPath;
    boost::filesystem::path* invalidPath;
}

@end

@implementation GETDataFileTests

- (void)setUp
{
    [super setUp];
    validPath = new boost::filesystem::path ("/Users/josh/Documents/Data/20140323182024/mm0/CoBo_AsAd0_2014-03-23T18/20/49.741_0000.graw");
}

- (void)tearDown
{
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

//- (void)testExample
//{
//    XCTFail(@"No implementation for \"%s\"", __PRETTY_FUNCTION__);
//}

//- (void)testConstructorWithValidPath
//{
//    dataFile = new GETDataFile(*validPath);
//    //XCTAssertNoThrow(frame = new GETFrame(dataFile->GetNextRawFrame()));
//}

@end
