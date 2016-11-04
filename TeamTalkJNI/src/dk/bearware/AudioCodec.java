/*
 * Copyright (c) 2005-2016, BearWare.dk
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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

package dk.bearware;

public class AudioCodec
{
    public int nCodec;
    public SpeexCodec speex = new SpeexCodec(true);
    public SpeexVBRCodec speex_vbr = new SpeexVBRCodec(true);
    public OpusCodec opus = new OpusCodec(true);
    public AudioCodec() {
        nCodec = Codec.NO_CODEC;
    }
    public AudioCodec(boolean set_defaults) {
        if(!set_defaults)
            return;
        nCodec = Codec.OPUS_CODEC;
        opus = new OpusCodec(true);
    }
}
