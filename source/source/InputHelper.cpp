#include "../headers/InputHelper.h"

void InputHelper::UpdateTrayMenu(const HMENU& hMenu, const INT& appliedFilter, const INT& appliedMode, const bool& show)
{
    UINT menuItemCount = GetMenuItemCount(hMenu);
    for (UINT index = 0; index < menuItemCount; index++)
    {
        MENUITEMINFO menuItem;
        menuItem.cbSize = sizeof(MENUITEMINFO);
        menuItem.fMask = MIIM_ID | MIIM_SUBMENU;
        GetMenuItemInfo(hMenu, index, TRUE, &menuItem);
        if (menuItem.wID == appliedFilter)
        {
            menuItem.fMask = MIIM_STATE;
            menuItem.fState = MFS_CHECKED;
            SetMenuItemInfo(hMenu, index, TRUE, &menuItem);
        }
        if (menuItem.hSubMenu)
        {
            HMENU hsub = menuItem.hSubMenu;
            UINT menuItemSubCount = GetMenuItemCount(hsub);

            for (UINT subIndex = 0; subIndex < menuItemSubCount; subIndex++)
            {
                MENUITEMINFO subMenuItem;
                subMenuItem.cbSize = sizeof(MENUITEMINFO);
                subMenuItem.fMask = MIIM_ID;
                GetMenuItemInfo(hsub, subIndex, TRUE, &subMenuItem);
                if (subMenuItem.wID == appliedFilter)
                {
                    subMenuItem.fMask = MIIM_STATE;
                    subMenuItem.fState = MFS_CHECKED;
                    SetMenuItemInfo(hsub, subIndex, TRUE, &subMenuItem);
                }
                if (subMenuItem.wID == appliedMode)
                {
                    subMenuItem.fMask = MIIM_STATE;
                    subMenuItem.fState = MFS_CHECKED;
                    SetMenuItemInfo(hsub, subIndex, TRUE, &subMenuItem);
                }
            }
        }
    }
    MENUITEMINFOA menuitem = { sizeof(MENUITEMINFOA) };
    GetMenuItemInfoA(hMenu, ID_SHOW_HIDE_ITEM, false, &menuitem);
    if (show)
    {
        menuitem.dwTypeData = (char*)"Hide";
    }
    else
    {
        menuitem.dwTypeData = (char*)"Show";
    }
    menuitem.fMask = MIIM_TYPE | MIIM_DATA;
    SetMenuItemInfoA(hMenu, ID_SHOW_HIDE_ITEM, false, &menuitem);
}

// Name says it all
void InputHelper::ShowContextMenu(const HWND& hWnd, const INT& appliedFilter, const INT & appliedMode, const bool& show)
{
    POINT pt;
    GetCursorPos(&pt);
    HMENU hMenu = LoadMenuW(NULL, MAKEINTRESOURCE(IDC_POPUPMENU));

    hMenu = GetSubMenu(hMenu, 0);

    UpdateTrayMenu(hMenu, appliedFilter, appliedMode, show);
    SetForegroundWindow(hWnd);

    TrackPopupMenu(hMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, hWnd, nullptr);
    DestroyMenu(hMenu);
}
