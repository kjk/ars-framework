#ifndef __ARSLEXIS_WINSYSUTILS_HPP__
#define __ARSLEXIS_WINSYSUTILS_HPP__

#include <BaseTypes.hpp>

/*
#include <windows.h>
#include <sipapi.h>

#include <Text.hpp>
 */
 
ulong_t random(ulong_t range);

/*
void sendEvent(uint_t event, short wph, short wpl, int lp);
struct EventData {LPARAM lParam; WPARAM wParam;};

int RectDx(RECT *rect);
int RectDy(RECT *rect);

void GetEditWinText(HWND hwnd, String &txtOut);
void SetEditWinText(HWND hwnd, const String& txt);

void ListCtrlFillFromList(HWND ctrlList, CharPtrList_t& strList, bool fRemoveDups);
bool ListCtrlGetSelectedItemText(HWND ctrl, String& txtOut);

bool GetSpecialFolderPath(String& pathOut, BOOL fCreate=TRUE);

int GetScrollBarDx();

bool  FDataConnectionEstablished();
bool  InitDataConnection();
void  DeinitDataConnection();

bool FDesktopIncludesMenuBar(SIPINFO *si);

void OverrideBackButton(HWND hwndMenuBar);

void DrawFancyRectangle(HDC hdc, RECT *rect);
 */
 
bool OpenURL(const char* url);

int Alert(HWND parent, UINT textId, UINT titleId = 0, UINT type = MB_OK|MB_APPLMODAL|MB_ICONERROR);

char_t* LoadString(UINT id, ulong_t* len = NULL);

char_t* GetAppDataPath();

#endif
