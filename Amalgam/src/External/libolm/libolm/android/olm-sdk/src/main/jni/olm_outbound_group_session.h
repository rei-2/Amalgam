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

#ifndef _OLMOUTBOUND_GROUP_SESSION_H
#define _OLMOUTBOUND_GROUP_SESSION_H

#include "olm_jni.h"
#include "olm/olm.h"
#include "olm/outbound_group_session.h"

#define OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(func_name) FUNC_DEF(OlmOutboundGroupSession,func_name)

#ifdef __cplusplus
extern "C" {
#endif

// session creation/destruction
JNIEXPORT void OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(releaseSessionJni)(JNIEnv *env, jobject thiz);
JNIEXPORT jlong OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(createNewSessionJni)(JNIEnv *env, jobject thiz);

JNIEXPORT jbyteArray OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(sessionIdentifierJni)(JNIEnv *env, jobject thiz);
JNIEXPORT jint OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(messageIndexJni)(JNIEnv *env, jobject thiz);
JNIEXPORT jbyteArray OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(sessionKeyJni)(JNIEnv *env, jobject thiz);

JNIEXPORT jbyteArray OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(encryptMessageJni)(JNIEnv *env, jobject thiz, jbyteArray aClearMsgBuffer);

// serialization
JNIEXPORT jbyteArray OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(serializeJni)(JNIEnv *env, jobject thiz, jbyteArray aKey);
JNIEXPORT jlong OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(deserializeJni)(JNIEnv *env, jobject thiz, jbyteArray aSerializedData, jbyteArray aKey);

#ifdef __cplusplus
}
#endif

#endif
