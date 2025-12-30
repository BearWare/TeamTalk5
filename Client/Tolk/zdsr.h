/**
 *  Product:        Tolk
 *  File:           zdsr.h
 *  Description:    API for the Zhengdu Screen Reader (ZDSR).
 *  Copyright:      (c) 2024, BearWare.dk
 *  License:        LGPLv3
 *  
 *  This file provides the C++ interface for ZDSR API.
 *  Based on ZDSRAPI documentation from https://www.zdsr.com/
 */

#ifndef _ZDSR_H_
#define _ZDSR_H_

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

// ZDSR API function types
typedef BOOL (WINAPI *ZDSR_IsRunning_t)();
typedef BOOL (WINAPI *ZDSR_Speak_t)(LPCWSTR text, BOOL interrupt);
typedef BOOL (WINAPI *ZDSR_Braille_t)(LPCWSTR text);
typedef BOOL (WINAPI *ZDSR_StopSpeech_t)();
typedef BOOL (WINAPI *ZDSR_IsSpeaking_t)();
typedef BOOL (WINAPI *ZDSR_Output_t)(LPCWSTR text, BOOL interrupt);

// Function declarations for dynamic loading
extern ZDSR_IsRunning_t ZDSR_IsRunning;
extern ZDSR_Speak_t ZDSR_Speak;
extern ZDSR_Braille_t ZDSR_Braille;
extern ZDSR_StopSpeech_t ZDSR_StopSpeech;
extern ZDSR_IsSpeaking_t ZDSR_IsSpeaking;
extern ZDSR_Output_t ZDSR_Output;

// Helper functions
BOOL LoadZDSRLibrary();
void UnloadZDSRLibrary();
BOOL IsZDSRLoaded();

#ifdef __cplusplus
}
#endif

#endif // _ZDSR_H_
