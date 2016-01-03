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

public interface Codec
{
    public static final int NO_CODEC                    = 0;
    public static final int SPEEX_CODEC                 = 1;
    public static final int SPEEX_VBR_CODEC             = 2;
    public static final int OPUS_CODEC                  = 3;
    public static final int WEBM_VP8_CODEC              = 128;
}
