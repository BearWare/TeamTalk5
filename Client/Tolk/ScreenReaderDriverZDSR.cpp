/**
 *  Product:        Tolk
 *  File:           ScreenReaderDriverZDSR.cpp
 *  Description:    Driver for the Zhengdu Screen Reader (ZDSR).
 *  Copyright:      (c) 2024, BearWare.dk
 *  License:        LGPLv3
 */

#include "ScreenReaderDriverZDSR.h"

// Global function pointers for ZDSR API
ZDSR_IsRunning_t ZDSR_IsRunning = NULL;
ZDSR_Speak_t ZDSR_Speak = NULL;
ZDSR_Braille_t ZDSR_Braille = NULL;
ZDSR_StopSpeech_t ZDSR_StopSpeech = NULL;
ZDSR_IsSpeaking_t ZDSR_IsSpeaking = NULL;
ZDSR_Output_t ZDSR_Output = NULL;

static HMODULE g_zdsrModule = NULL;

BOOL LoadZDSRLibrary() {
  if (g_zdsrModule) return TRUE;
  
  // Try to load the ZDSR API DLL
  g_zdsrModule = LoadLibrary(L"zdsrapi.dll");
  if (!g_zdsrModule) {
    // Try loading from common installation paths
    g_zdsrModule = LoadLibrary(L"C:\\Program Files\\ZDSR\\zdsrapi.dll");
  }
  if (!g_zdsrModule) {
    g_zdsrModule = LoadLibrary(L"C:\\Program Files (x86)\\ZDSR\\zdsrapi.dll");
  }
  
  if (!g_zdsrModule) return FALSE;
  
  // Load function pointers
  ZDSR_IsRunning = (ZDSR_IsRunning_t)GetProcAddress(g_zdsrModule, "ZDSR_IsRunning");
  ZDSR_Speak = (ZDSR_Speak_t)GetProcAddress(g_zdsrModule, "ZDSR_Speak");
  ZDSR_Braille = (ZDSR_Braille_t)GetProcAddress(g_zdsrModule, "ZDSR_Braille");
  ZDSR_StopSpeech = (ZDSR_StopSpeech_t)GetProcAddress(g_zdsrModule, "ZDSR_StopSpeech");
  ZDSR_IsSpeaking = (ZDSR_IsSpeaking_t)GetProcAddress(g_zdsrModule, "ZDSR_IsSpeaking");
  ZDSR_Output = (ZDSR_Output_t)GetProcAddress(g_zdsrModule, "ZDSR_Output");
  
  // Check if essential functions are loaded
  if (!ZDSR_IsRunning || !ZDSR_Speak || !ZDSR_StopSpeech) {
    UnloadZDSRLibrary();
    return FALSE;
  }
  
  return TRUE;
}

void UnloadZDSRLibrary() {
  if (g_zdsrModule) {
    FreeLibrary(g_zdsrModule);
    g_zdsrModule = NULL;
  }
  
  ZDSR_IsRunning = NULL;
  ZDSR_Speak = NULL;
  ZDSR_Braille = NULL;
  ZDSR_StopSpeech = NULL;
  ZDSR_IsSpeaking = NULL;
  ZDSR_Output = NULL;
}

BOOL IsZDSRLoaded() {
  return (g_zdsrModule != NULL);
}

ScreenReaderDriverZDSR::ScreenReaderDriverZDSR() :
  ScreenReaderDriver(L"ZDSR", true, true),
  initialized(false),
  libraryLoaded(false)
{
  if (IsRunning()) Initialize();
}

ScreenReaderDriverZDSR::~ScreenReaderDriverZDSR() {
  Finalize();
}

bool ScreenReaderDriverZDSR::Speak(const wchar_t *str, bool interrupt) {
  if (!initialized || !ZDSR_Speak) return false;
  return ZDSR_Speak(str, interrupt ? TRUE : FALSE) == TRUE;
}

bool ScreenReaderDriverZDSR::Braille(const wchar_t *str) {
  if (!initialized || !ZDSR_Braille) return false;
  return ZDSR_Braille(str) == TRUE;
}

bool ScreenReaderDriverZDSR::IsSpeaking() {
  if (!initialized || !ZDSR_IsSpeaking) return false;
  return ZDSR_IsSpeaking() == TRUE;
}

bool ScreenReaderDriverZDSR::Silence() {
  if (!initialized || !ZDSR_StopSpeech) return false;
  return ZDSR_StopSpeech() == TRUE;
}

bool ScreenReaderDriverZDSR::IsActive() {
  if (!IsRunning()) {
    Finalize();
    return false;
  }
  if (!initialized) Initialize();
  return initialized;
}

bool ScreenReaderDriverZDSR::Output(const wchar_t *str, bool interrupt) {
  if (!initialized) return false;
  
  // If ZDSR_Output is available, use it (outputs both speech and braille)
  if (ZDSR_Output) {
    return ZDSR_Output(str, interrupt ? TRUE : FALSE) == TRUE;
  }
  
  // Otherwise, try both speech and braille separately
  bool result = false;
  if (ZDSR_Speak) {
    result = ZDSR_Speak(str, interrupt ? TRUE : FALSE) == TRUE;
  }
  if (ZDSR_Braille) {
    ZDSR_Braille(str);
  }
  return result;
}

void ScreenReaderDriverZDSR::Initialize() {
  if (initialized) return;
  
  libraryLoaded = LoadZDSRLibrary() == TRUE;
  if (libraryLoaded) {
    initialized = true;
  }
}

void ScreenReaderDriverZDSR::Finalize() {
  if (initialized) {
    initialized = false;
  }
  if (libraryLoaded) {
    UnloadZDSRLibrary();
    libraryLoaded = false;
  }
}

bool ScreenReaderDriverZDSR::IsRunning() {
  // First check if the library can be loaded
  if (!IsZDSRLoaded()) {
    if (!LoadZDSRLibrary()) return false;
  }
  
  // Check if ZDSR is actually running
  if (ZDSR_IsRunning) {
    return ZDSR_IsRunning() == TRUE;
  }
  
  return false;
}
