#pragma once
#include <SDK/modeless_dialog.h>

class CModelessDialogMessages {
public:
	static BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult) {
		switch (uMsg) {
		case WM_INITDIALOG:
			modeless_dialog_manager::g_add(hWnd); break;
		case WM_DESTROY:
			modeless_dialog_manager::g_remove(hWnd); break;
		}
		return FALSE;
	}
};

#define FB2K_MODELESS_DIALOG_MESSAGES() CHAIN_MSG_MAP(CModelessDialogMessages)
