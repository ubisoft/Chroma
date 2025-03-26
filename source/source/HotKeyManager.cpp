#include "../headers/HotKeyManager.h"

HotKeyManager::HotKeyManager()
{
	InitAllHotkeys();
}

void HotKeyManager::InitAllHotkeys()
{
	m_hotkeyList.push_back(HotKey(ID_HOTKEY_INPUT, IDC_COMBOBOX_INPUT_H, IDC_COMBOBOX_INPUT_L));
	m_hotkeyList.push_back(HotKey(ID_HOTKEY_DEF_FLTR, IDC_COMBOBOX_DEF_FLTR_H, IDC_COMBOBOX_DEF_FLTR_L));
	m_hotkeyList.push_back(HotKey(ID_HOTKEY_PRO_FLTR, IDC_COMBOBOX_PRO_FLTR_H, IDC_COMBOBOX_PRO_FLTR_L));
	m_hotkeyList.push_back(HotKey(ID_HOTKEY_DEU_FLTR, IDC_COMBOBOX_DEU_FLTR_H, IDC_COMBOBOX_DEU_FLTR_L));
	m_hotkeyList.push_back(HotKey(ID_HOTKEY_TRI_FLTR, IDC_COMBOBOX_TRI_FLTR_H, IDC_COMBOBOX_TRI_FLTR_L));
	m_hotkeyList.push_back(HotKey(ID_HOTKEY_GRY_FLTR, IDC_COMBOBOX_GRY_FLTR_H, IDC_COMBOBOX_GRY_FLTR_L));
	m_hotkeyList.push_back(HotKey(ID_HOTKEY_TOOL_SCREENSHOT, IDC_COMBOBOX_SS_TOOL_H, IDC_COMBOBOX_SS_TOOL_L));
	m_hotkeyList.push_back(HotKey(ID_HOTKEY_TOOLGAME_SCREENSHOT, IDC_COMBOBOX_SS_TG_H, IDC_COMBOBOX_SS_TG_L));
}

bool HotKeyManager::ExistKeyCombination(HotKey& hKey)
{
	for (HotKey& hotKey : GetHotKeyList())
	{
		if (hKey.GetHotKeyID() != hotKey.GetHotKeyID() && 
			(hotKey.GetModKeyValue() == hKey.GetModKeyValue() && hotKey.GetAlphanumKeyValue() == hKey.GetAlphanumKeyValue()))
		{
			return true;
		}
	}
	return false;
}

HotKey* HotKeyManager::GetHotKey(int const& id)
{
	for (HotKey& hotKey : GetHotKeyList())
	{
		if (hotKey.GetHotKeyID() == id || hotKey.GetAlphanumKeyComID() == id || hotKey.GetModKeyComID() == id)
		{
			return &hotKey;
		}
	}
	return nullptr;
}

bool HotKeyManager::ValidateKeyCombinations()
{
	for (HotKey& hotKey : GetHotKeyList())
	{
		if ((hotKey.GetAlphanumKeyValue() != "" && hotKey.GetModKeyValue() == "") 
			|| (hotKey.GetAlphanumKeyValue() == "" && hotKey.GetModKeyValue() != ""))
		{
			return false;
		}
	}
	return true;
}

void HotKeyManager::RegisterAllHotKeys(HWND const& hwnd)
{
	for (HotKey& hotKey : GetHotKeyList())
	{
		int hotKeyID = hotKey.GetHotKeyID();
		UnregisterHotKey(hwnd, hotKeyID);
		HKL currentKBL = GetKeyboardLayout(0);
		
		UINT hInputKey{};
		std::string modKeyVal = hotKey.GetModKeyValue();
		if (modKeyVal == "ALT") { hInputKey = MOD_ALT; }
		else if (modKeyVal == "CTRL") { hInputKey = MOD_CONTROL; }			
		else if (modKeyVal == "SHIFT") { hInputKey = MOD_SHIFT; }			

		UINT vInputKey{};
		std::string alphanumKeyVal = hotKey.GetAlphanumKeyValue();
		vInputKey = VkKeyScanEx(alphanumKeyVal[0], currentKBL);
		RegisterHotKey(hwnd, hotKeyID, hInputKey | MOD_NOREPEAT, vInputKey);
	}
}

void HotKeyManager::ResetAllAssignments(HWND const& hwnd)
{
	for (HotKey& hotKey : GetHotKeyList())
	{
		hotKey.SetKeyValue(hotKey.GetModKeyComID(), std::string(""));
		hotKey.SetKeyValue(hotKey.GetAlphanumKeyComID(), std::string(""));
	}
	UnRegisterAllHotKeys(hwnd);
}

void HotKeyManager::UnRegisterAllHotKeys(HWND const& hwnd)
{
	for (HotKey& hotKey : GetHotKeyList())
	{
		int hotKeyID = hotKey.GetHotKeyID();
		UnregisterHotKey(hwnd, hotKeyID);
	}
}