#pragma once
#include "Cursor.h"
#include "IImage.h"
#include "KeyCode.h"
#include "MouseCode.h"

enum MOUSE_STATE
{
	UP,
	DOWN,
	MMOVE,
	DBLCLICK
};

enum EWebPageLoadError
{
	eLoadErrorNone = 0,
	eMimeTypeNotSupported,
	eCacheMiss,
	eBadURL,
	eConnectionProblem,
	eProxyConnectionProblem,
	eLoadErrorUnknown
};

class IHTML
{
public:
	virtual void OpenURL(const char*) = 0;
	virtual bool StopLoading() = 0;
	virtual bool Refresh() = 0;
	virtual bool Show(bool shown) = 0;
	virtual const char* GetOpenedPage() = 0;
	virtual void Obsolete_OnSize(int x, int y, int w, int h) = 0;
	virtual void GetHTMLSize(int& wide, int& tall) = 0;
	virtual void Clear() = 0;
	virtual void AddText(const char* text) = 0;
	virtual void Obsolete_OnMouse(MouseCode code, MOUSE_STATE s, int x, int y) = 0;
	virtual void Obsolete_OnChar(wchar_t unichar) = 0;
	virtual void Obsolete_OnKeyDown(KeyCode code) = 0;
	virtual IImage* GetBitmap() = 0;
	virtual void SetVisible(bool state) = 0;
	virtual void SetSize(int wide, int tall) = 0;
	virtual void OnMouse(MouseCode code, MOUSE_STATE s, int x, int y, bool bPopupMenuMenu) = 0;
	virtual void OnChar(wchar_t unichar, bool bPopupMenu) = 0;
	virtual void OnKeyDown(KeyCode code, bool bPopupMenu) = 0;
	virtual void ScrollV(int nPixels) = 0;
	virtual void ScrollH(int nPixels) = 0;
	virtual void OnMouseWheeled(int delta, bool bPopupMenu) = 0;
	virtual void OnKeyUp(KeyCode code, bool bPopupMenu) = 0;
	virtual void PostURL(const char* pchURL, const char* pchPostData) = 0;
	virtual void RunJavascript(const char* pchScript) = 0;
	virtual void SetMousePosition(int x, int y, bool bPopupMenu) = 0;
	virtual void SetUserAgentInfo(const wchar_t* pwchUserAgent) = 0;
	virtual void AddHeader(const char* pchHeader, const char* pchValue) = 0;
	virtual void SetFileDialogChoice(const char* pchFileName) = 0;
	virtual void HidePopup() = 0;
	virtual void SetHTMLFocus() = 0;
	virtual void KillHTMLFocus() = 0;
	virtual void HorizontalScrollBarSize(int& x, int& y, int& wide, int& tall) = 0;
	virtual void VerticalScrollBarSize(int& x, int& y, int& wide, int& tall) = 0;
	virtual int HorizontalScroll() = 0;
	virtual int VerticalScroll() = 0;
	virtual int HorizontalScrollMax() = 0;
	virtual int VerticalScrollMax() = 0;
	virtual bool IsHorizontalScrollBarVisible() = 0;
	virtual bool IsVeritcalScrollBarVisible() = 0;
	virtual void SetHorizontalScroll(int scroll) = 0;
	virtual void SetVerticalScroll(int scroll) = 0;
	virtual void ViewSource() = 0;
	virtual void Copy() = 0;
	virtual void Paste() = 0;
	virtual bool IsIERender() = 0;
	virtual void GetIDispatchPtr(void** pIDispatch) = 0;
	virtual void GetHTMLScroll(int& top, int& left) = 0;
};

class IHTMLEvents
{
public:
	virtual bool Obsolete_OnStartURL(const char* url, const char* target, bool first) = 0;
	virtual void Obsolete_OnFinishURL(const char* url) = 0;
	virtual void Obsolete_OnProgressURL(long current, long maximum) = 0;
	virtual void Obsolete_OnSetStatusText(const char* text) = 0;
	virtual void Obsolete_OnUpdate() = 0;
	virtual void Obsolete_OnLink() = 0;
	virtual void Obsolete_OffLink() = 0;
	virtual void OnURLChanged(const char* url, const char* pchPostData, bool bIsRedirect) = 0;
	virtual void OnFinishRequest(const char* url, const char* pageTitle) = 0;
	virtual bool OnStartRequestInternal(const char* url, const char* target, const char* pchPostData, bool bIsRedirect) = 0;
	virtual void ShowPopup(int x, int y, int wide, int tall) = 0;
	virtual void HidePopup() = 0;
	virtual bool OnPopupHTMLWindow(const char* pchURL, int x, int y, int wide, int tall) = 0;
	virtual void SetHTMLTitle(const char* pchTitle) = 0;
	virtual void OnLoadingResource(const char* pchURL) = 0;
	virtual void OnSetStatusText(const char* text) = 0;
	virtual void OnSetCursor(CursorCode cursor) = 0;
	virtual void OnFileLoadDialog(const char* pchTitle, const char* pchInitialFile) = 0;
	virtual void OnShowToolTip(const char* pchText) = 0;
	virtual void OnUpdateToolTip(const char* pchText) = 0;
	virtual void OnHideToolTip() = 0;
	virtual bool BOnCreateNewWindow(void** ppDispatch) = 0;
	virtual void OnLink() = 0;
	virtual void OffLink() = 0;
	virtual void OnCloseWindow() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnProgressRequest(long current, long maximum) = 0;
	virtual bool OnOpenNewTab(const char* pchURL, bool bForeground) = 0;
};