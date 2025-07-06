/*
 * Copyright 2019 New Vector Ltd
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

import org.junit.BeforeClass;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.MethodSorters;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

@RunWith(AndroidJUnit4.class)
@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class OlmSasTest {

    private static OlmManager mOlmManager;

    //Enable the native lib
    @BeforeClass
    public static void setUpClass() {
        // load native librandomBytesOfLength
        mOlmManager = new OlmManager();
    }

    @Test
    public void testSASCode() {
        OlmSAS aliceSas = null;
        OlmSAS bobSas = null;

        try {
            aliceSas = new OlmSAS();
            bobSas = new OlmSAS();

            String alicePKey = aliceSas.getPublicKey();
            String bobPKey = bobSas.getPublicKey();

            Log.e(OlmSasTest.class.getSimpleName(), "#### Alice pub Key is " + alicePKey);
            Log.e(OlmSasTest.class.getSimpleName(), "#### Bob pub Key is " + bobPKey);

            aliceSas.setTheirPublicKey(bobPKey);
            bobSas.setTheirPublicKey(alicePKey);

            int codeLength = 6;
            byte[] alice_sas = aliceSas.generateShortCode("SAS", codeLength);
            byte[] bob_sas = bobSas.generateShortCode("SAS", codeLength);

            Log.e(OlmSasTest.class.getSimpleName(), "#### Alice SAS is " + new String(alice_sas, "UTF-8"));
            Log.e(OlmSasTest.class.getSimpleName(), "#### Bob SAS is " + new String(bob_sas, "UTF-8"));

            assertEquals(codeLength, alice_sas.length);
            assertEquals(codeLength, bob_sas.length);
            assertArrayEquals(alice_sas, bob_sas);

            String aliceMac = aliceSas.calculateMac("Hello world!", "SAS");
            String bobMac = bobSas.calculateMac("Hello world!", "SAS");

            assertEquals(aliceMac, bobMac);

            Log.e(OlmSasTest.class.getSimpleName(), "#### Alice Mac is " + aliceMac);
            Log.e(OlmSasTest.class.getSimpleName(), "#### Bob Mac is " + bobMac);


            String aliceLongKdfMac = aliceSas.calculateMacLongKdf("Hello world!", "SAS");
            String bobLongKdfMac = bobSas.calculateMacLongKdf("Hello world!", "SAS");

            assertEquals("Mac should be the same", aliceLongKdfMac, bobLongKdfMac);

            Log.e(OlmSasTest.class.getSimpleName(), "#### Alice lkdf Mac is " + aliceLongKdfMac);
            Log.e(OlmSasTest.class.getSimpleName(), "#### Bob lkdf Mac is " + bobLongKdfMac);


        } catch (Exception e) {
            fail("OlmSas init failed " + e.getMessage());
            e.printStackTrace();
        } finally {
            if (aliceSas != null) {
                aliceSas.releaseSas();
            }
            if (bobSas != null) {
                bobSas.releaseSas();
            }
        }
    }

}
