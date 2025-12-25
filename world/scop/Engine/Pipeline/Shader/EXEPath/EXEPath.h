#pragma once
class EXEPath
{
public:
    static std::wstring GetExecutablePath() {
        wchar_t path[MAX_PATH];
        GetModuleFileNameW(nullptr, path, MAX_PATH);
        std::wstring exePath(path);
        int last_idx = 0;
        for (int i = 0; i < exePath.size(); i++) {
            if (exePath[i] == L'\\' || exePath[i] == L'/') // 둘 다 체크
                last_idx = i;
        }
        exePath = exePath.substr(0, last_idx);
        exePath += L"\\..\\scop\\";
        
        wchar_t fullPath[MAX_PATH];
        GetFullPathNameW(exePath.c_str(), MAX_PATH, fullPath, nullptr);
        std::wstring result(fullPath);
        if (!result.empty() && result.back() != L'\\' && result.back() != L'/') {
            result += L"\\";
        }
        return std::wstring(fullPath);
    }

public:
    static wstring ori_path;
};

