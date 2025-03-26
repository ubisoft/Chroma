#include "../headers/HotKey.h"

HotKey::HotKey(int const& hotkeyID, int const& modKeyComID, int const& alphanumKeyComID):
m_hotkeyID(hotkeyID),
m_modKeyComID(modKeyComID),
m_alphanumKeyComID(alphanumKeyComID)
{}

void HotKey::SetKeyValue(int const& comID, std::string keyVal)
{
	if (comID == m_modKeyComID)
	{
		m_modKeyVal = keyVal;
	}
	else
	{
		m_alphanumKeyVal = keyVal;
	}
}

void HotKey::ClearKeyValue()
{
	SetKeyValue(GetModKeyComID(), std::string(""));
	SetKeyValue(GetAlphanumKeyComID(), std::string(""));
}

