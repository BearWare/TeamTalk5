/**
 *  Product:        Tolk
 *  File:           ScreenReaderDriverZDSR.h
 *  Description:    Driver for the Zhengdu Screen Reader (ZDSR).
 *  Copyright:      (c) 2024, BearWare.dk
 *  License:        LGPLv3
 */

#ifndef _SCREEN_READER_DRIVER_ZDSR_H_
#define _SCREEN_READER_DRIVER_ZDSR_H_

#include "zdsr.h"
#include "ScreenReaderDriver.h"

class ScreenReaderDriverZDSR : public ScreenReaderDriver {
public:
  ScreenReaderDriverZDSR();
  ~ScreenReaderDriverZDSR();

public:
  bool Speak(const wchar_t *str, bool interrupt);
  bool Braille(const wchar_t *str);
  bool IsSpeaking();
  bool Silence();
  bool IsActive();
  bool Output(const wchar_t *str, bool interrupt);

private:
  void Initialize();
  void Finalize();
  bool IsRunning();

private:
  bool initialized;
  bool libraryLoaded;
};

#endif // _SCREEN_READER_DRIVER_ZDSR_H_
