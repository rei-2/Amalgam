LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)


LOCAL_MODULE := olm

SRC_ROOT_DIR := ../../../../..

include $(LOCAL_PATH)/$(SRC_ROOT_DIR)/common.mk
OLM_VERSION := $(MAJOR).$(MINOR).$(PATCH)

$(info LOCAL_PATH=$(LOCAL_PATH))
$(info SRC_ROOT_DIR=$(SRC_ROOT_DIR))
$(info OLM_VERSION=$(OLM_VERSION))

LOCAL_CPPFLAGS+= -std=c++11 -Wall
LOCAL_CONLYFLAGS+= -std=c99
LOCAL_CFLAGS+= -DOLMLIB_VERSION_MAJOR=$(MAJOR) \
-DOLMLIB_VERSION_MINOR=$(MINOR) \
-DOLMLIB_VERSION_PATCH=$(PATCH)

#LOCAL_CFLAGS+= -DNDK_DEBUG

LOCAL_CFLAGS+=-fstack-protector-all -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Wall
LOCAL_LDFLAGS=-z relro -z now

LOCAL_C_INCLUDES+= $(LOCAL_PATH)/$(SRC_ROOT_DIR)/include/ \
$(LOCAL_PATH)/$(SRC_ROOT_DIR)/lib

$(info LOCAL_C_INCLUDES=$(LOCAL_C_INCLUDES))

LOCAL_SRC_FILES := $(SRC_ROOT_DIR)/src/account.cpp \
$(SRC_ROOT_DIR)/src/base64.cpp \
$(SRC_ROOT_DIR)/src/cipher.cpp \
$(SRC_ROOT_DIR)/src/crypto.cpp \
$(SRC_ROOT_DIR)/src/memory.cpp \
$(SRC_ROOT_DIR)/src/message.cpp \
$(SRC_ROOT_DIR)/src/olm.cpp \
$(SRC_ROOT_DIR)/src/pickle.cpp \
$(SRC_ROOT_DIR)/src/ratchet.cpp \
$(SRC_ROOT_DIR)/src/session.cpp \
$(SRC_ROOT_DIR)/src/utility.cpp \
$(SRC_ROOT_DIR)/src/pk.cpp \
$(SRC_ROOT_DIR)/src/sas.c \
$(SRC_ROOT_DIR)/src/ed25519.c \
$(SRC_ROOT_DIR)/src/error.c \
$(SRC_ROOT_DIR)/src/inbound_group_session.c \
$(SRC_ROOT_DIR)/src/megolm.c \
$(SRC_ROOT_DIR)/src/outbound_group_session.c \
$(SRC_ROOT_DIR)/src/pickle_encoding.c \
$(SRC_ROOT_DIR)/lib/crypto-algorithms/sha256.c \
$(SRC_ROOT_DIR)/lib/crypto-algorithms/aes.c \
$(SRC_ROOT_DIR)/lib/curve25519-donna/curve25519-donna.c \
olm_account.cpp \
olm_session.cpp \
olm_jni_helper.cpp \
olm_inbound_group_session.cpp \
olm_outbound_group_session.cpp \
olm_utility.cpp \
olm_manager.cpp \
olm_pk.cpp \
olm_sas.cpp

LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)

