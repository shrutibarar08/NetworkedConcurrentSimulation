#include "SweetLoader.h"

#include <windows.h>
#include <string>
#include <sstream>
#include <iostream>


void SweetLoader::Load(const std::string& filePath)
{
    std::string buffer;
    if (!ReadFileToBuffer(filePath, buffer))
        return;

    std::istringstream input(buffer);
    std::string line;
    while (std::getline(input, line))
    {
        if (line.empty()) continue;
        ParseLine(line);
    }
}

void SweetLoader::Save(const std::string& filepath) const
{
    // Open or create the file
    std::wstring w_filepath = std::wstring(filepath.begin(), filepath.end());

    HANDLE file = CreateFile(
        w_filepath.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (file == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Failed to open file for writing: " << GetLastError() << "\n";
        return;
    }

    std::ostringstream oss;
    SaveRecursive(oss, ""); // use string stream to compose the text

    std::string content = oss.str();
    DWORD bytesWritten = 0;

    if (!WriteFile(file, content.data(), static_cast<DWORD>(content.size()), &bytesWritten, nullptr))
    {
        std::cerr << "WriteFile failed: " << GetLastError() << "\n";
    }

    CloseHandle(file);
}

SweetLoader& SweetLoader::operator=(const std::string& value)
{
    mValue = value;
    return *this;
}

SweetLoader& SweetLoader::operator[](const std::string& name)
{
    return mChildren[name];
}

void SweetLoader::DebugPrint(int indent) const
{
	std::string prefix(indent, ' ');

	if (!mValue.empty())
		std::cout << prefix << ": " << mValue << "\n";

	for (const auto& [key, child] : mChildren)
	{
		std::cout << prefix << key;
		child.DebugPrint(indent + 4);
	}
}

bool SweetLoader::Contains(const std::string& key) const
{
    return mChildren.contains(key);
}

void SweetLoader::SaveRecursive(std::ostream& os, const std::string& prefix) const
{
	if (!mValue.empty())
		os << prefix << ": " << mValue << "\n";

	for (const auto& [key, child] : mChildren)
	{
		std::string fullKey = prefix.empty() ? key : prefix + "::" + key;
		child.SaveRecursive(os, fullKey);
	}
}

size_t SweetLoader::FindKeyValueSeparator(const std::string& line)
{
    for (size_t i = 0; i < line.length(); ++i)
    {
        if (line[i] == ':' && (i == 0 || line[i - 1] != ':') && (i + 1 == line.length() || line[i + 1] != ':'))
        {
            return i;
        }
    }
    return std::string::npos;
}

bool SweetLoader::ReadFileToBuffer(const std::string& filePath, std::string& outBuffer)
{
    HANDLE file = CreateFileA(
        filePath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (file == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Failed to open file for reading: " << GetLastError() << "\n";
        return false;
    }

    DWORD fileSize = GetFileSize(file, nullptr);
    if (fileSize == INVALID_FILE_SIZE || fileSize == 0)
    {
        CloseHandle(file);
        return false;
    }

    outBuffer.resize(fileSize);
    DWORD bytesRead = 0;

    if (!ReadFile(file, outBuffer.data(), fileSize, &bytesRead, nullptr))
    {
        std::cerr << "ReadFile failed: " << GetLastError() << "\n";
        CloseHandle(file);
        return false;
    }

    CloseHandle(file);
    return true;
}

void SweetLoader::ParseLine(const std::string& line)
{
    size_t sep = FindKeyValueSeparator(line);
    if (sep == std::string::npos) return;

    std::string key = line.substr(0, sep);
    std::string value = line.substr(sep + 1);
    value.erase(0, value.find_first_not_of(" \t")); // trim leading whitespace

    SweetLoader* current = this;
    size_t start = 0;
    size_t end;

    while ((end = key.find("::", start)) != std::string::npos)
    {
        std::string segment = key.substr(start, end - start);
        current = &(*current)[segment];
        start = end + 2;
    }

    std::string finalKey = key.substr(start);
    current = &(*current)[finalKey];
    current->SetValue(value);
}
