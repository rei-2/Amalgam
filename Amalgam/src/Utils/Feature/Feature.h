#pragma once

#define ADD_FEATURE_CUSTOM(type, name, scope) namespace scope { inline type name; }
#define ADD_FEATURE(type, name) ADD_FEATURE_CUSTOM(type, name, F)