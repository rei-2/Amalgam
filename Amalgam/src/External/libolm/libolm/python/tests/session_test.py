# -*- coding: utf-8 -*-
import pytest

from olm import (Account, InboundSession, OlmMessage, OlmPreKeyMessage,
                 OlmSessionError, OutboundSession, Session)


class TestClass(object):
    def _create_session(self):
        alice = Account()
        bob = Account()
        bob.generate_one_time_keys(1)
        id_key = bob.identity_keys["curve25519"]
        one_time = list(bob.one_time_keys["curve25519"].values())[0]
        session = OutboundSession(alice, id_key, one_time)
        return alice, bob, session

    def test_session_create(self):
        _, _, session_1 = self._create_session()
        _, _, session_2 = self._create_session()
        assert session_1
        assert session_2
        assert session_1.id != session_2.id
        assert isinstance(session_1.id, str)

    def test_session_clear(self):
        _, _, session = self._create_session()
        del session

    def test_invalid_session_create(self):
        with pytest.raises(TypeError):
            Session()

    def test_session_pickle(self):
        alice, bob, session = self._create_session()
        Session.from_pickle(session.pickle()).id == session.id

    def test_session_invalid_pickle(self):
        with pytest.raises(ValueError):
            Session.from_pickle(b"")

    def test_wrong_passphrase_pickle(self):
        alice, bob, session = self._create_session()
        passphrase = "It's a secret to everybody"
        pickle = alice.pickle(passphrase)

        with pytest.raises(OlmSessionError):
            Session.from_pickle(pickle, "")

    def test_encrypt(self):
        plaintext = "It's a secret to everybody"
        alice, bob, session = self._create_session()
        message = session.encrypt(plaintext)

        assert (repr(message)
                == "OlmPreKeyMessage({})".format(message.ciphertext))

        assert (str(message)
                == "PRE_KEY {}".format(message.ciphertext))

        bob_session = InboundSession(bob, message)
        assert plaintext == bob_session.decrypt(message)

    def test_empty_message(self):
        with pytest.raises(ValueError):
            OlmPreKeyMessage("")
        empty = OlmPreKeyMessage("x")
        empty.ciphertext = ""
        alice, bob, session = self._create_session()

        with pytest.raises(ValueError):
            session.decrypt(empty)

    def test_inbound_with_id(self):
        plaintext = "It's a secret to everybody"
        alice, bob, session = self._create_session()
        message = session.encrypt(plaintext)
        alice_id = alice.identity_keys["curve25519"]
        bob_session = InboundSession(bob, message, alice_id)
        assert plaintext == bob_session.decrypt(message)

    def test_two_messages(self):
        plaintext = "It's a secret to everybody"
        alice, bob, session = self._create_session()
        message = session.encrypt(plaintext)
        alice_id = alice.identity_keys["curve25519"]
        bob_session = InboundSession(bob, message, alice_id)
        bob.remove_one_time_keys(bob_session)
        assert plaintext == bob_session.decrypt(message)

        bob_plaintext = "Grumble, Grumble"
        bob_message = bob_session.encrypt(bob_plaintext)

        assert (repr(bob_message)
                == "OlmMessage({})".format(bob_message.ciphertext))

        assert bob_plaintext == session.decrypt(bob_message)

    def test_matches(self):
        plaintext = "It's a secret to everybody"
        alice, bob, session = self._create_session()
        message = session.encrypt(plaintext)
        alice_id = alice.identity_keys["curve25519"]
        bob_session = InboundSession(bob, message, alice_id)
        assert plaintext == bob_session.decrypt(message)

        message_2nd = session.encrypt("Hey! Listen!")

        assert bob_session.matches(message_2nd) is True
        assert bob_session.matches(message_2nd, alice_id) is True

    def test_invalid(self):
        alice, bob, session = self._create_session()
        message = OlmMessage("x")

        with pytest.raises(TypeError):
            session.matches(message)

        message = OlmPreKeyMessage("x")
        message.ciphertext = ""

        with pytest.raises(ValueError):
            session.matches(message)

        with pytest.raises(ValueError):
            InboundSession(bob, message)

        with pytest.raises(ValueError):
            OutboundSession(alice, "", "x")

        with pytest.raises(ValueError):
            OutboundSession(alice, "x", "")

    def test_doesnt_match(self):
        plaintext = "It's a secret to everybody"
        alice, bob, session = self._create_session()
        message = session.encrypt(plaintext)
        alice_id = alice.identity_keys["curve25519"]
        bob_session = InboundSession(bob, message, alice_id)

        _, _, new_session = self._create_session()

        new_message = new_session.encrypt(plaintext)
        assert bob_session.matches(new_message) is False

    def test_invalid_unicode_decrypt(self):
        alice, bob, session = self._create_session()
        message = session.encrypt(b"\xed")

        bob_session = InboundSession(bob, message)
        plaintext = bob_session.decrypt(message)
        assert plaintext == u"�"
