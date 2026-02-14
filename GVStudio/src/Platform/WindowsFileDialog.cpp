#include "Platform/WindowsFileDialog.h"

#include <windows.h>
#include <shobjidl.h>

#pragma comment(lib, "Ole32.lib")

namespace
{
    std::string WideToUtf8(const std::wstring& wstr)
    {
        if (wstr.empty())
            return {};

        int size = WideCharToMultiByte(
            CP_UTF8, 0,
            wstr.data(), (int)wstr.size(),
            nullptr, 0,
            nullptr, nullptr);

        std::string result(size, 0);

        WideCharToMultiByte(
            CP_UTF8, 0,
            wstr.data(), (int)wstr.size(),
            result.data(), size,
            nullptr, nullptr);

        return result;
    }
}

namespace WindowsFileDialog
{
    std::string OpenProjectFile()
    {
        HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        if (FAILED(hr))
            return {};

        IFileOpenDialog* dialog = nullptr;

        hr = CoCreateInstance(CLSID_FileOpenDialog,
            nullptr,
            CLSCTX_ALL,
            IID_PPV_ARGS(&dialog));

        if (FAILED(hr))
        {
            CoUninitialize();
            return {};
        }

        COMDLG_FILTERSPEC filter[] =
        {
            { L"Gravitas Project (*.gProject)", L"*.gProject" }
        };

        dialog->SetFileTypes(1, filter);

        hr = dialog->Show(nullptr);

        std::string result;

        if (SUCCEEDED(hr))
        {
            IShellItem* item;
            if (SUCCEEDED(dialog->GetResult(&item)))
            {
                PWSTR path = nullptr;
                if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path)))
                {
                    result = WideToUtf8(path);
                    CoTaskMemFree(path);
                }
                item->Release();
            }
        }

        dialog->Release();
        CoUninitialize();

        return result;
    }
}

std::string WindowsFileDialog::SelectFolder()
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
        return {};

    IFileDialog* dialog = nullptr;

    hr = CoCreateInstance(CLSID_FileOpenDialog,
        nullptr,
        CLSCTX_ALL,
        IID_PPV_ARGS(&dialog));

    if (FAILED(hr))
    {
        CoUninitialize();
        return {};
    }

    DWORD options;
    dialog->GetOptions(&options);
    dialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);

    hr = dialog->Show(nullptr);

    std::string result;

    if (SUCCEEDED(hr))
    {
        IShellItem* item;
        if (SUCCEEDED(dialog->GetResult(&item)))
        {
            PWSTR path = nullptr;
            if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path)))
            {
                result = WideToUtf8(path);
                CoTaskMemFree(path);
            }
            item->Release();
        }
    }

    dialog->Release();
    CoUninitialize();

    return result;
}

