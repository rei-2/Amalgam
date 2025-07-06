/*
Copyright 2018, 2019 New Vector Ltd

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

"use strict";

var Olm = require('../olm');

describe("pk", function() {
    var encryption, decryption, signing;

    beforeEach(function(done) {
        Olm.init().then(function() {
            encryption = new Olm.PkEncryption();
            decryption = new Olm.PkDecryption();
            signing = new Olm.PkSigning();

            done();
        });
    });

    afterEach(function () {
        if (encryption !== undefined) {
            encryption.free();
            encryption = undefined;
        }
        if (decryption !== undefined) {
            decryption.free();
            decryption = undefined;
        }
        if (signing !== undefined) {
            signing.free();
            signing = undefined;
        }
    });

    it('should import & export keys from private parts', function () {
        var alice_private = new Uint8Array([
            0x77, 0x07, 0x6D, 0x0A, 0x73, 0x18, 0xA5, 0x7D,
            0x3C, 0x16, 0xC1, 0x72, 0x51, 0xB2, 0x66, 0x45,
            0xDF, 0x4C, 0x2F, 0x87, 0xEB, 0xC0, 0x99, 0x2A,
            0xB1, 0x77, 0xFB, 0xA5, 0x1D, 0xB9, 0x2C, 0x2A
        ]);
        var alice_public = decryption.init_with_private_key(alice_private);
        expect(alice_public).toEqual("hSDwCYkwp1R0i33ctD73Wg2/Og0mOBr066SpjqqbTmo");

        var alice_private_out = decryption.get_private_key();
        expect(alice_private_out).toEqual(alice_private);
    });

    it('should encrypt and decrypt', function () {
        var TEST_TEXT='têst1';
        var pubkey = decryption.generate_key();
        encryption.set_recipient_key(pubkey);
        var encrypted = encryption.encrypt(TEST_TEXT);
        var decrypted = decryption.decrypt(encrypted.ephemeral, encrypted.mac, encrypted.ciphertext);
        console.log(TEST_TEXT, "->", decrypted);
        expect(decrypted).toEqual(TEST_TEXT);

        TEST_TEXT='hot beverage: ☕';
        encryption.set_recipient_key(pubkey);
        encrypted = encryption.encrypt(TEST_TEXT);
        decrypted = decryption.decrypt(encrypted.ephemeral, encrypted.mac, encrypted.ciphertext);
        console.log(TEST_TEXT, "->", decrypted);
        expect(decrypted).toEqual(TEST_TEXT);
    });

    it('should pickle and unpickle', function () {
        var TEST_TEXT = 'têst1';
        var pubkey = decryption.generate_key();
        encryption.set_recipient_key(pubkey);
        var encrypted = encryption.encrypt(TEST_TEXT);

        var PICKLE_KEY = 'secret_key';
        var pickle = decryption.pickle(PICKLE_KEY);

        var new_decryption = new Olm.PkDecryption();
        var new_pubkey = new_decryption.unpickle(PICKLE_KEY, pickle);
        expect(new_pubkey).toEqual(pubkey);
        var decrypted = new_decryption.decrypt(encrypted.ephemeral, encrypted.mac, encrypted.ciphertext);
        console.log(TEST_TEXT, "->", decrypted);
        expect(decrypted).toEqual(TEST_TEXT);
        new_decryption.free();
    });

    it('should sign and verify', function () {
        var seed = new Uint8Array([
            0x77, 0x07, 0x6D, 0x0A, 0x73, 0x18, 0xA5, 0x7D,
            0x3C, 0x16, 0xC1, 0x72, 0x51, 0xB2, 0x66, 0x45,
            0xDF, 0x4C, 0x2F, 0x87, 0xEB, 0xC0, 0x99, 0x2A,
            0xB1, 0x77, 0xFB, 0xA5, 0x1D, 0xB9, 0x2C, 0x2A
        ]);

        var TEST_TEXT = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness.";
        //var seed = signing.generate_seed();
        var pubkey = signing.init_with_seed(seed);
        var sig = signing.sign(TEST_TEXT);

        var util = new Olm.Utility();
        util.ed25519_verify(pubkey, TEST_TEXT, sig);
        var verifyFailure;
        try {
            util.ed25519_verify(pubkey, TEST_TEXT, 'p' + sig.slice(1));
        } catch (e) {
            verifyFailure = e;
        }
        expect(verifyFailure).not.toBeNull();
        util.free();
    });
});
