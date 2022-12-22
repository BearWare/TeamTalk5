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

    audio_device_change_callback_t m_add_callback, m_remove_callback;

public:
    CMMNotificationClient(audio_device_change_callback_t add_dev_callback, audio_device_change_callback_t rm_dev_callback)
        : m_nRefs(0), m_add_callback(add_dev_callback), m_remove_callback(rm_dev_callback) { }

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
            break;
        case eCapture:
            break;
        }

        switch (role)
        {
        case eConsole:
            break;
        case eMultimedia:
            break;
        case eCommunications:
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
        switch (dwNewState)
        {
        case DEVICE_STATE_ACTIVE:
            if (m_add_callback)
                AudioChangeCallback(m_add_callback, AUDIODEVICE_ADD, pwstrDeviceId);
            break;
        case DEVICE_STATE_DISABLED:
            break;
        case DEVICE_STATE_NOTPRESENT:
            if (m_remove_callback)
                AudioChangeCallback(m_remove_callback, AUDIODEVICE_REMOVE, pwstrDeviceId);
            break;
        case DEVICE_STATE_UNPLUGGED:
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
            m_audnotify(std::bind(&AudioDeviceNotifyInit::AudioDeviceAdded, this, _2, _3),
                std::bind(&AudioDeviceNotifyInit::AudioDeviceRemoved, this, _2, _3))
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

        void AudioDeviceAdded(const LPCWSTR& name, const LPCWSTR& id)
        {
            std::lock_guard<std::mutex> g(m_mutex);
            for (auto& cb : m_callbacks)
                cb.second(AUDIODEVICE_ADD, name, id);
        }

        void AudioDeviceRemoved(const LPCWSTR& name, const LPCWSTR& id)
        {
            std::lock_guard<std::mutex> g(m_mutex);
            for (auto& cb : m_callbacks)
                cb.second(AUDIODEVICE_REMOVE, name, id);
        }

    } adninit;

    adninit.SetupCallback(owner, cb, enable);
}
