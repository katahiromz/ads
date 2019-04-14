// ADS (alternate data stream) support
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.coM.
// This file is public domain software.

#include "ads.hpp"
#include <cassert>

HRESULT ADS_read_entry(HANDLE hFile, std::vector<ADS_ENTRY>& entries,
                       LPVOID *ppContext)
{
    WIN32_STREAM_ID sid;
    DWORD cbToRead, cbDidRead;

    ADS_ENTRY entry;
    entry.name.clear();
    entry.Size.QuadPart = 0;

    ZeroMemory(&sid, sizeof(sid));
    cbToRead = DWORD(offsetof(WIN32_STREAM_ID, cStreamName));
    if (!BackupRead(hFile, (LPBYTE)&sid, cbToRead, &cbDidRead,
                    FALSE, FALSE, ppContext))
    {
        return E_FAIL;
    }

    if (cbDidRead == 0)
        return S_FALSE;

    if (cbDidRead != cbToRead)
        return E_FAIL;

    if (sid.dwStreamNameSize > 0)
    {
        LPWSTR name = LPWSTR(std::malloc(sid.dwStreamNameSize + sizeof(WCHAR)));
        if (!name)
            return E_OUTOFMEMORY;

        cbToRead = sid.dwStreamNameSize;
        if (!BackupRead(hFile, (LPBYTE)name, cbToRead, &cbDidRead,
                        FALSE, FALSE, ppContext))
        {
            std::free(name);
            return E_FAIL;
        }

        name[cbDidRead / sizeof(WCHAR)] = 0;

        entry.name = name;
        entry.Size = sid.Size;
        entries.push_back(entry);

        std::free(name);
    }

    if (sid.Size.QuadPart)
    {
        if (sid.dwStreamId == BACKUP_SPARSE_BLOCK)
        {
            cbToRead = sid.Size.LowPart;
            std::string buffer(cbToRead, 0);
            if (!BackupRead(hFile, (LPBYTE)&buffer[0], cbToRead, &cbDidRead,
                            FALSE, FALSE, ppContext))
            {
                return E_FAIL;
            }
        }
        else
        {
            DWORD dw1, dw2;
            if (!BackupSeek(hFile, sid.Size.LowPart, sid.Size.HighPart,
                            &dw1, &dw2, ppContext))
            {
                return E_FAIL;
            }
        }
    }

    return S_OK;
}

HRESULT ADS_get_entries(LPCWSTR filename, std::vector<ADS_ENTRY>& entries)
{
    entries.clear();

    HANDLE hFile;
    hFile = CreateFileW(filename,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_FLAG_BACKUP_SEMANTICS,
                        NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return E_FAIL;
    }

    LPVOID context = NULL;
    HRESULT hr = E_FAIL;
    for (;;)
    {
        hr = ADS_read_entry(hFile, entries, &context);
        if (hr != S_OK)
            break;
    }

    BackupRead(hFile, NULL, 0, NULL, TRUE, FALSE, &context);

    if (hr == S_FALSE)
        hr = S_OK;

    CloseHandle(hFile);

    return hr;
}

HANDLE ADS_open_file(LPCWSTR filename, LPCWSTR strname, BOOL bWrite)
{
    assert(strname[0] == L':');

    std::wstring pathname = filename;
    pathname += strname;

    HANDLE hFile;
    if (bWrite)
    {
        hFile = CreateFileW(pathname.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL,
                            CREATE_ALWAYS, 0, NULL);
    }
    else
    {
        hFile = CreateFileW(pathname.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                            OPEN_EXISTING, 0, NULL);
    }

    return hFile;
}

HRESULT ADS_get_data(LPCWSTR filename, ADS_ENTRY& entry, std::string& data)
{
    HANDLE hFile = ADS_open_file(filename, entry.name.c_str(), FALSE);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return E_FAIL;
    }

    entry.Size.LowPart = GetFileSize(hFile, LPDWORD(&entry.Size.HighPart));
    if (entry.Size.LowPart == 0xFFFFFFFF && GetLastError() != NO_ERROR)
    {
        CloseHandle(hFile);
        return E_FAIL;
    }

    if (entry.Size.HighPart)
    {
        CloseHandle(hFile);
        return E_OUTOFMEMORY;
    }

    data.resize(entry.Size.LowPart);

    if (data.size() > 0)
    {
        DWORD cbDidRead;
        if (!ReadFile(hFile, &data[0], entry.Size.LowPart, &cbDidRead, NULL))
        {
            data.clear();
            CloseHandle(hFile);
            return E_FAIL;
        }
    }

    CloseHandle(hFile);
    return S_OK;
}

HRESULT ADS_put_data(LPCWSTR filename, ADS_ENTRY& entry, const std::string& data)
{
    HANDLE hFile = ADS_open_file(filename, entry.name.c_str(), TRUE);
    if (hFile == INVALID_HANDLE_VALUE)
        return E_FAIL;

    entry.Size.QuadPart = data.size();

    HRESULT hr = E_FAIL;
    if (entry.Size.HighPart == 0)
    {
        if (data.size() > 0)
        {
            DWORD cbDidWrite;
            if (WriteFile(hFile, data.c_str(), entry.Size.LowPart, &cbDidWrite, NULL))
                hr = S_OK;
            else
                hr = E_FAIL;
        }
        else
        {
            hr = S_OK;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    CloseHandle(hFile);
    return hr;
}

HRESULT ADS_delete(LPCWSTR filename, LPCWSTR strname)
{
    assert(strname[0] == L':');

    std::wstring pathname = filename;
    pathname += strname;

    return DeleteFileW(pathname.c_str()) ? S_OK : E_FAIL;
}

HRESULT ADS_delete_all(LPCWSTR filename)
{
    HRESULT hr1, hr2;
    std::vector<ADS_ENTRY> entries;
    hr1 = ADS_get_entries(filename, entries);
    for (size_t i = 0; i < entries.size(); ++i)
    {
        hr2 = ADS_delete(filename, entries[i].name.c_str());
        if (FAILED(hr2))
        {
            hr1 = hr2;
        }
    }
    return hr1;
}
