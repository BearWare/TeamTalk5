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

#pragma once

#include <msxml.h>

// CHttpRequest

enum HttpState
{
    HTTP_CLOSED,
    HTTP_OPENED,
    HTTP_SENT,
    HTTP_RESPONSE
};

class CHttpRequest
{
public:
	CHttpRequest(const CString& url);
	~CHttpRequest();

    BOOL SendReady();
    void Send(const CString& szRequest);

    BOOL ResponseReady();
    CString GetResponse();

private:
    IXMLHttpRequest* m_pIXMLHttpRequest;
    HttpState m_state;
};


