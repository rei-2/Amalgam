#pragma once

#define MAKEID(d, c, b, a) (((int)(a) << 24) | ((int)(b) << 16) | ((int)(c) << 8) | ((int)(d)))
#define STRING_MATCHES_ID(p, id) ( (*((int *)(p)) == (id) ) ? true : false )
#define ID_TO_STRING(id, p) ((p)[3] = (((id)>>24) & 0xFF), (p)[2] = (((id)>>16) & 0xFF), (p)[1] = (((id)>>8) & 0xFF), (p)[0] = (((id)>>0) & 0xFF))
#define SETBITS(iBitVector, bits) ((iBitVector) |= (bits))
#define CLEARBITS(iBitVector, bits) ((iBitVector) &= ~(bits))
#define FBitSet(iBitVector, bits) ((iBitVector) & (bits))