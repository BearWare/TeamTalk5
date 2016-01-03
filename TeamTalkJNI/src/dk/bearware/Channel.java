/*
 * Copyright (c) 2005-2016, BearWare.dk
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

package dk.bearware;

public class Channel
{
    public int nParentID = 0;
    public int nChannelID = 0;
    public String szName = "";
    public String szTopic = "";
    public String szPassword = "";
    public boolean bPassword = false;
    public int uChannelType = ChannelType.CHANNEL_DEFAULT;
    public int nUserData = 0;
    public long nDiskQuota = 0;
    public String szOpPassword = "";
    public int nMaxUsers = 0;
    public AudioCodec audiocodec = new AudioCodec();
    public AudioConfig audiocfg = new AudioConfig();
	//TODO: 'transmitUsers'
    public Channel() {
    }
    public Channel(boolean default_audiocodec, boolean default_audiocfg) {
        audiocodec = new AudioCodec(default_audiocodec);
        audiocfg = new AudioConfig(default_audiocfg);
    }
}
