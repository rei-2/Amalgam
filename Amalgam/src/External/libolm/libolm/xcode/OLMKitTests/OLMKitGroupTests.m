/*
 Copyright 2016 OpenMarket Ltd
 Copyright 2016 Vector Creations Ltd

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

#include "olm/olm.h"

@interface OLMKitGroupTests : XCTestCase

@end

@implementation OLMKitGroupTests

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testAliceAndBob {
    NSError *error;

    OLMOutboundGroupSession *aliceSession = [[OLMOutboundGroupSession alloc] initOutboundGroupSession];
    XCTAssertGreaterThan(aliceSession.sessionIdentifier.length, 0);
    XCTAssertGreaterThan(aliceSession.sessionKey.length, 0);
    XCTAssertEqual(aliceSession.messageIndex, 0);

    // Store the session key before starting encrypting
    NSString *sessionKey = aliceSession.sessionKey;

    NSString *message = @"Hello!";
    NSString *aliceToBobMsg = [aliceSession encryptMessage:message error:&error];

    XCTAssertEqual(aliceSession.messageIndex, 1);
    XCTAssertGreaterThanOrEqual(aliceToBobMsg.length, 0);
    XCTAssertNil(error);

    OLMInboundGroupSession *bobSession = [[OLMInboundGroupSession alloc] initInboundGroupSessionWithSessionKey:sessionKey error:&error];
    XCTAssertEqualObjects(aliceSession.sessionIdentifier, bobSession.sessionIdentifier);
    XCTAssertNil(error);

    NSUInteger messageIndex;

    NSString *plaintext = [bobSession decryptMessage:aliceToBobMsg messageIndex:&messageIndex error:&error];
    XCTAssertEqualObjects(message, plaintext);

    XCTAssertEqual(messageIndex, 0);
    XCTAssertNil(error);
}

- (void)testOutboundGroupSessionSerialization {

    OLMOutboundGroupSession *aliceSession = [[OLMOutboundGroupSession alloc] initOutboundGroupSession];

    NSData *aliceData = [NSKeyedArchiver archivedDataWithRootObject:aliceSession];
    OLMOutboundGroupSession *aliceSession2 = [NSKeyedUnarchiver unarchiveObjectWithData:aliceData];

    XCTAssertEqualObjects(aliceSession2.sessionKey, aliceSession.sessionKey);
    XCTAssertEqualObjects(aliceSession2.sessionIdentifier, aliceSession.sessionIdentifier);
}

- (void)testInboundGroupSessionSerialization {

    OLMOutboundGroupSession *aliceSession = [[OLMOutboundGroupSession alloc] initOutboundGroupSession];

    OLMInboundGroupSession *bobSession = [[OLMInboundGroupSession alloc] initInboundGroupSessionWithSessionKey:aliceSession.sessionKey error:nil];

    NSData *bobData = [NSKeyedArchiver archivedDataWithRootObject:bobSession];
    OLMInboundGroupSession *bobSession2 = [NSKeyedUnarchiver unarchiveObjectWithData:bobData];

    XCTAssertEqualObjects(bobSession2.sessionIdentifier, aliceSession.sessionIdentifier);
}

- (void)testInboundGroupSessionImportExport {

    NSError *error;

    NSString *sessionKey = @"AgAAAAAwMTIzNDU2Nzg5QUJERUYwMTIzNDU2Nzg5QUJDREVGMDEyMzQ1Njc4OUFCREVGM" \
                            "DEyMzQ1Njc4OUFCQ0RFRjAxMjM0NTY3ODlBQkRFRjAxMjM0NTY3ODlBQkNERUYwMTIzND" \
                            "U2Nzg5QUJERUYwMTIzNDU2Nzg5QUJDREVGMDEyMw0bdg1BDq4Px/slBow06q8n/B9WBfw" \
                            "WYyNOB8DlUmXGGwrFmaSb9bR/eY8xgERrxmP07hFmD9uqA2p8PMHdnV5ysmgufE6oLZ5+" \
                            "8/mWQOW3VVTnDIlnwd8oHUYRuk8TCQ";

    NSString *message = @"AwgAEhAcbh6UpbByoyZxufQ+h2B+8XHMjhR69G8F4+qjMaFlnIXusJZX3r8LnRORG9T3D" \
                        "XFdbVuvIWrLyRfm4i8QRbe8VPwGRFG57B1CtmxanuP8bHtnnYqlwPsD";

    // init first inbound group session, and decrypt */
    OLMInboundGroupSession *session1 = [[OLMInboundGroupSession alloc] initInboundGroupSessionWithSessionKey:sessionKey error:&error];

    XCTAssertNil(error);
    XCTAssertTrue(session1.isVerified);

    // decrypt the message
    NSUInteger messageIndex;
    NSString *plaintext = [session1 decryptMessage:message messageIndex:&messageIndex error:&error];

    XCTAssertNil(error);
    XCTAssertEqualObjects(plaintext, @"Message");
    XCTAssertEqual(messageIndex, 0);

    // export the keys
    NSString *export = [session1 exportSessionAtMessageIndex:0 error:&error];

    XCTAssertNil(error);
    XCTAssertGreaterThan(export.length, 0);

    // free the old session to check there is no shared data
    session1 = nil;

    // import the keys into another inbound group session
    OLMInboundGroupSession *session2 = [[OLMInboundGroupSession alloc] initInboundGroupSessionWithImportedSession:export error:&error];

    XCTAssertNil(error);
    XCTAssert(session2);
    XCTAssertFalse(session2.isVerified);

    // decrypt the message with the new session
    NSString *plaintext2 = [session2 decryptMessage:message messageIndex:&messageIndex error:&error];

    XCTAssertNil(error);
    XCTAssertEqualObjects(plaintext2, @"Message");
    XCTAssertEqual(messageIndex, 0);
    XCTAssertTrue(session2.isVerified);
}

@end
