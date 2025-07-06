/*
 * Copyright 2016 OpenMarket Ltd
 * Copyright 2016 Vector Creations Ltd
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

import android.text.TextUtils;
import android.util.Log;

import androidx.test.ext.junit.runners.AndroidJUnit4;

import org.junit.BeforeClass;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.MethodSorters;

import java.util.Map;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

@RunWith(AndroidJUnit4.class)
@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class OlmUtilityTest {
    private static final String LOG_TAG = "OlmAccountTest";
    private static final int GENERATION_ONE_TIME_KEYS_NUMBER = 50;

    private static OlmManager mOlmManager;

    @BeforeClass
    public static void setUpClass() {
        // load native lib
        mOlmManager = new OlmManager();

        String version = mOlmManager.getOlmLibVersion();
        assertNotNull(version);
        Log.d(LOG_TAG, "## setUpClass(): lib version=" + version);
    }

    /**
     * Test the signing API
     */
    @Test
    public void test01VerifyEd25519Signing() {
        String fingerPrintKey = null;
        String errorMsg = null;
        String message = "{\"algorithms\":[\"m.megolm.v1.aes-sha2\",\"m.olm.v1.curve25519-aes-sha2\"],\"device_id\":\"YMBYCWTWCG\",\"keys\":{\"curve25519:YMBYCWTWCG\":\"KZFa5YUXV2EOdhK8dcGMMHWB67stdgAP4+xwiS69mCU\",\"ed25519:YMBYCWTWCG\":\"0cEgQJJqjgtXUGp4ZXQQmh36RAxwxr8HJw2E9v1gvA0\"},\"user_id\":\"@mxBob14774891254276b253f42-f267-43ec-bad9-767142bfea30:localhost:8480\"}";
        OlmAccount account = null;

        // create account
        try {
            account = new OlmAccount();
        } catch (OlmException e) {
            fail(e.getMessage());
        }
        assertNotNull(account);

        // sign message
        String messageSignature = null;

        try {
            messageSignature = account.signMessage(message);
        } catch (Exception e) {
            fail(e.getMessage());
        }

        assertNotNull(messageSignature);

        // get identities key (finger print key)
        Map<String, String> identityKeys = null;

        try {
            identityKeys = account.identityKeys();
        } catch (Exception e) {
            fail("identityKeys failed " + e.getMessage());
        }

        assertNotNull(identityKeys);
        fingerPrintKey = TestHelper.getFingerprintKey(identityKeys);
        assertFalse("fingerprint key missing", TextUtils.isEmpty(fingerPrintKey));

        // instantiate utility object
        OlmUtility utility = null;

        try {
            utility = new OlmUtility();
        } catch (Exception e) {
            fail("failed to create OlmUtility");
        }

        // verify signature
        errorMsg = null;
        try {
            utility.verifyEd25519Signature(messageSignature, fingerPrintKey, message);
        } catch (Exception e) {
            errorMsg = e.getMessage();
        }
        assertTrue(TextUtils.isEmpty(errorMsg));

        // check a bad signature is detected => errorMsg = BAD_MESSAGE_MAC
        String badSignature = "Bad signature Bad signature Bad signature..";

        errorMsg = null;
        try {
            utility.verifyEd25519Signature(badSignature, fingerPrintKey, message);
        } catch (Exception e) {
            errorMsg = e.getMessage();
        }
        assertFalse(TextUtils.isEmpty(errorMsg));

        // check bad fingerprint size => errorMsg = INVALID_BASE64
        String badSizeFingerPrintKey = fingerPrintKey.substring(fingerPrintKey.length() / 2);

        errorMsg = null;
        try {
            utility.verifyEd25519Signature(messageSignature, badSizeFingerPrintKey, message);
        } catch (Exception e) {
            errorMsg = e.getMessage();
        }
        assertFalse(TextUtils.isEmpty(errorMsg));

        utility.releaseUtility();
        assertTrue(utility.isReleased());

        account.releaseAccount();
        assertTrue(account.isReleased());
    }

    @Test
    public void test02sha256() {
        OlmUtility utility = null;

        try {
            utility = new OlmUtility();
        } catch (Exception e) {
            fail("OlmUtility creation failed");
        }
        String msgToHash = "The quick brown fox jumps over the lazy dog";

        String hashResult = utility.sha256(msgToHash);
        assertFalse(TextUtils.isEmpty(hashResult));

        utility.releaseUtility();
        assertTrue(utility.isReleased());
    }
}
