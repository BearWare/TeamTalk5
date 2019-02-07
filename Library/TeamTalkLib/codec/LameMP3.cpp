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

#include "LameMP3.h"
#include <ace/FILE_Connector.h>
#include <assert.h>
#include <myace/MyACE.h>

HMODULE hLAMEMod = 0;
BEINITSTREAM initStream = 0;
BEENCODECHUNK encChunk = 0;
BEDEINITSTREAM deinitStream = 0;
BECLOSESTREAM closeStream = 0;
BEWRITEINFOTAG writeInfoTag = 0;
BEWRITEVBRHEADER writeVBRHeader = 0;

#define LAME_DLL_FILENAME ACE_TEXT("lame_enc.dll")

ACE_TString GetLameMp3Path()
{
    ACE_TCHAR lameDLL[MAX_PATH] = ACE_TEXT("");
    ::GetModuleFileName(NULL, lameDLL, MAX_PATH);

    ACE_TCHAR* c = ACE_OS::strrchr(lameDLL, '\\');
    if(c)
    {
        c++;
        *c = 0;
        const ACE_TCHAR* szLame = LAME_DLL_FILENAME;
        ACE_OS::strcat(lameDLL, szLame); 
        return lameDLL;
    }
    return ACE_TString();
}

bool LameMP3::CanLoad()
{
    return ACE_OS::filesize(GetLameMp3Path().c_str())>0 ||
           ACE_OS::filesize(LAME_DLL_FILENAME)>0;
}

LameMP3::LameMP3()
: m_hMp3Stream(0)
{
    if(!hLAMEMod)
    {
        hLAMEMod = LoadLibrary(LAME_DLL_FILENAME);
        if(!hLAMEMod)
            hLAMEMod = LoadLibrary(GetLameMp3Path().c_str());

        if(!hLAMEMod)
            OutputDebugString(ACE_TEXT("ERROR: Unable to load lame_enc.dll\n"));
        else
        {
            initStream = (BEINITSTREAM)GetProcAddress(hLAMEMod, TEXT_BEINITSTREAM);
            encChunk = (BEENCODECHUNK)GetProcAddress(hLAMEMod, TEXT_BEENCODECHUNK);
            deinitStream = (BEDEINITSTREAM)GetProcAddress(hLAMEMod, TEXT_BEDEINITSTREAM);
            closeStream = (BECLOSESTREAM)GetProcAddress(hLAMEMod, TEXT_BECLOSESTREAM);
            writeInfoTag = (BEWRITEINFOTAG)GetProcAddress(hLAMEMod, TEXT_BEWRITEINFOTAG);
            writeVBRHeader = (BEWRITEVBRHEADER)GetProcAddress(hLAMEMod, TEXT_BEWRITEVBRHEADER);
        }
    }
}

LameMP3::~LameMP3()
{
    Close();
}

bool LameMP3::NewFile(const ACE_TString& filename, int samplerate, 
                      int channels, int bitrate)
{
    if(m_hMp3Stream)
        return false;

    //init MP3 encoder
    BE_CONFIG conf = {};
    conf.dwConfig = BE_CONFIG_LAME;

    conf.format.LHV1.dwStructVersion = CURRENT_STRUCT_VERSION;
    conf.format.LHV1.dwStructSize = CURRENT_STRUCT_SIZE;
    conf.format.LHV1.dwSampleRate = samplerate;
    conf.format.LHV1.dwReSampleRate = 0;
    conf.format.LHV1.nMode = channels == 1? BE_MP3_MODE_MONO : BE_MP3_MODE_STEREO;
    conf.format.LHV1.dwBitrate = bitrate;
    conf.format.LHV1.nPreset = 0; //LQP_NORMAL_QUALITY;
    conf.format.LHV1.nQuality = 0;
    conf.format.LHV1.bEnableVBR = 1;
    conf.format.LHV1.nVBRQuality = 4;
    conf.format.LHV1.bWriteVBRHeader = 1;
    conf.format.LHV1.bStrictIso = 0;

    DWORD dwBufferSize = 0;
    int err = initStream(&conf, &m_dwInSamples, &dwBufferSize, &m_hMp3Stream);
    if(err != 0)
        return false;

    m_dwChannels = channels;
    m_out_mp3data.resize(dwBufferSize);

    ACE_FILE_Connector con;
#if !defined(UNDER_CE)
    int ret = con.connect(m_outfile, ACE_FILE_Addr(filename.c_str()), 0, 
                          ACE_Addr::sap_any, 0, O_RDWR | O_CREAT | O_TRUNC);
#else
    int ret = con.connect(m_outfile, ACE_FILE_Addr(filename.c_str()),
        0, ACE_Addr::sap_any, 0, O_RDWR | O_CREAT, FILE_SHARE_READ | FILE_SHARE_WRITE);
#endif
    if(ret < 0)
    {
        closeStream(m_hMp3Stream);
        m_hMp3Stream = 0;
    }

    return ret >= 0;
}

void LameMP3::Close()
{
    if(!m_hMp3Stream)
        return;

    DWORD outLen = 0;
    int err = deinitStream(m_hMp3Stream, &m_out_mp3data[0], &outLen);
    assert(err == 0);
    if(outLen)
        m_outfile.send((const char*)&m_out_mp3data[0], outLen);

    ACE_TString filename = GetFileName();
    
    closeStream(m_hMp3Stream);
    
    m_outfile.close();
    
    if(filename.length())
    {
#if defined(UNICODE)
        //LAME doesn't support Unicode, so we just convert to locale
        //and hope for the best
        ACE_CString ascii = UnicodeToLocal(filename.c_str());
        err = writeInfoTag(m_hMp3Stream, ascii.c_str());
#else
        err = writeInfoTag(m_hMp3Stream, filename.c_str());
#endif
    }
    m_out_mp3data.resize(0);
    m_hMp3Stream = 0;
}

ACE_TString LameMP3::GetFileName() const
{
    ACE_FILE_Addr fileaddr;
    m_outfile.get_local_addr(fileaddr);
    return fileaddr.get_path_name()? fileaddr.get_path_name() : ACE_TString();
}

int LameMP3::Encode(const short* samples, int samples_count)
{
    assert(m_hMp3Stream);
    assert(m_out_mp3data.size());

    int encoded = 0;
    int processed = 0;
    while(processed < samples_count)
    {
        DWORD outLen = 0;
        short* samples_ptr = const_cast<short*>(&samples[processed*m_dwChannels]);
        int process = m_dwInSamples;
        if(processed*m_dwChannels + process > samples_count*m_dwChannels)
            process = samples_count*m_dwChannels - processed*m_dwChannels;
        int err = encChunk(m_hMp3Stream, process, samples_ptr, 
                           &m_out_mp3data[0], &outLen);

        processed += (process/m_dwChannels);

        assert(outLen<m_out_mp3data.size());
        assert(err == BE_ERR_SUCCESSFUL);
        if(err != BE_ERR_SUCCESSFUL)
            return -1;

        if(outLen>0)
        {
            m_outfile.send((const char*)&m_out_mp3data[0], outLen);
            encoded += outLen;
        }
    }
    return encoded;
}

