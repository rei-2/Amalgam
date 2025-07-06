/*
 * Copyright 2016 OpenMarket Ltd
 * Copyright 2016,2018,2019 Vector Creations Ltd
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

#include "olm_jni.h"

// constant strings
namespace AndroidOlmSdk
{
    static const char *CLASS_OLM_INBOUND_GROUP_SESSION = "org/matrix/olm/OlmInboundGroupSession";
    static const char *CLASS_OLM_OUTBOUND_GROUP_SESSION = "org/matrix/olm/OlmOutboundGroupSession";
    static const char *CLASS_OLM_SESSION = "org/matrix/olm/OlmSession";
    static const char *CLASS_OLM_ACCOUNT = "org/matrix/olm/OlmAccount";
    static const char *CLASS_OLM_UTILITY = "org/matrix/olm/OlmUtility";
    static const char *CLASS_OLM_PK_ENCRYPTION = "org/matrix/olm/OlmPkEncryption";
    static const char *CLASS_OLM_PK_DECRYPTION = "org/matrix/olm/OlmPkDecryption";
    static const char *CLASS_OLM_PK_SIGNING = "org/matrix/olm/OlmPkSigning";
    static const char *CLASS_OLM_SAS = "org/matrix/olm/OlmSAS";
}
