# -*- coding: utf-8 -*-
import pytest

from olm import InboundGroupSession, OlmGroupSessionError, OutboundGroupSession


class TestClass(object):
    def test_session_create(self):
        OutboundGroupSession()

    def test_session_id(self):
        session = OutboundGroupSession()
        assert isinstance(session.id, str)

    def test_session_index(self):
        session = OutboundGroupSession()
        assert isinstance(session.message_index, int)
        assert session.message_index == 0

    def test_outbound_pickle(self):
        session = OutboundGroupSession()
        pickle = session.pickle()

        assert (session.id == OutboundGroupSession.from_pickle(
            pickle).id)

    def test_invalid_unpickle(self):
        with pytest.raises(ValueError):
            OutboundGroupSession.from_pickle(b"")

        with pytest.raises(ValueError):
            InboundGroupSession.from_pickle(b"")

    def test_inbound_create(self):
        outbound = OutboundGroupSession()
        InboundGroupSession(outbound.session_key)

    def test_invalid_decrypt(self):
        outbound = OutboundGroupSession()
        inbound = InboundGroupSession(outbound.session_key)

        with pytest.raises(ValueError):
            inbound.decrypt("")

    def test_inbound_pickle(self):
        outbound = OutboundGroupSession()
        inbound = InboundGroupSession(outbound.session_key)
        pickle = inbound.pickle()
        InboundGroupSession.from_pickle(pickle)

    def test_inbound_export(self):
        outbound = OutboundGroupSession()
        inbound = InboundGroupSession(outbound.session_key)
        imported = InboundGroupSession.import_session(
            inbound.export_session(inbound.first_known_index)
        )
        assert "Test", 0 == imported.decrypt(outbound.encrypt("Test"))

    def test_first_index(self):
        outbound = OutboundGroupSession()
        inbound = InboundGroupSession(outbound.session_key)
        index = inbound.first_known_index
        assert isinstance(index, int)

    def test_encrypt(self, benchmark):
        benchmark.weave(OutboundGroupSession.encrypt, lazy=True)
        outbound = OutboundGroupSession()
        inbound = InboundGroupSession(outbound.session_key)
        assert "Test", 0 == inbound.decrypt(outbound.encrypt("Test"))

    def test_decrypt(self, benchmark):
        benchmark.weave(InboundGroupSession.decrypt, lazy=True)
        outbound = OutboundGroupSession()
        inbound = InboundGroupSession(outbound.session_key)
        assert "Test", 0 == inbound.decrypt(outbound.encrypt("Test"))

    def test_decrypt_twice(self):
        outbound = OutboundGroupSession()
        inbound = InboundGroupSession(outbound.session_key)
        outbound.encrypt("Test 1")
        message, index = inbound.decrypt(outbound.encrypt("Test 2"))
        assert isinstance(index, int)
        assert ("Test 2", 1) == (message, index)

    def test_decrypt_failure(self):
        outbound = OutboundGroupSession()
        inbound = InboundGroupSession(outbound.session_key)
        eve_outbound = OutboundGroupSession()
        with pytest.raises(OlmGroupSessionError):
            inbound.decrypt(eve_outbound.encrypt("Test"))

    def test_id(self):
        outbound = OutboundGroupSession()
        inbound = InboundGroupSession(outbound.session_key)
        assert outbound.id == inbound.id

    def test_inbound_fail(self):
        with pytest.raises(TypeError):
            InboundGroupSession()

    def test_oubtound_pickle_fail(self):
        outbound = OutboundGroupSession()
        pickle = outbound.pickle("Test")

        with pytest.raises(OlmGroupSessionError):
            OutboundGroupSession.from_pickle(pickle)

    def test_outbound_clear(self):
        session = OutboundGroupSession()
        del session

    def test_inbound_clear(self):
        outbound = OutboundGroupSession()
        inbound = InboundGroupSession(outbound.session_key)
        del inbound

    def test_invalid_unicode_decrypt(self):
        outbound = OutboundGroupSession()
        inbound = InboundGroupSession(outbound.session_key)

        text = outbound.encrypt(b"\xed")
        plaintext, _ = inbound.decrypt(text)

        print(plaintext)
        assert plaintext == u"�"

        plaintext, _ = inbound.decrypt(text, "ignore")
        assert plaintext == ""
