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

package dk.bearware.data;

public class Preferences {

	//duplicates of pref_connection.xml (isn't there an easier way to do this??)
	public static final String 
			PREF_GENERAL_NICKNAME = "nickname_text",
			PREF_GENERAL_SHOWUSERNAMES = "showusernames_checkbox",
			PREF_GENERAL_PUBLICSERVERS = "showpublicservers_checkbox",
			PREF_GENERAL_BEARWARE_CHECKED = "bearwareid_checkbox",
		    PREF_GENERAL_BEARWARE_USERNAME = "bearware_username",
            PREF_GENERAL_BEARWARE_TOKEN = "bearware_token";
	public static final String
			PREF_SOUNDSYSTEM_MEDIAFILE_VOLUME = "mediafilevolume_seekbar",
			PREF_SOUNDSYSTEM_VOICEACTIVATION_LEVEL = "voice_activation_level",
			PREF_SOUNDSYSTEM_MASTERVOLUME = "mastervolume",
			PREF_SOUNDSYSTEM_MICROPHONEGAIN = "microphonegain",
			PREF_SOUNDSYSTEM_MUTE_ON_TRANSMISSION = "mute_speakers_on_tx_checkbox",
			PREF_SOUNDSYSTEM_SPEAKERPHONE = "speakerphone_checkbox",
			PREF_SOUNDSYSTEM_BLUETOOTH_HEADSET = "bluetooth_headset_checkbox",
			PREF_SOUNDSYSTEM_VOICEPROCESSING = "voiceprocessing_checkbox";
	public static final String
			PREF_SUB_TEXTMESSAGE = "sub_txtmsg_checkbox",
			PREF_SUB_CHANMESSAGE = "sub_chanmsg_checkbox",
			PREF_SUB_BCAST_MESSAGES = "sub_bcastmsg_checkbox",
			PREF_SUB_VOICE = "sub_voice_checkbox",
			PREF_SUB_VIDCAP = "sub_video_checkbox",
			PREF_SUB_DESKTOP = "sub_desktop_checkbox",
			PREF_SUB_MEDIAFILE = "sub_mediafile_checkbox";
}
