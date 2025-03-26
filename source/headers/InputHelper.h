#pragma once

#include "pch.h"

class InputHelper
{

    static void UpdateTrayMenu(const HMENU& hMenu, const INT& appliedFilter, const INT& appliedMode, const bool& showa);

public:
    // Name says it all
    static void ShowContextMenu(const HWND& hWnd, const INT& appliedFilter, const INT& appliedMode, const bool & show);

};

