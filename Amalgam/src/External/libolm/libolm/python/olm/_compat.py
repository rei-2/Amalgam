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

from builtins import bytes, str
from typing import AnyStr

try:
    import secrets
    URANDOM = secrets.token_bytes  # pragma: no cover
except ImportError:  # pragma: no cover
    from os import urandom
    URANDOM = urandom  # type: ignore


def to_bytearray(string):
    # type: (AnyStr) -> bytes
    if isinstance(string, bytes):
        return bytearray(string)
    elif isinstance(string, str):
        return bytearray(string, "utf-8")

    raise TypeError("Invalid type {}".format(type(string)))


def to_bytes(string):
    # type: (AnyStr) -> bytes
    if isinstance(string, bytes):
        return string
    elif isinstance(string, str):
        return bytes(string, "utf-8")

    raise TypeError("Invalid type {}".format(type(string)))


def to_unicode_str(byte_string, errors="replace"):
    """Turn a byte string into a unicode string.

    Should be used everywhere where the input byte string might not be trusted
    and may contain invalid unicode values.

    Args:
        byte_string (bytes): The bytestring that will be converted to a native
            string.
        errors (str, optional): The error handling scheme that should be used
            to handle unicode decode errors. Can be one of "strict" (raise an
            UnicodeDecodeError exception, "ignore" (remove the offending
            characters), "replace" (replace the offending character with
            U+FFFD), "xmlcharrefreplace" as well as any other name registered
            with codecs.register_error that can handle UnicodeEncodeErrors.

    Returns the decoded native string.
    """
    return byte_string.decode(encoding="utf-8", errors=errors)
