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

#include "ServerXML.h"

#include <teamtalk/server/ServerNode.h>

bool LoadConfig(teamtalk::ServerXML& xmlSettings, const ACE_TString& cfgfile);
void RunWizard(teamtalk::ServerXML& xmlSettings);

bool ReadServerProperties(teamtalk::ServerXML& xmlSettings, teamtalk::ServerSettings& properties,
                          teamtalk::statchannels_t& channels);

#if defined(ENABLE_TEAMTALKPRO)
bool SetupEncryption(teamtalk::ServerNode& servernode, teamtalk::ServerXML& xmlSettings);
#endif

bool ConfigureServer(teamtalk::ServerNode& servernode, const teamtalk::ServerSettings& properties,
                     const teamtalk::statchannels_t& channels);

void ConvertChannels(const teamtalk::serverchannel_t& root, teamtalk::statchannels_t& channels, bool onlystatic = false);

void MakeStaticChannels(teamtalk::ServerNode& servernode, const teamtalk::statchannels_t& channels);
