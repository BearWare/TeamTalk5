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

#include <assert.h>
#include <ace/FILE_Connector.h>
#include <myace/MyACE.h>

#define WAVEHEADERSIZE 44


bool WriteWaveFileHeader(ACE_FILE_IO& file, const media::AudioFormat& fmt)
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

bool WriteWaveFileHeader(ACE_FILE_IO& file, const WAVEFORMATEX* waveformat, int len)
{
    const char RIFF[] = "RIFF";
    const char WAVE[] = "WAVEfmt ";
    const char DATA[] = "data";
    bool ret = true;
    ret &= file.send_n(RIFF, 4) >= 0; // RIFF header
    int32_t wavedatasize = 0x7FFFFFFF;
    ret &= file.send_n(&wavedatasize, 4) >= 0; //sizeof file wave data
    ret &= file.send_n(WAVE, 8) >= 0;
    ret &= file.send_n(&len, 4) >= 0; //header size
    ret &= file.send_n(waveformat, len) >= 0;
    ret &= file.send_n(DATA, 4) >= 0;
    ret &= file.send_n(&wavedatasize, 4) >= 0; //sizeof file wave data

    return ret;
}

bool UpdateWaveFileHeader(ACE_FILE_IO& file)
{
    assert(file.get_handle() != ACE_INVALID_HANDLE);
    ACE_OFF_T origin = file.tell();
    if (file.seek(0, SEEK_END) < 0)
        return false;

    bool success = false;

    ACE_OFF_T end = file.tell();
    if (file.seek(strlen("RIFF"), SEEK_SET) >= 0 && end < 0x100000000)
    {
        uint32_t wavedatasize = uint32_t(end) - 8 /* don't include RIFF and size field in total size */;
        uint32_t headersize = 0;
        if (file.send_n(&wavedatasize, 4) >= 0 &&
            file.seek(8, SEEK_CUR) >= 0 /* past 'WAVEfmt ' */ &&
            file.recv(&headersize, 4) >= 0 &&
            file.seek(headersize, SEEK_CUR) >= 0 /* past extra header size */ &&
            file.seek(4, SEEK_CUR) >= 0 /* past 'data' */)
        {
            wavedatasize = uint32_t(end - file.tell());
            wavedatasize -= 4; // don't include size-field as part of data size
            if (file.send_n(&wavedatasize, 4) >= 0)
            {
                success = true;
            }
        }
        else
        {
            MYTRACE(ACE_TEXT("Failed to update wave-file header\n"));
        }
    }

    return file.seek(origin, SEEK_SET) >= 0 && success;
}

WaveFile::WaveFile()
{
}

WaveFile::~WaveFile()
{
    m_wavfile.close();
}

bool WaveFile::NewFile(const ACE_TString& filename, const WAVEFORMATEX* waveformat, int len)
{
    if (m_wavfile.get_handle() != ACE_INVALID_HANDLE)
        return false;

    ACE_FILE_Connector con;

#if defined(WIN32)
    int ret = con.connect(m_wavfile, ACE_FILE_Addr(filename.c_str()),
        0, ACE_Addr::sap_any, 0, O_RDWR | O_CREAT | O_BINARY | O_TRUNC, FILE_SHARE_READ | FILE_SHARE_WRITE);
#else
    int ret = con.connect(m_wavfile, ACE_FILE_Addr(filename.c_str()));
#endif

    if (ret < 0)
        return false;
    else
    {
        return WriteWaveFileHeader(m_wavfile, waveformat, len);
    }
}

bool WaveFile::AppendData(const void* data, int len)
{
    return m_wavfile.send_n(data, len) >= 0 && UpdateWaveFileHeader(m_wavfile);
}

WavePCMFile::WavePCMFile()
{
    m_channels = 0;
}

WavePCMFile::~WavePCMFile()
{
    Close();
    MYTRACE(ACE_TEXT("Closing wave-file %s\n"), m_filepath.c_str());
}

bool WavePCMFile::OpenFile(const ACE_TString& filename, bool readonly)
{
    ACE_FILE_Connector con;

    int flags = readonly? O_RDONLY : O_RDWR | O_CREAT;
#if defined(WIN32)
    int share = readonly? FILE_SHARE_READ : FILE_SHARE_READ | FILE_SHARE_WRITE;
#else
    int share = ACE_DEFAULT_FILE_PERMS;
#endif    

    int ret = con.connect(m_wavfile, ACE_FILE_Addr(filename.c_str()),
        0, ACE_Addr::sap_any, 0, flags, share);

    if(ret>=0 && Valid())
    {
        SeekSamplesBegin();
        m_filepath = filename;
        return true;
    }

    m_wavfile.close();
    return false;
}

bool WavePCMFile::NewFile(const ACE_TString& filename, int samplerate, int channels)
{
    ACE_FILE_Connector con;

#if defined(WIN32)
    int ret = con.connect(m_wavfile, ACE_FILE_Addr(filename.c_str()),
        0, ACE_Addr::sap_any, 0, O_RDWR | O_CREAT | O_BINARY | O_TRUNC, FILE_SHARE_READ | FILE_SHARE_WRITE);
#else
    int ret = con.connect(m_wavfile, ACE_FILE_Addr(filename.c_str()));
#endif

    if(ret<0)
        return false;
    else
    {
        m_wavfile.truncate(0);
        m_filepath = filename;
        m_channels = channels;
        return WriteHeader(samplerate, channels);
    }
}


void WavePCMFile::Close()
{
    if(m_wavfile.get_handle() != ACE_INVALID_HANDLE)
    {
        UpdateWaveFileHeader(m_wavfile);
        m_wavfile.close();
        m_channels = 0;
    }
}

const ACE_TString WavePCMFile::FileName() const
{
    size_t pos = m_filepath.rfind('\\');
    if(pos != ACE_TString::npos)
        return m_filepath.substr(pos+1, m_filepath.length()-pos);
    else
        return m_filepath;
}

bool WavePCMFile::SeekSamplesBegin()
{
    assert(m_wavfile.get_handle() != ACE_INVALID_HANDLE);
    if(m_wavfile.get_handle() != ACE_INVALID_HANDLE)
    {
        m_wavfile.seek(WAVEHEADERSIZE, SEEK_SET);
        return (int)m_wavfile.tell() == WAVEHEADERSIZE;
    }
    return false;
}

bool WavePCMFile::SeekSamplesEnd()
{
    assert(m_wavfile.get_handle() != ACE_INVALID_HANDLE);
    if(m_wavfile.get_handle() != ACE_INVALID_HANDLE)
    {
        m_wavfile.seek(0, SEEK_END);
        return true;
    }
    return false;
}

int WavePCMFile::ReadSamples(short* buffer, int buffer_len)
{
    assert(m_wavfile.get_handle() != ACE_INVALID_HANDLE);
    ACE_OFF_T pos = m_wavfile.tell();
    int channels = GetChannels();
    if(!channels)
        return 0;

    ssize_t bytes = buffer_len*sizeof(short)*channels;
    bytes = m_wavfile.recv(buffer, bytes);
#ifdef _DEBUG
    assert(bytes>=0);
    bytes = ACE_OS::last_error();
#endif
    ACE_OFF_T pos2 = m_wavfile.tell();
    assert(pos2-pos>=0);
    return (int)(pos2-pos)/2/channels;
}

bool WavePCMFile::WriteHeader(int samplerate, int channels)
{
    assert(m_wavfile.get_handle() != ACE_INVALID_HANDLE);
    m_wavfile.seek(0, SEEK_SET);

    return WriteWaveFileHeader(m_wavfile, media::AudioFormat(samplerate, channels));
}

bool WavePCMFile::Valid()
{
    assert(m_wavfile.get_handle() != ACE_INVALID_HANDLE);

    ACE_OFF_T oldPos = m_wavfile.tell();
    const char RIFF[] ="RIFF";
    const char WAVE[] = "WAVEfmt ";
    char buff[8];
    bool valid = true;
    m_wavfile.seek(0, SEEK_SET);
    m_wavfile.recv(buff, 4);

    valid &= strncmp(buff, RIFF, 4) == 0;

    m_wavfile.seek(8, SEEK_SET);
    m_wavfile.recv(buff, 8);

    valid &= strncmp(buff, WAVE, 8) == 0;

    short compression_type = 0;
    m_wavfile.seek(20, SEEK_SET);
    m_wavfile.recv(&compression_type, 2);
    valid &= (compression_type == 1);
    
    short bitdepth = 0;
    m_wavfile.seek(34, SEEK_SET);
    m_wavfile.recv(&bitdepth, 2);
    valid &= (bitdepth == 16);
    
    m_wavfile.seek(oldPos, SEEK_SET);

    return valid;
}

int WavePCMFile::GetSampleRate()
{
    if(m_wavfile.get_handle() == ACE_INVALID_HANDLE)
        return 0;

    ACE_OFF_T oldPos = m_wavfile.tell();
    int samplerate = 0;
    m_wavfile.seek(24, SEEK_SET);
    m_wavfile.recv((char*)&samplerate, 4);

    m_wavfile.seek(oldPos, SEEK_SET);

    return samplerate;
}

int WavePCMFile::GetSamplesCount()
{
    if(m_wavfile.get_handle() == ACE_INVALID_HANDLE)
        return 0;
    else if(!UpdateWaveFileHeader(m_wavfile))
        return 0;

    ACE_OFF_T oldPos = m_wavfile.tell();
    uint16_t wBitsPerSample = 0, nChannels = 0;
    uint32_t nBytesCount = 0;
    m_wavfile.seek(22, SEEK_SET);
    m_wavfile.recv((char*)&nChannels, 2);
    m_wavfile.seek(34, SEEK_SET);
    m_wavfile.recv((char*)&wBitsPerSample, 2);
    m_wavfile.seek(40, SEEK_SET);
    m_wavfile.recv((char*)&nBytesCount, 4);

    m_wavfile.seek(oldPos, SEEK_SET);

    if (wBitsPerSample >= 8 && nChannels > 0)
        return nBytesCount / (wBitsPerSample / 8) / nChannels;

    return 0;
}

int WavePCMFile::GetChannels()
{
    if(m_wavfile.get_handle() == ACE_INVALID_HANDLE)
        return 0;

    if(m_channels != 0)
        return m_channels;

    ACE_OFF_T oldPos = m_wavfile.tell();
    short nChannels = 0;
    m_wavfile.seek(22, SEEK_SET);
    m_wavfile.recv((char*)&nChannels, 2);

    m_wavfile.seek(oldPos, SEEK_SET);

    return nChannels;
}

int WavePCMFile::WriteData(const void* data, int len)
{
    assert(m_wavfile.get_handle() != ACE_INVALID_HANDLE);
    ACE_OFF_T pos = m_wavfile.tell();
    if(pos - WAVEHEADERSIZE + len <= 0xFFFFFFFF)
        m_wavfile.send(reinterpret_cast<const char*>(data), len);
    ACE_OFF_T pos2 = m_wavfile.tell();
    int val = (int)(pos2 - pos);
    return val <= 0 && len > 0 ? 0 : val;
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
