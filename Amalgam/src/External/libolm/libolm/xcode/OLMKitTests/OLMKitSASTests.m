/*
 Copyright 2019 New Vector Ltd

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#import <XCTest/XCTest.h>
#import <OLMKit/OLMKit.h>

@interface OLMKitSASTests : XCTestCase {
    OLMSAS *alice;
    OLMSAS *bob;
}

@end

@implementation OLMKitSASTests

- (void)setUp {
    alice = [OLMSAS new];
    bob = [OLMSAS new];
}

- (void)tearDown {
    alice = nil;
    bob = nil;
}

- (void)testSASRandomness
{
    XCTAssertNotEqualObjects(alice.publicKey, bob.publicKey);
}

- (void)testSASBytesMatch {
    [alice setTheirPublicKey:bob.publicKey];
    [bob setTheirPublicKey:alice.publicKey];

    NSString *sas = @"SAS";
    NSUInteger length = 5;

    XCTAssertEqualObjects([alice generateBytes:sas length:length],
                          [bob generateBytes:sas length:length]);
}

- (void)testMACsMatch {
    [alice setTheirPublicKey:bob.publicKey];
    [bob setTheirPublicKey:alice.publicKey];

    NSString *string = @"test";
    NSString *info = @"MAC";

    NSError *aliceError, *bobError;
    XCTAssertEqualObjects([alice calculateMac:string info:info error:&aliceError],
                          [bob calculateMac:string info:info error:&bobError]);
    XCTAssertNil(aliceError);
    XCTAssertNil(bobError);
}

- (void)testMACLongKdfsMatch {
    [alice setTheirPublicKey:bob.publicKey];
    [bob setTheirPublicKey:alice.publicKey];

    NSString *string = @"test";
    NSString *info = @"MAC";

    NSError *aliceError, *bobError;
    XCTAssertEqualObjects([alice calculateMacLongKdf:string info:info error:&aliceError],
                          [bob calculateMacLongKdf:string info:info error:&bobError]);
    XCTAssertNotEqualObjects([alice calculateMacLongKdf:string info:info error:&aliceError],
                             [bob calculateMac:string info:info error:&bobError]);
    XCTAssertNil(aliceError);
    XCTAssertNil(bobError);
}


@end
