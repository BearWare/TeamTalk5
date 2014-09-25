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

#include "stdafx.h"
#include "HttpRequest.h"

CHttpRequest::CHttpRequest(const CString& url)
: m_pIXMLHttpRequest(NULL)
, m_state(HTTP_CLOSED)
{
    HRESULT hr;
    //hr = CoInitialize(NULL); 

    // Create XMLHttpRequest object and initialize m_pIXMLHttpRequest.
    hr = CoCreateInstance(CLSID_XMLHTTPRequest, NULL, CLSCTX_INPROC, 
        IID_IXMLHttpRequest, (void **)&m_pIXMLHttpRequest);

    if(FAILED(hr))
        return;

    VARIANT vAsync;
    vAsync.vt = VT_BOOL;
    vAsync.boolVal = VARIANT_TRUE;

    VARIANT vUser;
    vUser.vt = VT_BSTR;
    vUser.bstrVal = NULL;

    VARIANT vPassword;
    vPassword.vt = VT_BSTR;
    vPassword.bstrVal = NULL;

    BSTR str = url.AllocSysString();
    hr = m_pIXMLHttpRequest->open(_T("POST"), str, vAsync, vUser, vPassword);
    ASSERT(hr == S_OK);
}

CHttpRequest::~CHttpRequest()
{
    HRESULT hr;
    if(m_pIXMLHttpRequest)
    {
        //hr = m_pIXMLHttpRequest->abort();
        //ASSERT(hr == S_OK);
        hr = m_pIXMLHttpRequest->Release();
        ASSERT(hr == S_OK);
    }
    //CoUninitialize();
}

BOOL CHttpRequest::SendReady()
{
    if(!m_pIXMLHttpRequest || m_state != HTTP_CLOSED)
        return FALSE;
    HRESULT hr;

    long ret = 0;
    hr = m_pIXMLHttpRequest->get_readyState(&ret);
    ASSERT(hr == S_OK);
    if(hr == S_OK && ret == 1)
    {
        m_state = HTTP_OPENED;
        return TRUE;
    }

    return FALSE;
}

void CHttpRequest::Send(const CString& szRequest)
{
    if(!m_pIXMLHttpRequest || m_state != HTTP_OPENED)
        return;
    HRESULT hr;

    VARIANT vRequest;
    vRequest.vt = VT_BSTR;
    vRequest.bstrVal = szRequest.AllocSysString();

    long ret = 0;
    hr = m_pIXMLHttpRequest->get_readyState(&ret);
    ASSERT(hr == S_OK);
    if(hr == S_OK && ret == 1)
    {
        hr = m_pIXMLHttpRequest->send(vRequest);
        ASSERT(hr == S_OK);
        m_state = HTTP_SENT;
    }
}

BOOL CHttpRequest::ResponseReady()
{
    if(!m_pIXMLHttpRequest || m_state != HTTP_SENT)
        return FALSE;
    HRESULT hr;

    long ret = 0;
    hr = m_pIXMLHttpRequest->get_readyState(&ret);
    ASSERT(hr == S_OK);
    if(ret == 4)
    {
        m_state = HTTP_RESPONSE;
        return TRUE;
    }
    return FALSE;
}

CString CHttpRequest::GetResponse()
{
    if(!m_pIXMLHttpRequest || m_state != HTTP_RESPONSE)
        return CString();
    HRESULT hr;

    BSTR response;
    hr = m_pIXMLHttpRequest->get_responseText(&response);
    ASSERT(hr == S_OK);
    if(hr == S_OK)
    {
        m_state = HTTP_CLOSED;
         return response;
    }
    return CString();
}
