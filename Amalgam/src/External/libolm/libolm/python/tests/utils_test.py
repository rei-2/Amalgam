import base64
import hashlib

from olm import sha256
from olm._compat import to_bytes


class TestClass(object):
    def test_sha256(self):
        input1 = "It's a secret to everybody"
        input2 = "It's a secret to nobody"

        first_hash = sha256(input1)
        second_hash = sha256(input2)

        hashlib_hash = base64.b64encode(
            hashlib.sha256(to_bytes(input1)).digest()
        )

        hashlib_hash = hashlib_hash[:-1].decode()

        assert first_hash != second_hash
        assert hashlib_hash == first_hash
