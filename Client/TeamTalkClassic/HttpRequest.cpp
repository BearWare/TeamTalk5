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

    BSTR str = url.AllocSysString();
    hr = m_pIXMLHttpRequest->open(_T("POST"), str, _variant_t(VARIANT_TRUE), _variant_t(""), _variant_t(""));
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
    hr = m_pIXMLHttpRequest->setRequestHeader(_T("Content-Type"), _T("text/xml"));
    ASSERT(SUCCEEDED(hr));

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

int CHttpRequest::Status()
{
    long lStatus = -1;
    if(!m_pIXMLHttpRequest || m_state != HTTP_RESPONSE)
        return lStatus;

    m_pIXMLHttpRequest->get_status(&lStatus);

    return lStatus;
}
