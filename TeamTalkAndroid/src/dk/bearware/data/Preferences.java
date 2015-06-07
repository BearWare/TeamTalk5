/*
 * Copyright (c) 2005-2014, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

package dk.bearware.data;

public class Preferences {

	public static final String 
			PREF_GENERAL_NICKNAME = "nickname_text";
	
	public static final String //duplicates of pref_connection.xml (isn't there an easier way to do this??) 
			PREF_SUB_TEXTMESSAGE = "sub_txtmsg_checkbox",
			PREF_SUB_CHANMESSAGE = "sub_chanmsg_checkbox",
			PREF_SUB_BCAST_MESSAGES = "sub_bcastmsg_checkbox",
			PREF_SUB_VOICE = "sub_voice_checkbox",
			PREF_SUB_VIDCAP = "sub_video_checkbox",
			PREF_SUB_DESKTOP = "sub_desktop_checkbox",
			PREF_SUB_MEDIAFILE = "sub_mediafile_checkbox";
}
