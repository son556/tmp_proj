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
            if (static_cast<char>(exePath[i]) == '\\')
                last_idx = i;
        }
        exePath = exePath.substr(0, last_idx);
        exePath += L"\\..\\scop\\";
        
        wchar_t fullPath[MAX_PATH];
        GetFullPathNameW(exePath.c_str(), MAX_PATH, fullPath, nullptr);
        return std::wstring(fullPath);
    }

public:
    static wstring ori_path;
};

