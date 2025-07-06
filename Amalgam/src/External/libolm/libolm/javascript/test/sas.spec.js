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

var Olm = require('../olm');

describe("sas", function() {
    var alice, bob;

    beforeEach(function(done) {
        Olm.init().then(function() {
            alice = new Olm.SAS();
            bob = new Olm.SAS();

            done();
        });
    });

    afterEach(function () {
        if (alice !== undefined) {
            alice.free();
            alice = undefined;
        }
        if (bob !== undefined) {
            bob.free();
            bob = undefined;
        }
    });

    it('should create matching SAS bytes', function () {
        alice.set_their_key(bob.get_pubkey());
        bob.set_their_key(alice.get_pubkey());
        expect(alice.generate_bytes("SAS", 5).toString()).toEqual(bob.generate_bytes("SAS", 5).toString());
    });

    it('should create matching MACs', function () {
        alice.set_their_key(bob.get_pubkey());
        bob.set_their_key(alice.get_pubkey());
        expect(alice.calculate_mac("test", "MAC").toString()).toEqual(bob.calculate_mac("test", "MAC").toString());
    });

    it('should fail to generate bytes if their key is not set', function () {
        expect(alice.is_their_key_set()).toBeFalsy();
        expect(() => {
            alice.generate_bytes("SAS", 5);
        }).toThrow();
        alice.set_their_key(bob.get_pubkey());
        expect(alice.is_their_key_set()).toBeTruthy();
    });
});
