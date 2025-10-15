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

#include "mixer.h"

#include <tchar.h>
#include <windows.h>
#include <mmsystem.h>

#include <cstring>
#include <utility>

bool MixerWaveOut(int nWaveDeviceID, int mixerMask, InOutValue &inoutValue)
{
    bool result = false;
    HMIXER hTmpMixer = nullptr;
    DWORD mask = 0;

	MMRESULT res = mixerOpen(&hTmpMixer, nWaveDeviceID, 0, 0, MIXER_OBJECTF_WAVEOUT);

    if (res != MMSYSERR_NOERROR)
        return false;

    auto hMixer = (HMIXEROBJ)hTmpMixer;

    MIXERLINE ml = {};
    ml.cbStruct = sizeof(MIXERLINE);
    ml.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;

    if (mixerGetLineInfo(hMixer, &ml, MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE)
        == MMSYSERR_NOERROR) {
        int const num_devices = ml.cConnections;
        bool const skip = (mixerMask & MIXER_WAVEOUT_MASTER) == MIXER_WAVEOUT_MASTER;
        int index = 0;
        for (index = 0; index < num_devices && !skip; index++) 
        { 
            ml.dwSource = index;
            res = mixerGetLineInfo(hMixer, &ml, MIXER_GETLINEINFOF_SOURCE); 
            if(ml.dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT && ( (mixerMask & MIXER_WAVEOUT_WAVE) == MIXER_WAVEOUT_WAVE))
                break; 
            if(ml.dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE && ( (mixerMask & MIXER_WAVEOUT_MICROPHONE) == MIXER_WAVEOUT_MICROPHONE))
                break;
        }
        if(index>=num_devices)
            goto end;

        MIXERCONTROL mc = {}; 

        MIXERLINECONTROLS mlc = {};
        mlc.cbStruct = sizeof(MIXERLINECONTROLS);
        mlc.dwLineID = ml.dwLineID;
        if((mixerMask & MIXER_WAVEOUT_MUTE) != 0)
            mlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
        if((mixerMask & MIXER_WAVEOUT_VOLUME) != 0)
            mlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;

        mlc.cControls = 1; 
        mlc.cbmxctrl = sizeof(MIXERCONTROL); 
        mlc.pamxctrl = &mc; 

        mask = MIXER_GETLINECONTROLSF_ONEBYTYPE;

        if( (mixerMask & MIXER_WAVEOUT_MASTER) == MIXER_WAVEOUT_MASTER)
            mask |= MIXER_OBJECTF_HMIXER;

        MMRESULT res = mixerGetLineControls(hMixer, &mlc, mask);
        if (res == MMSYSERR_NOERROR) {
            UINT const xxx = MIXERCONTROL_CONTROLTYPE_MUX;
            if(    (mixerMask & MIXER_WAVEOUT_MUTE) != 0 )
            {
                MIXERCONTROLDETAILS_BOOLEAN mcdBool; 

                MIXERCONTROLDETAILS mcd = {}; 
                mcd.cbStruct = sizeof(MIXERCONTROLDETAILS); 
                mcd.dwControlID = mc.dwControlID; 
                mcd.cChannels = 1; 
                mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
                mcd.paDetails = &mcdBool;

                if((mixerMask & MIXER_WAVEOUT_GET) != 0)
                {
                    res = mixerGetControlDetails(hMixer, &mcd, MIXER_GETCONTROLDETAILSF_VALUE);
                    inoutValue.value = mcdBool.fValue;
                    result = true;
                } else if ((mixerMask & MIXER_WAVEOUT_SET) != 0) {
                    mcdBool.fValue = inoutValue.value;
                    res = mixerSetControlDetails(hMixer, &mcd, MIXER_SETCONTROLDETAILSF_VALUE);
                    result = true;
                }
            }

            if((mixerMask & MIXER_WAVEOUT_VOLUME) != 0)
            {
                MIXERCONTROLDETAILS_UNSIGNED mcdUnsigned;

                MIXERCONTROLDETAILS mcd = {}; 
                mcd.cbStruct = sizeof(MIXERCONTROLDETAILS); 
                mcd.dwControlID = mc.dwControlID; 
                mcd.cChannels = 1; 
                mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
                mcd.paDetails = &mcdUnsigned;

                if((mixerMask & MIXER_WAVEOUT_GET) != 0)
                {
                    res = mixerGetControlDetails(hMixer, &mcd, MIXER_GETCONTROLDETAILSF_VALUE); 
                    inoutValue.value = mcdUnsigned.dwValue;
                    result = true;
                } else if ((mixerMask & MIXER_WAVEOUT_SET) != 0) {
                    mcdUnsigned.dwValue = inoutValue.value;
                    res = mixerSetControlDetails(hMixer, &mcd, MIXER_SETCONTROLDETAILSF_VALUE); 
                    result = true;
                }
            }
        }
    }

end:

    // close the mixer device 
    mixerClose(hTmpMixer); 
    return result;
}

bool MixerWaveIn(int nWaveDeviceID, int mixerMask, InOutValue &inoutValue)
{
    bool result = false;
    HMIXER hTmpMixer = nullptr;
    DWORD const mask = 0;

	MMRESULT res = mixerOpen(&hTmpMixer, nWaveDeviceID, 0, 0, MIXER_OBJECTF_WAVEIN);

    if (res != MMSYSERR_NOERROR)
        return false;

    auto hMixer = (HMIXEROBJ)hTmpMixer;

    MIXERLINE ml = {};
    ml.cbStruct = sizeof(MIXERLINE);
    ml.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;

    if (mixerGetLineInfo(hMixer, &ml, MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE)
        == MMSYSERR_NOERROR) {
        if ((mixerMask & (MIXER_WAVEIN_VOLUME | MIXER_WAVEIN_BOOST | MIXER_WAVEIN_MUTE)) != 0) {
            DWORD const num_devices = ml.cConnections;
            DWORD dwCompIndex = 0;
            DWORD dwComponentTypeID = 0;
            for (dwCompIndex = 0; dwCompIndex < num_devices; dwCompIndex++) 
            { 
                ml.dwSource = dwCompIndex;
                if(mixerGetLineInfo(hMixer, &ml, MIXER_GETLINEINFOF_SOURCE) != MMSYSERR_NOERROR)
                    goto end;

                switch(ml.dwComponentType)
                {
                case MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE :
                    if( (mixerMask & MIXER_WAVEIN_MICROPHONE) == MIXER_WAVEIN_MICROPHONE)
                        dwComponentTypeID = ml.dwComponentType;
                    break;
                case MIXERLINE_COMPONENTTYPE_SRC_LINE :
                    if( (mixerMask & MIXER_WAVEIN_LINEIN) == MIXER_WAVEIN_LINEIN)
                        dwComponentTypeID = ml.dwComponentType;
                    break;
                case MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT :
                    if( (mixerMask & MIXER_WAVEIN_WAVEOUT) == MIXER_WAVEIN_WAVEOUT)
                        dwComponentTypeID = ml.dwComponentType;
                    break;
                }

                //user is search by index
                if ((mixerMask & MIXER_WAVEIN_BYINDEX) == MIXER_WAVEIN_BYINDEX
                    && std::cmp_equal(dwCompIndex, inoutValue.value))
                    dwComponentTypeID = ml.dwComponentType;

                if(dwComponentTypeID != 0)
                    break;    //the device has been found
            }
            if(dwCompIndex>=num_devices)
                goto end;

            MIXERCONTROL mc = {}; 

            MIXERLINECONTROLS mlc = {}; 
            mlc.cbStruct = sizeof(MIXERLINECONTROLS); 
            mlc.dwLineID = ml.dwLineID;
            mlc.cControls = 1; 
            mlc.cbmxctrl = sizeof(MIXERCONTROL); 
            mlc.pamxctrl = &mc;

            if( (mixerMask & MIXER_WAVEIN_VOLUME) == MIXER_WAVEIN_VOLUME)
            {
                mlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
                if(mixerGetLineControls(hMixer, &mlc, MIXER_GETLINECONTROLSF_ONEBYTYPE) != MMSYSERR_NOERROR)
                    goto end;
            }
            else if( (mixerMask & MIXER_WAVEIN_BOOST) == MIXER_WAVEIN_BOOST)
            {
                //TODO: this is not the correct way to query. Here we just take the
                //first control and claim it's boost.
                if(mixerGetLineControls(hMixer, &mlc, MIXER_GETLINECONTROLSF_ALL) != MMSYSERR_NOERROR)
                    goto end;
            }
            else if( (mixerMask & MIXER_WAVEIN_MUTE) == MIXER_WAVEIN_MUTE)
            {
                mlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
                if(mixerGetLineControls(hMixer, &mlc, MIXER_GETLINECONTROLSF_ONEBYTYPE) != MMSYSERR_NOERROR)
                    goto end;
            }
            else
                goto end;

            if ((mixerMask & (MIXER_WAVEIN_BOOST | MIXER_WAVEIN_MUTE)) != 0) {
                MIXERCONTROLDETAILS_BOOLEAN mcdBool = {}; 

                MIXERCONTROLDETAILS mcd = {}; 
                mcd.cbStruct = sizeof(MIXERCONTROLDETAILS); 
                mcd.dwControlID = mc.dwControlID; 
                mcd.cChannels = 1; 
                mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
                mcd.paDetails = &mcdBool;

                if ((mixerMask & MIXER_WAVEIN_GET) != 0) {
                    res = mixerGetControlDetails(hMixer, &mcd, MIXER_GETCONTROLDETAILSF_VALUE);
                    inoutValue.value = mcdBool.fValue;
                    result = true;
                } else if ((mixerMask & MIXER_WAVEIN_SET) != 0) {
                    mcdBool.fValue = inoutValue.value;
                    res = mixerSetControlDetails(hMixer, &mcd, MIXER_SETCONTROLDETAILSF_VALUE);
                    result = true;
                }
            }
            if((mixerMask & MIXER_WAVEIN_VOLUME) == MIXER_WAVEIN_VOLUME)
            {
                MIXERCONTROLDETAILS_UNSIGNED mcdUnsigned;

                MIXERCONTROLDETAILS mcd = {}; 
                mcd.cbStruct = sizeof(MIXERCONTROLDETAILS); 
                mcd.dwControlID = mc.dwControlID; 
                mcd.cChannels = 1; 
                mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
                mcd.paDetails = &mcdUnsigned;

                if ((mixerMask & MIXER_WAVEIN_GET) != 0) {
                    res = mixerGetControlDetails(hMixer, &mcd, MIXER_GETCONTROLDETAILSF_VALUE); 
                    inoutValue.value = mcdUnsigned.dwValue; 
                    result = true;
                } else if ((mixerMask & MIXER_WAVEIN_SET) != 0) {
                    mcdUnsigned.dwValue = inoutValue.value;
                    res = mixerSetControlDetails(hMixer, &mcd, MIXER_SETCONTROLDETAILSF_VALUE); 
                    result = true;
                }
            }
        } else {
            MIXERCONTROL mc = {}; 

            MIXERLINECONTROLS mlc = {}; 
            mlc.cbStruct = sizeof(MIXERLINECONTROLS); 
            mlc.dwLineID = ml.dwLineID;
            mlc.cControls = 1; 
            mlc.cbmxctrl = sizeof(MIXERCONTROL); 
            mlc.pamxctrl = &mc;
            mlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MIXER;

            MMRESULT res = mixerGetLineControls(hMixer, &mlc, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE);
            if(res != MMSYSERR_NOERROR)
            {
                mlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUX;
                res = mixerGetLineControls(hMixer, &mlc, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE);
            }

            if(res != MMSYSERR_NOERROR)
                goto end;

            //get the number of mixer devices
            if( (mixerMask & MIXER_WAVEIN_GETCOUNT) == MIXER_WAVEIN_GETCOUNT)
            {
                inoutValue.value = mc.cMultipleItems;
                result = true;
                goto end;
            }

            MIXERCONTROLDETAILS_LISTTEXT mcdString[100] = {};

            MIXERCONTROLDETAILS mcd = {}; 
            mcd.cbStruct = sizeof(MIXERCONTROLDETAILS); 
            mcd.dwControlID = mc.dwControlID; 
            mcd.cChannels = 1; 
            mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_LISTTEXT); 
            mcd.paDetails = mcdString;
            mcd.cMultipleItems = mc.cMultipleItems;

            if(mixerGetControlDetails(hMixer, &mcd, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_LISTTEXT) != MMSYSERR_NOERROR)
                goto end;

            DWORD dwComponentTypeID = 0;
            DWORD dwCompIndex = 0;
            for(dwCompIndex=0;dwCompIndex<mcd.cMultipleItems;dwCompIndex++)
            {
                // get the line information
                MIXERLINE mxl;
                mxl.cbStruct = sizeof(MIXERLINE);
                mxl.dwLineID = mcdString[dwCompIndex].dwParam1;
                if(mixerGetLineInfo(hMixer, &mxl, MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_LINEID) == MMSYSERR_NOERROR)
                {
                    switch(mxl.dwComponentType)
                    {
                    case MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE :
                        if( (mixerMask & MIXER_WAVEIN_MICROPHONE) == MIXER_WAVEIN_MICROPHONE)
                            dwComponentTypeID = mxl.dwComponentType;
                        break;
                    case MIXERLINE_COMPONENTTYPE_SRC_LINE :
                        if( (mixerMask & MIXER_WAVEIN_LINEIN) == MIXER_WAVEIN_LINEIN)
                            dwComponentTypeID = mxl.dwComponentType;
                        break;
                    case MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT :
                        if( (mixerMask & MIXER_WAVEIN_WAVEOUT) == MIXER_WAVEIN_WAVEOUT)
                            dwComponentTypeID = mxl.dwComponentType;
                        break;
                    }
                }

                //user is search by index
                if ((mixerMask & MIXER_WAVEIN_BYINDEX) == MIXER_WAVEIN_BYINDEX
                    && std::cmp_equal(dwCompIndex, inoutValue.value)) {
                    dwComponentTypeID = mxl.dwComponentType;
                    if( (mixerMask & MIXER_WAVEIN_NAME) == MIXER_WAVEIN_NAME)
                    {
                        _tcsncpy(inoutValue.name, mxl.szName, sizeof(inoutValue.name)/sizeof(TCHAR));
                        result = true;
                        goto end;
                    }
                }

                if(dwComponentTypeID != 0)
                    break;    //the device has been found
            }
            if(dwCompIndex>=mcd.cMultipleItems)
                goto end;

            MIXERCONTROLDETAILS_BOOLEAN mcdBool[100] = {}; 
            mcd.cbStruct = sizeof(MIXERCONTROLDETAILS); 
            mcd.dwControlID = mc.dwControlID; 
            mcd.cChannels = 1; 
            mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN); 
            mcd.paDetails = mcdBool;
            mcd.cMultipleItems = mc.cMultipleItems;

            if(mixerGetControlDetails(hMixer, &mcd, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE) != MMSYSERR_NOERROR)
                goto end;

            if ((mixerMask & MIXER_WAVEIN_GET) != 0) {
                inoutValue.value = mcdBool[dwCompIndex].fValue;
                result = true;
            }

            if ((mixerMask & MIXER_WAVEIN_SET) != 0) {
                if(mlc.dwControlType == MIXERCONTROL_CONTROLTYPE_MUX)
                    memset(mcdBool, 0, sizeof(mcdBool));

                mcdBool[dwCompIndex].fValue = 1;
                res = mixerSetControlDetails(hMixer, &mcd, MIXER_SETCONTROLDETAILSF_VALUE);
                result = (res == MMSYSERR_NOERROR);
            }
        }
    }

end:

    // close the mixer device 
    mixerClose(hTmpMixer); 
    return result;
}

int MixerGetCount()
{
    return mixerGetNumDevs();
}

bool MixerGetName(int nMixerIndex, TCHAR name[MIXER_STR_LEN])
{
    MIXERCAPS caps = {};
	if(mixerGetDevCaps(nMixerIndex, &caps, sizeof(caps)) == MMSYSERR_NOERROR)
	{
		_tcsncpy(name, caps.szPname, MIXER_STR_LEN);
		return true;
	}
	return false;
}

bool MixerGetWaveInName(int nWaveDeviceID, TCHAR name[MIXER_STR_LEN])
{
    bool const result = false;
    HMIXER hMixer = nullptr;
    DWORD const mask = 0;
    MIXERCAPS caps = {};

    MMRESULT res = mixerOpen(&hMixer, nWaveDeviceID, 0, 0, MIXER_OBJECTF_WAVEIN);

    if(res != MMSYSERR_NOERROR)
        return false;

    res = mixerGetDevCaps((UINT_PTR)hMixer, reinterpret_cast<LPMIXERCAPS>(&caps), sizeof(caps));
    if(res != MMSYSERR_NOERROR)
    {
        mixerClose(hMixer);
        return false;
    }
    _tcsncpy(name, caps.szPname, MIXER_STR_LEN);

    mixerClose(hMixer);
    return true;
}

bool MixerGetWaveOutName(int nWaveDeviceID, TCHAR name[MIXER_STR_LEN])
{
    HMIXER hMixer = nullptr;
    MIXERCAPS caps = {};

	MMRESULT res = mixerOpen(&hMixer, nWaveDeviceID, 0, 0, MIXER_OBJECTF_WAVEOUT);

	if(res != MMSYSERR_NOERROR)
		return false;

    res = mixerGetDevCaps((UINT_PTR)hMixer, reinterpret_cast<LPMIXERCAPS>(&caps), sizeof(caps));
	if(res != MMSYSERR_NOERROR)
    {
        mixerClose(hMixer);
    return false;
    }

    _tcsncpy(name, caps.szPname, MIXER_STR_LEN);

    mixerClose(hMixer);

    return true;
}
