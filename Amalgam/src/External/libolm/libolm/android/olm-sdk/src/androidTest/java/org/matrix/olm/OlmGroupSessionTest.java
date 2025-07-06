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

import android.content.Context;
import android.text.TextUtils;
import android.util.Log;

import androidx.test.core.app.ApplicationProvider;
import androidx.test.ext.junit.runners.AndroidJUnit4;

import org.junit.BeforeClass;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.MethodSorters;


import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

@RunWith(AndroidJUnit4.class)
@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class OlmGroupSessionTest {
    private static final String LOG_TAG = "OlmSessionTest";
    private final String FILE_NAME_SERIAL_OUT_SESSION = "SerialOutGroupSession";
    private final String FILE_NAME_SERIAL_IN_SESSION = "SerialInGroupSession";

    private static OlmManager mOlmManager;
    private static OlmOutboundGroupSession mAliceOutboundGroupSession;
    private static String mAliceSessionIdentifier;
    private static long mAliceMessageIndex;
    private static final String CLEAR_MESSAGE1 = "Hello!";
    private static String mAliceToBobMessage;
    private static OlmInboundGroupSession mBobInboundGroupSession;
    private static String mAliceOutboundSessionKey;
    private static String mBobSessionIdentifier;
    private static String mBobDecryptedMessage;

    @BeforeClass
    public static void setUpClass(){
        // load native lib
        mOlmManager = new OlmManager();

        String version = mOlmManager.getOlmLibVersion();
        assertNotNull(version);
        Log.d(LOG_TAG, "## setUpClass(): lib version="+version);
    }

    /**
     * Basic test:
     * - alice creates an outbound group session
     * - bob creates an inbound group session with alice's outbound session key
     * - alice encrypts a message with its session
     * - bob decrypts the encrypted message with its session
     * - decrypted message is identical to original alice message
     */
    @Test
    public void test01CreateOutboundSession() {
        // alice creates OUTBOUND GROUP SESSION
        try {
            mAliceOutboundGroupSession = new OlmOutboundGroupSession();
        } catch (OlmException e) {
            fail("Exception in OlmOutboundGroupSession, Exception code=" + e.getExceptionCode());
        }
    }

    @Test
    public void test02GetOutboundGroupSessionIdentifier() {
        // test session ID
        mAliceSessionIdentifier = null;

        try {
            mAliceSessionIdentifier = mAliceOutboundGroupSession.sessionIdentifier();
        } catch (Exception e) {
            fail(e.getMessage());
        }

        assertNotNull(mAliceSessionIdentifier);
        assertTrue(mAliceSessionIdentifier.length() > 0);
    }

    @Test
    public void test03GetOutboundGroupSessionKey() {
        // test session Key
        mAliceOutboundSessionKey = null;

        try {
            mAliceOutboundSessionKey = mAliceOutboundGroupSession.sessionKey();
        } catch (Exception e) {
            fail(e.getMessage());
        }
        assertNotNull(mAliceOutboundSessionKey);
        assertTrue(mAliceOutboundSessionKey.length() > 0);
    }

    @Test
    public void test04GetOutboundGroupMessageIndex() {
        // test message index before any encryption
        mAliceMessageIndex = mAliceOutboundGroupSession.messageIndex();
        assertEquals(0, mAliceMessageIndex);
    }

    @Test
    public void test05OutboundGroupEncryptMessage() {
        // alice encrypts a message to bob
        try {
            mAliceToBobMessage = mAliceOutboundGroupSession.encryptMessage(CLEAR_MESSAGE1);
        } catch (Exception e) {
            fail("Exception in bob encryptMessage, Exception code=" + e.getMessage());
        }
        assertFalse(TextUtils.isEmpty(mAliceToBobMessage));

        // test message index after encryption is incremented
        mAliceMessageIndex = mAliceOutboundGroupSession.messageIndex();
        assertEquals(1, mAliceMessageIndex);
    }

    @Test
    public void test06CreateInboundGroupSession() {
        // bob creates INBOUND GROUP SESSION with alice outbound key
        try {
            mBobInboundGroupSession = new OlmInboundGroupSession(mAliceOutboundSessionKey);
        } catch (OlmException e) {
            fail("Exception in bob OlmInboundGroupSession, Exception code=" + e.getExceptionCode());
        }
    }

    @Test
    public void test08GetInboundGroupSessionIdentifier() {
        // check both session identifiers are equals
        mBobSessionIdentifier = null;

        try {
            mBobSessionIdentifier = mBobInboundGroupSession.sessionIdentifier();
        } catch (Exception e) {
            fail(e.getMessage());
        }
        assertFalse(TextUtils.isEmpty(mBobSessionIdentifier));
    }

    @Test
    public void test09SessionIdentifiersAreIdentical() {
        // check both session identifiers are equals: alice vs bob
        assertEquals(mAliceSessionIdentifier, mBobSessionIdentifier);
    }

    @Test
    public void test10InboundDecryptMessage() {
        mBobDecryptedMessage = null;
        OlmInboundGroupSession.DecryptMessageResult result = null;

        try {
            result = mBobInboundGroupSession.decryptMessage(mAliceToBobMessage);
        } catch (Exception e) {
            fail(e.getMessage());
        }

        // test decrypted message
        mBobDecryptedMessage = result.mDecryptedMessage;
        assertFalse(TextUtils.isEmpty(mBobDecryptedMessage));
        assertEquals(0, result.mIndex);
    }

    @Test
    public void test11InboundDecryptedMessageIdentical() {
        // test decrypted message
        assertEquals(mBobDecryptedMessage, CLEAR_MESSAGE1);
    }

    @Test
    public void test12ReleaseOutboundSession() {
        // release group sessions
        mAliceOutboundGroupSession.releaseSession();
    }

    @Test
    public void test13ReleaseInboundSession() {
        // release group sessions
        mBobInboundGroupSession.releaseSession();
    }

    @Test
    public void test14CheckUnreleaseedCount() {
        assertTrue(mAliceOutboundGroupSession.isReleased());
        assertTrue(mBobInboundGroupSession.isReleased());
    }

    @Test
    public void test15SerializeOutboundSession() {
        OlmOutboundGroupSession outboundGroupSessionRef=null;
        OlmOutboundGroupSession outboundGroupSessionSerial;

        // create one OUTBOUND GROUP SESSION
        try {
            outboundGroupSessionRef = new OlmOutboundGroupSession();
        } catch (OlmException e) {
            fail("Exception in OlmOutboundGroupSession, Exception code=" + e.getExceptionCode());
        }
        assertNotNull(outboundGroupSessionRef);


        // serialize alice session
        Context context = ApplicationProvider.getApplicationContext();
        try {
            FileOutputStream fileOutput = context.openFileOutput(FILE_NAME_SERIAL_OUT_SESSION, Context.MODE_PRIVATE);
            ObjectOutputStream objectOutput = new ObjectOutputStream(fileOutput);
            objectOutput.writeObject(outboundGroupSessionRef);
            objectOutput.flush();
            objectOutput.close();

            // deserialize session
            FileInputStream fileInput = context.openFileInput(FILE_NAME_SERIAL_OUT_SESSION);
            ObjectInputStream objectInput = new ObjectInputStream(fileInput);
            outboundGroupSessionSerial = (OlmOutboundGroupSession) objectInput.readObject();
            assertNotNull(outboundGroupSessionSerial);
            objectInput.close();

            // get sessions keys
            String sessionKeyRef = outboundGroupSessionRef.sessionKey();
            String sessionKeySerial = outboundGroupSessionSerial.sessionKey();
            assertFalse(TextUtils.isEmpty(sessionKeyRef));
            assertFalse(TextUtils.isEmpty(sessionKeySerial));

            // session keys comparison
            assertEquals(sessionKeyRef, sessionKeySerial);

            // get sessions IDs
            String sessionIdRef = outboundGroupSessionRef.sessionIdentifier();
            String sessionIdSerial = outboundGroupSessionSerial.sessionIdentifier();
            assertFalse(TextUtils.isEmpty(sessionIdRef));
            assertFalse(TextUtils.isEmpty(sessionIdSerial));

            // session IDs comparison
            assertEquals(sessionIdRef, sessionIdSerial);

            outboundGroupSessionRef.releaseSession();
            outboundGroupSessionSerial.releaseSession();

            assertTrue(outboundGroupSessionRef.isReleased());
            assertTrue(outboundGroupSessionSerial.isReleased());
        } catch (FileNotFoundException e) {
            Log.e(LOG_TAG, "## test15SerializeOutboundSession(): Exception FileNotFoundException Msg=="+e.getMessage());
            fail(e.getMessage());
        } catch (ClassNotFoundException e) {
            Log.e(LOG_TAG, "## test15SerializeOutboundSession(): Exception ClassNotFoundException Msg==" + e.getMessage());
            fail(e.getMessage());
        } catch (OlmException e) {
            Log.e(LOG_TAG, "## test15SerializeOutboundSession(): Exception OlmException Msg==" + e.getMessage());
            fail(e.getMessage());
        } catch (IOException e) {
            Log.e(LOG_TAG, "## test15SerializeOutboundSession(): Exception IOException Msg==" + e.getMessage());
            fail(e.getMessage());
        } catch (Exception e) {
            Log.e(LOG_TAG, "## test15SerializeOutboundSession(): Exception Msg==" + e.getMessage());
            fail(e.getMessage());
        }
    }

    @Test
    public void test16SerializeInboundSession() {
        OlmOutboundGroupSession aliceOutboundGroupSession=null;
        OlmInboundGroupSession bobInboundGroupSessionRef=null;
        OlmInboundGroupSession bobInboundGroupSessionSerial;

        // alice creates OUTBOUND GROUP SESSION
        try {
            aliceOutboundGroupSession = new OlmOutboundGroupSession();
        } catch (OlmException e) {
            fail("Exception in OlmOutboundGroupSession, Exception code=" + e.getExceptionCode());
        }
        assertNotNull(aliceOutboundGroupSession);

        // get the session key from the outbound group session
        String sessionKeyRef = null;

        try {
            sessionKeyRef = aliceOutboundGroupSession.sessionKey();
        } catch (Exception e) {
            fail(e.getMessage());
        }
        assertNotNull(sessionKeyRef);

        // bob creates INBOUND GROUP SESSION
        try {
            bobInboundGroupSessionRef = new OlmInboundGroupSession(sessionKeyRef);
        } catch (OlmException e) {
            fail("Exception in OlmInboundGroupSession, Exception code=" + e.getExceptionCode());
        }
        assertNotNull(bobInboundGroupSessionRef);

        // serialize alice session
        Context context = ApplicationProvider.getApplicationContext();
        try {
            FileOutputStream fileOutput = context.openFileOutput(FILE_NAME_SERIAL_IN_SESSION, Context.MODE_PRIVATE);
            ObjectOutputStream objectOutput = new ObjectOutputStream(fileOutput);
            objectOutput.writeObject(bobInboundGroupSessionRef);
            objectOutput.flush();
            objectOutput.close();

            // deserialize session
            FileInputStream fileInput = context.openFileInput(FILE_NAME_SERIAL_IN_SESSION);
            ObjectInputStream objectInput = new ObjectInputStream(fileInput);
            bobInboundGroupSessionSerial = (OlmInboundGroupSession)objectInput.readObject();
            assertNotNull(bobInboundGroupSessionSerial);
            objectInput.close();

            // get sessions IDs
            String aliceSessionId = aliceOutboundGroupSession.sessionIdentifier();
            String sessionIdRef = bobInboundGroupSessionRef.sessionIdentifier();
            String sessionIdSerial = bobInboundGroupSessionSerial.sessionIdentifier();
            assertFalse(TextUtils.isEmpty(aliceSessionId));
            assertFalse(TextUtils.isEmpty(sessionIdRef));
            assertFalse(TextUtils.isEmpty(sessionIdSerial));

            // session IDs comparison
            assertEquals(aliceSessionId, sessionIdSerial);
            assertEquals(sessionIdRef, sessionIdSerial);

            aliceOutboundGroupSession.releaseSession();
            bobInboundGroupSessionRef.releaseSession();
            bobInboundGroupSessionSerial.releaseSession();

            assertTrue(aliceOutboundGroupSession.isReleased());
            assertTrue(bobInboundGroupSessionRef.isReleased());
            assertTrue(bobInboundGroupSessionSerial.isReleased());
        } catch (FileNotFoundException e) {
            Log.e(LOG_TAG, "## test16SerializeInboundSession(): Exception FileNotFoundException Msg=="+e.getMessage());
            fail(e.getMessage());
        } catch (ClassNotFoundException e) {
            Log.e(LOG_TAG, "## test16SerializeInboundSession(): Exception ClassNotFoundException Msg==" + e.getMessage());
            fail(e.getMessage());
        } catch (OlmException e) {
            Log.e(LOG_TAG, "## test16SerializeInboundSession(): Exception OlmException Msg==" + e.getMessage());
            fail(e.getMessage());
        } catch (IOException e) {
            Log.e(LOG_TAG, "## test16SerializeInboundSession(): Exception IOException Msg==" + e.getMessage());
            fail(e.getMessage());
        } catch (Exception e) {
            Log.e(LOG_TAG, "## test16SerializeInboundSession(): Exception Msg==" + e.getMessage());
            fail(e.getMessage());
        }
    }

    /**
     * Create multiple outbound group sessions and check that session Keys are different.
     * This test validates random series are provide enough random values.
     */
    @Test
    public void test17MultipleOutboundSession() {
        OlmOutboundGroupSession outboundGroupSession1;
        OlmOutboundGroupSession outboundGroupSession2;
        OlmOutboundGroupSession outboundGroupSession3;
        OlmOutboundGroupSession outboundGroupSession4;
        OlmOutboundGroupSession outboundGroupSession5;
        OlmOutboundGroupSession outboundGroupSession6;
        OlmOutboundGroupSession outboundGroupSession7;
        OlmOutboundGroupSession outboundGroupSession8;

        try {
            outboundGroupSession1 = new OlmOutboundGroupSession();
            outboundGroupSession2 = new OlmOutboundGroupSession();
            outboundGroupSession3 = new OlmOutboundGroupSession();
            outboundGroupSession4 = new OlmOutboundGroupSession();
            outboundGroupSession5 = new OlmOutboundGroupSession();
            outboundGroupSession6 = new OlmOutboundGroupSession();
            outboundGroupSession7 = new OlmOutboundGroupSession();
            outboundGroupSession8 = new OlmOutboundGroupSession();

            // get the session key from the outbound group sessions
            String sessionKey1 = outboundGroupSession1.sessionKey();
            String sessionKey2 = outboundGroupSession2.sessionKey();
            assertNotEquals(sessionKey1, sessionKey2);

            String sessionKey3 = outboundGroupSession3.sessionKey();
            assertNotEquals(sessionKey2, sessionKey3);

            String sessionKey4 = outboundGroupSession4.sessionKey();
            assertNotEquals(sessionKey3, sessionKey4);

            String sessionKey5 = outboundGroupSession5.sessionKey();
            assertNotEquals(sessionKey4, sessionKey5);

            String sessionKey6 = outboundGroupSession6.sessionKey();
            assertNotEquals(sessionKey5, sessionKey6);

            String sessionKey7 = outboundGroupSession7.sessionKey();
            assertNotEquals(sessionKey6, sessionKey7);

            String sessionKey8 = outboundGroupSession8.sessionKey();
            assertNotEquals(sessionKey7, sessionKey8);

            // get the session IDs from the outbound group sessions
            String sessionId1 = outboundGroupSession1.sessionIdentifier();
            String sessionId2 = outboundGroupSession2.sessionIdentifier();
            assertNotEquals(sessionId1, sessionId2);

            String sessionId3 = outboundGroupSession3.sessionKey();
            assertNotEquals(sessionId2, sessionId3);

            String sessionId4 = outboundGroupSession4.sessionKey();
            assertNotEquals(sessionId3, sessionId4);

            String sessionId5 = outboundGroupSession5.sessionKey();
            assertNotEquals(sessionId4, sessionId5);

            String sessionId6 = outboundGroupSession6.sessionKey();
            assertNotEquals(sessionId5, sessionId6);

            String sessionId7 = outboundGroupSession7.sessionKey();
            assertNotEquals(sessionId6, sessionId7);

            String sessionId8 = outboundGroupSession8.sessionKey();
            assertNotEquals(sessionId7, sessionId8);

            outboundGroupSession1.releaseSession();
            outboundGroupSession2.releaseSession();
            outboundGroupSession3.releaseSession();
            outboundGroupSession4.releaseSession();
            outboundGroupSession5.releaseSession();
            outboundGroupSession6.releaseSession();
            outboundGroupSession7.releaseSession();
            outboundGroupSession8.releaseSession();

            assertTrue(outboundGroupSession1.isReleased());
            assertTrue(outboundGroupSession2.isReleased());
            assertTrue(outboundGroupSession3.isReleased());
            assertTrue(outboundGroupSession4.isReleased());
            assertTrue(outboundGroupSession5.isReleased());
            assertTrue(outboundGroupSession6.isReleased());
            assertTrue(outboundGroupSession7.isReleased());
            assertTrue(outboundGroupSession8.isReleased());
        } catch (OlmException e) {
            fail("Exception in OlmOutboundGroupSession, Exception code=" + e.getExceptionCode());
        }
    }

    /**
     * Specific test for the following run time error:
     * "JNI DETECTED ERROR IN APPLICATION: input is not valid Modified UTF-8: illegal start byte 0xf0 in call to NewStringUTF".<br>
     * When the msg to decrypt contain emojis, depending on the android platform, the NewStringUTF() behaves differently and
     * can even crash.
     * This issue is described in details here: https://github.com/eclipsesource/J2V8/issues/142
     */
    @Test
    public void test18TestBadCharacterCrashInDecrypt() {
        OlmInboundGroupSession bobInboundGroupSession=null;

        // values taken from a "real life" crash case
        String sessionKeyRef = "AgAAAAycZE6AekIctJWYxd2AWLOY15YmxZODm/WkgbpWkyycp6ytSp/R+wo84jRrzBNWmv6ySLTZ9R0EDOk9VI2eZyQ6Efdwyo1mAvrWvTkZl9yALPdkOIVHywyG65f1SNiLrnsln3hgsT1vUrISGyKtsljoUgQpr3JDPEhD0ilAi63QBjhnGCW252b+7nF+43rb6O6lwm93LaVwe2341Gdp6EkhTUvetALezEqDOtKN00wVqAbq0RQAnUJIowxHbMswg+FyoR1K1oCjnVEoF23O9xlAn5g1XtuBZP3moJlR2lwsBA";
        String msgToDecryptWithEmoji = "AwgNEpABpjs+tYF+0y8bWtzAgYAC3N55p5cPJEEiGPU1kxIHSY7f2aG5Fj4wmcsXUkhDv0UePj922kgf+Q4dFsPHKq2aVA93n8DJAQ/FRfcM98B9E6sKCZ/PsCF78uBvF12Aaq9D3pUHBopdd7llUfVq29d5y6ZwX5VDoqV2utsATkKjXYV9CbfZuvvBMQ30ZLjEtyUUBJDY9K4FxEFcULytA/IkVnATTG9ERuLF/yB6ukSFR+iUWRYAmtuOuU0k9BvaqezbGqNoK5Grlkes+dYX6/0yUObumcw9/iAI";

        // bob creates INBOUND GROUP SESSION
        try {
            bobInboundGroupSession = new OlmInboundGroupSession(sessionKeyRef);
        } catch (OlmException e) {
            fail("Exception in test18TestBadCharacterCrashInDecrypt, Exception code=" + e.getExceptionCode());
        }

        OlmInboundGroupSession.DecryptMessageResult result = null;

        try {
            result = bobInboundGroupSession.decryptMessage(msgToDecryptWithEmoji);
        } catch (Exception e) {
            fail("Exception in test18TestBadCharacterCrashInDecrypt, Exception code=" + e.getMessage());
        }

        assertNotNull(result.mDecryptedMessage);
        assertEquals(13, result.mIndex);
    }

    /**
     * Specific test to check an error message is returned by decryptMessage() API.<br>
     * A corrupted encrypted message is passed, and a INVALID_BASE64 is
     * espexted.
    **/
    @Test
    public void test19TestErrorMessageReturnedInDecrypt() {
        OlmInboundGroupSession bobInboundGroupSession=null;
        final String EXPECTED_ERROR_MESSAGE= "INVALID_BASE64";

        String sessionKeyRef    = "AgAAAAycZE6AekIctJWYxd2AWLOY15YmxZODm/WkgbpWkyycp6ytSp/R+wo84jRrzBNWmv6ySLTZ9R0EDOk9VI2eZyQ6Efdwyo1mAvrWvTkZl9yALPdkOIVHywyG65f1SNiLrnsln3hgsT1vUrISGyKtsljoUgQpr3JDPEhD0ilAi63QBjhnGCW252b+7nF+43rb6O6lwm93LaVwe2341Gdp6EkhTUvetALezEqDOtKN00wVqAbq0RQAnUJIowxHbMswg+FyoR1K1oCjnVEoF23O9xlAn5g1XtuBZP3moJlR2lwsBA";
        String corruptedEncryptedMsg = "AwgANYTHINGf87ge45ge7gr*/rg5ganything4gr41rrgr4re55tanythingmcsXUkhDv0UePj922kgf+";

        // valid INBOUND GROUP SESSION
        try {
            bobInboundGroupSession = new OlmInboundGroupSession(sessionKeyRef);
        } catch (OlmException e) {
            fail("Exception in test19TestErrorMessageReturnedInDecrypt, Exception code=" + e.getExceptionCode());
        }

        String exceptionMessage = null;
        try {
            bobInboundGroupSession.decryptMessage(corruptedEncryptedMsg);
        } catch (OlmException e) {
            exceptionMessage = e.getMessage();
        }

        assertEquals(EXPECTED_ERROR_MESSAGE, exceptionMessage);
    }


    /**
     * Test the import/export functions.<br>
     **/
    @Test
    public void test20TestInboundGroupSessionImportExport() {

        String sessionKey = "AgAAAAAwMTIzNDU2Nzg5QUJERUYwMTIzNDU2Nzg5QUJDREVGMDEyMzQ1Njc4OUFCREVGM" +
                                    "DEyMzQ1Njc4OUFCQ0RFRjAxMjM0NTY3ODlBQkRFRjAxMjM0NTY3ODlBQkNERUYwMTIzND" +
                                    "U2Nzg5QUJERUYwMTIzNDU2Nzg5QUJDREVGMDEyMw0bdg1BDq4Px/slBow06q8n/B9WBfw" +
                                    "WYyNOB8DlUmXGGwrFmaSb9bR/eY8xgERrxmP07hFmD9uqA2p8PMHdnV5ysmgufE6oLZ5+" +
                                    "8/mWQOW3VVTnDIlnwd8oHUYRuk8TCQ";

        String message = "AwgAEhAcbh6UpbByoyZxufQ+h2B+8XHMjhR69G8F4+qjMaFlnIXusJZX3r8LnRORG9T3D" +
                            "XFdbVuvIWrLyRfm4i8QRbe8VPwGRFG57B1CtmxanuP8bHtnnYqlwPsD";


        OlmInboundGroupSession inboundGroupSession = null;

        try {
            inboundGroupSession = new OlmInboundGroupSession(sessionKey);
        } catch (Exception e) {
            fail("OlmInboundGroupSession failed " + e.getMessage());
        }

        boolean isVerified = false;

        try {
            isVerified = inboundGroupSession.isVerified();
        } catch (Exception e) {
            fail("isVerified failed " + e.getMessage());
        }

        assertTrue(isVerified);

        OlmInboundGroupSession.DecryptMessageResult result = null;

        try {
            result = inboundGroupSession.decryptMessage(message);
        } catch (Exception e) {
            fail("decryptMessage failed " + e.getMessage());
        }

        assertTrue(TextUtils.equals(result.mDecryptedMessage, "Message"));
        assertEquals(0, result.mIndex);

        String export = null;

        try {
            export = inboundGroupSession.export(0);
        } catch (Exception e) {
            fail("export failed " + e.getMessage());
        }
        assertFalse(TextUtils.isEmpty(export));

        long index = -1;
        try {
            index = inboundGroupSession.getFirstKnownIndex();
        } catch (Exception e) {
            fail("getFirstKnownIndex failed " + e.getMessage());
        }
        assertTrue(index >=0);

        inboundGroupSession.releaseSession();
        inboundGroupSession = null;

        OlmInboundGroupSession inboundGroupSession2 = null;

        try {
            inboundGroupSession2 = inboundGroupSession.importSession(export);
        } catch (Exception e) {
            fail("OlmInboundGroupSession failed " + e.getMessage());
        }

        try {
            isVerified = inboundGroupSession2.isVerified();
        } catch (Exception e) {
            fail("isVerified failed " + e.getMessage());
        }

        assertFalse(isVerified);

        result = null;
        try {
            result = inboundGroupSession2.decryptMessage(message);
        } catch (Exception e) {
            fail("decryptMessage failed " + e.getMessage());
        }

        assertTrue(TextUtils.equals(result.mDecryptedMessage, "Message"));
        assertEquals(0, result.mIndex);

        try {
            isVerified = inboundGroupSession2.isVerified();
        } catch (Exception e) {
            fail("isVerified failed " + e.getMessage());
        }

        assertTrue(isVerified);
        inboundGroupSession2.releaseSession();
    }
}

