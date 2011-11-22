#ifndef _ALU_H_
#define _ALU_H_

#define ALUAPI
#define ALUAPIENTRY __cdecl

#define BUFFERSIZE 48000
#define FRACTIONBITS 14
#define FRACTIONMASK ((1L<<FRACTIONBITS)-1)
#define OUTPUTCHANNELS 2

#include "altypes.h"

#ifdef __cplusplus
extern "C" {
#endif

ALUAPI ALint	ALUAPIENTRY aluF2L(ALfloat value);
ALUAPI ALshort	ALUAPIENTRY aluF2S(ALfloat value);
ALUAPI ALvoid	ALUAPIENTRY aluCrossproduct(ALfloat *inVector1,ALfloat *inVector2,ALfloat *outVector);
ALUAPI ALfloat	ALUAPIENTRY aluDotproduct(ALfloat *inVector1,ALfloat *inVector2);
ALUAPI ALvoid	ALUAPIENTRY aluNormalize(ALfloat *inVector);
ALUAPI ALvoid	ALUAPIENTRY aluMatrixVector(ALfloat matrix[3][3],ALfloat *vector);
ALUAPI ALvoid	ALUAPIENTRY aluCalculateSourceParameters(ALuint source,ALuint channels,ALfloat *drysend,ALfloat *wetsend,ALfloat *pitch);
ALUAPI ALvoid	ALUAPIENTRY aluMixData(ALvoid *context,ALvoid *buffer,ALsizei size,ALenum format);
ALUAPI ALvoid	ALUAPIENTRY aluSetReverb(ALvoid *Reverb,ALuint Environment);
ALUAPI ALvoid	ALUAPIENTRY aluReverb(ALvoid *Reverb,ALfloat Buffer[][2],ALsizei BufferSize);

#ifdef __cplusplus
}
#endif

#endif

