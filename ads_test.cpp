// ads_test.cpp
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.coM.
// This file is public domain software.

#include "ads.hpp"
#include <cstdio>

extern "C"
int wmain(int argc, WCHAR **wargv)
{
    HRESULT hr;

    if (argc == 2)
    {
        std::vector<ADS_ENTRY> entries;
        hr = ADS_get_entries(wargv[1], entries);

        for (size_t i = 0; i < entries.size(); ++i)
        {
            std::string data;
            hr = ADS_get_data(wargv[1], entries[i], data);
            printf("%d: %ls: %s\n", int(i), entries[i].name.c_str(), data.c_str());
        }

        return 0;
    }

    if (argc == 3)
    {
        ADS_ENTRY entry;
        entry.name = wargv[2];

        if (entry.name == L"--delete")
        {
            hr = ADS_delete_all(wargv[1]);
        }
        else
        {
            if (entry.name[0] != L':')
            {
                printf("ERROR\n");
                return -1;
            }
            std::string data;
            hr = ADS_get_data(wargv[1], entry, data);
            printf("%s\n", data.c_str());
        }

        return hr;
    }

    if (argc == 4)
    {
        ADS_ENTRY entry;
        entry.name = wargv[2];
        if (entry.name[0] != L':')
        {
            printf("ERROR\n");
            return -1;
        }

        std::wstring value = wargv[3];
        if (value == L"--delete")
        {
            hr = ADS_delete(wargv[1], wargv[2]);
        }
        else
        {
            char buf[256];
            WideCharToMultiByte(CP_ACP, 0, wargv[3], -1, buf, 256, NULL, NULL);
            hr = ADS_put_data(wargv[1], entry, buf);
        }
        return hr;
    }

    printf("Usage #1: ads_test file\n");
    printf("Usage #2: ads_test file :name\n");
    printf("Usage #3: ads_test file :name \"text\"\n");
    return 0;
}

int main(int argc, char **argv)
{
    int argc0;
    LPWSTR *wargv = CommandLineToArgvW(GetCommandLineW(), &argc0);
    int ret = wmain(argc0, wargv);
    LocalFree(wargv);
    return ret;
}
