#pragma once
#include <string>
#include <unordered_set>

namespace Global
{
inline std::wstring ZEN_BROWSER = L"zen.exe";
inline std::unordered_set<std::wstring> VSCodeSeries = {L"Code.exe", L"Code - Insiders.exe", L"VSCodium.exe"};
inline bool IsVSCodeLike = false;
} // namespace Global