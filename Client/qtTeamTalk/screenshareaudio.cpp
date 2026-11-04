// SPDX-License-Identifier: GPL-3.0-or-later

#include "screenshareaudio.h"
#include "common.h"
#include <QtGlobal>

#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>
#include <QNetworkDatagram>
#include <QNetworkInterface>
#include <QNetworkAddressEntry>
#include <QHostAddress>
#include <QTime>
#include <QUdpSocket>
#include <QtEndian>

#if defined(Q_OS_WIN32)
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <ksmedia.h>
#include <objbase.h>
#include <audioclientactivationparams.h>
#include <wrl.h>
#include <wrl/implements.h>
#pragma comment(lib, "Mmdevapi.lib")
#endif

class ScreenShareAudio::Impl
{
public:
    std::atomic<bool> running { false };
    std::thread worker;
    TTInstance* instance = nullptr;
    QString reaStreamIdentifier;
    bool useReaStream = false;
    unsigned long sharedProcessId = 0;
    bool includeSharedProcess = false;
    std::atomic<int> volume { 50 };
    std::mutex initMutex;
    std::condition_variable initCondition;
    bool initDone = false;
    bool initSuccess = false;

#if defined(Q_OS_WIN32)
    struct ComApartment
    {
        const HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        ~ComApartment() { if (SUCCEEDED(result)) CoUninitialize(); }
    };

    static float sliderGain(int value)
    {
        if (value <= 0)
            return 0.0f;
        const double reference = 82.832 * std::exp(0.0508 * 50.0) - 50.0;
        const double selected = 82.832 * std::exp(0.0508 * value) - 50.0;
        return reference > 0.0 ? float(selected / reference) : 1.0f;
    }

    struct CaptureEndpoint
    {
        IMMDevice* device = nullptr;
        IAudioClient* audioClient = nullptr;
        IAudioCaptureClient* captureClient = nullptr;
        WAVEFORMATEX* format = nullptr;

        ~CaptureEndpoint() { close(); }

        bool open(IMMDeviceEnumerator* enumerator, EDataFlow flow, ERole role,
                  bool loopback, const QString& requestedDeviceId = QString())
        {
            const HRESULT deviceResult = requestedDeviceId.isEmpty()
                ? enumerator->GetDefaultAudioEndpoint(flow, role, &device)
                : enumerator->GetDevice(
                    reinterpret_cast<LPCWSTR>(requestedDeviceId.utf16()), &device);
            if (FAILED(deviceResult) ||
                FAILED(device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr,
                                        reinterpret_cast<void**>(&audioClient))) ||
                FAILED(audioClient->GetMixFormat(&format)))
                return false;

            const DWORD flags = loopback ? AUDCLNT_STREAMFLAGS_LOOPBACK : 0;
            if (FAILED(audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, flags,
                                               0, 0, format, nullptr)) ||
                FAILED(audioClient->GetService(__uuidof(IAudioCaptureClient),
                                               reinterpret_cast<void**>(&captureClient))) ||
                FAILED(audioClient->Start()))
                return false;
            return true;
        }

        void close()
        {
            if (audioClient)
                audioClient->Stop();
            if (captureClient)
                captureClient->Release();
            if (audioClient)
                audioClient->Release();
            if (device)
                device->Release();
            if (format)
                CoTaskMemFree(format);
            captureClient = nullptr;
            audioClient = nullptr;
            device = nullptr;
            format = nullptr;
        }
    };

    class ProcessLoopbackActivator final :
        public Microsoft::WRL::RuntimeClass<
            Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
            Microsoft::WRL::FtmBase,
            IActivateAudioInterfaceCompletionHandler>
    {
    public:
        ProcessLoopbackActivator() : completed(CreateEvent(nullptr, FALSE, FALSE, nullptr)) {}
        ~ProcessLoopbackActivator()
        {
            if (audioClient)
                audioClient->Release();
            if (completed)
                CloseHandle(completed);
        }

        STDMETHOD(ActivateCompleted)(IActivateAudioInterfaceAsyncOperation* operation) override
        {
            IUnknown* activated = nullptr;
            result = operation->GetActivateResult(&activationResult, &activated);
            if (SUCCEEDED(result) && SUCCEEDED(activationResult) && activated)
                result = activated->QueryInterface(IID_PPV_ARGS(&audioClient));
            if (activated)
                activated->Release();
            SetEvent(completed);
            return S_OK;
        }

        HANDLE completed = nullptr;
        HRESULT result = E_FAIL;
        HRESULT activationResult = E_FAIL;
        IAudioClient* audioClient = nullptr;
    };

    static bool openProcessLoopback(CaptureEndpoint& endpoint, DWORD processId,
                                    bool includeProcess)
    {
        AUDIOCLIENT_ACTIVATION_PARAMS params = {};
        params.ActivationType = AUDIOCLIENT_ACTIVATION_TYPE_PROCESS_LOOPBACK;
        params.ProcessLoopbackParams.TargetProcessId = processId;
        params.ProcessLoopbackParams.ProcessLoopbackMode = includeProcess
            ? PROCESS_LOOPBACK_MODE_INCLUDE_TARGET_PROCESS_TREE
            : PROCESS_LOOPBACK_MODE_EXCLUDE_TARGET_PROCESS_TREE;

        PROPVARIANT activation = {};
        activation.vt = VT_BLOB;
        activation.blob.cbSize = sizeof(params);
        activation.blob.pBlobData = reinterpret_cast<BYTE*>(&params);

        auto handler = Microsoft::WRL::Make<ProcessLoopbackActivator>();
        IActivateAudioInterfaceAsyncOperation* operation = nullptr;
        const HRESULT started = ActivateAudioInterfaceAsync(
            VIRTUAL_AUDIO_DEVICE_PROCESS_LOOPBACK, __uuidof(IAudioClient),
            &activation, handler.Get(), &operation);
        if (FAILED(started) || !operation ||
            WaitForSingleObject(handler->completed, 5000) != WAIT_OBJECT_0 ||
            FAILED(handler->result) || FAILED(handler->activationResult) ||
            !handler->audioClient)
        {
            if (operation)
                operation->Release();
            return false;
        }
        operation->Release();
        endpoint.audioClient = handler->audioClient;
        handler->audioClient = nullptr;
        endpoint.format = static_cast<WAVEFORMATEX*>(
            CoTaskMemAlloc(sizeof(WAVEFORMATEX)));
        if (!endpoint.format)
            return false;
        *endpoint.format = {};
        endpoint.format->wFormatTag = WAVE_FORMAT_PCM;
        endpoint.format->nChannels = 2;
        endpoint.format->nSamplesPerSec = 48000;
        endpoint.format->wBitsPerSample = 16;
        endpoint.format->nBlockAlign =
            endpoint.format->nChannels * endpoint.format->wBitsPerSample / 8;
        endpoint.format->nAvgBytesPerSec =
            endpoint.format->nSamplesPerSec * endpoint.format->nBlockAlign;
        HRESULT stage = endpoint.audioClient->Initialize(
                AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK,
                0, 0, endpoint.format, nullptr);
        if (SUCCEEDED(stage))
            stage = endpoint.audioClient->GetService(
                __uuidof(IAudioCaptureClient),
                reinterpret_cast<void**>(&endpoint.captureClient));
        if (SUCCEEDED(stage))
            stage = endpoint.audioClient->Start();
        if (FAILED(stage))
            return false;
        return true;
    }

    static bool isFloatFormat(const WAVEFORMATEX* format)
    {
        if (format->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
            return true;
        if (format->wFormatTag != WAVE_FORMAT_EXTENSIBLE)
            return false;
        const auto* extensible = reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(format);
        return extensible->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
    }

    static float sampleAt(const BYTE* data, UINT32 frame, int channel,
                          const WAVEFORMATEX* format)
    {
        const int channels = format->nChannels;
        const int index = int(frame) * channels + qMin(channel, channels - 1);
        if (isFloatFormat(format) && format->wBitsPerSample == 32)
            return reinterpret_cast<const float*>(data)[index];
        if (format->wBitsPerSample == 16)
            return reinterpret_cast<const int16_t*>(data)[index] / 32768.0f;
        if (format->wBitsPerSample == 32)
            return reinterpret_cast<const int32_t*>(data)[index] / 2147483648.0f;
        if (format->wBitsPerSample == 24)
        {
            const BYTE* value = data + index * 3;
            int32_t sample = int32_t(value[0]) | (int32_t(value[1]) << 8) |
                             (int32_t(value[2]) << 16);
            if ((sample & 0x800000) != 0)
                sample |= ~0xFFFFFF;
            return sample / 8388608.0f;
        }
        return 0.0f;
    }

    static void appendPacket(std::deque<float>& output, const BYTE* data,
                             UINT32 frames, DWORD flags, const WAVEFORMATEX* format)
    {
        if (!frames || !format || !format->nSamplesPerSec)
            return;
        const UINT32 outputFrames = UINT32((uint64_t(frames) * 48000u +
                                            format->nSamplesPerSec / 2) /
                                           format->nSamplesPerSec);
        for (UINT32 i = 0; i < outputFrames; ++i)
        {
            const double sourcePosition =
                double(i) * double(format->nSamplesPerSec) / 48000.0;
            const UINT32 sourceFrame = qMin<UINT32>(
                UINT32(sourcePosition), frames - 1);
            const UINT32 nextFrame = qMin<UINT32>(sourceFrame + 1, frames - 1);
            const float fraction = float(sourcePosition - sourceFrame);
            if ((flags & AUDCLNT_BUFFERFLAGS_SILENT) != 0 || !data)
            {
                output.push_back(0.0f);
                output.push_back(0.0f);
            }
            else
            {
                const float left = sampleAt(data, sourceFrame, 0, format);
                const float nextLeft = sampleAt(data, nextFrame, 0, format);
                const int rightChannel = format->nChannels > 1 ? 1 : 0;
                const float right = sampleAt(data, sourceFrame, rightChannel, format);
                const float nextRight = sampleAt(data, nextFrame, rightChannel, format);
                output.push_back(left + (nextLeft - left) * fraction);
                output.push_back(right + (nextRight - right) * fraction);
            }
        }
    }

    static void drain(CaptureEndpoint& endpoint, std::deque<float>& output)
    {
        if (!endpoint.captureClient)
            return;
        UINT32 packetFrames = 0;
        while (SUCCEEDED(endpoint.captureClient->GetNextPacketSize(&packetFrames)) && packetFrames)
        {
            BYTE* data = nullptr;
            DWORD flags = 0;
            UINT64 devicePosition = 0, qpcPosition = 0;
            UINT32 frames = 0;
            if (FAILED(endpoint.captureClient->GetBuffer(&data, &frames, &flags,
                                                         &devicePosition, &qpcPosition)))
                break;
            appendPacket(output, data, frames, flags, endpoint.format);
            endpoint.captureClient->ReleaseBuffer(frames);
        }
    }

    static bool isLocalAddress(const QHostAddress& address)
    {
        if (address.isLoopback())
            return true;
        for (const auto& local : QNetworkInterface::allAddresses())
        {
            if (local == address)
                return true;
        }
        return false;
    }

    void drainReaStream(QUdpSocket& socket, std::deque<float>& output)
    {
        constexpr int headerSize = 47;
        constexpr int maxPacketSize = headerSize + 1200;
        const QByteArray expectedIdentifier = reaStreamIdentifier.toUtf8();
        int packetsRead = 0;
        while (socket.hasPendingDatagrams() && packetsRead++ < 64)
        {
            QNetworkDatagram datagram = socket.receiveDatagram(maxPacketSize);
            const QByteArray packet = datagram.data();
            if (!isLocalAddress(datagram.senderAddress()) ||
                packet.size() < headerSize || packet.size() > maxPacketSize ||
                packet.left(4) != QByteArrayLiteral("MRSR"))
                continue;

            const auto* bytes = reinterpret_cast<const uchar*>(packet.constData());
            const quint32 packetSize = qFromLittleEndian<quint32>(bytes + 4);
            if (packetSize != quint32(packet.size()))
                continue;
            const QByteArray identifier(packet.constData() + 8, 32);
            const int terminator = identifier.indexOf('\0');
            const QByteArray receivedIdentifier = terminator >= 0
                ? identifier.left(terminator) : identifier;
            if (receivedIdentifier != expectedIdentifier)
                continue;

            const int channels = bytes[40];
            const quint32 sampleRate = qFromLittleEndian<quint32>(bytes + 41);
            const quint16 blockLength = qFromLittleEndian<quint16>(bytes + 45);
            if (channels < 1 || channels > 64 || sampleRate < 8000 ||
                sampleRate > 192000 || blockLength != packet.size() - headerSize ||
                blockLength % (channels * int(sizeof(float))) != 0)
                continue;

            const int frames = blockLength / (channels * int(sizeof(float)));
            if (frames <= 0)
                continue;
            const char* samples = packet.constData() + headerSize;
            const int rightChannel = channels > 1 ? 1 : 0;
            const int outputFrames = qMax(1, int((quint64(frames) * 48000 +
                                                   sampleRate / 2) / sampleRate));
            for (int i = 0; i < outputFrames; ++i)
            {
                const double sourcePosition = double(i) * sampleRate / 48000.0;
                const int first = qMin(int(sourcePosition), frames - 1);
                const int second = qMin(first + 1, frames - 1);
                const float fraction = float(sourcePosition - first);
                float frame[2] = {};
                const int selectedChannels[2] = { 0, rightChannel };
                for (int channel = 0; channel < 2; ++channel)
                {
                    const size_t channelOffset = size_t(selectedChannels[channel]) *
                                                 size_t(frames) * sizeof(float);
                    float a = 0.0f, b = 0.0f;
                    std::memcpy(&a, samples + channelOffset + size_t(first) * sizeof(float), sizeof(float));
                    std::memcpy(&b, samples + channelOffset + size_t(second) * sizeof(float), sizeof(float));
                    if (!std::isfinite(a)) a = 0.0f;
                    if (!std::isfinite(b)) b = 0.0f;
                    frame[channel] = a + (b - a) * fraction;
                }
                output.push_back(frame[0]);
                output.push_back(frame[1]);
            }
        }
    }

    static void trimLatency(std::deque<float>& samples)
    {
        // Bound capture latency to 100 ms at 48 kHz stereo. Drop complete
        // stereo frames so channel alignment is never disturbed.
        constexpr size_t maxSamples = 48000u * 2u / 10u;
        while (samples.size() > maxSamples + 1u)
        {
            samples.pop_front();
            samples.pop_front();
        }
    }

    void run()
    {
        // Declared first so COM outlives all endpoint interfaces below.
        ComApartment apartment;
        Microsoft::WRL::ComPtr<IMMDeviceEnumerator> enumerator;
        CaptureEndpoint computer;
        QUdpSocket reaStreamSocket;
        const bool enumeratorReady = !useReaStream && !sharedProcessId &&
            SUCCEEDED(apartment.result) && SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
                                       CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
                                       reinterpret_cast<void**>(enumerator.GetAddressOf())));
        const bool reaStreamReady = !useReaStream || reaStreamSocket.bind(
            QHostAddress::AnyIPv4, 58710,
            QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
        const bool computerReady = useReaStream ? reaStreamReady : (sharedProcessId
            ? openProcessLoopback(computer, sharedProcessId, includeSharedProcess)
            : enumeratorReady &&
              computer.open(enumerator.Get(), eRender, eMultimedia, true));
        const bool captureReady = computerReady;
        {
            std::lock_guard<std::mutex> lock(initMutex);
            initSuccess = captureReady;
            initDone = true;
        }
        initCondition.notify_one();

        if (captureReady)
        {
            std::deque<float> computerSamples;
            // Feed 10 ms PCM blocks without changing the channel's Opus
            // bitrate, complexity, sample rate or frame/packet duration.
            constexpr int captureFrames = 480;
            std::vector<short> pcm(captureFrames * 2);
            uint32_t sampleIndex = 0;
            bool pcmPending = false;
            while (running.load())
            {
                drain(computer, computerSamples);
                if (useReaStream)
                    drainReaStream(reaStreamSocket, computerSamples);
                trimLatency(computerSamples);

                if (pcmPending)
                {
                    AudioBlock pendingBlock = {};
                    pendingBlock.nStreamID = 0x5343524E;
                    pendingBlock.nSampleRate = 48000;
                    pendingBlock.nChannels = 2;
                    pendingBlock.lpRawAudio = pcm.data();
                    pendingBlock.nSamples = captureFrames;
                    pendingBlock.uSampleIndex = sampleIndex;
                    if (TT_InsertAudioBlock(instance, &pendingBlock))
                    {
                        sampleIndex += pendingBlock.nSamples;
                        pcmPending = false;
                    }
                    else
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(5));
                        continue;
                    }
                }

                while (computerSamples.size() >= pcm.size())
                {
                    for (size_t i = 0; i < pcm.size(); ++i)
                    {
                        float mixed = 0.0f;
                        if (!computerSamples.empty())
                        {
                            // Match TeamTalk's other 0..100 sliders: 50 is
                            // unity gain, values above 50 amplify up to the
                            // application's supported maximum.
                            const float shareGain = sliderGain(volume.load());
                            mixed = computerSamples.front() * shareGain;
                            computerSamples.pop_front();
                        }
                        // Soft-limit amplified source peaks to avoid hard clipping.
                        if (mixed > 0.95f)
                            mixed = 0.95f +
                                    0.05f * std::tanh((mixed - 0.95f) / 0.05f);
                        else if (mixed < -0.95f)
                            mixed = -0.95f +
                                    0.05f * std::tanh((mixed + 0.95f) / 0.05f);
                        mixed = qMax(-1.0f, qMin(1.0f, mixed));
                        pcm[i] = short(std::lrint(mixed * 32767.0f));
                    }

                    AudioBlock block = {};
                    block.nStreamID = 0x5343524E;
                    block.nSampleRate = 48000;
                    block.nChannels = 2;
                    block.lpRawAudio = pcm.data();
                    block.nSamples = captureFrames;
                    block.uSampleIndex = sampleIndex;
                    if (TT_InsertAudioBlock(instance, &block))
                    {
                        sampleIndex += block.nSamples;
                    }
                    else
                    {
                        pcmPending = true;
                        break;
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        }

        TT_InsertAudioBlock(instance, nullptr);
        running.store(false);
    }
#else
    void run() {}
#endif
};

ScreenShareAudio::ScreenShareAudio()
    : m_impl(new Impl)
{
}

ScreenShareAudio::~ScreenShareAudio()
{
    stop();
}

bool ScreenShareAudio::start(TTInstance* instance,
                             unsigned long sharedProcessId,
                             bool includeSharedProcess,
                             bool useReaStream,
                             const QString& reaStreamIdentifier)
{
#if defined(Q_OS_WIN32)
    stop();
    // An invalid selected window must never fall back to whole-device audio.
    if (!useReaStream && includeSharedProcess && sharedProcessId == 0)
        return false;
    m_impl->instance = instance;
    m_impl->sharedProcessId = DWORD(sharedProcessId);
    m_impl->includeSharedProcess = includeSharedProcess;
    m_impl->useReaStream = useReaStream;
    m_impl->reaStreamIdentifier = reaStreamIdentifier;
    {
        std::lock_guard<std::mutex> lock(m_impl->initMutex);
        m_impl->initDone = false;
        m_impl->initSuccess = false;
    }
    m_impl->running.store(true);
    m_impl->worker = std::thread([this]() { m_impl->run(); });
    std::unique_lock<std::mutex> lock(m_impl->initMutex);
    const bool signalled = m_impl->initCondition.wait_for(
        lock, std::chrono::seconds(7), [this]() { return m_impl->initDone; });
    const bool success = signalled && m_impl->initSuccess;
    lock.unlock();
    if (!success)
        stop();
    return success;
#else
    Q_UNUSED(instance);
    Q_UNUSED(useReaStream);
    Q_UNUSED(reaStreamIdentifier);
    return false;
#endif
}

unsigned long ScreenShareAudio::processIdForWindow(void* window)
{
#if defined(Q_OS_WIN32)
    const HWND hwnd = static_cast<HWND>(window);
    DWORD ownerId = 0;
    GetWindowThreadProcessId(hwnd, &ownerId);
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, ownerId);
    if (!process)
        return ownerId;
    wchar_t path[MAX_PATH] = {};
    DWORD size = MAX_PATH;
    const bool isFrameHost = QueryFullProcessImageNameW(process, 0, path, &size) &&
        QString::fromWCharArray(path, int(size)).endsWith(
            QStringLiteral("\\ApplicationFrameHost.exe"), Qt::CaseInsensitive);
    CloseHandle(process);
    if (!isFrameHost)
        return ownerId;

    // UWP frame windows belong to a silent Windows host. The embedded
    // CoreWindow belongs to the application whose process tree emits audio.
    struct Search { DWORD owner; DWORD app = 0; } search { ownerId };
    EnumChildWindows(hwnd, [](HWND child, LPARAM param) -> BOOL {
        auto& search = *reinterpret_cast<Search*>(param);
        wchar_t className[128] = {};
        DWORD childId = 0;
        GetWindowThreadProcessId(child, &childId);
        if (childId && childId != search.owner &&
            GetClassNameW(child, className, 128) &&
            wcscmp(className, L"Windows.UI.Core.CoreWindow") == 0)
        {
            search.app = childId;
            return FALSE;
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&search));
    return search.app ? search.app : ownerId;
#else
    Q_UNUSED(window);
    return 0;
#endif
}

void ScreenShareAudio::stop()
{
    m_impl->running.store(false);
    if (m_impl->worker.joinable())
        m_impl->worker.join();
}

bool ScreenShareAudio::isRunning() const
{
    return m_impl->running.load();
}

void ScreenShareAudio::setVolume(int volume)
{
    m_impl->volume.store(qBound(0, volume, 100));
}
