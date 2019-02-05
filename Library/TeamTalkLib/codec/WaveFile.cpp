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

WaveFile::WaveFile()
{
    m_channels = 0;
}

WaveFile::~WaveFile()
{
    Close();
    MYTRACE(ACE_TEXT("Closing wave-file %s\n"), m_filepath.c_str());
}

bool WaveFile::OpenFile(const ACE_TString& filename, bool readonly)
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

bool WaveFile::NewFile(const ACE_TString& filename, int samplerate, int channels)
{
    ACE_FILE_Connector con;

#if !defined(UNDER_CE)
    int ret = con.connect(m_wavfile, ACE_FILE_Addr(filename.c_str()));
#elif defined(WIN32)
    int ret = con.connect(m_wavfile, ACE_FILE_Addr(filename.c_str()),
        0, ACE_Addr::sap_any, 0, O_RDWR | O_CREAT, FILE_SHARE_READ | FILE_SHARE_WRITE);
#else
    int ret = con.connect(m_wavfile, ACE_FILE_Addr(filename.c_str()),
        0, ACE_Addr::sap_any, 0, O_RDWR | O_CREAT);
#endif

    if(ret<0)
    {
        //assert(fopen(filename.c_str(), "wb+") == 0);
        //assert(CreateFile(filename.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS | TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)>0);
        return false;
    }
    else
    {
        m_wavfile.truncate(0);
        m_filepath = filename;
        m_channels = channels;
        return WriteHeader(samplerate, channels);
    }
}


void WaveFile::Close()
{
    if(m_wavfile.get_handle() != ACE_INVALID_HANDLE)
    {
        WriteHeaderLength();
        m_wavfile.close();
        m_channels = 0;
    }
}

const ACE_TString WaveFile::FileName() const
{
    size_t pos = m_filepath.rfind('\\');
    if(pos != ACE_TString::npos)
        return m_filepath.substr(pos+1, m_filepath.length()-pos);
    else
        return m_filepath;
}

bool WaveFile::SeekSamplesBegin()
{
    assert(m_wavfile.get_handle() != ACE_INVALID_HANDLE);
    if(m_wavfile.get_handle() != ACE_INVALID_HANDLE)
    {
        m_wavfile.seek(WAVEHEADERSIZE, SEEK_SET);
        return (int)m_wavfile.tell() == WAVEHEADERSIZE;
    }
    return false;
}

bool WaveFile::SeekSamplesEnd()
{
    assert(m_wavfile.get_handle() != ACE_INVALID_HANDLE);
    if(m_wavfile.get_handle() != ACE_INVALID_HANDLE)
    {
        m_wavfile.seek(0, SEEK_END);
        return true;
    }
    return false;
}

int WaveFile::ReadSamples(short* buffer, int buffer_len)
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

bool WaveFile::WriteHeader(int samplerate, int channels)
{
    assert(m_wavfile.get_handle() != ACE_INVALID_HANDLE);
    m_wavfile.seek(0, SEEK_SET);

    int tmp = 0;
    const char RIFF[] ="RIFF";
    const char WAVE[] = "WAVEfmt ";
    const char DATA[] = "data";

    WriteData(RIFF, 4);
    tmp = 0x7FFFFFFF;
    WriteData(&tmp, 4);    //sizeof file wave data
    WriteData(WAVE, 8);
    tmp = 16;
    WriteData(&tmp, 4);    //header size
    tmp = 1; //PCM uncompressed
    WriteData(&tmp, 2);    //type
    tmp = channels;
    WriteData(&tmp, 2);    //channels
    tmp = samplerate;
    WriteData(&tmp, 4);    //sampling rate
    tmp = samplerate*2*channels;
    WriteData(&tmp, 4);    //bytes per second
    tmp = 2 * channels;
    WriteData(&tmp, 2);    //block align
    tmp = 16;
    WriteData(&tmp, 2);    //bit depth
    WriteData(DATA, (int)strlen(DATA)); //4
    tmp = 0x7FFFFFFF; 
    return WriteData(&tmp, 4) == 4; //sizeof file wave data
    //total 44 bytes
}

bool WaveFile::WriteHeaderLength()
{
    assert(m_wavfile.get_handle() != ACE_INVALID_HANDLE);
    ACE_OFF_T ret_pos = m_wavfile.tell();
    m_wavfile.seek(0, SEEK_END);
    ACE_OFF_T pos = m_wavfile.tell();
    ACE_OFF_T byteCount = pos - WAVEHEADERSIZE;
    byteCount = byteCount>0?byteCount:0;
    m_wavfile.seek(strlen("RIFF"), SEEK_SET);
    assert((int)m_wavfile.tell() == 4);
    WriteData(&byteCount, 4);    //write byte count
    m_wavfile.seek(40, SEEK_SET);
    bool b = WriteData(&byteCount, 4) == 4;
    m_wavfile.seek(ret_pos, SEEK_SET);
    assert(m_wavfile.tell() == ret_pos);
    return b;
}

bool WaveFile::Valid()
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

int WaveFile::GetSampleRate()
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

int WaveFile::GetSamplesCount()
{
    if(m_wavfile.get_handle() == ACE_INVALID_HANDLE)
        return 0;
    else if(!WriteHeaderLength())
        return 0;

    ACE_OFF_T oldPos = m_wavfile.tell();
    int nBitsPerSample = 0, nBytesCount = 0;
    m_wavfile.seek(34, SEEK_SET);
    m_wavfile.recv((char*)&nBitsPerSample, 2);
    m_wavfile.seek(40, SEEK_SET);
    m_wavfile.recv((char*)&nBytesCount, 4);

    m_wavfile.seek(oldPos, SEEK_SET);

    if(nBitsPerSample>0)
        return nBytesCount / (nBitsPerSample / 8);

    return 0;
}

int WaveFile::GetChannels()
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

int WaveFile::WriteData(const void* data, int len)
{
    assert(m_wavfile.get_handle() != ACE_INVALID_HANDLE);
    ACE_OFF_T pos = m_wavfile.tell();
    if(pos - WAVEHEADERSIZE + len <= 0xFFFFFFFF)
        m_wavfile.send(reinterpret_cast<const char*>(data), len);
    ACE_OFF_T pos2 = m_wavfile.tell();
    int val = (int)(pos2 - pos);
    return val <= 0 && len > 0 ? 0 : val;
}

bool WaveFile::AppendSamples(const short* buffer, int samples_len)
{
    if (WriteData(buffer, samples_len * sizeof(short)*GetChannels())>0)
    {
        WriteHeaderLength(); //update header (so crash will still leave wav-file valid)
        return true;
    }
    return false;
}
