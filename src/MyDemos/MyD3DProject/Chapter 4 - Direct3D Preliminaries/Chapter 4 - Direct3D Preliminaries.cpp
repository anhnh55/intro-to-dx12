// Chapter 4 - Direct3D Preliminaries.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <dxgi.h>
#include <dxgi1_4.h>
#include <intsafe.h>
#include <iostream>
#include <string>
#include <vector>
#include <wrl/client.h>
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

void LogAdaptersInfo();
void LogAdapterOutputs(IDXGIAdapter* adapter);
void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
std::vector<IDXGIAdapter*> adapterList;
int main()
{
    CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
    LogAdaptersInfo();
    LogAdapterOutputs(adapterList[0]);
}

void LogAdaptersInfo()
{
    UINT i = 0;
    IDXGIAdapter* adapter = nullptr;
    

    while (dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC desc;
        adapter->GetDesc(&desc);
        std::wstring text = L"Adapter: ";
        text += desc.Description;
        text += L"\n";
        std::wcout << text;
        ++i;
        adapterList.push_back(adapter);
    }
}

void LogAdapterOutputs(IDXGIAdapter* adapter)
{
    UINT i = 0;
    IDXGIOutput* output = nullptr;
    while (adapter->EnumOutputs(i, &output) !=
        DXGI_ERROR_NOT_FOUND)
    {
        DXGI_OUTPUT_DESC desc;
        output->GetDesc(&desc);
        std::wstring text = L"Output: " ;
        text += desc.DeviceName;
        text += L"\n";
        std::wcout << text;
        LogOutputDisplayModes(output,
            DXGI_FORMAT_B8G8R8A8_UNORM);
        output->Release();
        ++i;
    }
}

void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
    UINT count = 0;
    UINT flags = 0;

    // Call with nullptr to get list count.
    output->GetDisplayModeList(format, flags, &count, nullptr);

    std::vector<DXGI_MODE_DESC> modeList(count);
    output->GetDisplayModeList(format, flags, &count, &modeList[0]);

    for (DXGI_MODE_DESC& x : modeList)
    {
        UINT n = x.RefreshRate.Numerator;
        UINT d = x.RefreshRate.Denominator;
        std::cout << "Width: " << x.Width << "Height: " << x.Height << "\tRefresh Rate: " << (float)n/d << std::endl;
    }
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
