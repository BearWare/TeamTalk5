#include "AudioDeviceNotify.h"

#include <myace/MyACE.h>
#include <assert.h>
#include <mmdeviceapi.h>
#include <atlbase.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <mutex>
#include <map>

using namespace std::placeholders;

void AudioChangeCallback(audio_device_change_callback_t cb, AudioDevEvent event, LPCWSTR pwstrDeviceId)
{
    CComPtr<IMMDeviceEnumerator> pEnumerator;
    CComPtr<IMMDevice> pDevice;
    CComPtr<IPropertyStore> pProps;
    HRESULT hr = S_OK;

    // Get enumerator for audio endpoint devices.
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER,
        __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);

    if (SUCCEEDED(hr))
    {
        hr = pEnumerator->GetDevice(pwstrDeviceId, &pDevice);
    }
    if (SUCCEEDED(hr))
    {
        hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
    }
    if (SUCCEEDED(hr))
    {
        // Get the endpoint device's friendly-name property.
        PROPVARIANT varString;
        PropVariantInit(&varString);
        hr = pProps->GetValue(PKEY_Device_FriendlyName, &varString);
        if (SUCCEEDED(hr))
            cb(event, varString.pwszVal, pwstrDeviceId);
        PropVariantClear(&varString);
    }
}

class CMMNotificationClient : public IMMNotificationClient
{
    LONG m_nRefs;

    audio_device_change_callback_t m_callback;

public:
    CMMNotificationClient(audio_device_change_callback_t cb)
        : m_nRefs(0), m_callback(cb) { }

    ~CMMNotificationClient() { }

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return InterlockedIncrement(&m_nRefs);
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        ULONG ulRef = InterlockedDecrement(&m_nRefs);
        if (0 == ulRef)
        {
            delete this;
        }
        return ulRef;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID** ppvInterface)
    {
        if (IID_IUnknown == riid)
        {
            AddRef();
            *ppvInterface = (IUnknown*)this;
        }
        else if (__uuidof(IMMNotificationClient) == riid)
        {
            AddRef();
            *ppvInterface = (IMMNotificationClient*)this;
        }
        else
        {
            *ppvInterface = NULL;
            return E_NOINTERFACE;
        }
        return S_OK;
    }

    // Callback methods for device-event notifications.

    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId)
    {
        switch (flow)
        {
        case eRender:
            if (m_callback)
                AudioChangeCallback(m_callback, AUDIODEVICE_NEW_DEFAULT_OUTPUT, pwstrDeviceId);
            break;
        case eCapture:
            if (m_callback)
                AudioChangeCallback(m_callback, AUDIODEVICE_NEW_DEFAULT_INPUT, pwstrDeviceId);
            break;
        }

        switch (role)
        {
        case eConsole:
            break;
        case eMultimedia:
            break;
        case eCommunications:
            if (m_callback && flow == eCapture)
                AudioChangeCallback(m_callback, AUDIODEVICE_NEW_DEFAULT_INPUT_COMDEVICE, pwstrDeviceId);
            if (m_callback && flow == eRender)
                AudioChangeCallback(m_callback, AUDIODEVICE_NEW_DEFAULT_OUTPUT_COMDEVICE, pwstrDeviceId);
            break;
        }

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId)
    {
        MYTRACE(ACE_TEXT("IMMNotificationClient::OnDeviceAdded(%s)\n"), pwstrDeviceId);
        return S_OK;
    };

    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId)
    {
        MYTRACE(ACE_TEXT("IMMNotificationClient::OnDeviceRemoved(%s)\n"), pwstrDeviceId);
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
    {
        /*
        Constant/value	Description

        DEVICE_STATE_ACTIVE
        0x00000001
        The audio endpoint device is active. That is, the audio
        adapter that connects to the endpoint device is present and
        enabled. In addition, if the endpoint device plugs into a jack
        on the adapter, then the endpoint device is plugged in.

        DEVICE_STATE_DISABLED
        0x00000002
        The audio endpoint device is disabled. The user has disabled
        the device in the Windows multimedia control panel,
        Mmsys.cpl. For more information, see Remarks.

        DEVICE_STATE_NOTPRESENT
        0x00000004
        The audio endpoint device is not present because the audio
        adapter that connects to the endpoint device has been removed
        from the system, or the user has disabled the adapter device
        in Device Manager.

        DEVICE_STATE_UNPLUGGED
        0x00000008
        The audio endpoint device is unplugged. The audio adapter that
        contains the jack for the endpoint device is present and
        enabled, but the endpoint device is not plugged into the
        jack. Only a device with jack-presence detection can be in
        this state. For more information about jack-presence
        detection, see Audio Endpoint Devices.

        DEVICE_STATEMASK_ALL
        0x0000000F
        */

        switch (dwNewState)
        {
        case DEVICE_STATE_ACTIVE:
            if (m_callback)
                AudioChangeCallback(m_callback, AUDIODEVICE_ADD, pwstrDeviceId);
            break;
        case DEVICE_STATE_DISABLED:
            break;
        case DEVICE_STATE_NOTPRESENT:
            if (m_callback)
                AudioChangeCallback(m_callback, AUDIODEVICE_REMOVE, pwstrDeviceId);
            break;
        case DEVICE_STATE_UNPLUGGED:
            if (m_callback)
                AudioChangeCallback(m_callback, AUDIODEVICE_UNPLUGGED, pwstrDeviceId);
            break;
        }
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
    {
        return S_OK;
    }
};


void RegisterAudioDeviceChange(void* owner, audio_device_change_callback_t cb, bool enable)
{
    static class AudioDeviceNotifyInit {
        CMMNotificationClient m_audnotify;
        CComPtr<IMMDeviceEnumerator> m_pEnumerator;

        std::map<void*, audio_device_change_callback_t > m_callbacks;
        std::mutex m_mutex;

    public:
        AudioDeviceNotifyInit() :
            m_audnotify(std::bind(&AudioDeviceNotifyInit::AudioDeviceChange, this, _1, _2, _3))
        {
            HRESULT hr = S_OK;
            hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER,
                __uuidof(IMMDeviceEnumerator), (void**)&m_pEnumerator);
            if (SUCCEEDED(hr))
            {
                hr = m_pEnumerator->RegisterEndpointNotificationCallback(&m_audnotify);
                MYTRACE_COND(!SUCCEEDED(hr), ACE_TEXT("Failed to register audio device notification callback\n"));
            }
        }

        ~AudioDeviceNotifyInit()
        {
            if (m_pEnumerator)
            {
                HRESULT hr = m_pEnumerator->UnregisterEndpointNotificationCallback(&m_audnotify);
                MYTRACE_COND(!SUCCEEDED(hr), ACE_TEXT("Failed to unregister audio device notification callback\n"));
            }
        }

        void SetupCallback(void* owner, audio_device_change_callback_t cb, bool enable)
        {
            std::lock_guard<std::mutex> g(m_mutex);
            if (enable)
                m_callbacks[owner] = cb;
            else
                m_callbacks.erase(owner);
        }

        void AudioDeviceChange(AudioDevEvent event, const LPCWSTR& name, const LPCWSTR& id)
        {
            std::lock_guard<std::mutex> g(m_mutex);
            for (auto& cb : m_callbacks)
                cb.second(event, name, id);
        }

    } adninit;

    adninit.SetupCallback(owner, cb, enable);
}
