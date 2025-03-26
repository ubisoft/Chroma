#pragma once
#include "pch.h"
#include "HotKeyManager.h"

extern std::string g_iniFilePath;

class IniParser
{
public:	

	static std::string ParseSSSectionData(std::string const sectionName);

	static void ParseIniData();

	static void WriteToIni(std::string const sectionName, std::string folderPath);

	static void WriteToIni();

private:
	static BOOL IniSectionExist(std::string const sectionName);

	static void ParseSectionData(HotKey& hotKey);
};

