# -*- coding: utf-8 -*-
# libolm python bindings
# Copyright © 2015-2017 OpenMarket Ltd
# Copyright © 2018 Damir Jelić <poljar@termina.org.uk>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""libolm Session module.

This module contains the Olm Session part of the Olm library.

It is used to establish a peer-to-peer encrypted communication channel between
two Olm accounts.

Examples:
    >>> alice = Account()
    >>> bob = Account()
    >>> bob.generate_one_time_keys(1)
    >>> id_key = bob.identity_keys['curve25519']
    >>> one_time = list(bob.one_time_keys["curve25519"].values())[0]
    >>> session = OutboundSession(alice, id_key, one_time)

"""

# pylint: disable=redefined-builtin,unused-import
from builtins import bytes, super
from typing import AnyStr, Optional, Type

# pylint: disable=no-name-in-module
from _libolm import ffi, lib  # type: ignore

from ._compat import URANDOM, to_bytearray, to_bytes, to_unicode_str
from ._finalize import track_for_finalization

# This is imported only for type checking purposes
if False:
    from .account import Account  # pragma: no cover


class OlmSessionError(Exception):
    """libolm Session exception."""


class _OlmMessage(object):
    def __init__(self, ciphertext, message_type):
        # type: (AnyStr, ffi.cdata) -> None
        if not ciphertext:
            raise ValueError("Ciphertext can't be empty")

        # I don't know why mypy wants a type annotation here nor why AnyStr
        # doesn't work
        self.ciphertext = ciphertext  # type: ignore
        self.message_type = message_type

    def __str__(self):
        # type: () -> str
        type_to_prefix = {
            lib.OLM_MESSAGE_TYPE_PRE_KEY: "PRE_KEY",
            lib.OLM_MESSAGE_TYPE_MESSAGE: "MESSAGE"
        }

        prefix = type_to_prefix[self.message_type]
        return "{} {}".format(prefix, self.ciphertext)


class OlmPreKeyMessage(_OlmMessage):
    """Olm prekey message class

    Prekey messages are used to establish an Olm session. After the first
    message exchange the session switches to normal messages
    """

    def __init__(self, ciphertext):
        # type: (AnyStr) -> None
        """Create a new Olm prekey message with the supplied ciphertext

        Args:
            ciphertext(str): The ciphertext of the prekey message.
        """
        _OlmMessage.__init__(self, ciphertext, lib.OLM_MESSAGE_TYPE_PRE_KEY)

    def __repr__(self):
        # type: () -> str
        return "OlmPreKeyMessage({})".format(self.ciphertext)


class OlmMessage(_OlmMessage):
    """Olm message class"""

    def __init__(self, ciphertext):
        # type: (AnyStr) -> None
        """Create a new Olm message with the supplied ciphertext

        Args:
            ciphertext(str): The ciphertext of the message.
        """
        _OlmMessage.__init__(self, ciphertext, lib.OLM_MESSAGE_TYPE_MESSAGE)

    def __repr__(self):
        # type: () -> str
        return "OlmMessage({})".format(self.ciphertext)


def _clear_session(session):
    # type: (ffi.cdata) -> None
    lib.olm_clear_session(session)


class Session(object):
    """libolm Session class.
    This is an abstract class that can't be instantiated except when unpickling
    a previously pickled InboundSession or OutboundSession object with
    from_pickle.
    """

    def __new__(cls):
        # type: (Type[Session]) -> Session

        obj = super().__new__(cls)
        obj._buf = ffi.new("char[]", lib.olm_session_size())
        obj._session = lib.olm_session(obj._buf)
        track_for_finalization(obj, obj._session, _clear_session)
        return obj

    def __init__(self):
        # type: () -> None
        if type(self) is Session:
            raise TypeError("Session class may not be instantiated.")

        if False:
            self._session = self._session  # type: ffi.cdata

    def _check_error(self, ret):
        # type: (int) -> None
        if ret != lib.olm_error():
            return

        last_error = ffi.string(lib.olm_session_last_error(self._session)).decode()

        raise OlmSessionError(last_error)

    def pickle(self, passphrase=""):
        # type: (Optional[str]) -> bytes
        """Store an Olm session.

        Stores a session as a base64 string. Encrypts the session using the
        supplied passphrase. Returns a byte object containing the base64
        encoded string of the pickled session. Raises OlmSessionError on
        failure.

        Args:
            passphrase(str, optional): The passphrase to be used to encrypt
                the session.
        """
        byte_key = bytearray(passphrase, "utf-8") if passphrase else b""

        pickle_length = lib.olm_pickle_session_length(self._session)
        pickle_buffer = ffi.new("char[]", pickle_length)

        try:
            self._check_error(
                lib.olm_pickle_session(self._session,
                                       ffi.from_buffer(byte_key),
                                       len(byte_key),
                                       pickle_buffer, pickle_length))
        finally:
            # clear out copies of the passphrase
            for i in range(0, len(byte_key)):
                    byte_key[i] = 0

        return ffi.unpack(pickle_buffer, pickle_length)

    @classmethod
    def from_pickle(cls, pickle, passphrase=""):
        # type: (bytes, Optional[str]) -> Session
        """Load a previously stored Olm session.

        Loads a session from a pickled base64 string and returns a Session
        object. Decrypts the session using the supplied passphrase. Raises
        OlmSessionError on failure. If the passphrase doesn't match the one
        used to encrypt the session then the error message for the
        exception will be "BAD_ACCOUNT_KEY". If the base64 couldn't be decoded
        then the error message will be "INVALID_BASE64".

        Args:
            pickle(bytes): Base64 encoded byte string containing the pickled
                session
            passphrase(str, optional): The passphrase used to encrypt the
                session.
        """
        if not pickle:
            raise ValueError("Pickle can't be empty")

        byte_key = bytearray(passphrase, "utf-8") if passphrase else b""
        # copy because unpickle will destroy the buffer
        pickle_buffer = ffi.new("char[]", pickle)

        session = cls.__new__(cls)

        try:
            ret = lib.olm_unpickle_session(session._session,
                                           ffi.from_buffer(byte_key),
                                           len(byte_key),
                                           pickle_buffer,
                                           len(pickle))
            session._check_error(ret)
        finally:
            # clear out copies of the passphrase
            for i in range(0, len(byte_key)):
                    byte_key[i] = 0

        return session

    def encrypt(self, plaintext):
        # type: (AnyStr) -> _OlmMessage
        """Encrypts a message using the session. Returns the ciphertext as a
        base64 encoded string on success. Raises OlmSessionError on failure.

        Args:
            plaintext(str): The plaintext message that will be encrypted.
        """
        byte_plaintext = to_bytearray(plaintext)

        r_length = lib.olm_encrypt_random_length(self._session)
        random = URANDOM(r_length)

        try:
            message_type = lib.olm_encrypt_message_type(self._session)

            self._check_error(message_type)

            ciphertext_length = lib.olm_encrypt_message_length(
                self._session, len(byte_plaintext)
            )
            ciphertext_buffer = ffi.new("char[]", ciphertext_length)

            self._check_error(lib.olm_encrypt(
                self._session,
                ffi.from_buffer(byte_plaintext), len(byte_plaintext),
                ffi.from_buffer(random), r_length,
                ciphertext_buffer, ciphertext_length,
            ))
        finally:
            # clear out copies of plaintext
            if byte_plaintext is not plaintext:
                for i in range(0, len(byte_plaintext)):
                    byte_plaintext[i] = 0

        if message_type == lib.OLM_MESSAGE_TYPE_PRE_KEY:
            return OlmPreKeyMessage(
                ffi.unpack(
                    ciphertext_buffer,
                    ciphertext_length
                ).decode())
        elif message_type == lib.OLM_MESSAGE_TYPE_MESSAGE:
            return OlmMessage(
                ffi.unpack(
                    ciphertext_buffer,
                    ciphertext_length
                ).decode())
        else:  # pragma: no cover
            raise ValueError("Unknown message type")

    def decrypt(self, message, unicode_errors="replace"):
        # type: (_OlmMessage, str) -> str
        """Decrypts a message using the session. Returns the plaintext string
        on success. Raises OlmSessionError on failure. If the base64 couldn't
        be decoded then the error message will be "INVALID_BASE64". If the
        message is for an unsupported version of the protocol the error message
        will be "BAD_MESSAGE_VERSION". If the message couldn't be decoded then
        the error message will be "BAD_MESSAGE_FORMAT". If the MAC on the
        message was invalid then the error message will be "BAD_MESSAGE_MAC".

        Args:
            message(OlmMessage): The Olm message that will be decrypted. It can
                be either an OlmPreKeyMessage or an OlmMessage.
            unicode_errors(str, optional): The error handling scheme to use for
                unicode decoding errors. The default is "replace" meaning that
                the character that was unable to decode will be replaced with
                the unicode replacement character (U+FFFD). Other possible
                values are "strict", "ignore" and "xmlcharrefreplace" as well
                as any other name registered with codecs.register_error that
                can handle UnicodeEncodeErrors.
        """
        if not message.ciphertext:
            raise ValueError("Ciphertext can't be empty")

        byte_ciphertext = to_bytes(message.ciphertext)
        # make a copy the ciphertext buffer, because
        # olm_decrypt_max_plaintext_length wants to destroy something
        ciphertext_buffer = ffi.new("char[]", byte_ciphertext)

        max_plaintext_length = lib.olm_decrypt_max_plaintext_length(
            self._session, message.message_type, ciphertext_buffer,
            len(byte_ciphertext)
        )
        self._check_error(max_plaintext_length)
        plaintext_buffer = ffi.new("char[]", max_plaintext_length)

        # make a copy the ciphertext buffer, because
        # olm_decrypt_max_plaintext_length wants to destroy something
        ciphertext_buffer = ffi.new("char[]", byte_ciphertext)
        plaintext_length = lib.olm_decrypt(
            self._session, message.message_type,
            ciphertext_buffer, len(byte_ciphertext),
            plaintext_buffer, max_plaintext_length
        )
        self._check_error(plaintext_length)
        plaintext = to_unicode_str(
            ffi.unpack(plaintext_buffer, plaintext_length),
            errors=unicode_errors
        )

        # clear out copies of the plaintext
        lib.memset(plaintext_buffer, 0, max_plaintext_length)

        return plaintext

    @property
    def id(self):
        # type: () -> str
        """str: An identifier for this session. Will be the same for both
        ends of the conversation.
        """
        id_length = lib.olm_session_id_length(self._session)
        id_buffer = ffi.new("char[]", id_length)

        self._check_error(
            lib.olm_session_id(self._session, id_buffer, id_length)
        )
        return ffi.unpack(id_buffer, id_length).decode()

    def matches(self, message, identity_key=None):
        # type: (OlmPreKeyMessage, Optional[AnyStr]) -> bool
        """Checks if the PRE_KEY message is for this in-bound session.
        This can happen if multiple messages are sent to this session before
        this session sends a message in reply. Returns True if the session
        matches. Returns False if the session does not match. Raises
        OlmSessionError on failure. If the base64 couldn't be decoded then the
        error message will be "INVALID_BASE64". If the message was for an
        unsupported protocol version then the error message will be
        "BAD_MESSAGE_VERSION". If the message couldn't be decoded then then the
        error message will be * "BAD_MESSAGE_FORMAT".

        Args:
            message(OlmPreKeyMessage): The Olm prekey message that will checked
                if it is intended for this session.
            identity_key(str, optional): The identity key of the sender. To
                check if the message was also sent using this identity key.
        """
        if not isinstance(message, OlmPreKeyMessage):
            raise TypeError("Matches can only be called with prekey messages.")

        if not message.ciphertext:
            raise ValueError("Ciphertext can't be empty")

        ret = None

        byte_ciphertext = to_bytes(message.ciphertext)
        # make a copy, because olm_matches_inbound_session(_from) will distroy
        # it
        message_buffer = ffi.new("char[]", byte_ciphertext)

        if identity_key:
            byte_id_key = to_bytes(identity_key)

            ret = lib.olm_matches_inbound_session_from(
                self._session,
                ffi.from_buffer(byte_id_key), len(byte_id_key),
                message_buffer, len(byte_ciphertext)
            )

        else:
            ret = lib.olm_matches_inbound_session(
                self._session,
                message_buffer, len(byte_ciphertext))

        self._check_error(ret)

        return bool(ret)

    def describe(self, buffer_length=600):
        # type: (int) -> str
        """
        Generate a string describing the internal state of an olm session
        for debugging and logging purposes.

        Args:
            buffer_length(int): The size of buffer for the string.
                If the buffer is not large enough to hold the entire string, it
                will be truncated and will end with "...".  A buffer length of
                600 will be enough to hold any output.
        """
        describe_buffer = ffi.new("char[]", buffer_length)
        lib.olm_session_describe(
            self._session, describe_buffer, buffer_length
        )
        return ffi.string(describe_buffer).decode()


class InboundSession(Session):
    """Inbound Olm session for p2p encrypted communication.
    """

    def __new__(cls, account, message, identity_key=None):
        # type: (Account, OlmPreKeyMessage, Optional[AnyStr]) -> Session
        return super().__new__(cls)

    def __init__(self, account, message, identity_key=None):
        # type: (Account, OlmPreKeyMessage, Optional[AnyStr]) -> None
        """Create a new inbound Olm session.

        Create a new in-bound session for sending/receiving messages from an
        incoming prekey message. Raises OlmSessionError on failure. If the
        base64 couldn't be decoded then error message will be "INVALID_BASE64".
        If the message was for an unsupported protocol version then
        the errror message will be "BAD_MESSAGE_VERSION". If the message
        couldn't be decoded then then the error message will be
        "BAD_MESSAGE_FORMAT". If the message refers to an unknown one-time
        key then the error message will be "BAD_MESSAGE_KEY_ID".

        Args:
            account(Account): The Olm Account that will be used to create this
                session.
            message(OlmPreKeyMessage): The Olm prekey message that will checked
                that will be used to create this session.
            identity_key(str, optional): The identity key of the sender. To
                check if the message was also sent using this identity key.
        """
        if not message.ciphertext:
            raise ValueError("Ciphertext can't be empty")

        super().__init__()
        byte_ciphertext = to_bytes(message.ciphertext)
        message_buffer = ffi.new("char[]", byte_ciphertext)

        if identity_key:
            byte_id_key = to_bytes(identity_key)
            identity_key_buffer = ffi.new("char[]", byte_id_key)
            self._check_error(lib.olm_create_inbound_session_from(
                self._session,
                account._account,
                identity_key_buffer, len(byte_id_key),
                message_buffer, len(byte_ciphertext)
            ))
        else:
            self._check_error(lib.olm_create_inbound_session(
                self._session,
                account._account,
                message_buffer, len(byte_ciphertext)
            ))


class OutboundSession(Session):
    """Outbound Olm session for p2p encrypted communication."""

    def __new__(cls, account, identity_key, one_time_key):
        # type: (Account, AnyStr, AnyStr) -> Session
        return super().__new__(cls)

    def __init__(self, account, identity_key, one_time_key):
        # type: (Account, AnyStr, AnyStr) -> None
        """Create a new outbound Olm session.

        Creates a new outbound session for sending messages to a given
        identity key and one-time key.

        Raises OlmSessionError on failure. If the keys couldn't be decoded as
        base64 then the error message will be "INVALID_BASE64".

        Args:
            account(Account): The Olm Account that will be used to create this
                session.
            identity_key(str): The identity key of the person with whom we want
                to start the session.
            one_time_key(str): A one-time key from the person with whom we want
                to start the session.
        """
        if not identity_key:
            raise ValueError("Identity key can't be empty")

        if not one_time_key:
            raise ValueError("One-time key can't be empty")

        super().__init__()

        byte_id_key = to_bytes(identity_key)
        byte_one_time = to_bytes(one_time_key)

        session_random_length = lib.olm_create_outbound_session_random_length(
            self._session)

        random = URANDOM(session_random_length)

        self._check_error(lib.olm_create_outbound_session(
            self._session,
            account._account,
            ffi.from_buffer(byte_id_key), len(byte_id_key),
            ffi.from_buffer(byte_one_time), len(byte_one_time),
            ffi.from_buffer(random), session_random_length
        ))
