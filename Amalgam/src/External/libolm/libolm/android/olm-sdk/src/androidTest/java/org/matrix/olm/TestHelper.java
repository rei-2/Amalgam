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

import java.util.ArrayList;
import java.util.Map;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

/**
 * Helper class providing helper methods used in the Olm Android SDK unit tests.
 */
public class TestHelper {

    /**
     * Return the identity key {@link OlmAccount#JSON_KEY_IDENTITY_KEY} from the JSON object.
     * @param aIdentityKeysMap result of {@link OlmAccount#identityKeys()}
     * @return identity key string if operation succeed, null otherwise
     */
    static public String getIdentityKey(Map<String, String> aIdentityKeysMap){
        String idKey = null;

        try {
            idKey = aIdentityKeysMap.get(OlmAccount.JSON_KEY_IDENTITY_KEY);
        } catch (Exception e) {
            fail("Exception MSg=" + e.getMessage());
        }
        return idKey;
    }

    /**
     * Return the fingerprint key {@link OlmAccount#JSON_KEY_FINGER_PRINT_KEY} from the JSON object.
     * @param aIdentityKeysMap result of {@link OlmAccount#identityKeys()}
     * @return fingerprint key string if operation succeed, null otherwise
     */
    static public String getFingerprintKey(Map<String, String> aIdentityKeysMap) {
        String fingerprintKey = null;

        try {
            fingerprintKey = aIdentityKeysMap.get(OlmAccount.JSON_KEY_FINGER_PRINT_KEY);
        } catch (Exception e) {
            fail("Exception MSg=" + e.getMessage());
        }
        return fingerprintKey;
    }

    /**
     * Return the first one time key from the JSON object.
     * @param aIdentityKeysMap result of {@link OlmAccount#oneTimeKeys()}
     * @param aKeyPosition the position of the key to be retrieved
     * @return one time key string if operation succeed, null otherwise
     */
    static public String getOneTimeKey(Map<String, Map<String, String>> aIdentityKeysMap, int aKeyPosition) {
        String firstOneTimeKey = null;

        try {
            Map<String, String> generatedKeys = aIdentityKeysMap.get(OlmAccount.JSON_KEY_ONE_TIME_KEY);
            assertNotNull(OlmAccount.JSON_KEY_ONE_TIME_KEY + " object is missing", generatedKeys);

            firstOneTimeKey = (new ArrayList<>(generatedKeys.values())).get(aKeyPosition - 1);
        } catch (Exception e) {
            fail("Exception Msg=" + e.getMessage());
        }
        return firstOneTimeKey;
    }
}
