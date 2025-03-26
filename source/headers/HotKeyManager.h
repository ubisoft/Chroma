#pragma once
#include "pch.h"
#include "HotKey.h"

class HotKeyManager
{
public:
	HotKeyManager();
	~HotKeyManager() {};

	static HotKeyManager& GetInstance() 
	{
		static HotKeyManager instance;
		return instance;
	}

	std::vector<HotKey>& GetHotKeyList() { return m_hotkeyList; }
	bool ExistKeyCombination(HotKey& hKey);
	HotKey* GetHotKey(int const& id);
	void RegisterAllHotKeys(HWND const& hwnd);
	void UnRegisterAllHotKeys(HWND const& hwnd);
	void ResetAllAssignments(HWND const& hwnd);
	bool ValidateKeyCombinations();

private:

	HotKeyManager(const HotKeyManager&);
	const HotKeyManager& operator=(const HotKeyManager&);

	void InitAllHotkeys();	
	std::vector<HotKey> m_hotkeyList; 
};

