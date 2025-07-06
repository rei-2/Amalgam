from builtins import int

import pytest

from olm import Account, OlmAccountError, OlmVerifyError, ed25519_verify
from olm._compat import to_bytes


class TestClass(object):
    def test_to_bytes(self):
        assert isinstance(to_bytes("a"), bytes)
        assert isinstance(to_bytes(u"a"), bytes)
        assert isinstance(to_bytes(b"a"), bytes)
        assert isinstance(to_bytes(r"a"), bytes)
        with pytest.raises(TypeError):
            to_bytes(0)

    def test_account_creation(self):
        alice = Account()
        assert alice.identity_keys
        assert len(alice.identity_keys) == 2

    def test_account_pickle(self):
        alice = Account()
        pickle = alice.pickle()
        assert (alice.identity_keys == Account.from_pickle(pickle)
                .identity_keys)

    def test_invalid_unpickle(self):
        with pytest.raises(ValueError):
            Account.from_pickle(b"")

    def test_passphrase_pickle(self):
        alice = Account()
        passphrase = "It's a secret to everybody"
        pickle = alice.pickle(passphrase)
        assert (alice.identity_keys == Account.from_pickle(
            pickle, passphrase).identity_keys)

    def test_wrong_passphrase_pickle(self):
        alice = Account()
        passphrase = "It's a secret to everybody"
        pickle = alice.pickle(passphrase)

        with pytest.raises(OlmAccountError):
            Account.from_pickle(pickle, "")

    def test_one_time_keys(self):
        alice = Account()
        alice.generate_one_time_keys(10)
        one_time_keys = alice.one_time_keys
        assert one_time_keys
        assert len(one_time_keys["curve25519"]) == 10

    def test_max_one_time_keys(self):
        alice = Account()
        assert isinstance(alice.max_one_time_keys, int)

    def test_publish_one_time_keys(self):
        alice = Account()
        alice.generate_one_time_keys(10)
        one_time_keys = alice.one_time_keys

        assert one_time_keys
        assert len(one_time_keys["curve25519"]) == 10

        alice.mark_keys_as_published()
        assert not alice.one_time_keys["curve25519"]

    def test_clear(self):
        alice = Account()
        del alice

    def test_valid_signature(self):
        message = "It's a secret to everybody"
        alice = Account()

        signature = alice.sign(message)
        signing_key = alice.identity_keys["ed25519"]

        assert signature
        assert signing_key

        ed25519_verify(signing_key, message, signature)

    def test_invalid_signature(self):
        message = "It's a secret to everybody"
        alice = Account()
        bob = Account()

        signature = alice.sign(message)
        signing_key = bob.identity_keys["ed25519"]

        assert signature
        assert signing_key

        with pytest.raises(OlmVerifyError):
            ed25519_verify(signing_key, message, signature)

    def test_signature_verification_twice(self):
        message = "It's a secret to everybody"
        alice = Account()

        signature = alice.sign(message)
        signing_key = alice.identity_keys["ed25519"]

        assert signature
        assert signing_key

        ed25519_verify(signing_key, message, signature)
        assert signature == alice.sign(message)

        ed25519_verify(signing_key, message, signature)
        assert signature == alice.sign(message)
