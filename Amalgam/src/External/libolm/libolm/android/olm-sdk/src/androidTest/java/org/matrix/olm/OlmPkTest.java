/*
 * Copyright 2018,2019 New Vector Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.matrix.olm;

import android.util.Log;

import androidx.test.ext.junit.runners.AndroidJUnit4;

import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.MethodSorters;

import java.util.Arrays;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

@RunWith(AndroidJUnit4.class)
@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class OlmPkTest {
    private static final String LOG_TAG = "OlmPkEncryptionTest";

    private static OlmPkEncryption mOlmPkEncryption;
    private static OlmPkDecryption mOlmPkDecryption;
    private static OlmPkSigning mOlmPkSigning;

    @Test
    public void test01EncryptAndDecrypt() {
        try {
            mOlmPkEncryption = new OlmPkEncryption();
        } catch (OlmException e) {
            e.printStackTrace();
            fail("OlmPkEncryption failed " + e.getMessage());
        }
        try {
            mOlmPkDecryption = new OlmPkDecryption();
        } catch (OlmException e) {
            e.printStackTrace();
            fail("OlmPkEncryption failed " + e.getMessage());
        }

        assertNotNull(mOlmPkEncryption);
        assertNotNull(mOlmPkDecryption);

        String key = null;
        try {
            key = mOlmPkDecryption.generateKey();
        } catch (OlmException e) {
            fail("Exception in generateKey, Exception code=" + e.getExceptionCode());
        }
        Log.d(LOG_TAG, "Ephemeral Key: " + key);
        try {
            mOlmPkEncryption.setRecipientKey(key);
        } catch (OlmException e) {
            fail("Exception in setRecipientKey, Exception code=" + e.getExceptionCode());
        }

        String clearMessage = "Public key test";
        OlmPkMessage message = null;
        try {
            message = mOlmPkEncryption.encrypt(clearMessage);
        } catch (OlmException e) {
            fail("Exception in encrypt, Exception code=" + e.getExceptionCode());
        }
        Log.d(LOG_TAG, "message: " + message.mCipherText + " " + message.mMac + " " + message.mEphemeralKey);

        String decryptedMessage = null;
        try {
            decryptedMessage = mOlmPkDecryption.decrypt(message);
        } catch (OlmException e) {
            fail("Exception in decrypt, Exception code=" + e.getExceptionCode());
        }
        assertEquals(clearMessage, decryptedMessage);

        mOlmPkEncryption.releaseEncryption();
        mOlmPkDecryption.releaseDecryption();
        assertTrue(mOlmPkEncryption.isReleased());
        assertTrue(mOlmPkDecryption.isReleased());
    }

    @Test
    public void test02PrivateKey() {
        try {
            mOlmPkDecryption = new OlmPkDecryption();
        } catch (OlmException e) {
            e.printStackTrace();
            fail("OlmPkEncryption failed " + e.getMessage());
        }

        assertNotNull(mOlmPkDecryption);

        byte[] privateKey = {
                (byte) 0x77, (byte) 0x07, (byte) 0x6D, (byte) 0x0A,
                (byte) 0x73, (byte) 0x18, (byte) 0xA5, (byte) 0x7D,
                (byte) 0x3C, (byte) 0x16, (byte) 0xC1, (byte) 0x72,
                (byte) 0x51, (byte) 0xB2, (byte) 0x66, (byte) 0x45,
                (byte) 0xDF, (byte) 0x4C, (byte) 0x2F, (byte) 0x87,
                (byte) 0xEB, (byte) 0xC0, (byte) 0x99, (byte) 0x2A,
                (byte) 0xB1, (byte) 0x77, (byte) 0xFB, (byte) 0xA5,
                (byte) 0x1D, (byte) 0xB9, (byte) 0x2C, (byte) 0x2A
        };

        assertEquals(privateKey.length, OlmPkDecryption.privateKeyLength());

        try {
            mOlmPkDecryption.setPrivateKey(privateKey);
        } catch (OlmException e) {
            fail("Exception in setPrivateKey, Exception code=" + e.getExceptionCode());
        }

        byte[] privateKeyCopy = null;

        try {
            privateKeyCopy = mOlmPkDecryption.privateKey();
        } catch (OlmException e) {
            fail("Exception in privateKey, Exception code=" + e.getExceptionCode());
        }

        assertArrayEquals(privateKey, privateKeyCopy);

        mOlmPkDecryption.releaseDecryption();
        assertTrue(mOlmPkDecryption.isReleased());
    }

    @Test
    public void test03Signing() {
        try {
            mOlmPkSigning = new OlmPkSigning();
        } catch (OlmException e) {
            e.printStackTrace();
            fail("OlmPkSigning failed " + e.getMessage());
        }

        assertNotNull(mOlmPkSigning);

        byte[] seed = null;
        try {
            seed = OlmPkSigning.generateSeed();
        } catch (OlmException e) {
            e.printStackTrace();
            fail("generateSeed failed " + e.getMessage());
        }

        assertEquals(seed.length, OlmPkSigning.seedLength());

        String pubkey = null;
        try {
            pubkey = mOlmPkSigning.initWithSeed(seed);
        } catch (OlmException e) {
            e.printStackTrace();
            fail("initWithSeed failed " + e.getMessage());
        }

        String message = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness.";

        String signature = null;
        try {
            signature = mOlmPkSigning.sign(message);
        } catch (OlmException e) {
            e.printStackTrace();
            fail("sign failed " + e.getMessage());
        }

        OlmUtility olmUtility = null;
        try {
            olmUtility = new OlmUtility();
        } catch (OlmException e) {
            e.printStackTrace();
            fail("olmUtility failed " + e.getMessage());
        }

        try {
            olmUtility.verifyEd25519Signature(signature, pubkey, message);
        } catch (OlmException e) {
            e.printStackTrace();
            fail("Signature verification failed " + e.getMessage());
        }

        mOlmPkSigning.releaseSigning();
        assertTrue(mOlmPkSigning.isReleased());

        olmUtility.releaseUtility();
    }
}
