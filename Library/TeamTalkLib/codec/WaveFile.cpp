/*
 * Copyright (c) 2005-2018, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
 */

#include "WaveFile.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstddef>

constexpr auto WAVEHEADERSIZE = 44;
constexpr auto DEBUG_WAVEFILE = 0;

bool WriteWaveFileHeader(MyFile& file, const media::AudioFormat& fmt)
{
    WAVEFORMATEX waveformat;
    waveformat.wFormatTag = 1;
    waveformat.nChannels = fmt.channels;
    waveformat.nSamplesPerSec = fmt.samplerate;
    waveformat.nAvgBytesPerSec = PCM16_BYTES(fmt.samplerate, fmt.channels);
    waveformat.nBlockAlign = uint16_t(PCM16_BYTES(1, fmt.channels));
    waveformat.wBitsPerSample = 16;
    waveformat.cbSize = 0;

    static_assert(sizeof(WAVEFORMATEX) - 2 /* cbSize */ == SIZEOF_WAVEFORMATEX, "WAVEFORMATEX not packed");

    return WriteWaveFileHeader(file, &waveformat, SIZEOF_WAVEFORMATEX);
}

bool WriteWaveFileHeader(MyFile& file, const WAVEFORMATEX* waveformat, int len)
{
    const char RIFF[] = "RIFF";
    const char WAVE[] = "WAVEfmt ";
    const char DATA[] = "data";
    bool ret = true;
    ret &= file.Write(RIFF, 4) == 4; // RIFF header
    int32_t wavedatasize = 0x7FFFFFFF;
    ret &= file.Write(reinterpret_cast<const char*>(&wavedatasize), 4) == 4; //sizeof file wave data
    ret &= file.Write(WAVE, 8) == 8;
    ret &= file.Write(reinterpret_cast<const char*>(&len), 4) == 4; //header size
    ret &= file.Write(reinterpret_cast<const char*>(waveformat), len) == len;
    ret &= file.Write(DATA, 4) == 4;
    ret &= file.Write(reinterpret_cast<const char*>(&wavedatasize), 4) == 4; //sizeof file wave data

    return ret;
}

bool UpdateWaveFileHeader(MyFile& file)
{
    auto origin = file.Tell();
    if (!file.Seek(0, std::ios_base::end))
        return false;

    bool success = false;
    char riff[4];
    char wavefmt[8];
    char data[4];

    auto end = file.Tell();
    if (file.Seek(0, std::ios_base::beg) &&
        file.Read(riff, strlen("RIFF")) > 0 && end < 0x100000000)
    {
        assert(memcmp(riff, "RIFF", 4) == 0);

        uint32_t wavedatasize = uint32_t(end) - 8 /* don't include RIFF and size field in total size */;
        uint32_t headersize = 0;
        if (file.Write(reinterpret_cast<const char*>(&wavedatasize), 4) == 4 &&
            file.Read(wavefmt, 8) == 8 /* past 'WAVEfmt ' */ &&
            file.Read(reinterpret_cast<char*>(&headersize), 4) == 4 &&
            file.Seek(headersize, std::ios_base::cur) /* past extra header size */ &&
            file.Read(data, 4) == 4 /* past 'data' */)
        {
            assert(memcmp(wavefmt, "WAVEfmt ", 8) == 0);
            assert(memcmp(data, "data", 4) == 0);
            wavedatasize = uint32_t(end - file.Tell());
            wavedatasize -= 4; // don't include size-field as part of data size
            if (file.Write(reinterpret_cast<const char*>(&wavedatasize), 4) == 4)
            {
                success = true;
            }
        }
        else
        {
            MYTRACE_COND(DEBUG_WAVEFILE, ACE_TEXT("Failed to update wave-file header\n"));
        }
    }

    return file.Seek(origin, std::ios_base::beg) && success;
}

WaveFile::WaveFile()
= default;

WaveFile::~WaveFile()
= default;

bool WaveFile::NewFile(const ACE_TString& filename, const WAVEFORMATEX* waveformat, int len)
{
    if (!m_wavfile.NewFile(filename))
        return false;
    
            return WriteWaveFileHeader(m_wavfile, waveformat, len);
   
}

bool WaveFile::AppendData(const void* data, int len)
{
    return m_wavfile.Write(reinterpret_cast<const char*>(data), len) == len && UpdateWaveFileHeader(m_wavfile);
}

WavePCMFile::WavePCMFile() : m_channels(0)
{
    
}

WavePCMFile::~WavePCMFile()
{
    Close();
    MYTRACE_COND(DEBUG_WAVEFILE, ACE_TEXT("Closing wave-file %s\n"), m_filepath.c_str());
}

bool WavePCMFile::OpenFile(const ACE_TString& filename, bool readonly)
{
    if(m_wavfile.Open(filename, readonly) && Valid())
    {
        SeekSamplesBegin();
        m_filepath = filename;
        return true;
    }

    m_wavfile.Close();
    return false;
}

bool WavePCMFile::NewFile(const ACE_TString& filename, int samplerate, int channels)
{
    if (m_wavfile.NewFile(filename))
    {
        m_filepath = filename;
        m_channels = channels;
        return WriteHeader(samplerate, channels);
    }
    return false;
}

bool WavePCMFile::NewFile(const ACE_TString& filename, const media::AudioFormat& fmt)
{
    return NewFile(filename, fmt.samplerate, fmt.channels);
}

void WavePCMFile::Close()
{
    UpdateWaveFileHeader(m_wavfile);
    m_wavfile.Close();
    m_channels = 0;
}

ACE_TString WavePCMFile::FileName() const
{
    size_t const pos = m_filepath.rfind('\\');
    if(pos != ACE_TString::npos)
        return m_filepath.substr(pos+1, m_filepath.length()-pos);
            return m_filepath;
}

bool WavePCMFile::SeekSamplesBegin()
{
    m_wavfile.Seek(WAVEHEADERSIZE, std::ios_base::beg);
    return m_wavfile.Tell() == WAVEHEADERSIZE;
}

bool WavePCMFile::SeekSamplesEnd()
{
    return m_wavfile.Seek(0, std::ios_base::end);
}

int WavePCMFile::ReadSamples(short* buffer, int buffer_len)
{
    auto pos = m_wavfile.Tell();
    int const channels = GetChannels();
    if(channels == 0)
        return 0;

    std::streamsize bytes = buffer_len * sizeof(short) * channels;
    bytes = m_wavfile.Read(reinterpret_cast<char*>(buffer), bytes);
#ifdef _DEBUG
    assert(bytes>=0);
    bytes = ACE_OS::last_error();
#endif
    auto pos2 = m_wavfile.Tell();
    assert(pos2-pos>=0);
    return (int)(pos2-pos)/2/channels;
}

bool WavePCMFile::WriteHeader(int samplerate, int channels)
{
    m_wavfile.Seek(0, std::ios_base::beg);

    return WriteWaveFileHeader(m_wavfile, media::AudioFormat(samplerate, channels));
}

bool WavePCMFile::Valid()
{
    auto oldPos = m_wavfile.Tell();
    const char RIFF[] ="RIFF";
    const char WAVE[] = "WAVEfmt ";
    char buff[8];
    bool valid = true;
    m_wavfile.Seek(0, std::ios_base::beg);
    m_wavfile.Read(buff, 4);

    valid &= strncmp(buff, RIFF, 4) == 0;

    m_wavfile.Seek(8, std::ios_base::beg);
    m_wavfile.Read(buff, 8);

    valid &= strncmp(buff, WAVE, 8) == 0;

    short compression_type = 0;
    m_wavfile.Seek(20, std::ios_base::beg);
    m_wavfile.Read(reinterpret_cast<char*>(&compression_type), 2);
    valid &= (compression_type == 1);
    
    short bitdepth = 0;
    m_wavfile.Seek(34, std::ios_base::beg);
    m_wavfile.Read(reinterpret_cast<char*>(&bitdepth), 2);
    valid &= (bitdepth == 16);
    
    m_wavfile.Seek(oldPos, std::ios_base::beg);

    return valid;
}

int WavePCMFile::GetSampleRate()
{
    auto oldPos = m_wavfile.Tell();
    int samplerate = 0;
    m_wavfile.Seek(24, std::ios_base::beg);
    m_wavfile.Read(reinterpret_cast<char*>(&samplerate), 4);

    m_wavfile.Seek(oldPos, std::ios_base::beg);

    return samplerate;
}

int WavePCMFile::GetSamplesCount()
{
    if (!UpdateWaveFileHeader(m_wavfile))
        return 0;

    auto oldPos = m_wavfile.Tell();
    uint16_t wBitsPerSample = 0;
    uint16_t nChannels = 0;
    uint32_t nBytesCount = 0;
    m_wavfile.Seek(22, std::ios_base::beg);
    m_wavfile.Read(reinterpret_cast<char*>(&nChannels), 2);
    m_wavfile.Seek(34, std::ios_base::beg);
    m_wavfile.Read(reinterpret_cast<char*>(&wBitsPerSample), 2);
    m_wavfile.Seek(40, std::ios_base::beg);
    m_wavfile.Read(reinterpret_cast<char*>(&nBytesCount), 4);

    m_wavfile.Seek(oldPos, std::ios_base::beg);

    if (wBitsPerSample >= 8 && nChannels > 0)
        return nBytesCount / (wBitsPerSample / 8) / nChannels;

    return 0;
}

int WavePCMFile::GetChannels()
{
    if(m_channels != 0)
        return m_channels;

    auto oldPos = m_wavfile.Tell();
    short nChannels = 0;
    m_wavfile.Seek(22, std::ios_base::beg);
    m_wavfile.Read(reinterpret_cast<char*>(&nChannels), 2);

    m_wavfile.Seek(oldPos, std::ios_base::beg);

    return nChannels;
}

int WavePCMFile::WriteData(const void* data, int len)
{
    auto pos = m_wavfile.Tell();
    if(pos - WAVEHEADERSIZE + len <= 0xFFFFFFFF)
        m_wavfile.Write(reinterpret_cast<const char*>(data), len);
    auto pos2 = m_wavfile.Tell();
    auto val = (pos2 - pos);
    return val <= 0 && len > 0 ? 0 : int(val);
}

bool WavePCMFile::AppendSamples(const short* buffer, int samples_len)
{
    if (WriteData(buffer, samples_len * sizeof(short)*GetChannels())>0)
    {
        UpdateWaveFileHeader(m_wavfile); //update header (so crash will still leave wav-file valid)
        return true;
    }
    return false;
}
