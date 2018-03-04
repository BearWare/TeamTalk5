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

#if !defined(SERVERDEFS_H)
#define SERVERDEFS_H

#include <ace/Log_Msg.h>

#define TT_ERROR(TXT) ACE_DEBUG((LM_ERROR, ACE_TEXT("%D %s\n"), TXT))
#define TT_TRACE(TXT) ACE_DEBUG((LM_TRACE, ACE_TEXT("%D %s\n"), TXT))
#define TT_LOG(TXT)  ACE_DEBUG((LM_INFO, ACE_TEXT("%D %s\n"), TXT))
#define TT_SYSLOG(TXT) ACE_DEBUG((LM_ERROR, ACE_TEXT("%s\n"), TXT))

#endif
