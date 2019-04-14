// ADS (alternate data stream) support
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.coM.
// This file is public domain software.
#ifndef ADS_HPP_
#define ADS_HPP_ 2

#ifndef _INC_WINDOWS
    #include <windows.h>
#endif
#include <string>
#include <vector>

struct ADS_ENTRY
{
    std::wstring name;
    LARGE_INTEGER Size;
};

HRESULT ADS_read_entry(HANDLE hFile, std::vector<ADS_ENTRY>& entries, LPVOID *ppContext);
HRESULT ADS_get_entries(LPCWSTR filename, std::vector<ADS_ENTRY>& entries);
HANDLE ADS_open_file(LPCWSTR filename, LPCWSTR strname, BOOL bWrite);
HRESULT ADS_get_data(LPCWSTR filename, ADS_ENTRY& entry, std::string& data);
HRESULT ADS_put_data(LPCWSTR filename, ADS_ENTRY& entry, const std::string& data);
HRESULT ADS_delete(LPCWSTR filename, LPCWSTR strname);
HRESULT ADS_delete_all(LPCWSTR filename);

#endif  // ndef ADS_HPP_
