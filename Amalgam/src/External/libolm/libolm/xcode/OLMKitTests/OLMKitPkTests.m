/*
 Copyright 2018 New Vector Ltd

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

/**
 Tests are inspired from js tests.
 */
@interface OLMKitPkTests : XCTestCase {
    OLMPkEncryption *encryption;
    OLMPkDecryption *decryption;
}

@end

@implementation OLMKitPkTests

- (void)setUp {
    encryption = [OLMPkEncryption new];
    decryption = [OLMPkDecryption new];
}

- (void)tearDown {
    encryption = nil;
    decryption = nil;
}

- (void)testImportExportKeys {
    UInt8 alicePrivateBytes[] = {
                            0x77, 0x07, 0x6D, 0x0A, 0x73, 0x18, 0xA5, 0x7D,
                            0x3C, 0x16, 0xC1, 0x72, 0x51, 0xB2, 0x66, 0x45,
                            0xDF, 0x4C, 0x2F, 0x87, 0xEB, 0xC0, 0x99, 0x2A,
                            0xB1, 0x77, 0xFB, 0xA5, 0x1D, 0xB9, 0x2C, 0x2A
    };

    NSData *alicePrivate = [NSData dataWithBytes:alicePrivateBytes length:sizeof(alicePrivateBytes)];

    NSError *error;
    NSString *alicePublic = [decryption setPrivateKey:alicePrivate error:&error];
    XCTAssertNil(error);
    XCTAssertEqualObjects(alicePublic, @"hSDwCYkwp1R0i33ctD73Wg2/Og0mOBr066SpjqqbTmo");

    NSData *alicePrivateOut = decryption.privateKey;
    XCTAssertNil(error);
    XCTAssertEqualObjects(alicePrivateOut, alicePrivate);
}

- (void)testEncryptAndDecrypt {

    NSString *pubKey = [decryption generateKey:nil];
    NSLog(@"Ephemeral Key: %@", pubKey);
    XCTAssertNotNil(pubKey);

    NSString *TEST_TEXT = @"têst1";
    NSError *error;
    [encryption setRecipientKey:pubKey];
    OLMPkMessage *message = [encryption encryptMessage:TEST_TEXT error:&error];
    NSLog(@"message: %@ %@ %@", message.ciphertext, message.mac, message.ephemeralKey);
    XCTAssertNil(error);
    XCTAssertNotNil(message);
    XCTAssertNotNil(message.ciphertext);
    XCTAssertNotNil(message.mac);
    XCTAssertNotNil(message.ephemeralKey);

    NSString *decrypted = [decryption decryptMessage:message error:&error];
    XCTAssertNil(error);
    XCTAssertEqualObjects(decrypted, TEST_TEXT);

    TEST_TEXT = @"hot beverage: ☕";
    [encryption setRecipientKey:pubKey];
    message = [encryption encryptMessage:TEST_TEXT error:&error];
    decrypted = [decryption decryptMessage:message error:&error];
    XCTAssertEqualObjects(decrypted, TEST_TEXT);
}

- (void)testOLMPkDecryptionSerialization {
    NSString *TEST_TEXT = @"têst1";
    NSString *pubKey = [decryption generateKey:nil];
    [encryption setRecipientKey:pubKey];
    OLMPkMessage *encrypted = [encryption encryptMessage:TEST_TEXT error:nil];


    NSData *pickle = [NSKeyedArchiver archivedDataWithRootObject:decryption];
    decryption = nil;

    OLMPkDecryption *newDecryption = [NSKeyedUnarchiver unarchiveObjectWithData:pickle];

    NSError *error;
    NSString *decrypted = [newDecryption decryptMessage:encrypted error:&error];
    XCTAssertEqualObjects(decrypted, TEST_TEXT);
}

- (void)testSignAndVerify {

    UInt8 seedBytes[] = {
        0x77, 0x07, 0x6D, 0x0A, 0x73, 0x18, 0xA5, 0x7D,
        0x3C, 0x16, 0xC1, 0x72, 0x51, 0xB2, 0x66, 0x45,
        0xDF, 0x4C, 0x2F, 0x87, 0xEB, 0xC0, 0x99, 0x2A,
        0xB1, 0x77, 0xFB, 0xA5, 0x1D, 0xB9, 0x2C, 0x2A
    };

    NSData *seed = [NSData dataWithBytes:seedBytes length:sizeof(seedBytes)];

    NSString *TEST_TEXT = @"We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness.";

    OLMPkSigning *signing = [OLMPkSigning new];

    NSError *error;
    NSString *pubKey = [signing doInitWithSeed:seed error:&error];
    XCTAssertNotNil(pubKey);
    XCTAssertNil(error);

    NSString *sig = [signing sign:TEST_TEXT error:&error];
    XCTAssertNotNil(sig);
    XCTAssertNil(error);

    OLMUtility *util = [OLMUtility new];
    BOOL verify = [util verifyEd25519Signature:sig key:pubKey message:[TEST_TEXT dataUsingEncoding:NSUTF8StringEncoding] error:&error];
    XCTAssertTrue(verify);
    XCTAssertNil(error);

    NSString *badSig = [sig stringByReplacingCharactersInRange:NSMakeRange(0, 1) withString:@"p"];
    verify = [util verifyEd25519Signature:badSig key:pubKey message:[TEST_TEXT dataUsingEncoding:NSUTF8StringEncoding] error:&error];
    XCTAssertFalse(verify);
    XCTAssertNotNil(error);
}

@end
