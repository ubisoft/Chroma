#include "../headers/IniParser.h"

BOOL IniParser::IniSectionExist(std::string const sectionName)
{
    char output[100];
    long hkey_result = GetPrivateProfileStringA(sectionName.c_str(), NULL, "", output, 100, g_iniFilePath.c_str());
    return hkey_result > 0;
}

void IniParser::ParseSectionData(HotKey& hotKey)
{
    std::string hotKeyID = std::to_string(hotKey.GetHotKeyID());
    if (IniSectionExist(hotKeyID))
    {
        char iniData[200];
        long hkey_result = GetPrivateProfileSectionA(hotKeyID.c_str(), iniData, 200, g_iniFilePath.c_str());
        int begin = 0;
        int end = 0;
        std::string pairString = "";
        while (iniData[end] != '\0' || iniData[end + 1] != '\0')
        {
            if (iniData[end] == '\0')
            {
                size_t index = pairString.find("=");
                hotKey.SetKeyValue(stoi(pairString.substr(0, index)), pairString.substr(index + 1, end - begin - index - 1));
                pairString = "";
                end++;
                begin = end;
            }
            pairString = pairString + iniData[end];
            end++;
        }
        size_t index = pairString.find("=");
        hotKey.SetKeyValue(stoi(pairString.substr(0, index)), pairString.substr(index + 1, end - begin - index - 1));
    }    
}

std::string IniParser::ParseSSSectionData(std::string const sectionName)
{
    char iniData[MAX_PATH];
    long hkey_result = GetPrivateProfileSectionA(sectionName.c_str(), iniData, MAX_PATH, g_iniFilePath.c_str());
    int begin = 0;
    int end = 0;
    std::string pairString = "";
    while (iniData[end] != '\0')
    {
        pairString = pairString + iniData[end];
        end++;
    }
    size_t index = pairString.find("=");
    std::string folderPath = pairString.substr(index + 1, end - begin - index - 1);
    return folderPath;
}

void IniParser::ParseIniData()
{
    for (HotKey& hotKey : HotKeyManager::GetInstance().GetHotKeyList())
    {
        ParseSectionData(hotKey);
    }
}

void IniParser::WriteToIni(std::string const sectionName, std::string folderPath)
{
    WritePrivateProfileStringA(sectionName.c_str(), std::to_string(IDC_SSBROWSE_BUTTON).c_str(), folderPath.c_str(), g_iniFilePath.c_str());
}

void IniParser::WriteToIni()
{
    for (HotKey& hotKey : HotKeyManager::GetInstance().GetHotKeyList())
    {
        WritePrivateProfileStringA(
            std::to_string(hotKey.GetHotKeyID()).c_str(), 
            std::to_string(hotKey.GetModKeyComID()).c_str(), 
            hotKey.GetModKeyValue().c_str(), 
            g_iniFilePath.c_str()
        );

        WritePrivateProfileStringA(
            std::to_string(hotKey.GetHotKeyID()).c_str(),
            std::to_string(hotKey.GetAlphanumKeyComID()).c_str(),
            hotKey.GetAlphanumKeyValue().c_str(),
            g_iniFilePath.c_str()
        );
    }
}