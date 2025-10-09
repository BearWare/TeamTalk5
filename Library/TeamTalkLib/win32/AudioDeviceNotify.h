#if !defined(AUDIODEVICENOTIFY_H)
#define AUDIODEVICENOTIFY_H

#include <ace/ace_wchar.h>

#include <functional>

enum AudioDevEvent
{
    AUDIODEVICE_ADD,
    AUDIODEVICE_REMOVE,
    AUDIODEVICE_UNPLUGGED,
    AUDIODEVICE_NEW_DEFAULT_INPUT,
    AUDIODEVICE_NEW_DEFAULT_OUTPUT,
    AUDIODEVICE_NEW_DEFAULT_INPUT_COMDEVICE,
    AUDIODEVICE_NEW_DEFAULT_OUTPUT_COMDEVICE,
};

typedef std::function< void(AudioDevEvent event, const LPCWSTR& name, const LPCWSTR& id) > audio_device_change_callback_t;

void RegisterAudioDeviceChange(void* owner, audio_device_change_callback_t cb, bool enable);

#endif
