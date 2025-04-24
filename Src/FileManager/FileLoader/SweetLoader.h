#pragma once
#include <string>
#include <unordered_map>
#include <fstream>

class SweetLoader
{
public:
	void Load(const std::string& filePath);
	void Save(const std::string& filePath) const;

	SweetLoader& operator=(const std::string& value);
	SweetLoader& operator[](const std::string& name);

	void DebugPrint(int indent = 0) const;

	const std::string& GetValue() const { return mValue; }
	void SetValue(const std::string& val) { mValue = val; }

private:
	void SaveRecursive(std::ostream& os, const std::string& prefix) const;
	size_t FindKeyValueSeparator(const std::string& line);
	bool ReadFileToBuffer(const std::string& filePath, std::string& outBuffer);
	void ParseLine(const std::string& line);

private:
	std::string mValue{};
	std::unordered_map<std::string, SweetLoader> mChildren;
};
