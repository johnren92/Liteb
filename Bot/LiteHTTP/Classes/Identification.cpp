#include <iostream>
#include <Windows.h>
#include <sstream>

#pragma comment(lib, "Advapi32.lib")

std::string identifier(std::wstring wmiClass, std::wstring wmiProperty)
{
    std::wstring result = L"";
    IWbemLocator* pLoc = NULL;
    IWbemServices* pSvc = NULL;
    HRESULT hres;

    // Initialize COM.
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
        return "";

    // Initialize security
    hres = CoInitializeSecurity(
        NULL,
        -1,      // COM negotiates service
        NULL,    // Authentication services
        NULL,    // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,    // Default authentication 
        RPC_C_IMP_LEVEL_IMPERSONATE,  // Default Impersonation  
        NULL,             // Authentication info 
        EOAC_NONE,        // Additional capabilities 
        NULL              // Reserved
    );


    // Obtain the initial locator to WMI
    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);

    if (FAILED(hres))
    {
        CoUninitialize();
        return "";
    }

    // Connect to WMI through the IWbemLocator::ConnectServer method
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL,
        NULL,
        0,
        NULL,
        0,
        0,
        &pSvc
    );

    if (FAILED(hres))
    {
        pLoc->Release();
        CoUninitialize();
        return "";
    }

    // Set the IWbemServices proxy so that impersonation of the user (client) occurs.
    hres = CoSetProxyBlanket(
        pSvc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE
    );

    if (FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return "";
    }

    // Use the IWbemServices pointer to make requests of WMI. 
    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t((L"SELECT * FROM ") + wmiClass),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return "";
    }

    // Get the data from the first WMI object
    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;
    if (pEnumerator)
    {
        hres = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

        if (uReturn == 1)
        {
            VARIANT vtProp;
            hres = pclsObj->Get(wmiProperty.c_str(), 0, &vtProp, 0, 0);
            if (SUCCEEDED(hres))
            {
                result = vtProp.bstrVal;
                VariantClear(&vtProp);
            }
        }
    }

    pSvc->Release();
    pLoc->
