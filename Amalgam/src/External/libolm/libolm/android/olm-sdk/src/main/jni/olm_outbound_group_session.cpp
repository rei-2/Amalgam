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

#include "olm_outbound_group_session.h"

using namespace AndroidOlmSdk;

/**
 * Release the session allocation made by initializeOutboundGroupSessionMemory().<br>
 * This method MUST be called when java counter part account instance is done.
 *
 */
JNIEXPORT void OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(releaseSessionJni)(JNIEnv *env, jobject thiz)
{
    LOGD("## releaseSessionJni(): OutBound group session IN");

    OlmOutboundGroupSession* sessionPtr = (OlmOutboundGroupSession*)getOutboundGroupSessionInstanceId(env,thiz);

    if (!sessionPtr)
    {
        LOGE(" ## releaseSessionJni(): failure - invalid outbound group session instance");
    }
    else
    {
        LOGD(" ## releaseSessionJni(): sessionPtr=%p",sessionPtr);

#ifdef ENABLE_JNI_LOG
        size_t retCode = olm_clear_outbound_group_session(sessionPtr);
        LOGD(" ## releaseSessionJni(): clear_outbound_group_session=%lu",static_cast<long unsigned int>(retCode));
#else
        olm_clear_outbound_group_session(sessionPtr);
#endif

        LOGD(" ## releaseSessionJni(): free IN");
        free(sessionPtr);
        LOGD(" ## releaseSessionJni(): free OUT");
    }
}

/**
 * Initialize a new outbound group session and return it to JAVA side.<br>
 * Since a C prt is returned as a jlong, special care will be taken
 * to make the cast (OlmOutboundGroupSession* => jlong) platform independent.
 * @return the initialized OlmOutboundGroupSession* instance or throw an exception
 **/
JNIEXPORT jlong OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(createNewSessionJni)(JNIEnv *env, jobject thiz)
{
    const char* errorMessage = NULL;

    OlmOutboundGroupSession* sessionPtr = NULL;
    size_t sessionSize = 0;

    LOGD("## createNewSessionJni(): outbound group session IN");
    sessionSize = olm_outbound_group_session_size();

    if (0 == sessionSize)
    {
        LOGE(" ## createNewSessionJni(): failure - outbound group session size = 0");
        errorMessage = "outbound group session size = 0";
    }
    else if (!(sessionPtr = (OlmOutboundGroupSession*)malloc(sessionSize)))
    {
        LOGE(" ## createNewSessionJni(): failure - outbound group session OOM");
        errorMessage = "outbound group session OOM";
    }
    else
    {
        sessionPtr = olm_outbound_group_session(sessionPtr);
        LOGD(" ## createNewSessionJni(): success - outbound group session size=%lu",static_cast<long unsigned int>(sessionSize));

        // compute random buffer
        size_t randomLength = olm_init_outbound_group_session_random_length(sessionPtr);
        uint8_t *randomBuffPtr = NULL;

        LOGW(" ## createNewSessionJni(): randomLength=%lu",static_cast<long unsigned int>(randomLength));

        if ((0 != randomLength) && !setRandomInBuffer(env, &randomBuffPtr, randomLength))
        {
            LOGE(" ## createNewSessionJni(): failure - random buffer init");
            errorMessage = "random buffer init";
        }
        else
        {
            if (0 == randomLength)
            {
                LOGW(" ## createNewSessionJni(): random buffer is not required");
            }

            size_t sessionResult = olm_init_outbound_group_session(sessionPtr, randomBuffPtr, randomLength);

            if (sessionResult == olm_error()) {
                errorMessage = (const char *)olm_outbound_group_session_last_error(sessionPtr);
                LOGE(" ## createNewSessionJni(): failure - init outbound session creation  Msg=%s", errorMessage);
            }
            else
            {
                LOGD(" ## createNewSessionJni(): success - result=%lu", static_cast<long unsigned int>(sessionResult));
            }

            // clear the random buffer
            memset(randomBuffPtr, 0, randomLength);
            free(randomBuffPtr);
        }
    }

    if (errorMessage)
    {
        if (sessionPtr)
        {
            olm_clear_outbound_group_session(sessionPtr);
            free(sessionPtr);
        }

        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return (jlong)(intptr_t)sessionPtr;
}

/**
 * Return the session identifier.
 * An exception is thrown if the operation fails.
 * @return the session identifier
 */
JNIEXPORT jbyteArray OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(sessionIdentifierJni)(JNIEnv *env, jobject thiz)
{
    LOGD("## sessionIdentifierJni(): outbound group session IN");

    const char* errorMessage = NULL;
    OlmOutboundGroupSession *sessionPtr = (OlmOutboundGroupSession*)getOutboundGroupSessionInstanceId(env,thiz);
    jbyteArray returnValue = 0;
    
    if (!sessionPtr)
    {
        LOGE(" ## sessionIdentifierJni(): failure - invalid outbound group session instance");
        errorMessage = "invalid outbound group session instance";
    }
    else
    {
        // get the size to alloc
        size_t lengthSessionId = olm_outbound_group_session_id_length(sessionPtr);
        LOGD(" ## sessionIdentifierJni(): outbound group session lengthSessionId=%lu",static_cast<long unsigned int>(lengthSessionId));

        uint8_t *sessionIdPtr =  (uint8_t*)malloc(lengthSessionId*sizeof(uint8_t));

        if (!sessionIdPtr)
        {
           LOGE(" ## sessionIdentifierJni(): failure - outbound identifier allocation OOM");
           errorMessage = "outbound identifier allocation OOM";
        }
        else
        {
            size_t result = olm_outbound_group_session_id(sessionPtr, sessionIdPtr, lengthSessionId);

            if (result == olm_error())
            {
                errorMessage = reinterpret_cast<const char*>(olm_outbound_group_session_last_error(sessionPtr));
                LOGE(" ## sessionIdentifierJni(): failure - outbound group session identifier failure Msg=%s", errorMessage);
            }
            else
            {
                returnValue = env->NewByteArray(result);
                env->SetByteArrayRegion(returnValue, 0 , result, (jbyte*)sessionIdPtr);

                LOGD(" ## sessionIdentifierJni(): success - outbound group session identifier result=%lu sessionId= %.*s",static_cast<long unsigned int>(result), static_cast<int>(result), reinterpret_cast<char*>(sessionIdPtr));
            }

            // free alloc
            free(sessionIdPtr);
        }
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return returnValue;
}


/**
 * Get the current message index for this session.<br>
 * Each message is sent with an increasing index, this
 * method returns the index for the next message.
 * An exception is thrown if the operation fails.
 * @return current session index
 */
JNIEXPORT jint OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(messageIndexJni)(JNIEnv *env, jobject thiz)
{
    OlmOutboundGroupSession *sessionPtr = NULL;
    jint indexRetValue = 0;

    LOGD("## messageIndexJni(): IN");

    if (!(sessionPtr = (OlmOutboundGroupSession*)getOutboundGroupSessionInstanceId(env,thiz)))
    {
        LOGE(" ## messageIndexJni(): failure - invalid outbound group session instance");
    }
    else
    {
        indexRetValue = static_cast<jint>(olm_outbound_group_session_message_index(sessionPtr));
    }

    LOGD(" ## messageIndexJni(): success - index=%d",indexRetValue);

    return indexRetValue;
}

/**
 * Return the session key.
 * An exception is thrown if the operation fails.
 * @return the session key
 */
JNIEXPORT jbyteArray OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(sessionKeyJni)(JNIEnv *env, jobject thiz)
{
    LOGD("## sessionKeyJni(): outbound group session IN");

    const char* errorMessage = NULL;
    OlmOutboundGroupSession *sessionPtr = (OlmOutboundGroupSession*)getOutboundGroupSessionInstanceId(env,thiz);
    jbyteArray returnValue = 0;

    if (!sessionPtr)
    {
        LOGE(" ## sessionKeyJni(): failure - invalid outbound group session instance");
        errorMessage = "invalid outbound group session instance";
    }
    else
    {
        // get the size to alloc
        size_t sessionKeyLength = olm_outbound_group_session_key_length(sessionPtr);
        LOGD(" ## sessionKeyJni(): sessionKeyLength=%lu",static_cast<long unsigned int>(sessionKeyLength));

        uint8_t *sessionKeyPtr = (uint8_t*)malloc(sessionKeyLength*sizeof(uint8_t));

        if (!sessionKeyPtr)
        {
           LOGE(" ## sessionKeyJni(): failure - session key allocation OOM");
           errorMessage = "session key allocation OOM";
        }
        else
        {
            size_t result = olm_outbound_group_session_key(sessionPtr, sessionKeyPtr, sessionKeyLength);

            if (result == olm_error())
            {
                errorMessage = (const char *)olm_outbound_group_session_last_error(sessionPtr);
                LOGE(" ## sessionKeyJni(): failure - session key failure Msg=%s", errorMessage);
            }
            else
            {
                LOGD(" ## sessionKeyJni(): success - outbound group session key result=%lu sessionKey=%.*s",static_cast<long unsigned int>(result), static_cast<int>(result), reinterpret_cast<char*>(sessionKeyPtr));

                returnValue = env->NewByteArray(result);
                env->SetByteArrayRegion(returnValue, 0 , result, (jbyte*)sessionKeyPtr);
            }

            // free alloc
            free(sessionKeyPtr);
        }
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return returnValue;
}

/**
 * Encrypt a bytes buffer messages.
 * An exception is thrown if the operation fails.
 * @param aClearMsgBuffer  the message to encode
 * @return the encoded message
 */
JNIEXPORT jbyteArray OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(encryptMessageJni)(JNIEnv *env, jobject thiz, jbyteArray aClearMsgBuffer)
{
    LOGD("## encryptMessageJni(): IN");

    const char* errorMessage = NULL;
    jbyteArray encryptedMsgRet = 0;

    OlmOutboundGroupSession *sessionPtr = NULL;
    jbyte* clearMsgPtr = NULL;
    jboolean clearMsgIsCopied = JNI_FALSE;

    if (!(sessionPtr = (OlmOutboundGroupSession*)getOutboundGroupSessionInstanceId(env,thiz)))
    {
        LOGE(" ## encryptMessageJni(): failure - invalid outbound group session ptr=NULL");
        errorMessage = "invalid outbound group session ptr=NULL";
    }
    else if (!aClearMsgBuffer)
    {
        LOGE(" ## encryptMessageJni(): failure - invalid clear message");
        errorMessage = "invalid clear message";
    }
    else if (!(clearMsgPtr = env->GetByteArrayElements(aClearMsgBuffer, &clearMsgIsCopied)))
    {
        LOGE(" ## encryptMessageJni(): failure - clear message JNI allocation OOM");
        errorMessage = "clear message JNI allocation OOM";
    }
    else
    {
        // get clear message length
        size_t clearMsgLength = (size_t)env->GetArrayLength(aClearMsgBuffer);
        LOGD(" ## encryptMessageJni(): clearMsgLength=%lu",static_cast<long unsigned int>(clearMsgLength));

        // compute max encrypted length
        size_t encryptedMsgLength = olm_group_encrypt_message_length(sessionPtr,clearMsgLength);
        uint8_t *encryptedMsgPtr = (uint8_t*)malloc(encryptedMsgLength*sizeof(uint8_t));

        if (!encryptedMsgPtr)
        {
            LOGE(" ## encryptMessageJni(): failure - encryptedMsgPtr buffer OOM");
            errorMessage = "encryptedMsgPtr buffer OOM";
        }
        else
        {
            LOGD(" ## encryptMessageJni(): estimated encryptedMsgLength=%lu",static_cast<long unsigned int>(encryptedMsgLength));

            size_t encryptedLength = olm_group_encrypt(sessionPtr,
                                                       (uint8_t*)clearMsgPtr,
                                                       clearMsgLength,
                                                       encryptedMsgPtr,
                                                       encryptedMsgLength);


            if (encryptedLength == olm_error())
            {
                errorMessage = olm_outbound_group_session_last_error(sessionPtr);
                LOGE(" ## encryptMessageJni(): failure - olm_group_decrypt_max_plaintext_length Msg=%s", errorMessage);
            }
            else
            {
                LOGD(" ## encryptMessageJni(): encrypted returnedLg=%lu plainTextMsgPtr=%.*s",static_cast<long unsigned int>(encryptedLength), static_cast<int>(encryptedLength), reinterpret_cast<char*>(encryptedMsgPtr));

                encryptedMsgRet = env->NewByteArray(encryptedLength);
                env->SetByteArrayRegion(encryptedMsgRet, 0 , encryptedLength, (jbyte*)encryptedMsgPtr);
            }

            free(encryptedMsgPtr);
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
 * Serialize and encrypt session instance into a base64 string.<br>
 * An exception is thrown if the operation fails.
 * @param aKey key used to encrypt the serialized session data
 * @return a base64 string if operation succeed, null otherwise
 **/
JNIEXPORT jbyteArray OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(serializeJni)(JNIEnv *env, jobject thiz, jbyteArray aKeyBuffer)
{
    const char* errorMessage = NULL;
    jbyteArray returnValue = 0;

    jbyte* keyPtr = NULL;
    jboolean keyWasCopied = JNI_FALSE;
    OlmOutboundGroupSession* sessionPtr = NULL;

    LOGD("## outbound group session serializeJni(): IN");

    if (!(sessionPtr = (OlmOutboundGroupSession*)getOutboundGroupSessionInstanceId(env,thiz)))
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
        errorMessage = "keyPtr JNI allocation OOM";
    }
    else
    {
        size_t pickledLength = olm_pickle_outbound_group_session_length(sessionPtr);
        size_t keyLength = (size_t)env->GetArrayLength(aKeyBuffer);
        LOGD(" ## serializeJni(): pickledLength=%lu keyLength=%lu",static_cast<long unsigned int>(pickledLength), static_cast<long unsigned int>(keyLength));

        void *pickledPtr = malloc(pickledLength*sizeof(uint8_t));

        if(!pickledPtr)
        {
            LOGE(" ## serializeJni(): failure - pickledPtr buffer OOM");
            errorMessage = "pickledPtr buffer OOM";
        }
        else
        {
            size_t result = olm_pickle_outbound_group_session(sessionPtr,
                                                             (void const *)keyPtr,
                                                              keyLength,
                                                              (void*)pickledPtr,
                                                              pickledLength);
            if (result == olm_error())
            {
                errorMessage = olm_outbound_group_session_last_error(sessionPtr);
                LOGE(" ## serializeJni(): failure - olm_pickle_outbound_group_session() Msg=%s", errorMessage);
            }
            else
            {
                LOGD(" ## serializeJni(): success - result=%lu pickled=%.*s", static_cast<long unsigned int>(result), static_cast<int>(result), static_cast<char*>(pickledPtr));

                returnValue = env->NewByteArray(pickledLength);
                env->SetByteArrayRegion(returnValue, 0 , pickledLength, (jbyte*)pickledPtr);
            }
        }

        free(pickledPtr);
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
JNIEXPORT jlong OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(deserializeJni)(JNIEnv *env, jobject thiz, jbyteArray aSerializedDataBuffer, jbyteArray aKeyBuffer)
{
    const char* errorMessage = NULL;
    size_t sessionSize = olm_outbound_group_session_size();
    OlmOutboundGroupSession* sessionPtr = NULL;

    jbyte* keyPtr = NULL;
    jboolean keyWasCopied = JNI_FALSE;
    jbyte* pickledPtr = NULL;

    LOGD("## deserializeJni(): IN");

    if (!sessionSize)
    {
        LOGE(" ## deserializeJni(): failure - outbound group session size = 0");
        errorMessage = "outbound group session size = 0";
    }
    else if (!(sessionPtr = (OlmOutboundGroupSession*)malloc(sessionSize)))
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
        errorMessage = "invalid serialized data";
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
        sessionPtr = olm_outbound_group_session(sessionPtr);
        size_t pickledLength = (size_t)env->GetArrayLength(aSerializedDataBuffer);
        size_t keyLength = (size_t)env->GetArrayLength(aKeyBuffer);
        LOGD(" ## deserializeJni(): pickledLength=%lu keyLength=%lu",static_cast<long unsigned int>(pickledLength), static_cast<long unsigned int>(keyLength));
        LOGD(" ## deserializeJni(): pickled=%.*s", static_cast<int>(pickledLength), (char const *)pickledPtr);

        size_t result = olm_unpickle_outbound_group_session(sessionPtr,
                                                            (void const *)keyPtr,
                                                            keyLength,
                                                            (void*)pickledPtr,
                                                            pickledLength);
        if (result == olm_error())
        {
            errorMessage = olm_outbound_group_session_last_error(sessionPtr);
            LOGE(" ## deserializeJni(): failure - olm_unpickle_outbound_group_session() Msg=%s", errorMessage);
        }
        else
        {
            LOGD(" ## deserializeJni(): success - result=%lu ", static_cast<long unsigned int>(result));
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
            olm_clear_outbound_group_session(sessionPtr);
            free(sessionPtr);
        }
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return (jlong)(intptr_t)sessionPtr;
}

