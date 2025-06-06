#pragma once
#include <string>

namespace FanyUtils
{
std::string GetIMEDataDirPath();
void SendKeys(std::wstring pinyin);
std::wstring string_to_wstring(const std::string &str);
std::string wstring_to_string(const std::wstring &wstr);
std::string to_lower_copy(const std::string &str);
std::wstring GetCurrentProcessName();
} // namespace FanyUtils