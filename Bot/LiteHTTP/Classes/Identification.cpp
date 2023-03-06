#include <iostream>
#include <Windows.h>
#include <sstream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <vector>

using namespace std;

string identifier(string wmiClass, string wmiProperty) {
    string result;
    IWbemLocator *pLoc = NULL;
    IWbemServices *pSvc = NULL;
    IEnumWbemClassObject* pEnumerator = NULL;
    BSTR ClassName = (wmiClass.empty()) ? NULL : SysAllocString(L"WMIObject");
    BSTR MethodName = (wmiProperty.empty()) ? NULL : SysAllocString(L"GetHashCode");
    BSTR ParameterName = (wmiProperty.empty()) ? NULL : SysAllocString(L"StrToHash");
    BSTR ClassNameSpace = SysAllocString(L"root\\cimv2");

    HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        return "";
    }

    hres = CoInitializeSecurity(NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);

    if (FAILED(hres)) {
        CoUninitialize();
        return "";
    }

    hres = CoCreateInstance(CLSID_WbemLocator, 0,
        CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);

    if (FAILED(hres)) {
        CoUninitialize();
        return "";
    }

    hres = pLoc->ConnectServer(ClassNameSpace, NULL, NULL, 0,
        NULL, 0, 0, &pSvc);

    if (FAILED(hres)) {
        pLoc->Release();
        CoUninitialize();
        return "";
    }

    hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return "";
    }

    hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t(("SELECT * FROM " + wmiClass).c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return "";
    }

    IWbemClassObject *pclsObj;
    ULONG uReturn = 0;
    vector<string> identifiers;

    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
            &pclsObj, &uReturn);

        if (uReturn == 0) {
            break;
        }

        VARIANT vtProp;
        hr = pclsObj->Get(_bstr_t(wmiProperty.c_str()), 0, &vtProp, 0, 0);
        if (vtProp.vt == VT_NULL) {
            continue;
        }

        string id;
        if (vtProp.vt == VT_BSTR) {
            id = (char*)vtProp.bstrVal;
        } else if (vtProp.vt == VT_I4) {
            id = to_string(vtProp.lVal);
        } else if (vtProp.vt == VT_R8) {
            id = to_string(vtProp.dblVal);
        }
