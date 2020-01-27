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

#include "WinFirewall.h"
#include <assert.h>
#include <Strsafe.h>
#include <VersionHelpers.h>

HRESULT CoCreateInstanceAsAdmin(HWND hwnd, REFCLSID rclsid, REFIID riid, __out void ** ppv)
{
    BIND_OPTS3 bo;
    WCHAR  wszCLSID[50];
    WCHAR  wszMonikerName[300];

    StringFromGUID2(rclsid, wszCLSID, sizeof(wszCLSID)/sizeof(wszCLSID[0])); 
    HRESULT hr = StringCchPrintf(wszMonikerName, sizeof(wszMonikerName)/sizeof(wszMonikerName[0]), L"Elevation:Administrator!new:%s", wszCLSID);
    if (FAILED(hr))
        return hr;
    memset(&bo, 0, sizeof(bo));
    bo.cbStruct = sizeof(bo);
    bo.hwnd = hwnd;
    bo.dwClassContext  = CLSCTX_LOCAL_SERVER;
    return CoGetObject(wszMonikerName, &bo, riid, ppv);
}

/* Code from: http://msdn.microsoft.com/en-us/library/aa364726(VS.85).aspx */

HRESULT WindowsFirewallInitialize(OUT INetFwProfile** fwProfile, BOOL bInvokeUAE)
{
    HRESULT hr = S_OK;
    INetFwMgr* fwMgr = NULL;
    INetFwPolicy* fwPolicy = NULL;

    assert(fwProfile != NULL);

    *fwProfile = NULL;

    if(bInvokeUAE)
        hr = CoCreateInstanceAsAdmin(NULL, __uuidof(NetFwMgr), 
        __uuidof(INetFwMgr), (void**)&fwMgr);
    else
    {
        // Create an instance of the firewall settings manager.
        hr = CoCreateInstance(
            __uuidof(NetFwMgr),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(INetFwMgr),
            (void**)&fwMgr
            );
    }
    if (FAILED(hr))
    {
        MYTRACE(ACE_TEXT("CoCreateInstance failed: 0x%08lx\n"), hr);
        goto error;
    }

    // Retrieve the local firewall policy.
    hr = fwMgr->get_LocalPolicy(&fwPolicy);
    if (FAILED(hr))
    {
        MYTRACE(ACE_TEXT("get_LocalPolicy failed: 0x%08lx\n"), hr);
        goto error;
    }

    // Retrieve the firewall profile currently in effect.
    hr = fwPolicy->get_CurrentProfile(fwProfile);
    if (FAILED(hr))
    {
        MYTRACE(ACE_TEXT("get_CurrentProfile failed: 0x%08lx\n"), hr);
        goto error;
    }

error:

    // Release the local firewall policy.
    if (fwPolicy != NULL)
    {
        fwPolicy->Release();
    }

    // Release the firewall settings manager.
    if (fwMgr != NULL)
    {
        fwMgr->Release();
    }

    return hr;
}


void WindowsFirewallCleanup(IN INetFwProfile* fwProfile)
{
    // Release the firewall profile.
    if (fwProfile != NULL)
    {
        fwProfile->Release();
    }
}


HRESULT WindowsFirewallIsOn(IN INetFwProfile* fwProfile, OUT BOOL* fwOn)
{
    HRESULT hr = S_OK;
    VARIANT_BOOL fwEnabled;

    assert(fwProfile != NULL);
    assert(fwOn != NULL);

    *fwOn = FALSE;

    // Get the current state of the firewall.
    hr = fwProfile->get_FirewallEnabled(&fwEnabled);
    if (FAILED(hr))
    {
        MYTRACE(ACE_TEXT("get_FirewallEnabled failed: 0x%08lx\n"), hr);
        goto error;
    }

    // Check to see if the firewall is on.
    if (fwEnabled != VARIANT_FALSE)
    {
        *fwOn = TRUE;
        MYTRACE(ACE_TEXT("The firewall is on.\n"));
    }
    else
    {
        MYTRACE(ACE_TEXT("The firewall is off.\n"));
    }

error:

    return hr;
}


HRESULT WindowsFirewallTurnOn(IN INetFwProfile* fwProfile)
{
    HRESULT hr = S_OK;
    BOOL fwOn;

    assert(fwProfile != NULL);

    // Check to see if the firewall is off.
    hr = WindowsFirewallIsOn(fwProfile, &fwOn);
    if (FAILED(hr))
    {
        MYTRACE(ACE_TEXT("WindowsFirewallIsOn failed: 0x%08lx\n"), hr);
        goto error;
    }

    // If it is, turn it on.
    if (!fwOn)
    {
        // Turn the firewall on.
        hr = fwProfile->put_FirewallEnabled(VARIANT_TRUE);
        if (FAILED(hr))
        {
            MYTRACE(ACE_TEXT("put_FirewallEnabled failed: 0x%08lx\n"), hr);
            goto error;
        }

        MYTRACE(ACE_TEXT("The firewall is now on.\n"));
    }

error:

    return hr;
}


HRESULT WindowsFirewallTurnOff(IN INetFwProfile* fwProfile)
{
    HRESULT hr = S_OK;
    BOOL fwOn;

    assert(fwProfile != NULL);

    // Check to see if the firewall is on.
    hr = WindowsFirewallIsOn(fwProfile, &fwOn);
    if (FAILED(hr))
    {
        MYTRACE(ACE_TEXT("WindowsFirewallIsOn failed: 0x%08lx\n"), hr);
        goto error;
    }

    // If it is, turn it off.
    if (fwOn)
    {
        // Turn the firewall off.
        hr = fwProfile->put_FirewallEnabled(VARIANT_FALSE);
        if (FAILED(hr))
        {
            MYTRACE(ACE_TEXT("put_FirewallEnabled failed: 0x%08lx\n"), hr);
            goto error;
        }

        MYTRACE(ACE_TEXT("The firewall is now off.\n"));
    }

error:

    return hr;
}


HRESULT WindowsFirewallAppIsEnabled(
    IN INetFwProfile* fwProfile,
    IN const wchar_t* fwProcessImageFileName,
    OUT BOOL* fwAppEnabled
    )
{
    HRESULT hr = S_OK;
    BSTR fwBstrProcessImageFileName = NULL;
    VARIANT_BOOL fwEnabled;
    INetFwAuthorizedApplication* fwApp = NULL;
    INetFwAuthorizedApplications* fwApps = NULL;

    assert(fwProfile != NULL);
    assert(fwProcessImageFileName != NULL);
    assert(fwAppEnabled != NULL);

    *fwAppEnabled = FALSE;

    // Retrieve the authorized application collection.
    hr = fwProfile->get_AuthorizedApplications(&fwApps);
    if (FAILED(hr))
    {
        MYTRACE(ACE_TEXT("get_AuthorizedApplications failed: 0x%08lx\n"), hr);
        goto error;
    }

    // Allocate a BSTR for the process image file name.
    fwBstrProcessImageFileName = SysAllocString(fwProcessImageFileName);
    if (fwBstrProcessImageFileName == NULL)
    {
        hr = E_OUTOFMEMORY;
        MYTRACE(ACE_TEXT("SysAllocString failed: 0x%08lx\n"), hr);
        goto error;
    }

    // Attempt to retrieve the authorized application.
    hr = fwApps->Item(fwBstrProcessImageFileName, &fwApp);
    if (SUCCEEDED(hr))
    {
        // Find out if the authorized application is enabled.
        hr = fwApp->get_Enabled(&fwEnabled);
        if (FAILED(hr))
        {
            MYTRACE(ACE_TEXT("get_Enabled failed: 0x%08lx\n"), hr);
            goto error;
        }

        if (fwEnabled != VARIANT_FALSE)
        {
            // The authorized application is enabled.
            *fwAppEnabled = TRUE;

            MYTRACE(ACE_TEXT(
                "Authorized application %lS is enabled in the firewall.\n"),
                fwProcessImageFileName
                );
        }
        else
        {
            MYTRACE(ACE_TEXT(
                "Authorized application %lS is disabled in the firewall.\n"),
                fwProcessImageFileName
                );
        }
    }
    else
    {
        // The authorized application was not in the collection.
        hr = S_OK;

        MYTRACE(ACE_TEXT(
            "Authorized application %lS is disabled in the firewall.\n"),
            fwProcessImageFileName
            );
    }

error:

    // Free the BSTR.
    SysFreeString(fwBstrProcessImageFileName);

    // Release the authorized application instance.
    if (fwApp != NULL)
    {
        fwApp->Release();
    }

    // Release the authorized application collection.
    if (fwApps != NULL)
    {
        fwApps->Release();
    }

    return hr;
}


HRESULT WindowsFirewallAddApp(
    IN INetFwProfile* fwProfile,
    IN const wchar_t* fwProcessImageFileName,
    IN const wchar_t* fwName
    )
{
    HRESULT hr = S_OK;
    BOOL fwAppEnabled;
    BSTR fwBstrName = NULL;
    BSTR fwBstrProcessImageFileName = NULL;
    INetFwAuthorizedApplication* fwApp = NULL;
    INetFwAuthorizedApplications* fwApps = NULL;

    assert(fwProfile != NULL);
    assert(fwProcessImageFileName != NULL);
    assert(fwName != NULL);

    // First check to see if the application is already authorized.
    hr = WindowsFirewallAppIsEnabled(
        fwProfile,
        fwProcessImageFileName,
        &fwAppEnabled
        );
    if (FAILED(hr))
    {
        MYTRACE(ACE_TEXT("WindowsFirewallAppIsEnabled failed: 0x%08lx\n"), hr);
        goto error;
    }

    // Only add the application if it isn't already authorized.
    if (!fwAppEnabled)
    {
        // Retrieve the authorized application collection.
        hr = fwProfile->get_AuthorizedApplications(&fwApps);
        if (FAILED(hr))
        {
            MYTRACE(ACE_TEXT("get_AuthorizedApplications failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Create an instance of an authorized application.
        hr = CoCreateInstance(
            __uuidof(NetFwAuthorizedApplication),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(INetFwAuthorizedApplication),
            (void**)&fwApp
            );
        if (FAILED(hr))
        {
            MYTRACE(ACE_TEXT("CoCreateInstance failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Allocate a BSTR for the process image file name.
        fwBstrProcessImageFileName = SysAllocString(fwProcessImageFileName);
        if (fwBstrProcessImageFileName == NULL)
        {
            hr = E_OUTOFMEMORY;
            MYTRACE(ACE_TEXT("SysAllocString failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Set the process image file name.
        hr = fwApp->put_ProcessImageFileName(fwBstrProcessImageFileName);
        if (FAILED(hr))
        {
            MYTRACE(ACE_TEXT("put_ProcessImageFileName failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Allocate a BSTR for the application friendly name.
        fwBstrName = SysAllocString(fwName);
        if (SysStringLen(fwBstrName) == 0)
        {
            hr = E_OUTOFMEMORY;
            MYTRACE(ACE_TEXT("SysAllocString failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Set the application friendly name.
        hr = fwApp->put_Name(fwBstrName);
        if (FAILED(hr))
        {
            MYTRACE(ACE_TEXT("put_Name failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Set scope (added by bdr -- begin)
        //hr = fwApp->put_Scope(NET_FW_SCOPE_ALL);
        //if (FAILED(hr))
        //{
        //    MYTRACE(ACE_TEXT("put_Scope failed: 0x%08lx\n"), hr);
        //    goto error;
        //}

        //hr = fwApp->put_RemoteAddresses(L"*");
        //if (FAILED(hr))
        //{
        //    MYTRACE(ACE_TEXT("put_Scope failed: 0x%08lx\n"), hr);
        //    goto error;
        //}
        // Set scope (added by bdr -- end)


        // Add the application to the collection.
        hr = fwApps->Add(fwApp);
        if (FAILED(hr))
        {
            MYTRACE(ACE_TEXT("Add failed: 0x%08lx\n"), hr);
            goto error;
        }

        MYTRACE(ACE_TEXT(
            "Authorized application %lS is now enabled in the firewall.\n"),
            fwProcessImageFileName
            );
    }

error:

    // Free the BSTRs.
    SysFreeString(fwBstrName);
    SysFreeString(fwBstrProcessImageFileName);

    // Release the authorized application instance.
    if (fwApp != NULL)
    {
        fwApp->Release();
    }

    // Release the authorized application collection.
    if (fwApps != NULL)
    {
        fwApps->Release();
    }

    return hr;
}

HRESULT WindowsFirewallRemoveApp(
    IN INetFwProfile* fwProfile,
    IN const wchar_t* fwProcessImageFileName
    )
{
    HRESULT hr = S_OK;
    BOOL fwAppEnabled;
    BSTR fwBstrName = NULL;
    BSTR fwBstrProcessImageFileName = NULL;
    INetFwAuthorizedApplication* fwApp = NULL;
    INetFwAuthorizedApplications* fwApps = NULL;

    assert(fwProfile != NULL);
    assert(fwProcessImageFileName != NULL);

    // First check to see if the application is already authorized.
    hr = WindowsFirewallAppIsEnabled(
        fwProfile,
        fwProcessImageFileName,
        &fwAppEnabled
        );
    if (FAILED(hr))
    {
        MYTRACE(ACE_TEXT("WindowsFirewallAppIsEnabled failed: 0x%08lx\n"), hr);
        goto error;
    }

    // Only remove the application if it is already authorized.
    if (fwAppEnabled)
    {
        // Retrieve the authorized application collection.
        hr = fwProfile->get_AuthorizedApplications(&fwApps);
        if (FAILED(hr))
        {
            MYTRACE(ACE_TEXT("get_AuthorizedApplications failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Create an instance of an authorized application.
        hr = CoCreateInstance(
            __uuidof(NetFwAuthorizedApplication),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(INetFwAuthorizedApplication),
            (void**)&fwApp
            );
        if (FAILED(hr))
        {
            MYTRACE(ACE_TEXT("CoCreateInstance failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Allocate a BSTR for the process image file name.
        fwBstrProcessImageFileName = SysAllocString(fwProcessImageFileName);
        if (fwBstrProcessImageFileName == NULL)
        {
            hr = E_OUTOFMEMORY;
            MYTRACE(ACE_TEXT("SysAllocString failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Add the application to the collection.
        hr = fwApps->Remove(fwBstrProcessImageFileName);
        if (FAILED(hr))
        {
            MYTRACE(ACE_TEXT("Add failed: 0x%08lx\n"), hr);
            goto error;
        }

        MYTRACE(ACE_TEXT(
            "Authorized application %lS is now removed in the firewall.\n"),
            fwProcessImageFileName
            );
    }

error:

    // Free the BSTRs.
    SysFreeString(fwBstrName);
    SysFreeString(fwBstrProcessImageFileName);

    // Release the authorized application instance.
    if (fwApp != NULL)
    {
        fwApp->Release();
    }

    // Release the authorized application collection.
    if (fwApps != NULL)
    {
        fwApps->Release();
    }

    return hr;
}

HRESULT WindowsFirewallPortIsEnabled(
    IN INetFwProfile* fwProfile,
    IN LONG portNumber,
    IN NET_FW_IP_PROTOCOL ipProtocol,
    OUT BOOL* fwPortEnabled
    )
{
    HRESULT hr = S_OK;
    VARIANT_BOOL fwEnabled;
    INetFwOpenPort* fwOpenPort = NULL;
    INetFwOpenPorts* fwOpenPorts = NULL;

    assert(fwProfile != NULL);
    assert(fwPortEnabled != NULL);

    *fwPortEnabled = FALSE;

    // Retrieve the globally open ports collection.
    hr = fwProfile->get_GloballyOpenPorts(&fwOpenPorts);
    if (FAILED(hr))
    {
        MYTRACE(ACE_TEXT("get_GloballyOpenPorts failed: 0x%08lx\n"), hr);
        goto error;
    }

    // Attempt to retrieve the globally open port.
    hr = fwOpenPorts->Item(portNumber, ipProtocol, &fwOpenPort);
    if (SUCCEEDED(hr))
    {
        // Find out if the globally open port is enabled.
        hr = fwOpenPort->get_Enabled(&fwEnabled);
        if (FAILED(hr))
        {
            MYTRACE(ACE_TEXT("get_Enabled failed: 0x%08lx\n"), hr);
            goto error;
        }

        if (fwEnabled != VARIANT_FALSE)
        {
            // The globally open port is enabled.
            *fwPortEnabled = TRUE;

            MYTRACE(ACE_TEXT("Port %ld is open in the firewall.\n"), portNumber);
        }
        else
        {
            MYTRACE(ACE_TEXT("Port %ld is not open in the firewall.\n"), portNumber);
        }
    }
    else
    {
        // The globally open port was not in the collection.
        hr = S_OK;

        MYTRACE(ACE_TEXT("Port %ld is not open in the firewall.\n"), portNumber);
    }

error:

    // Release the globally open port.
    if (fwOpenPort != NULL)
    {
        fwOpenPort->Release();
    }

    // Release the globally open ports collection.
    if (fwOpenPorts != NULL)
    {
        fwOpenPorts->Release();
    }

    return hr;
}


HRESULT WindowsFirewallPortAdd(
    IN INetFwProfile* fwProfile,
    IN LONG portNumber,
    IN NET_FW_IP_PROTOCOL ipProtocol,
    IN const wchar_t* name
    )
{
    HRESULT hr = S_OK;
    BOOL fwPortEnabled;
    BSTR fwBstrName = NULL;
    INetFwOpenPort* fwOpenPort = NULL;
    INetFwOpenPorts* fwOpenPorts = NULL;

    assert(fwProfile != NULL);
    assert(name != NULL);

    // First check to see if the port is already added.
    hr = WindowsFirewallPortIsEnabled(
        fwProfile,
        portNumber,
        ipProtocol,
        &fwPortEnabled
        );
    if (FAILED(hr))
    {
        MYTRACE(ACE_TEXT("WindowsFirewallPortIsEnabled failed: 0x%08lx\n"), hr);
        goto error;
    }

    // Only add the port if it isn't already added.
    if (!fwPortEnabled)
    {
        // Retrieve the collection of globally open ports.
        hr = fwProfile->get_GloballyOpenPorts(&fwOpenPorts);
        if (FAILED(hr))
        {
            MYTRACE(ACE_TEXT("get_GloballyOpenPorts failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Create an instance of an open port.
        hr = CoCreateInstance(
            __uuidof(NetFwOpenPort),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(INetFwOpenPort),
            (void**)&fwOpenPort
            );
        if (FAILED(hr))
        {
            MYTRACE(ACE_TEXT("CoCreateInstance failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Set the port number.
        hr = fwOpenPort->put_Port(portNumber);
        if (FAILED(hr))
        {
            MYTRACE(ACE_TEXT("put_Port failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Set the IP protocol.
        hr = fwOpenPort->put_Protocol(ipProtocol);
        if (FAILED(hr))
        {
            MYTRACE(ACE_TEXT("put_Protocol failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Allocate a BSTR for the friendly name of the port.
        fwBstrName = SysAllocString(name);
        if (SysStringLen(fwBstrName) == 0)
        {
            hr = E_OUTOFMEMORY;
            MYTRACE(ACE_TEXT("SysAllocString failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Set the friendly name of the port.
        hr = fwOpenPort->put_Name(fwBstrName);
        if (FAILED(hr))
        {
            MYTRACE(ACE_TEXT("put_Name failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Opens the port and adds it to the collection.
        hr = fwOpenPorts->Add(fwOpenPort);
        if (FAILED(hr))
        {
            MYTRACE(ACE_TEXT("Add failed: 0x%08lx\n"), hr);
            goto error;
        }

        MYTRACE(ACE_TEXT("Port %ld is now open in the firewall.\n"), portNumber);
    }

error:

    // Free the BSTR.
    SysFreeString(fwBstrName);

    // Release the open port instance.
    if (fwOpenPort != NULL)
    {
        fwOpenPort->Release();
    }

    // Release the globally open ports collection.
    if (fwOpenPorts != NULL)
    {
        fwOpenPorts->Release();
    }

    return hr;
}

//
//int __cdecl wmain(int argc, wchar_t* argv[])
//{
//    HRESULT hr = S_OK;
//    HRESULT comInit = E_FAIL;
//    INetFwProfile* fwProfile = NULL;
//
//    // Initialize COM.
//    comInit = CoInitializeEx(
//        0,
//        COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE
//        );
//
//    // Ignore RPC_E_CHANGED_MODE; this just means that COM has already been
//    // initialized with a different mode. Since we don't care what the mode is,
//    // we'll just use the existing mode.
//    if (comInit != RPC_E_CHANGED_MODE)
//    {
//        hr = comInit;
//        if (FAILED(hr))
//        {
//            MYTRACE(ACE_TEXT("CoInitializeEx failed: 0x%08lx\n"), hr);
//            goto error;
//        }
//    }
//
//    // Retrieve the firewall profile currently in effect.
//    hr = WindowsFirewallInitialize(&fwProfile);
//    if (FAILED(hr))
//    {
//        MYTRACE(ACE_TEXT("WindowsFirewallInitialize failed: 0x%08lx\n"), hr);
//        goto error;
//    }
//
//    // Turn off the firewall.
//    hr = WindowsFirewallTurnOff(fwProfile);
//    if (FAILED(hr))
//    {
//        MYTRACE(ACE_TEXT("WindowsFirewallTurnOff failed: 0x%08lx\n"), hr);
//        goto error;
//    }
//
//    // Turn on the firewall.
//    hr = WindowsFirewallTurnOn(fwProfile);
//    if (FAILED(hr))
//    {
//        MYTRACE(ACE_TEXT("WindowsFirewallTurnOn failed: 0x%08lx\n"), hr);
//        goto error;
//    }
//
//    // Add Windows Messenger to the authorized application collection.
//    hr = WindowsFirewallAddApp(
//        fwProfile,
//        L"%ProgramFiles%\\Messenger\\msmsgs.exe"),
//        L"Windows Messenger"
//        );
//    if (FAILED(hr))
//    {
//        MYTRACE(ACE_TEXT("WindowsFirewallAddApp failed: 0x%08lx\n"), hr);
//        goto error;
//    }
//
//    // Add TCP::80 to list of globally open ports.
//    hr = WindowsFirewallPortAdd(fwProfile, 80, NET_FW_IP_PROTOCOL_TCP, L"WWW");
//    if (FAILED(hr))
//    {
//        MYTRACE(ACE_TEXT("WindowsFirewallPortAdd failed: 0x%08lx\n"), hr);
//        goto error;
//    }
//
//error:
//
//    // Release the firewall profile.
//    WindowsFirewallCleanup(fwProfile);
//
//    // Uninitialize COM.
//    if (SUCCEEDED(comInit))
//    {
//        CoUninitialize();
//    }
//
//    return 0;
//}

WinFirewall::WinFirewall(bool acquire_admin)
: m_fw(NULL)
{
    HRESULT hr = WindowsFirewallInitialize(&m_fw, acquire_admin);
}

WinFirewall::~WinFirewall()
{
    if(m_fw)
    {
        WindowsFirewallCleanup(m_fw);
        m_fw = NULL;
    }
}

bool WinFirewall::HasUAE()
{
    //detect windows version
    return IsWindowsVistaOrGreater();
}

bool WinFirewall::IsFirewallOn()
{
    if(!m_fw)
        return false;

    BOOL bOn = FALSE;
    HRESULT hr = WindowsFirewallIsOn(m_fw, &bOn);
    return bOn;
}

bool WinFirewall::EnableFirewall(bool enable)
{
    if(!m_fw)
        return false;

    HRESULT hr = enable? WindowsFirewallTurnOn(m_fw) : WindowsFirewallTurnOff(m_fw);
    return SUCCEEDED(hr);
}

bool WinFirewall::IsApplicationFirewalled(const ACE_TString& exefile)
{
    if(!m_fw)
        return false;

    BOOL bEnabled = FALSE;
    HRESULT hr = WindowsFirewallAppIsEnabled(m_fw, exefile.c_str(), &bEnabled);
    return bEnabled;
}

bool WinFirewall::AddException(const ACE_TString& exefile,
                               const ACE_TString& name)
{
    if(!m_fw)
        return false;

    HRESULT hr = WindowsFirewallAddApp(m_fw, exefile.c_str(), name.c_str());
    return SUCCEEDED(hr);
}

bool WinFirewall::RemoveException(const ACE_TString& exefile)
{
    if(!m_fw)
        return false;
    HRESULT hr = WindowsFirewallRemoveApp(m_fw, exefile.c_str());
    return SUCCEEDED(hr);
}
