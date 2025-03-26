#pragma once
#include <string>
class HotKey
{
public:

	HotKey() {};
	HotKey(int const& hotkeyID, int const& modKeyComID, int const& alphanumKeyComID);

	void SetKeyValue(int const& comID, std::string keyVal);
	std::string& GetModKeyValue() { return m_modKeyVal; }
	std::string& GetAlphanumKeyValue() { return m_alphanumKeyVal; }

	int& GetHotKeyID() { return m_hotkeyID; }
	int& GetModKeyComID() { return m_modKeyComID; }
	int& GetAlphanumKeyComID() { return m_alphanumKeyComID; }
	void ClearKeyValue();


private:

	int m_hotkeyID;
	int m_modKeyComID;
	int m_alphanumKeyComID;
	std::string m_modKeyVal="";
	std::string m_alphanumKeyVal="";
};

