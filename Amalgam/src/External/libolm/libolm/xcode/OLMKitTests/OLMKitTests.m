/*
Copyright 2016 Chris Ballinger
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

@interface OLMKitTests : XCTestCase

@end

@implementation OLMKitTests

- (void)testAliceAndBob {
    OLMAccount *bob = [[OLMAccount alloc] initNewAccount];
    [bob generateOneTimeKeys:5];
    
    [self _testAliceAndBob:bob withBobKeys:bob.oneTimeKeys];
}

- (void)testAliceAndBobFallbackKey {
    OLMAccount *bob = [[OLMAccount alloc] initNewAccount];
    [bob generateFallbackKey];
    
    [self _testAliceAndBob:bob withBobKeys:bob.unpublishedFallbackKey];

}

- (void)testMarkAsPublishedFallbackKey {
    OLMAccount *bob = [[OLMAccount alloc] initNewAccount];
    [bob generateFallbackKey];
    
    
    NSDictionary *unpublished = bob.unpublishedFallbackKey;
    __block NSString *bobKeyValue = ((NSDictionary *) unpublished[@"curve25519"]).allValues.lastObject;
    
    XCTAssertNotNil(bobKeyValue);
    
    [bob markOneTimeKeysAsPublished];
    
    NSDictionary *unpublishedAfter = bob.unpublishedFallbackKey;
    
    __block NSString *bobKeyValueAfter = ((NSDictionary *) unpublishedAfter[@"curve25519"]).allValues.lastObject;
    
    XCTAssertNil(bobKeyValueAfter);
}

- (void)_testAliceAndBob:(OLMAccount *)bob withBobKeys:(NSDictionary *)bobKeys {
    XCTAssertNotNil(bob);
    XCTAssertNotNil(bobKeys);
    
    NSError *error;
    
    NSString *bobIdKey = bob.identityKeys[@"curve25519"];
    XCTAssertNotNil(bobIdKey);
    
    __block NSString *bobKeyValue = nil;
    [bobKeys[@"curve25519"] enumerateKeysAndObjectsUsingBlock:^(id  _Nonnull key, id  _Nonnull obj, BOOL * _Nonnull stop) {
        bobKeyValue = obj;
    }];
    XCTAssert([bobKeyValue isKindOfClass:[NSString class]]);
    
    OLMAccount *alice = [[OLMAccount alloc] initNewAccount];
    
    OLMSession *aliceSession = [[OLMSession alloc] initOutboundSessionWithAccount:alice theirIdentityKey:bobIdKey theirOneTimeKey:bobKeyValue error:nil];
    NSString *message = @"Hello!";
    OLMMessage *aliceToBobMsg = [aliceSession encryptMessage:message error:&error];
    XCTAssertNil(error);
    
    OLMSession *bobSession = [[OLMSession alloc] initInboundSessionWithAccount:bob oneTimeKeyMessage:aliceToBobMsg.ciphertext error:nil];
    NSString *plaintext = [bobSession decryptMessage:aliceToBobMsg error:&error];
    XCTAssertEqualObjects(message, plaintext);
    XCTAssertNil(error);

    XCTAssert([bobSession matchesInboundSession:aliceToBobMsg.ciphertext]);
    XCTAssertFalse([aliceSession matchesInboundSession:@"ARandomOtkMessage"]);

    NSString *aliceIdKey = alice.identityKeys[@"curve25519"];
    XCTAssert([bobSession matchesInboundSessionFrom:aliceIdKey oneTimeKeyMessage:aliceToBobMsg.ciphertext]);
    XCTAssertFalse([bobSession matchesInboundSessionFrom:@"ARandomIdKey" oneTimeKeyMessage:aliceToBobMsg.ciphertext]);
    XCTAssertFalse([bobSession matchesInboundSessionFrom:aliceIdKey oneTimeKeyMessage:@"ARandomOtkMessage"]);

    BOOL success = [bob removeOneTimeKeysForSession:bobSession];
    XCTAssertTrue(success);
}

- (void)testBackAndForthWithOneTimeKeys {
    OLMAccount *bob = [[OLMAccount alloc] initNewAccount];
    [bob generateOneTimeKeys:1];
    
    [self _testBackAndForthWithBob:bob andBobKeys:bob.oneTimeKeys];
}

- (void)testBackAndForthWithFallbackKey {
    OLMAccount *bob = [[OLMAccount alloc] initNewAccount];
    [bob generateFallbackKey];
    
    [self _testBackAndForthWithBob:bob andBobKeys:bob.unpublishedFallbackKey];
}

- (void)_testBackAndForthWithBob:(OLMAccount *)bob andBobKeys:(NSDictionary *)bobKeys {
    XCTAssertNotNil(bob);
    XCTAssertNotNil(bobKeys);
    
    NSString *bobIdKey = bob.identityKeys[@"curve25519"];
    XCTAssertNotNil(bobIdKey);

    __block NSString *bobKeyValue = nil;
    [bobKeys[@"curve25519"] enumerateKeysAndObjectsUsingBlock:^(id  _Nonnull key, id  _Nonnull obj, BOOL * _Nonnull stop) {
        bobKeyValue = obj;
    }];
    XCTAssert([bobKeyValue isKindOfClass:[NSString class]]);
    
    OLMAccount *alice = [[OLMAccount alloc] initNewAccount];
    
    OLMSession *aliceSession = [[OLMSession alloc] initOutboundSessionWithAccount:alice theirIdentityKey:bobIdKey theirOneTimeKey:bobKeyValue error:nil];
    NSString *message = @"Hello I'm Alice!";
    OLMMessage *aliceToBobMsg = [aliceSession encryptMessage:message error:nil];
    
    OLMSession *bobSession = [[OLMSession alloc] initInboundSessionWithAccount:bob oneTimeKeyMessage:aliceToBobMsg.ciphertext error:nil];
    NSString *plaintext = [bobSession decryptMessage:aliceToBobMsg error:nil];
    XCTAssertEqualObjects(message, plaintext);
    
    BOOL success = [bob removeOneTimeKeysForSession:bobSession];
    XCTAssertTrue(success);
    
    NSString *msg1 = @"Hello I'm Bob!";
    NSString *msg2 = @"Isn't life grand?";
    NSString *msg3 = @"Let's go to the opera.";
    
    OLMMessage *eMsg1 = [bobSession encryptMessage:msg1 error:nil];
    OLMMessage *eMsg2 = [bobSession encryptMessage:msg2 error:nil];
    OLMMessage *eMsg3 = [bobSession encryptMessage:msg3 error:nil];
    
    NSString *dMsg1 = [aliceSession decryptMessage:eMsg1 error:nil];
    NSString *dMsg2 = [aliceSession decryptMessage:eMsg2 error:nil];
    NSString *dMsg3 = [aliceSession decryptMessage:eMsg3 error:nil];
    XCTAssertEqualObjects(msg1, dMsg1);
    XCTAssertEqualObjects(msg2, dMsg2);
    XCTAssertEqualObjects(msg3, dMsg3);
}

- (void)testAccountSerialization {
    OLMAccount *bob = [[OLMAccount alloc] initNewAccount];
    [bob generateOneTimeKeys:5];
    [bob generateFallbackKey];
    NSDictionary *bobIdKeys = bob.identityKeys;
    NSDictionary *bobOneTimeKeys = bob.oneTimeKeys;
    NSDictionary *bobFallbackKey = bob.unpublishedFallbackKey;
    
    NSError *error;
    NSData *bobData = [NSKeyedArchiver archivedDataWithRootObject:bob requiringSecureCoding:NO error:&error];
    XCTAssertNil(error);
    
    OLMAccount *bob2 = [NSKeyedUnarchiver unarchivedObjectOfClass:[OLMAccount class] fromData:bobData error:&error];
    XCTAssertNil(error);
    
    NSDictionary *bobIdKeys2 = bob2.identityKeys;
    NSDictionary *bobOneTimeKeys2 = bob2.oneTimeKeys;
    NSDictionary *bobFallbackKey2 = bob2.unpublishedFallbackKey;
    
    XCTAssertEqualObjects(bobIdKeys, bobIdKeys2);
    XCTAssertEqualObjects(bobOneTimeKeys, bobOneTimeKeys2);
    XCTAssertEqualObjects(bobFallbackKey, bobFallbackKey2);
}

- (void)testSessionSerializationWithOneTimeKey {
    OLMAccount *bob = [[OLMAccount alloc] initNewAccount];
    [bob generateOneTimeKeys:1];
    
    [self _testSessionSerializationWithBob:bob bobKeys:bob.oneTimeKeys];
}

- (void)testSessionSerializationWithFallbackKey {
    OLMAccount *bob = [[OLMAccount alloc] initNewAccount];
    [bob generateFallbackKey];
    
    [self _testSessionSerializationWithBob:bob bobKeys:bob.unpublishedFallbackKey];
}

- (void)_testSessionSerializationWithBob:(OLMAccount *)bob bobKeys:(NSDictionary *)bobKeys {
    NSError *error;
    
    NSString *bobIdKey = bob.identityKeys[@"curve25519"];
    XCTAssertNotNil(bobIdKey);
    
    __block NSString *bobKeyValue = nil;
    [bobKeys[@"curve25519"] enumerateKeysAndObjectsUsingBlock:^(id  _Nonnull key, id  _Nonnull obj, BOOL * _Nonnull stop) {
        bobKeyValue = obj;
    }];
    XCTAssert([bobKeyValue isKindOfClass:[NSString class]]);
    
    OLMAccount *alice = [[OLMAccount alloc] initNewAccount];
    
    OLMSession *aliceSession = [[OLMSession alloc] initOutboundSessionWithAccount:alice theirIdentityKey:bobIdKey theirOneTimeKey:bobKeyValue error:nil];
    NSString *message = @"Hello I'm Alice!";
    
    OLMMessage *aliceToBobMsg = [aliceSession encryptMessage:message error:&error];
    XCTAssertNil(error);
    
    OLMSession *bobSession = [[OLMSession alloc] initInboundSessionWithAccount:bob oneTimeKeyMessage:aliceToBobMsg.ciphertext error:nil];
    NSString *plaintext = [bobSession decryptMessage:aliceToBobMsg error:nil];
    XCTAssertEqualObjects(message, plaintext);
    
    BOOL success = [bob removeOneTimeKeysForSession:bobSession];
    XCTAssertTrue(success);
    
    NSString *msg1 = @"Hello I'm Bob!";
    NSString *msg2 = @"Isn't life grand?";
    NSString *msg3 = @"Let's go to the opera.";
    
    OLMMessage *eMsg1 = [bobSession encryptMessage:msg1 error:nil];
    OLMMessage *eMsg2 = [bobSession encryptMessage:msg2 error:nil];
    OLMMessage *eMsg3 = [bobSession encryptMessage:msg3 error:nil];
    
    NSData *aliceData = [NSKeyedArchiver archivedDataWithRootObject:aliceSession requiringSecureCoding:NO error:&error];
    XCTAssertNil(error);
    
    OLMSession *alice2 = [NSKeyedUnarchiver unarchivedObjectOfClass:[OLMSession class] fromData:aliceData error:&error];
    XCTAssertNil(error);
    
    NSString *dMsg1 = [alice2 decryptMessage:eMsg1 error:nil];
    NSString *dMsg2 = [alice2 decryptMessage:eMsg2 error:nil];
    NSString *dMsg3 = [alice2 decryptMessage:eMsg3 error:nil];
    XCTAssertEqualObjects(msg1, dMsg1);
    XCTAssertEqualObjects(msg2, dMsg2);
    XCTAssertEqualObjects(msg3, dMsg3);
}

- (void)testEd25519Signing {
    NSError *error;

    OLMUtility *olmUtility = [[OLMUtility alloc] init];
    OLMAccount *alice = [[OLMAccount alloc] initNewAccount];

    NSDictionary *aJSON = @{
                            @"key1": @"value1",
                            @"key2": @"value2"
                            };
    NSData *message = [NSKeyedArchiver archivedDataWithRootObject:aJSON requiringSecureCoding:NO error:&error];
    XCTAssertNil(error);
    
    NSString *signature = [alice signMessage:message];

    NSString *aliceEd25519Key = alice.identityKeys[@"ed25519"];

    BOOL result = [olmUtility verifyEd25519Signature:signature key:aliceEd25519Key message:message error:&error];
    XCTAssert(result);
    XCTAssertNil(error);
}

@end
