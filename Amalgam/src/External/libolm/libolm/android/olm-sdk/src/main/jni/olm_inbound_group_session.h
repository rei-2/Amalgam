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

#ifndef _OLMINBOUND_GROUP_SESSION_H
#define _OLMINBOUND_GROUP_SESSION_H

#include "olm_jni.h"
#include "olm/olm.h"
#include "olm/inbound_group_session.h"

#define OLM_INBOUND_GROUP_SESSION_FUNC_DEF(func_name) FUNC_DEF(OlmInboundGroupSession,func_name)

#ifdef __cplusplus
extern "C" {
#endif

// session creation/destruction
JNIEXPORT void OLM_INBOUND_GROUP_SESSION_FUNC_DEF(releaseSessionJni)(JNIEnv *env, jobject thiz);
JNIEXPORT jlong OLM_INBOUND_GROUP_SESSION_FUNC_DEF(createNewSessionJni)(JNIEnv *env, jobject thiz, jbyteArray aSessionKeyBuffer, jboolean isImported);

JNIEXPORT jbyteArray OLM_INBOUND_GROUP_SESSION_FUNC_DEF(sessionIdentifierJni)(JNIEnv *env, jobject thiz);
JNIEXPORT jbyteArray OLM_INBOUND_GROUP_SESSION_FUNC_DEF(decryptMessageJni)(JNIEnv *env, jobject thiz, jbyteArray aEncryptedMsg, jobject aDecryptIndex);

JNIEXPORT jlong OLM_INBOUND_GROUP_SESSION_FUNC_DEF(firstKnownIndexJni)(JNIEnv *env, jobject thiz);
JNIEXPORT jboolean OLM_INBOUND_GROUP_SESSION_FUNC_DEF(isVerifiedJni)(JNIEnv *env, jobject thiz);

JNIEXPORT jbyteArray OLM_INBOUND_GROUP_SESSION_FUNC_DEF(exportJni)(JNIEnv *env, jobject thiz, jlong messageIndex);

// serialization
JNIEXPORT jbyteArray OLM_INBOUND_GROUP_SESSION_FUNC_DEF(serializeJni)(JNIEnv *env, jobject thiz, jbyteArray aKey);
JNIEXPORT jlong OLM_INBOUND_GROUP_SESSION_FUNC_DEF(deserializeJni)(JNIEnv *env, jobject thiz, jbyteArray aSerializedData, jbyteArray aKey);

#ifdef __cplusplus
}
#endif

#endif
