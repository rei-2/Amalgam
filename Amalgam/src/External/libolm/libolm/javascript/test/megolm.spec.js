/*
Copyright 2016 OpenMarket Ltd
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

"use strict";

var Olm = require('../olm');

describe("megolm", function() {
    var aliceSession, bobSession;

    beforeEach(function(done) {
        Olm.init().then(function() {
            aliceSession = new Olm.OutboundGroupSession();
            bobSession = new Olm.InboundGroupSession();

            done();
        });
    });

    afterEach(function() {
        if (aliceSession !== undefined) {
            aliceSession.free();
            aliceSession = undefined;
        }

        if (bobSession !== undefined) {
            bobSession.free();
            bobSession = undefined;
        }
    });

    it("should encrypt and decrypt", function() {
        aliceSession.create();
        expect(aliceSession.message_index()).toEqual(0);
        bobSession.create(aliceSession.session_key());

        var TEST_TEXT='têst1';
        var encrypted = aliceSession.encrypt(TEST_TEXT);
        var decrypted = bobSession.decrypt(encrypted);
        console.log(TEST_TEXT, "->", decrypted);
        expect(decrypted.plaintext).toEqual(TEST_TEXT);
        expect(decrypted.message_index).toEqual(0);

        TEST_TEXT='hot beverage: ☕';
        encrypted = aliceSession.encrypt(TEST_TEXT);
        decrypted = bobSession.decrypt(encrypted);
        console.log(TEST_TEXT, "->", decrypted);
        expect(decrypted.plaintext).toEqual(TEST_TEXT);
        expect(decrypted.message_index).toEqual(1);

        // shorter text, to spot buffer overruns
        TEST_TEXT='☕';
        encrypted = aliceSession.encrypt(TEST_TEXT);
        decrypted = bobSession.decrypt(encrypted);
        console.log(TEST_TEXT, "->", decrypted);
        expect(decrypted.plaintext).toEqual(TEST_TEXT);
        expect(decrypted.message_index).toEqual(2);
    });
});
