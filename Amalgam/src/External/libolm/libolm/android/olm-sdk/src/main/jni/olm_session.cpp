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

#include "olm_session.h"

using namespace AndroidOlmSdk;

/**
* Init memory allocation for a session creation.<br>
* Make sure releaseSessionJni() is called when one is done with the session instance.
* @return valid memory allocation, NULL otherwise
**/
OlmSession* initializeSessionMemory()
{
    size_t sessionSize = olm_session_size();
    OlmSession* sessionPtr = (OlmSession*)malloc(sessionSize);

    if (sessionPtr)
    {
        // init session object
        sessionPtr = olm_session(sessionPtr);
        LOGD("## initializeSessionMemory(): success - OLM session size=%lu",static_cast<long unsigned int>(sessionSize));
    }
    else
    {
        LOGE("## initializeSessionMemory(): failure - OOM");
    }

    return sessionPtr;
}

JNIEXPORT jlong OLM_SESSION_FUNC_DEF(createNewSessionJni)(JNIEnv *env, jobject thiz)
{
    LOGD("## createNewSessionJni(): IN");
    OlmSession* accountPtr = initializeSessionMemory();

    if (!accountPtr)
    {
        LOGE("## initNewAccount(): failure - init session OOM");
        env->ThrowNew(env->FindClass("java/lang/Exception"), "init session OOM");
    }
    else
    {
        LOGD(" ## createNewSessionJni(): success - accountPtr=%p (jlong)(intptr_t)accountPtr=%lld",accountPtr,(jlong)(intptr_t)accountPtr);
    }

    return (jlong)(intptr_t)accountPtr;
}

JNIEXPORT void OLM_SESSION_FUNC_DEF(releaseSessionJni)(JNIEnv *env, jobject thiz)
{
    LOGD("## releaseSessionJni(): IN");
    OlmSession* sessionPtr = getSessionInstanceId(env, thiz);

    if (!sessionPtr)
    {
        LOGE("## releaseSessionJni(): failure - invalid Session ptr=NULL");
    }
    else
    {
        olm_clear_session(sessionPtr);

        // even if free(NULL) does not crash, logs are performed for debug purpose
        free(sessionPtr);
    }
}

// *********************************************************************
// ********************** OUTBOUND SESSION *****************************
// *********************************************************************
/**
 * Create a new in-bound session for sending/receiving messages from an
 * incoming PRE_KEY message.<br> The recipient is defined as the entity
 * with whom the session is established.
 * @param aOlmAccountId account instance
 * @param aTheirIdentityKey the identity key of the recipient
 * @param aTheirOneTimeKey the one time key of the recipient or an exception is thrown
 **/
JNIEXPORT void OLM_SESSION_FUNC_DEF(initOutboundSessionJni)(JNIEnv *env, jobject thiz, jlong aOlmAccountId, jbyteArray aTheirIdentityKeyBuffer, jbyteArray aTheirOneTimeKeyBuffer)
{
    OlmSession* sessionPtr = getSessionInstanceId(env, thiz);
    const char* errorMessage = NULL;
    OlmAccount* accountPtr = NULL;

    if (!sessionPtr)
    {
        LOGE("## initOutboundSessionJni(): failure - invalid Session ptr=NULL");
        errorMessage = "invalid Session ptr=NULL";
    }
    else if (!(accountPtr = (OlmAccount*)aOlmAccountId))
    {
        LOGE("## initOutboundSessionJni(): failure - invalid Account ptr=NULL");
        errorMessage = "invalid Account ptr=NULL";
    }
    else if (!aTheirIdentityKeyBuffer || !aTheirOneTimeKeyBuffer)
    {
        LOGE("## initOutboundSessionJni(): failure - invalid keys");
        errorMessage = "invalid keys";
    }
    else
    {
        size_t randomSize = olm_create_outbound_session_random_length(sessionPtr);
        uint8_t *randomBuffPtr = NULL;

        LOGD("## initOutboundSessionJni(): randomSize=%lu",static_cast<long unsigned int>(randomSize));

        if ( (0 != randomSize) && !setRandomInBuffer(env, &randomBuffPtr, randomSize))
        {
            LOGE("## initOutboundSessionJni(): failure - random buffer init");
            errorMessage = "random buffer init";
        }
        else
        {
            jbyte* theirIdentityKeyPtr = NULL;
            jbyte* theirOneTimeKeyPtr = NULL;

            // convert identity & one time keys to C strings
            if (!(theirIdentityKeyPtr = env->GetByteArrayElements(aTheirIdentityKeyBuffer, 0)))
            {
                LOGE("## initOutboundSessionJni(): failure - identityKey JNI allocation OOM");
                errorMessage = "identityKey JNI allocation OOM";
            }
            else if (!(theirOneTimeKeyPtr = env->GetByteArrayElements(aTheirOneTimeKeyBuffer, 0)))
            {
                LOGE("## initOutboundSessionJni(): failure - one time Key JNI allocation OOM");
                errorMessage = "one time Key JNI allocation OOM";
            }
            else
            {
                size_t theirIdentityKeyLength = (size_t)env->GetArrayLength(aTheirIdentityKeyBuffer);
                size_t theirOneTimeKeyLength  = (size_t)env->GetArrayLength(aTheirOneTimeKeyBuffer);
                LOGD("## initOutboundSessionJni(): identityKey=%.*s oneTimeKey=%.*s", static_cast<int>(theirIdentityKeyLength), theirIdentityKeyPtr, static_cast<int>(theirOneTimeKeyLength), theirOneTimeKeyPtr);

                size_t sessionResult = olm_create_outbound_session(sessionPtr,
                                                            accountPtr,
                                                            theirIdentityKeyPtr,
                                                            theirIdentityKeyLength,
                                                            theirOneTimeKeyPtr,
                                                            theirOneTimeKeyLength,
                                                            (void*)randomBuffPtr,
                                                            randomSize);
                if (sessionResult == olm_error()) {
                    errorMessage = (const char *)olm_session_last_error(sessionPtr);
                    LOGE("## initOutboundSessionJni(): failure - session creation  Msg=%s", errorMessage);
                }
                else
                {
                    LOGD("## initOutboundSessionJni(): success - result=%lu", static_cast<long unsigned int>(sessionResult));
                }
            }

            if (theirIdentityKeyPtr)
            {
                env->ReleaseByteArrayElements(aTheirIdentityKeyBuffer, theirIdentityKeyPtr, JNI_ABORT);
            }

            if (theirOneTimeKeyPtr)
            {
                env->ReleaseByteArrayElements(aTheirOneTimeKeyBuffer, theirOneTimeKeyPtr, JNI_ABORT);
            }

            if (randomBuffPtr)
            {
                memset(randomBuffPtr, 0, randomSize);
                free(randomBuffPtr);
            }
        }
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }
}


// *********************************************************************
// *********************** INBOUND SESSION *****************************
// *********************************************************************
/**
 * Create a new in-bound session for sending/receiving messages from an
 * incoming PRE_KEY message.<br>
 * An exception is thrown if the operation fails.
 * @param aOlmAccountId account instance
 * @param aOneTimeKeyMsg PRE_KEY message
 */
JNIEXPORT void OLM_SESSION_FUNC_DEF(initInboundSessionJni)(JNIEnv *env, jobject thiz, jlong aOlmAccountId, jbyteArray aOneTimeKeyMsgBuffer)
{
    const char* errorMessage = NULL;
    OlmSession *sessionPtr = getSessionInstanceId(env,thiz);
    OlmAccount *accountPtr = NULL;
    size_t sessionResult;

    if (!sessionPtr)
    {
        LOGE("## initInboundSessionJni(): failure - invalid Session ptr=NULL");
        errorMessage = "invalid Session ptr=NULL";
    }
    else if (!(accountPtr = (OlmAccount*)aOlmAccountId))
    {
        LOGE("## initInboundSessionJni(): failure - invalid Account ptr=NULL");
        errorMessage = "invalid Account ptr=NULL";
    }
    else if (!aOneTimeKeyMsgBuffer)
    {
        LOGE("## initInboundSessionJni(): failure - invalid message");
        errorMessage = "invalid message";
    }
    else
    {
        jbyte* messagePtr = env->GetByteArrayElements(aOneTimeKeyMsgBuffer, 0);

        if (!messagePtr)
        {
            LOGE("## initInboundSessionJni(): failure - message JNI allocation OOM");
            errorMessage = "message JNI allocation OOM";
        }
        else
        {
            size_t messageLength = (size_t)env->GetArrayLength(aOneTimeKeyMsgBuffer);
            LOGD("## initInboundSessionJni(): messageLength=%lu message=%.*s", static_cast<long unsigned int>(messageLength), static_cast<int>(messageLength), messagePtr);

            sessionResult = olm_create_inbound_session(sessionPtr, accountPtr, (void*)messagePtr , messageLength);

            if (sessionResult == olm_error())
            {
                errorMessage = olm_session_last_error(sessionPtr);
                LOGE("## initInboundSessionJni(): failure - init inbound session creation  Msg=%s", errorMessage);
            }
            else
            {
                LOGD("## initInboundSessionJni(): success - result=%lu", static_cast<long unsigned int>(sessionResult));
            }

            // free local alloc
            env->ReleaseByteArrayElements(aOneTimeKeyMsgBuffer, messagePtr, JNI_ABORT);
        }
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }
}

/**
 * Create a new in-bound session for sending/receiving messages from an
 * incoming PRE_KEY message based on the recipient identity key.<br>
 * An exception is thrown if the operation fails.
 * @param aOlmAccountId account instance
 * @param aTheirIdentityKey the identity key of the recipient
 * @param aOneTimeKeyMsg encrypted message
 */
JNIEXPORT void OLM_SESSION_FUNC_DEF(initInboundSessionFromIdKeyJni)(JNIEnv *env, jobject thiz, jlong aOlmAccountId, jbyteArray aTheirIdentityKeyBuffer, jbyteArray aOneTimeKeyMsgBuffer)
{
    const char* errorMessage = NULL;

    OlmSession *sessionPtr = getSessionInstanceId(env, thiz);
    OlmAccount *accountPtr = NULL;
    jbyte *messagePtr = NULL;
    jbyte *theirIdentityKeyPtr = NULL;
    size_t sessionResult;

    if (!sessionPtr)
    {
        LOGE("## initInboundSessionFromIdKeyJni(): failure - invalid Session ptr=NULL");
        errorMessage = "invalid Session ptr=NULL";
    }
    else if (!(accountPtr = (OlmAccount*)aOlmAccountId))
    {
        LOGE("## initInboundSessionFromIdKeyJni(): failure - invalid Account ptr=NULL");
        errorMessage = "invalid Account ptr=NULL";
    }
    else if (!aTheirIdentityKeyBuffer)
    {
        LOGE("## initInboundSessionFromIdKeyJni(): failure - invalid theirIdentityKey");
        errorMessage = "invalid theirIdentityKey";
    }
    else if (!aOneTimeKeyMsgBuffer)
    {
        LOGE("## initInboundSessionJni(): failure - invalid one time key message");
        errorMessage = "invalid invalid one time key message";
    }
    else if (!(messagePtr = env->GetByteArrayElements(aOneTimeKeyMsgBuffer, 0)))
    {
        LOGE("## initInboundSessionFromIdKeyJni(): failure - message JNI allocation OOM");
        errorMessage = "message JNI allocation OOM";
    }
    else if(!(theirIdentityKeyPtr = env->GetByteArrayElements(aTheirIdentityKeyBuffer, 0)))
    {
        LOGE("## initInboundSessionFromIdKeyJni(): failure - theirIdentityKey JNI allocation OOM");
        errorMessage = "theirIdentityKey JNI allocation OOM";
    }
    else
    {
        size_t messageLength = (size_t)env->GetArrayLength(aOneTimeKeyMsgBuffer);
        size_t theirIdentityKeyLength = (size_t)env->GetArrayLength(aTheirIdentityKeyBuffer);

        LOGD("## initInboundSessionFromIdKeyJni(): message=%.*s messageLength=%lu", static_cast<int>(messageLength), messagePtr, static_cast<long unsigned int>(messageLength));

        sessionResult = olm_create_inbound_session_from(sessionPtr, accountPtr, theirIdentityKeyPtr, theirIdentityKeyLength, (void*)messagePtr , messageLength);
        if (sessionResult == olm_error())
        {
            errorMessage = (const char *)olm_session_last_error(sessionPtr);
            LOGE("## initInboundSessionFromIdKeyJni(): failure - init inbound session creation  Msg=%s", errorMessage);
        }
        else
        {
            LOGD("## initInboundSessionFromIdKeyJni(): success - result=%lu", static_cast<long unsigned int>(sessionResult));
        }
     }

     // free local alloc
     if (messagePtr)
     {
        env->ReleaseByteArrayElements(aOneTimeKeyMsgBuffer, messagePtr, JNI_ABORT);
     }

     if (theirIdentityKeyPtr)
     {
        env->ReleaseByteArrayElements(aTheirIdentityKeyBuffer, theirIdentityKeyPtr, JNI_ABORT);
     }

     if (errorMessage)
     {
         env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
     }
}

/**
 * Checks if the PRE_KEY message is for this in-bound session.<br>
 * This API may be used to process a "m.room.encrypted" event when type = 1 (PRE_KEY).
 * @param aOneTimeKeyMsg PRE KEY message
 * @return true if the PRE_KEY message matches
 */
JNIEXPORT jboolean OLM_SESSION_FUNC_DEF(matchesInboundSessionJni)(JNIEnv *env, jobject thiz, jbyteArray aOneTimeKeyMsgBuffer)
{
    jboolean retCode = JNI_FALSE;
    OlmSession *sessionPtr = getSessionInstanceId(env, thiz);
    jbyte *messagePtr = NULL;

    if (!sessionPtr)
    {
        LOGE("## matchesInboundSessionJni(): failure - invalid Session ptr=NULL");
    }
    else if (!aOneTimeKeyMsgBuffer)
    {
        LOGE("## matchesInboundSessionJni(): failure - invalid one time key message");
    }
    else if (!(messagePtr = env->GetByteArrayElements(aOneTimeKeyMsgBuffer, 0)))
    {
        LOGE("## matchesInboundSessionJni(): failure - one time key JNI allocation OOM");
    }
    else
    {
        size_t messageLength = (size_t)env->GetArrayLength(aOneTimeKeyMsgBuffer);

        size_t matchResult = olm_matches_inbound_session(sessionPtr, (void*)messagePtr , messageLength);
        //if(matchResult == olm_error()) {
        // for now olm_matches_inbound_session() returns 1 when it succeeds, otherwise 1- or 0
        if (matchResult != 1) {
            LOGE("## matchesInboundSessionJni(): failure - no match  Msg=%s",(const char *)olm_session_last_error(sessionPtr));
        }
        else
        {
            retCode = JNI_TRUE;
            LOGD("## matchesInboundSessionJni(): success - result=%lu", static_cast<long unsigned int>(matchResult));
        }
    }

    // free local alloc
    if (messagePtr)
    {
        env->ReleaseByteArrayElements(aOneTimeKeyMsgBuffer, messagePtr, JNI_ABORT);
    }

    return retCode;
}

/**
 * Checks if the PRE_KEY message is for this in-bound session based on the sender identity key.<br>
 * This API may be used to process a "m.room.encrypted" event when type = 1 (PRE_KEY).
 * @param aTheirIdentityKey the identity key of the sender
 * @param aOneTimeKeyMsg PRE KEY message
 * @return true if the PRE_KEY message matches.
 */
JNIEXPORT jboolean JNICALL OLM_SESSION_FUNC_DEF(matchesInboundSessionFromIdKeyJni)(JNIEnv *env, jobject thiz, jbyteArray aTheirIdentityKeyBuffer, jbyteArray aOneTimeKeyMsgBuffer)
{
    jboolean retCode = JNI_FALSE;
    OlmSession *sessionPtr = getSessionInstanceId(env, thiz);
    jbyte *messagePtr = NULL;
    jbyte *theirIdentityKeyPtr = NULL;

    if (!sessionPtr)
    {
        LOGE("## matchesInboundSessionFromIdKeyJni(): failure - invalid Session ptr=NULL");
    }
    else if (!aTheirIdentityKeyBuffer)
    {
        LOGE("## matchesInboundSessionFromIdKeyJni(): failure - invalid theirIdentityKey");
    }
    else if (!(theirIdentityKeyPtr = env->GetByteArrayElements(aTheirIdentityKeyBuffer, 0)))
    {
        LOGE("## matchesInboundSessionFromIdKeyJni(): failure - theirIdentityKey JNI allocation OOM");
    }
    else if (!aOneTimeKeyMsgBuffer)
    {
        LOGE("## matchesInboundSessionFromIdKeyJni(): failure - invalid one time key message");
    }
    else if (!(messagePtr = env->GetByteArrayElements(aOneTimeKeyMsgBuffer, 0)))
    {
        LOGE("## matchesInboundSessionFromIdKeyJni(): failure - one time key JNI allocation OOM");
    }
    else
    {
        size_t identityKeyLength = (size_t)env->GetArrayLength(aTheirIdentityKeyBuffer);
        size_t messageLength = (size_t)env->GetArrayLength(aOneTimeKeyMsgBuffer);
        size_t matchResult = olm_matches_inbound_session_from(sessionPtr, (void const *)theirIdentityKeyPtr, identityKeyLength, (void*)messagePtr , messageLength);

        //if(matchResult == olm_error()) {
        // for now olm_matches_inbound_session() returns 1 when it succeeds, otherwise 1- or 0
        if (matchResult != 1)
        {
            LOGE("## matchesInboundSessionFromIdKeyJni(): failure - no match  Msg=%s",(const char *)olm_session_last_error(sessionPtr));
        }
        else
        {
            retCode = JNI_TRUE;
            LOGD("## matchesInboundSessionFromIdKeyJni(): success - result=%lu", static_cast<long unsigned int>(matchResult));
        }
    }

    // free local alloc
    if (theirIdentityKeyPtr)
    {
        env->ReleaseByteArrayElements(aTheirIdentityKeyBuffer, theirIdentityKeyPtr, JNI_ABORT);
    }

    if (messagePtr)
    {
        env->ReleaseByteArrayElements(aOneTimeKeyMsgBuffer, messagePtr, JNI_ABORT);
    }

    return retCode;
}

/**
 * Encrypt a message using the session.<br>
 * An exception is thrown if the operation fails.
 * @param aClearMsg clear text message
 * @param [out] aEncryptedMsg ciphered message
 * @return the encrypted message
 */
JNIEXPORT jbyteArray OLM_SESSION_FUNC_DEF(encryptMessageJni)(JNIEnv *env, jobject thiz, jbyteArray aClearMsgBuffer, jobject aEncryptedMsg)
{
    jbyteArray encryptedMsgRet = 0;
    const char* errorMessage = NULL;

    OlmSession *sessionPtr = getSessionInstanceId(env, thiz);
    jbyte *clearMsgPtr = NULL;
    jboolean clearMsgIsCopied = JNI_FALSE;
    jclass encryptedMsgJClass = 0;
    jfieldID typeMsgFieldId;

    LOGD("## encryptMessageJni(): IN ");

    if (!sessionPtr)
    {
        LOGE("## encryptMessageJni(): failure - invalid Session ptr=NULL");
        errorMessage = "invalid Session ptr=NULL";
    }
    else if (!aClearMsgBuffer)
    {
        LOGE("## encryptMessageJni(): failure - invalid clear message");
        errorMessage = "invalid clear message";
    }
    else if (!aEncryptedMsg)
    {
        LOGE("## encryptMessageJni(): failure - invalid encrypted message");
        errorMessage = "invalid encrypted message";
    }
    else if (!(clearMsgPtr = env->GetByteArrayElements(aClearMsgBuffer, &clearMsgIsCopied)))
    {
        LOGE("## encryptMessageJni(): failure - clear message JNI allocation OOM");
        errorMessage = "clear message JNI allocation OOM";
    }
    else if (!(encryptedMsgJClass = env->GetObjectClass(aEncryptedMsg)))
    {
        LOGE("## encryptMessageJni(): failure - unable to get crypted message class");
        errorMessage = "unable to get crypted message class";
    }
    else if (!(typeMsgFieldId = env->GetFieldID(encryptedMsgJClass,"mType","J")))
    {
        LOGE("## encryptMessageJni(): failure - unable to get message type field");
        errorMessage = "unable to get message type field";
    }
    else
    {
        // get message type
        size_t messageType = olm_encrypt_message_type(sessionPtr);
        uint8_t *randomBuffPtr = NULL;

        // compute random buffer
        // Note: olm_encrypt_random_length() can return 0, which means
        // it just does not need new random data to encrypt a new message
        size_t randomLength = olm_encrypt_random_length(sessionPtr);

        LOGD("## encryptMessageJni(): randomLength=%lu", static_cast<long unsigned int>(randomLength));

        if ((0 != randomLength) && !setRandomInBuffer(env, &randomBuffPtr, randomLength))
        {
            LOGE("## encryptMessageJni(): failure - random buffer init");
            errorMessage = "random buffer init";
        }
        else
        {
            // alloc buffer for encrypted message
            size_t clearMsgLength = (size_t)env->GetArrayLength(aClearMsgBuffer);
            size_t encryptedMsgLength = olm_encrypt_message_length(sessionPtr, clearMsgLength);

            void *encryptedMsgPtr = malloc(encryptedMsgLength*sizeof(uint8_t));

            if (!encryptedMsgPtr)
            {
                LOGE("## encryptMessageJni(): failure - encryptedMsgPtr buffer OOM");
                errorMessage = "encryptedMsgPtr buffer OOM";
            }
            else
            {
                if (0 == randomLength)
                {
                    LOGW("## encryptMessageJni(): random buffer is not required");
                }

                LOGD("## encryptMessageJni(): messageType=%lu randomLength=%lu clearMsgLength=%lu encryptedMsgLength=%lu",static_cast<long unsigned int>(messageType),static_cast<long unsigned int>(randomLength), static_cast<long unsigned int>(clearMsgLength), static_cast<long unsigned int>(encryptedMsgLength));
                // encrypt message
                size_t result = olm_encrypt(sessionPtr,
                                            (void const *)clearMsgPtr,
                                            clearMsgLength,
                                            randomBuffPtr,
                                            randomLength,
                                            encryptedMsgPtr,
                                            encryptedMsgLength);
                if (result == olm_error())
                {
                    errorMessage = (const char *)olm_session_last_error(sessionPtr);
                    LOGE("## encryptMessageJni(): failure - Msg=%s", errorMessage);
                }
                else
                {
                    // update message type: PRE KEY or normal
                    env->SetLongField(aEncryptedMsg, typeMsgFieldId, (jlong)messageType);

                    encryptedMsgRet = env->NewByteArray(encryptedMsgLength);
                    env->SetByteArrayRegion(encryptedMsgRet, 0 , encryptedMsgLength, (jbyte*)encryptedMsgPtr);

                    LOGD("## encryptMessageJni(): success - result=%lu Type=%lu encryptedMsg=%.*s", static_cast<long unsigned int>(result), static_cast<unsigned long int>(messageType), static_cast<int>(result), (const char*)encryptedMsgPtr);
                }

                free(encryptedMsgPtr);
            }

            memset(randomBuffPtr, 0, randomLength);
            free(randomBuffPtr);
        }
    }

    // free alloc
    if (clearMsgPtr)
    {
        if (clearMsgIsCopied)
        {
            memset(clearMsgPtr, 0, (size_t)env->GetArrayLength(aClearMsgBuffer));
        }
        env->ReleaseByteArrayElements(aClearMsgBuffer, clearMsgPtr, JNI_ABORT);
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return encryptedMsgRet;
}

/**
 * Decrypt a message using the session.<br>
 * An exception is thrown if the operation fails.
 * @param aEncryptedMsg message to decrypt
 * @return decrypted message if operation succeed
 */
JNIEXPORT jbyteArray OLM_SESSION_FUNC_DEF(decryptMessageJni)(JNIEnv *env, jobject thiz, jobject aEncryptedMsg)
{
    const char* errorMessage = NULL;

    jbyteArray decryptedMsgRet = 0;

    jclass encryptedMsgJClass = 0;
    jstring encryptedMsgJstring = 0; // <= obtained from encryptedMsgFieldId
    // field IDs
    jfieldID encryptedMsgFieldId;
    jfieldID typeMsgFieldId;
    // ptrs
    OlmSession *sessionPtr = getSessionInstanceId(env, thiz);
    const char *encryptedMsgPtr = NULL; // <= obtained from encryptedMsgJstring
    uint8_t *plainTextMsgPtr = NULL;
    char *tempEncryptedPtr = NULL;

    LOGD("## decryptMessageJni(): IN - OlmSession");

    if (!sessionPtr)
    {
        LOGE("## decryptMessageJni(): failure - invalid Session ptr=NULL");
        errorMessage = "invalid Session ptr=NULL";
    }
    else if (!aEncryptedMsg)
    {
        LOGE("## decryptMessageJni(): failure - invalid encrypted message");
        errorMessage = "invalid encrypted message";
    }
    else if (!(encryptedMsgJClass = env->GetObjectClass(aEncryptedMsg)))
    {
        LOGE("## decryptMessageJni(): failure - unable to get encrypted message class");
        errorMessage = "unable to get encrypted message class";
    }
    else if (!(encryptedMsgFieldId = env->GetFieldID(encryptedMsgJClass,"mCipherText","Ljava/lang/String;")))
    {
        LOGE("## decryptMessageJni(): failure - unable to get message field");
        errorMessage = "unable to get message field";
    }
    else if (!(typeMsgFieldId = env->GetFieldID(encryptedMsgJClass,"mType","J")))
    {
        LOGE("## decryptMessageJni(): failure - unable to get message type field");
        errorMessage = "unable to get message type field";
    }
    else if (!(encryptedMsgJstring = (jstring)env->GetObjectField(aEncryptedMsg, encryptedMsgFieldId)))
    {
        LOGE("## decryptMessageJni(): failure - JNI encrypted object ");
        errorMessage = "JNI encrypted object";
    }
    else if (!(encryptedMsgPtr = env->GetStringUTFChars(encryptedMsgJstring, 0)))
    {
        LOGE("## decryptMessageJni(): failure - encrypted message JNI allocation OOM");
        errorMessage = "encrypted message JNI allocation OOM";
    }
    else
    {
        // get message type
        size_t encryptedMsgType = (size_t)env->GetLongField(aEncryptedMsg, typeMsgFieldId);
        // get encrypted message length
        size_t encryptedMsgLength = (size_t)env->GetStringUTFLength(encryptedMsgJstring);

        // create a dedicated temp buffer to be used in next Olm API calls
        tempEncryptedPtr = static_cast<char*>(malloc(encryptedMsgLength*sizeof(uint8_t)));
        memcpy(tempEncryptedPtr, encryptedMsgPtr, encryptedMsgLength);
        LOGD("## decryptMessageJni(): MsgType=%lu encryptedMsgLength=%lu encryptedMsg=%.*s",static_cast<long unsigned int>(encryptedMsgType),static_cast<long unsigned int>(encryptedMsgLength), static_cast<int>(encryptedMsgLength), encryptedMsgPtr);

        // get max plaintext length
        size_t maxPlainTextLength = olm_decrypt_max_plaintext_length(sessionPtr,
                                                                     static_cast<size_t>(encryptedMsgType),
                                                                     static_cast<void*>(tempEncryptedPtr),
                                                                     encryptedMsgLength);
        // Note: tempEncryptedPtr is destroyed by olm_decrypt_max_plaintext_length()

        if (maxPlainTextLength == olm_error())
        {
            errorMessage = (const char *)olm_session_last_error(sessionPtr);
            LOGE("## decryptMessageJni(): failure - olm_decrypt_max_plaintext_length Msg=%s", errorMessage);
        }
        else
        {
            LOGD("## decryptMessageJni(): maxPlaintextLength=%lu",static_cast<long unsigned int>(maxPlainTextLength));

            // allocate output decrypted message
            plainTextMsgPtr = static_cast<uint8_t*>(malloc(maxPlainTextLength*sizeof(uint8_t)));

            // decrypt, but before reload encrypted buffer (previous one was destroyed)
            memcpy(tempEncryptedPtr, encryptedMsgPtr, encryptedMsgLength);
            size_t plaintextLength = olm_decrypt(sessionPtr,
                                                 encryptedMsgType,
                                                 (void*)tempEncryptedPtr,
                                                 encryptedMsgLength,
                                                 plainTextMsgPtr,
                                                 maxPlainTextLength);
            if (plaintextLength == olm_error())
            {
                errorMessage = (const char *)olm_session_last_error(sessionPtr);
                LOGE("## decryptMessageJni(): failure - olm_decrypt Msg=%s", errorMessage);
            }
            else
            {
                decryptedMsgRet = env->NewByteArray(plaintextLength);
                env->SetByteArrayRegion(decryptedMsgRet, 0 , plaintextLength, (jbyte*)plainTextMsgPtr);

                LOGD(" ## decryptMessageJni(): UTF-8 Conversion - decrypted returnedLg=%lu OK",static_cast<long unsigned int>(plaintextLength));
            }

            memset(plainTextMsgPtr, 0, maxPlainTextLength);
        }
    }

    // free alloc
    if (encryptedMsgPtr)
    {
        env->ReleaseStringUTFChars(encryptedMsgJstring, encryptedMsgPtr);
    }

    if (tempEncryptedPtr)
    {
        free(tempEncryptedPtr);
    }

    if (plainTextMsgPtr)
    {
        free(plainTextMsgPtr);
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return decryptedMsgRet;
}

/**
 * Get the session identifier for this session.
 * An exception is thrown if the operation fails.
 * @return the session identifier
 */
JNIEXPORT jbyteArray OLM_SESSION_FUNC_DEF(getSessionIdentifierJni)(JNIEnv *env, jobject thiz)
{
     const char* errorMessage = NULL;
     jbyteArray returnValue = 0;

     LOGD("## getSessionIdentifierJni(): IN ");

     OlmSession *sessionPtr = getSessionInstanceId(env, thiz);

     if (!sessionPtr)
     {
         LOGE("## getSessionIdentifierJni(): failure - invalid Session ptr=NULL");
         errorMessage = "invalid Session ptr=NULL";
     }
     else
     {
         // get the size to alloc to contain the id
         size_t lengthSessionId = olm_session_id_length(sessionPtr);
         LOGD("## getSessionIdentifierJni(): lengthSessionId=%lu",static_cast<long unsigned int>(lengthSessionId));

         void *sessionIdPtr = malloc(lengthSessionId*sizeof(uint8_t));

         if (!sessionIdPtr)
         {
            LOGE("## getSessionIdentifierJni(): failure - identifier allocation OOM");
            errorMessage = "identifier allocation OOM";
         }
         else
         {
             size_t result = olm_session_id(sessionPtr, sessionIdPtr, lengthSessionId);

             if (result == olm_error())
             {
                 errorMessage = (const char *)olm_session_last_error(sessionPtr);
                 LOGE("## getSessionIdentifierJni(): failure - get session identifier failure Msg=%s", errorMessage);
             }
             else
             {
                 LOGD("## getSessionIdentifierJni(): success - result=%lu sessionId=%.*s",static_cast<long unsigned int>(result), static_cast<int>(result), (char*)sessionIdPtr);

                 returnValue = env->NewByteArray(result);
                 env->SetByteArrayRegion(returnValue, 0 , result, (jbyte*)sessionIdPtr);
             }

             free(sessionIdPtr);
         }
     }

     if (errorMessage)
     {
         env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
     }

     return returnValue;
}

JNIEXPORT jbyteArray OLM_SESSION_FUNC_DEF(olmSessionDescribeJni(JNIEnv *env, jobject thiz))
{
    const char* errorMessage = NULL;
    jbyteArray returnValue = 0;

    LOGD("## olmSessionDescribeJni(): IN ");

    OlmSession *sessionPtr = getSessionInstanceId(env, thiz);

    if (!sessionPtr)
    {
        LOGE("## olmSessionDescribeJni(): failure - invalid Session ptr=NULL");
        errorMessage = "invalid Session ptr=NULL";
    }
    else
    {
        int maxLength = 600;
        char* describePtr = NULL;
        describePtr = (char*) malloc(maxLength * sizeof *describePtr);
        if (!describePtr)
        {
            LOGE("## olmSessionDescribeJni(): failure - describe allocation OOM");
            errorMessage = "describe allocation OOM";
        }
        else
        {
            olm_session_describe(sessionPtr, describePtr, maxLength);
            int length = strlen(describePtr);
            if (length == 0)
            {
                LOGE("## olmSessionDescribeJni(): failure - get session describe");
            }
            else
            {
                LOGD("## olmSessionDescribeJni(): success - describe=%.*s", (char*)describePtr);

                returnValue = env->NewByteArray(length);
                env->SetByteArrayRegion(returnValue, 0, length, (jbyte*)describePtr);
            }

            free(describePtr);
        }
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return returnValue;
}

/**
 * Serialize and encrypt session instance.<br>
 * An exception is thrown if the operation fails.
 * @param aKeyBuffer key used to encrypt the serialized account data
 * @return the serialised account as bytes buffer.
 **/
JNIEXPORT jbyteArray OLM_SESSION_FUNC_DEF(serializeJni)(JNIEnv *env, jobject thiz, jbyteArray aKeyBuffer)
{
    const char* errorMessage = NULL;
    jbyteArray returnValue = 0;

    jbyte* keyPtr = NULL;
    jboolean keyWasCopied = JNI_FALSE;
    OlmSession* sessionPtr = getSessionInstanceId(env, thiz);

    LOGD("## serializeJni(): IN");

    if (!sessionPtr)
    {
        LOGE(" ## serializeJni(): failure - invalid session ptr");
        errorMessage = "invalid session ptr";
    }
    else if (!aKeyBuffer)
    {
        LOGE(" ## serializeJni(): failure - invalid key");
        errorMessage = "invalid key";
    }
    else if (!(keyPtr = env->GetByteArrayElements(aKeyBuffer, &keyWasCopied)))
    {
        LOGE(" ## serializeJni(): failure - keyPtr JNI allocation OOM");
        errorMessage = "ikeyPtr JNI allocation OOM";
    }
    else
    {
        size_t pickledLength = olm_pickle_session_length(sessionPtr);
        size_t keyLength = (size_t)env->GetArrayLength(aKeyBuffer);
        LOGD(" ## serializeJni(): pickledLength=%lu keyLength=%lu",static_cast<long unsigned int>(pickledLength), static_cast<long unsigned int>(keyLength));

        void *pickledPtr = malloc(pickledLength*sizeof(uint8_t));

        if (!pickledPtr)
        {
            LOGE(" ## serializeJni(): failure - pickledPtr buffer OOM");
            errorMessage = "pickledPtr buffer OOM";
        }
        else
        {
            size_t result = olm_pickle_session(sessionPtr,
                                              (void const *)keyPtr,
                                              keyLength,
                                              (void*)pickledPtr,
                                              pickledLength);
            if (result == olm_error())
            {
                errorMessage = olm_session_last_error(sessionPtr);
                LOGE(" ## serializeJni(): failure - olm_pickle_session() Msg=%s", errorMessage);
            }
            else
            {
                LOGD(" ## serializeJni(): success - result=%lu pickled=%.*s", static_cast<long unsigned int>(result), static_cast<int>(pickledLength), static_cast<char*>(pickledPtr));

                returnValue = env->NewByteArray(pickledLength);
                env->SetByteArrayRegion(returnValue, 0 , pickledLength, (jbyte*)pickledPtr);
            }

            free(pickledPtr);
        }
    }

    // free alloc
    if (keyPtr)
    {
        if (keyWasCopied) {
            memset(keyPtr, 0, (size_t)env->GetArrayLength(aKeyBuffer));
        }
        env->ReleaseByteArrayElements(aKeyBuffer, keyPtr, JNI_ABORT);
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return returnValue;
}

/**
 * Allocate a new session and initialize it with the serialisation data.<br>
 * An exception is thrown if the operation fails.
 * @param aSerializedData the session serialisation buffer
 * @param aKey the key used to encrypt the serialized account data
 * @return the deserialized session
 **/
JNIEXPORT jlong OLM_SESSION_FUNC_DEF(deserializeJni)(JNIEnv *env, jobject thiz, jbyteArray aSerializedDataBuffer, jbyteArray aKeyBuffer)
{
    const char* errorMessage = NULL;
    OlmSession* sessionPtr = initializeSessionMemory();
    jbyte* keyPtr = NULL;
    jboolean keyWasCopied = JNI_FALSE;
    jbyte* pickledPtr = NULL;

    LOGD("## deserializeJni(): IN");

    if (!sessionPtr)
    {
        LOGE(" ## deserializeJni(): failure - session failure OOM");
        errorMessage = "session failure OOM";
    }
    else if (!aKeyBuffer)
    {
        LOGE(" ## deserializeJni(): failure - invalid key");
        errorMessage = "invalid key";
    }
    else if (!aSerializedDataBuffer)
    {
        LOGE(" ## deserializeJni(): failure - serialized data");
        errorMessage = "serialized data";
    }
    else if (!(keyPtr = env->GetByteArrayElements(aKeyBuffer, &keyWasCopied)))
    {
        LOGE(" ## deserializeJni(): failure - keyPtr JNI allocation OOM");
        errorMessage = "keyPtr JNI allocation OOM";
    }
    else if (!(pickledPtr = env->GetByteArrayElements(aSerializedDataBuffer, 0)))
    {
        LOGE(" ## deserializeJni(): failure - pickledPtr JNI allocation OOM");
        errorMessage = "pickledPtr JNI allocation OOM";
    }
    else
    {
        size_t pickledLength = (size_t)env->GetArrayLength(aSerializedDataBuffer);
        size_t keyLength = (size_t)env->GetArrayLength(aKeyBuffer);
        LOGD(" ## deserializeJni(): pickledLength=%lu keyLength=%lu",static_cast<long unsigned int>(pickledLength), static_cast<long unsigned int>(keyLength));
        LOGD(" ## deserializeJni(): pickled=%.*s",static_cast<int>(pickledLength), (char const *)pickledPtr);

        size_t result = olm_unpickle_session(sessionPtr,
                                             (void const *)keyPtr,
                                             keyLength,
                                             (void*)pickledPtr,
                                             pickledLength);
        if (result == olm_error())
        {
            errorMessage = olm_session_last_error(sessionPtr);
            LOGE(" ## deserializeJni(): failure - olm_unpickle_account() Msg=%s", errorMessage);
        }
        else
        {
            LOGD(" ## initJni(): success - result=%lu ", static_cast<long unsigned int>(result));
        }
    }

    // free alloc
    if (keyPtr)
    {
        if (keyWasCopied) {
            memset(keyPtr, 0, (size_t)env->GetArrayLength(aKeyBuffer));
        }
        env->ReleaseByteArrayElements(aKeyBuffer, keyPtr, JNI_ABORT);
    }

    if (pickledPtr)
    {
        env->ReleaseByteArrayElements(aSerializedDataBuffer, pickledPtr, JNI_ABORT);
    }

    if (errorMessage)
    {
        if (sessionPtr)
        {
            olm_clear_session(sessionPtr);
            free(sessionPtr);
        }
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return (jlong)(intptr_t)sessionPtr;
}
