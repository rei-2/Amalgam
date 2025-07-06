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

if (!Object.keys) {
    Object.keys = function(o) {
        var k=[], p;
        for (p in o) if (Object.prototype.hasOwnProperty.call(o,p)) k.push(p);
        return k;
    }
}

describe("olm", function() {
    var aliceAccount, bobAccount;
    var aliceSession, bobSession;

    beforeEach(function(done) {
        // This should really be in a beforeAll, but jasmine-node
        // doesn't support that
        Olm.init().then(function() {
            aliceAccount = new Olm.Account();
            bobAccount = new Olm.Account();
            aliceSession = new Olm.Session();
            bobSession = new Olm.Session();

            done();
        });
    });

    afterEach(function() {
        if (aliceAccount !== undefined) {
            aliceAccount.free();
            aliceAccount = undefined;
        }

        if (bobAccount !== undefined) {
            bobAccount.free();
            bobAccount = undefined;
        }

        if (aliceSession !== undefined) {
            aliceSession.free();
            aliceSession = undefined;
        }

        if (bobSession !== undefined) {
            bobSession.free();
            bobSession = undefined;
        }
    });

    it('should encrypt and decrypt', function() {
        aliceAccount.create();
        bobAccount.create();

        bobAccount.generate_one_time_keys(1);
        var bobOneTimeKeys = JSON.parse(bobAccount.one_time_keys()).curve25519;
        bobAccount.mark_keys_as_published();

        var bobIdKey = JSON.parse(bobAccount.identity_keys()).curve25519;

        var otk_id = Object.keys(bobOneTimeKeys)[0];

        aliceSession.create_outbound(
            aliceAccount, bobIdKey, bobOneTimeKeys[otk_id]
        );

        var TEST_TEXT='têst1';
        var encrypted = aliceSession.encrypt(TEST_TEXT);
        expect(encrypted.type).toEqual(0);
        bobSession.create_inbound(bobAccount, encrypted.body);
        bobAccount.remove_one_time_keys(bobSession);
        var decrypted = bobSession.decrypt(encrypted.type, encrypted.body);
        console.log(TEST_TEXT, "->", decrypted);
        expect(decrypted).toEqual(TEST_TEXT);

        TEST_TEXT='hot beverage: ☕';
        encrypted = bobSession.encrypt(TEST_TEXT);
        expect(encrypted.type).toEqual(1);
        decrypted = aliceSession.decrypt(encrypted.type, encrypted.body);
        console.log(TEST_TEXT, "->", decrypted);
        expect(decrypted).toEqual(TEST_TEXT);
    });
});
