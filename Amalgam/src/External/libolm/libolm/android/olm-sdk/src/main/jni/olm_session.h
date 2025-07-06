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

#ifndef _OLMSESSION_H
#define _OLMSESSION_H

#include "olm_jni.h"
#include "olm/olm.h"

#define OLM_SESSION_FUNC_DEF(func_name) FUNC_DEF(OlmSession,func_name)

#ifdef __cplusplus
extern "C" {
#endif

// session creation/destruction
JNIEXPORT void OLM_SESSION_FUNC_DEF(releaseSessionJni)(JNIEnv *env, jobject thiz);
JNIEXPORT jlong OLM_SESSION_FUNC_DEF(createNewSessionJni)(JNIEnv *env, jobject thiz);

// outbound session
JNIEXPORT void OLM_SESSION_FUNC_DEF(initOutboundSessionJni)(JNIEnv *env, jobject thiz, jlong aOlmAccountId, jbyteArray aTheirIdentityKey, jbyteArray aTheirOneTimeKey);

// inbound sessions: establishment based on PRE KEY message
JNIEXPORT void OLM_SESSION_FUNC_DEF(initInboundSessionJni)(JNIEnv *env, jobject thiz, jlong aOlmAccountId, jbyteArray aOneTimeKeyMsg);
JNIEXPORT void OLM_SESSION_FUNC_DEF(initInboundSessionFromIdKeyJni)(JNIEnv *env, jobject thiz, jlong aOlmAccountId, jbyteArray aTheirIdentityKey, jbyteArray aOneTimeKeyMsg);

// match inbound sessions: based on PRE KEY message
JNIEXPORT jboolean OLM_SESSION_FUNC_DEF(matchesInboundSessionJni)(JNIEnv *env, jobject thiz, jbyteArray aOneTimeKeyMsg);
JNIEXPORT jboolean OLM_SESSION_FUNC_DEF(matchesInboundSessionFromIdKeyJni)(JNIEnv *env, jobject thiz, jbyteArray aTheirIdentityKey, jbyteArray aOneTimeKeyMsg);

// encrypt/decrypt
JNIEXPORT jbyteArray OLM_SESSION_FUNC_DEF(encryptMessageJni)(JNIEnv *env, jobject thiz, jbyteArray aClearMsg, jobject aEncryptedMsg);
JNIEXPORT jbyteArray OLM_SESSION_FUNC_DEF(decryptMessageJni)(JNIEnv *env, jobject thiz, jobject aEncryptedMsg);

JNIEXPORT jbyteArray OLM_SESSION_FUNC_DEF(getSessionIdentifierJni)(JNIEnv *env, jobject thiz);
JNIEXPORT jbyteArray OLM_SESSION_FUNC_DEF(olmSessionDescribeJni)(JNIEnv *env, jobject thiz);

// serialization
JNIEXPORT jbyteArray OLM_SESSION_FUNC_DEF(serializeJni)(JNIEnv *env, jobject thiz, jbyteArray aKey);
JNIEXPORT jlong OLM_SESSION_FUNC_DEF(deserializeJni)(JNIEnv *env, jobject thiz, jbyteArray aSerializedData, jbyteArray aKey);

#ifdef __cplusplus
}
#endif

#endif
