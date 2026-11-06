/* Win32 stand-in for the Tk paperclip. Behavior follows vigor/vigor.tcl. */

#define _WIN32_WINNT 0x0A00
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../vigor/vigor_empty.xbm"
#include "../vigor/vigor_rest.xbm"
#include "../vigor/vigor_ent_0001.xbm"
#include "../vigor/vigor_ent_0002.xbm"
#include "../vigor/vigor_ent_0003.xbm"
#include "../vigor/vigor_ent_0004.xbm"
#include "../vigor/vigor_wav_0001.xbm"
#include "../vigor/vigor_wav_0002.xbm"
#include "../vigor/v2cp0000.xbm"
#include "../vigor/v2cp0001.xbm"
#include "../vigor/v2cp0002.xbm"
#include "../vigor/v2cp0003.xbm"
#include "../vigor/v2cp0004.xbm"

enum {
	FR_EMPTY, FR_REST, FR_ENT1, FR_ENT2, FR_ENT3, FR_ENT4,
	FR_WAV1, FR_WAV2, FR_CP0, FR_CP1, FR_CP2, FR_CP3, FR_CP4,
	FR_N
};

enum { KIND_COMMENT, KIND_CONFIRM, KIND_WIZARD, KIND_EULA };
enum { ACT_NONE, ACT_SERVER, ACT_COMMENT, ACT_SERVER_THEN_COMMENT };

#define ID_OK 1001
#define ID_CANCEL 1002
#define ID_T0 1003
#define ID_T1 1004
#define ID_T2 1005

#define SERVER_MSG \
	"If you want to take advantage of the cancel feature, you need to upgrade to the server edition of Vigor.  Contact your sales representative for information."

static const char eula_text[] =
	"END-USER LICENSE AGREEMENT\r\n\r\n"
	"IMPORTANT-READ CAREFULLY: This End-User License Agreement (\"EULA\") "
	"is NOT a legal agreement between you (either an individual or a single entity, "
	"so take that Fare) and some other unnamed entity for whatever product we feel like "
	"it applies to.  This includes media, printed materials, online or electronic "
	"documentation, products purchased because of or used with the product, or anything "
	"that happens to be nearby (\"SOFTWARE PRODUCT\").  By installing, copying, "
	"downloading, accessing, using, thinking about, hearing of, or being in the same "
	"universe as the SOFTWARE PRODUCT, you agree to be bound by the terms of this "
	"EULA.  If you do not agree to the terms of this EULA, you can try to get your "
	"money back, but let's face it, that ain't gonna happen anyway, so why bother?\r\n\r\n"
	"SOFTWARE PRODUCT LICENSE\r\n\r\n"
	"The SOFTWARE PRODUCT is protected by copyright laws, international copyright "
	"treaties, several vaguely defined IPO laws and treaties that are easily clouded "
	"in a court of law, and a guy named Bubba with a shotgun.\r\n\r\n"
	"1. GRANT OF LICENSE. This EULA grants you the following rights:\r\n"
	"* The right to do whatever the publisher says.\r\n"
	"* Nothing else.\r\n\r\n"
	"2. RESPONSIBILITIES OF THE USER.  By agreeing to this license, you agree to "
	"the following:\r\n"
	"* Give the provider your firstborn son, should the provider deem such is "
	"necessary and valuable.\r\n"
	"* Become Bill's towel boy for life.\r\n"
	"* Whatever else we think of later on, you already agreed to from the shrink-wrap "
	"license anyway\r\n\r\n"
	"THIS TECHNOLOGY IS NOT FAULT TOLERANT AND IS NOT DESIGNED, MANUFACTURED, OR INTENDED "
	"FOR USE OR RESALE AS ON-LINE CONTROL EQUIPMENT IN HAZARDOUS ENVIRONMENTS "
	"REQUIRING FAIL-SAFE PERFORMANCE, SUCH AS THE OPERATION OF NUCLEAR FACILITIES, "
	"AIRCRAFT NAVIGATION OR COMMUNICATIONS SYSTEMS, AIR TRAFFIC CONTROL, DIRECT LIFE "
	"SUPPORT MACHINES, WEAPONS SYSTEMS, INTERNET SERVICE PROVIDERS, TECHNICAL "
	"SUPPORT LINES, NETWORK OPERATIONS CENTERS, IN WHICH THE FAILURE OF THE TECHNOLOGY "
	"OR NORMAL DAY-TO-DAY OPERATIONS COULD LEAD DIRECTLY TO DEATH, PERSONAL INJURY, "
	"OR SEVERE PHYSICAL OR ENVIRONMENTAL DAMAGE.\r\n\r\n"
	"LIMITED WARRANTY\r\n"
	"You have no warranty rights under this agreement whatsoever, expressed or "
	"implied.  We don't want to hear from you if you've got any problems.  We don't "
	"give you a number to call for just that reason.  You've got to call the poor "
	"schmuck who sold this to you; they have to support you themselves, and we won't help "
	"them.  We don't care.  We don't have to.\r\n\r\n"
	"If you're reading this because it's being displayed as part of the Vigor startup "
	"sequence, then please let me know by email at joelh@gnu.org.  This dialog box is "
	"intended to be long enough so that it won't be displayed on the screen.\r\n\r\n"
	"Exception: I know that you can click in the text box, and then hold down the "
	"down arrow key, and thus bypass the scroll overrides.  I'll figure out a way to keep "
	"that from happening later.";

static HBITMAP frames[FR_N];
static int cur_frame;
static HWND assistant;
static HWND dlg;
static HWND ctl_text;
static HWND ctl_btn[4];
static int ctl_n;
static int dlg_kind;
static int dlg_done;
static int pending_act;
static char pending[2048];
static char wiz_task[3][256];
static char wiz_body[2048];
static HFONT font;
static HBRUSH white;
static int corner; /* 0 TL, 1 TR, 2 BL, 3 BR */
static int bubble_top;
static int aw, ah, dw, dh;
static int alive;
static int hover_asst, hover_dlg;
static int in_pump;
static ULONGLONG hold_until;
static ULONGLONG next_random;
static ULONGLONG cp_until;
static int anim_q[24];
static int anim_len, anim_pos;
static ULONGLONG anim_due;
static WNDPROC eula_old;
static int rng_ready;
static unsigned rng_seed;

static void comment_text(const char *text);
static void confirm_text(const char *text);
static void dispatch_pending(void);
static void pump_messages(void);
static void anim_play(const int *seq, int n, int block);

static int
v_random(int limit)
{
	const unsigned max = 259200;

	if (!rng_ready) {
		rng_seed = (unsigned)(GetCurrentProcessId() + time(NULL)) % max;
		rng_ready = 1;
	}
	rng_seed = (unsigned)(((unsigned long long)rng_seed * 7141u + 54773u) % max);
	return (int)((double)rng_seed * limit / (double)max);
}

static void
unquote(const char *s, char *dst, size_t n)
{
	size_t len;

	if (s == NULL)
		s = "";
	len = strlen(s);
	if (len >= 2 && s[0] == '"' && s[len - 1] == '"') {
		s++;
		len -= 2;
	}
	if (len >= n)
		len = n - 1;
	memcpy(dst, s, len);
	dst[len] = '\0';
}

static HBITMAP
make_xbm(const char *bits, int w, int h)
{
	BITMAPINFO bi;
	unsigned int *pix;
	HBITMAP hb;
	const unsigned char *src;
	int x, y, stride;

	memset(&bi, 0, sizeof bi);
	bi.bmiHeader.biSize = sizeof bi.bmiHeader;
	bi.bmiHeader.biWidth = w;
	bi.bmiHeader.biHeight = -h;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	hb = CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, (void **)&pix, NULL, 0);
	if (hb == NULL)
		return (NULL);
	src = (const unsigned char *)bits;
	stride = (w + 7) / 8;
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			int on = src[y * stride + (x / 8)] & (1u << (x % 8));
			pix[y * w + x] = on ? 0u : 0x00FFFFFFu;
		}
	}
	return (hb);
}

static void
load_frames(void)
{
	frames[FR_EMPTY] = make_xbm(vigor_empty_bits, vigor_empty_width, vigor_empty_height);
	frames[FR_REST] = make_xbm(vigor_rest_bits, vigor_rest_width, vigor_rest_height);
	frames[FR_ENT1] = make_xbm(vigor_ent_0001_bits, vigor_ent_0001_width, vigor_ent_0001_height);
	frames[FR_ENT2] = make_xbm(vigor_ent_0002_bits, vigor_ent_0002_width, vigor_ent_0002_height);
	frames[FR_ENT3] = make_xbm(vigor_ent_0003_bits, vigor_ent_0003_width, vigor_ent_0003_height);
	frames[FR_ENT4] = make_xbm(vigor_ent_0004_bits, vigor_ent_0004_width, vigor_ent_0004_height);
	frames[FR_WAV1] = make_xbm(vigor_wav_0001_bits, vigor_wav_0001_width, vigor_wav_0001_height);
	frames[FR_WAV2] = make_xbm(vigor_wav_0002_bits, vigor_wav_0002_width, vigor_wav_0002_height);
	frames[FR_CP0] = make_xbm(v2cp0000_bits, v2cp0000_width, v2cp0000_height);
	frames[FR_CP1] = make_xbm(v2cp0001_bits, v2cp0001_width, v2cp0001_height);
	frames[FR_CP2] = make_xbm(v2cp0002_bits, v2cp0002_width, v2cp0002_height);
	frames[FR_CP3] = make_xbm(v2cp0003_bits, v2cp0003_width, v2cp0003_height);
	frames[FR_CP4] = make_xbm(v2cp0004_bits, v2cp0004_width, v2cp0004_height);
}

static void
set_frame(int id)
{
	if (id < 0 || id >= FR_N)
		return;
	cur_frame = id;
	if (assistant != NULL)
		InvalidateRect(assistant, NULL, TRUE);
}

static void
anim_tick(void)
{
	ULONGLONG now;

	if (anim_pos >= anim_len)
		return;
	now = GetTickCount64();
	if (now < anim_due)
		return;
	set_frame(anim_q[anim_pos++]);
	anim_due = now + 250;
}

static void
anim_play(const int *seq, int n, int block)
{
	if (n > 24)
		n = 24;
	memcpy(anim_q, seq, (size_t)n * sizeof(int));
	anim_len = n;
	anim_pos = 0;
	anim_due = GetTickCount64();
	if (!block)
		return;
	while (anim_pos < anim_len) {
		ULONGLONG due;

		set_frame(anim_q[anim_pos++]);
		due = GetTickCount64() + 250;
		while (GetTickCount64() < due) {
			pump_messages();
			Sleep(10);
		}
	}
}

static void
seg(HDC hdc, int x1, int y1, int x2, int y2)
{
	MoveToEx(hdc, x1, y1, NULL);
	LineTo(hdc, x2, y2);
}

static void
draw_bubble(HDC hdc, int width, int height)
{
	RECT r;
	HPEN pen, old;
	int ox = 16;
	int oy = bubble_top ? 0 : height - 36;

	r.left = 0;
	r.top = oy;
	r.right = width;
	r.bottom = oy + 36;
	FillRect(hdc, &r, white);
	pen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
	old = (HPEN)SelectObject(hdc, pen);
	if (bubble_top && corner == 0) {
		seg(hdc, ox + 0, oy + 34, ox + 51, oy + 34);
		seg(hdc, ox + 51, oy + 34, ox + 34, oy + 3);
		seg(hdc, ox + 34, oy + 3, ox + 68, oy + 34);
		seg(hdc, ox + 68, oy + 34, ox + 102, oy + 34);
	} else if (bubble_top) {
		seg(hdc, ox + 102, oy + 34, ox + 51, oy + 34);
		seg(hdc, ox + 51, oy + 34, ox + 68, oy + 3);
		seg(hdc, ox + 68, oy + 3, ox + 34, oy + 34);
		seg(hdc, ox + 34, oy + 34, ox + 0, oy + 34);
	} else if (corner == 2) {
		seg(hdc, ox + 0, oy + 0, ox + 51, oy + 0);
		seg(hdc, ox + 51, oy + 0, ox + 34, oy + 30);
		seg(hdc, ox + 34, oy + 30, ox + 68, oy + 0);
		seg(hdc, ox + 68, oy + 0, ox + 102, oy + 0);
	} else {
		seg(hdc, ox + 102, oy + 0, ox + 51, oy + 0);
		seg(hdc, ox + 51, oy + 0, ox + 68, oy + 30);
		seg(hdc, ox + 68, oy + 30, ox + 34, oy + 0);
		seg(hdc, ox + 34, oy + 0, ox + 0, oy + 0);
	}
	SelectObject(hdc, old);
	DeleteObject(pen);
}

static void
layout_dialog(void)
{
	RECT rc;
	int top, by, th, gap, bw, x, i;

	if (dlg == NULL || ctl_text == NULL)
		return;
	GetClientRect(dlg, &rc);
	top = bubble_top ? 40 : 8;
	by = (bubble_top ? rc.bottom : rc.bottom - 40) - 36;
	th = by - top - 6;
	if (th < 48)
		th = 48;
	MoveWindow(ctl_text, 8, top, rc.right - 16, th, TRUE);
	if (ctl_n < 1)
		return;
	gap = 8;
	bw = (rc.right - 16 - gap * (ctl_n - 1)) / ctl_n;
	if (bw < 48)
		bw = 48;
	x = 8;
	for (i = 0; i < ctl_n; i++) {
		MoveWindow(ctl_btn[i], x, by, bw, 28, TRUE);
		x += bw + gap;
	}
}

/* Themed buttons keep a stale face after SetWindowPos copies the old pixels. */
static void
repaint_tree(HWND hwnd)
{
	HWND child;

	if (hwnd == NULL)
		return;
	RedrawWindow(hwnd, NULL, NULL,
	    RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_FRAME | RDW_UPDATENOW);
	for (child = GetWindow(hwnd, GW_CHILD); child != NULL; child = GetWindow(child, GW_HWNDNEXT)) {
		RedrawWindow(child, NULL, NULL,
		    RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	}
}

static void
corner_point(int outer_w, int outer_h, int dialog, int *x, int *y)
{
	RECT wa;
	int off = dialog ? 200 : 0;

	SystemParametersInfoA(SPI_GETWORKAREA, 0, &wa, 0);
	switch (corner) {
	case 0:
		*x = wa.left;
		*y = wa.top + off;
		break;
	case 1:
		*x = wa.right - outer_w;
		*y = wa.top + off;
		break;
	case 2:
		*x = wa.left;
		*y = wa.bottom - outer_h - off;
		break;
	default:
		*x = wa.right - outer_w;
		*y = wa.bottom - outer_h - off;
		break;
	}
	if (*x < wa.left)
		*x = wa.left;
	if (*y < wa.top)
		*y = wa.top;
	if (*x + outer_w > wa.right)
		*x = wa.right - outer_w;
	if (*y + outer_h > wa.bottom)
		*y = wa.bottom - outer_h;
}

static void
place_windows(void)
{
	int x, y;

	bubble_top = corner < 2;
	if (assistant != NULL) {
		corner_point(aw, ah, 0, &x, &y);
		SetWindowPos(assistant, HWND_TOPMOST, x, y, aw, ah,
		    SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOCOPYBITS);
		repaint_tree(assistant);
	}
	if (dlg != NULL) {
		corner_point(dw, dh, 1, &x, &y);
		SetWindowPos(dlg, HWND_TOPMOST, x, y, dw, dh,
		    SWP_SHOWWINDOW | SWP_NOCOPYBITS);
		layout_dialog();
		repaint_tree(dlg);
	}
}

static void
rotate_pos(void)
{
	int next;
	ULONGLONG now = GetTickCount64();

	if (now < hold_until)
		return;
	hold_until = now + 500;
	do {
		next = v_random(4);
	} while (next == corner);
	corner = next;
	place_windows();
}

static void
poll_hover(void)
{
	POINT pt;
	HWND h;

	if (!GetCursorPos(&pt))
		return;
	h = WindowFromPoint(pt);
	if (assistant != NULL && (h == assistant || IsChild(assistant, h))) {
		if (!hover_asst) {
			hover_asst = 1;
			rotate_pos();
		}
	} else
		hover_asst = 0;
	if (dlg != NULL && (h == dlg || IsChild(dlg, h))) {
		if (!hover_dlg) {
			hover_dlg = 1;
			rotate_pos();
		}
	} else
		hover_dlg = 0;
}

static void
pump_messages(void)
{
	MSG msg;

	while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
}

static LRESULT CALLBACK
asst_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		if (frames[cur_frame] != NULL) {
			HDC mem = CreateCompatibleDC(hdc);
			HGDIOBJ old = SelectObject(mem, frames[cur_frame]);
			BitBlt(hdc, 10, 10, 128, 128, mem, 0, 0, SRCCOPY);
			SelectObject(mem, old);
			DeleteDC(mem);
		}
		EndPaint(hwnd, &ps);
		return (0);
	}
	case WM_CLOSE:
		if (dlg == NULL) {
			confirm_text("Are you sure you don't want to close the Vigor assistant?");
			comment_text("Assistant not closed.");
		}
		return (0);
	}
	return (DefWindowProcA(hwnd, msg, wp, lp));
}

static void
finish_dialog(int act)
{
	pending_act = act;
	dlg_done = 1;
	if (dlg != NULL)
		DestroyWindow(dlg);
}

static void
choose_task(int which)
{
	if (which <= 0) {
		strncpy(pending, wiz_body, sizeof pending - 1);
	} else {
		snprintf(pending, sizeof pending,
		    "If you want to %s then you need to invoke the \"%s\" Whizzer.",
		    wiz_task[which], wiz_task[which]);
	}
	pending[sizeof pending - 1] = '\0';
	finish_dialog(ACT_COMMENT);
}

static void
cancel_dialog(void)
{
	if (dlg_kind == KIND_WIZARD) {
		strncpy(pending, wiz_body, sizeof pending - 1);
		pending[sizeof pending - 1] = '\0';
		finish_dialog(ACT_SERVER_THEN_COMMENT);
	} else if (dlg_kind == KIND_COMMENT || dlg_kind == KIND_EULA)
		finish_dialog(ACT_NONE);
	else
		finish_dialog(ACT_SERVER);
}

static LRESULT CALLBACK
eula_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_VSCROLL || msg == WM_MOUSEWHEEL || msg == WM_KEYDOWN || msg == WM_CHAR) {
		SendMessageA(hwnd, EM_SETSEL, 0, 0);
		SendMessageA(hwnd, EM_SCROLLCARET, 0, 0);
		return (0);
	}
	return (CallWindowProcA(eula_old, hwnd, msg, wp, lp));
}

static LRESULT CALLBACK
dlg_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_PAINT: {
		PAINTSTRUCT ps;
		RECT rc;
		HDC hdc = BeginPaint(hwnd, &ps);
		GetClientRect(hwnd, &rc);
		draw_bubble(hdc, rc.right, rc.bottom);
		EndPaint(hwnd, &ps);
		return (0);
	}
	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case ID_OK:
			finish_dialog(ACT_NONE);
			break;
		case ID_CANCEL:
			cancel_dialog();
			break;
		case ID_T0:
			choose_task(0);
			break;
		case ID_T1:
			choose_task(1);
			break;
		case ID_T2:
			choose_task(2);
			break;
		}
		return (0);
	case WM_CLOSE:
		cancel_dialog();
		return (0);
	case WM_DESTROY:
		if (hwnd == dlg)
			dlg = NULL;
		ctl_text = NULL;
		ctl_n = 0;
		return (0);
	}
	return (DefWindowProcA(hwnd, msg, wp, lp));
}

static void
classic_face(HWND hwnd)
{
	typedef HRESULT (WINAPI *theme_fn)(HWND, const wchar_t *, const wchar_t *);
	static theme_fn set_theme;
	static int ready;

	if (!ready) {
		HMODULE mod = LoadLibraryA("uxtheme.dll");
		if (mod != NULL)
			set_theme = (theme_fn)GetProcAddress(mod, "SetWindowTheme");
		ready = 1;
	}
	if (set_theme != NULL)
		set_theme(hwnd, L"", L"");
}

static HWND
add_btn(const char *label, int id)
{
	HWND b;

	b = CreateWindowA("BUTTON", label,
	    WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
	    0, 0, 40, 24, dlg, (HMENU)(INT_PTR)id, GetModuleHandleA(NULL), NULL);
	SendMessageA(b, WM_SETFONT, (WPARAM)font, TRUE);
	classic_face(b);
	if (ctl_n < 4)
		ctl_btn[ctl_n++] = b;
	return (b);
}

static void
modal(void)
{
	static const int bye[] = { FR_WAV1, FR_REST };

	dlg_done = 0;
	while (!dlg_done && dlg != NULL) {
		MsgWaitForMultipleObjects(0, NULL, FALSE, 15, QS_ALLINPUT);
		anim_tick();
		poll_hover();
		pump_messages();
	}
	anim_play(bye, 2, 0);
}

static void
open_dialog(int kind, const char *text, int wide)
{
	static const int wave[] = {
		FR_REST, FR_WAV1, FR_WAV2, FR_WAV1, FR_WAV2, FR_WAV1, FR_WAV2
	};
	DWORD style = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN;
	DWORD ex = WS_EX_TOPMOST | WS_EX_DLGMODALFRAME;
	RECT r;
	int client_h = kind == KIND_EULA ? 360 : 230;

	dlg_kind = kind;
	ctl_n = 0;
	bubble_top = corner < 2;
	r.left = 0;
	r.top = 0;
	r.right = wide;
	r.bottom = client_h;
	AdjustWindowRectEx(&r, style, FALSE, ex);
	dw = r.right - r.left;
	dh = r.bottom - r.top;
	dlg = CreateWindowExA(ex, "VigorSays", "Vigor Says", style,
	    0, 0, dw, dh, NULL, NULL, GetModuleHandleA(NULL), NULL);
	if (dlg == NULL)
		return;
	ctl_text = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", text,
	    WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | WS_VSCROLL,
	    0, 0, 10, 10, dlg, NULL, GetModuleHandleA(NULL), NULL);
	SendMessageA(ctl_text, WM_SETFONT, (WPARAM)font, TRUE);
	if (kind == KIND_EULA) {
		eula_old = (WNDPROC)SetWindowLongPtrA(ctl_text, GWLP_WNDPROC, (LONG_PTR)eula_proc);
		SendMessageA(ctl_text, EM_SETSEL, 0, 0);
	}
	place_windows();
	ShowWindow(dlg, SW_SHOW);
	SetForegroundWindow(dlg);
	anim_play(wave, 7, 0);
}

static void
dispatch_pending(void)
{
	int act = pending_act;
	char buf[2048];

	pending_act = ACT_NONE;
	memcpy(buf, pending, sizeof buf);
	buf[sizeof buf - 1] = '\0';
	if (act == ACT_SERVER || act == ACT_SERVER_THEN_COMMENT)
		comment_text(SERVER_MSG);
	if (act == ACT_COMMENT || act == ACT_SERVER_THEN_COMMENT)
		comment_text(buf);
}

static void
comment_text(const char *text)
{
	pending_act = ACT_NONE;
	pending[0] = '\0';
	open_dialog(KIND_COMMENT, text, 460);
	if (dlg == NULL)
		return;
	add_btn("OK", ID_OK);
	layout_dialog();
	modal();
	dispatch_pending();
}

static void
confirm_text(const char *text)
{
	int cancel_left;

	pending_act = ACT_NONE;
	pending[0] = '\0';
	open_dialog(KIND_CONFIRM, text, 460);
	if (dlg == NULL)
		return;
	cancel_left = v_random(5) == 0;
	if (cancel_left)
		add_btn("Cancel", ID_CANCEL);
	add_btn("OK", ID_OK);
	if (!cancel_left)
		add_btn("Cancel", ID_CANCEL);
	layout_dialog();
	modal();
	dispatch_pending();
}

static void
wizard_text(const char *wizname, const char *task0, const char *task1,
    const char *task2, const char *descr)
{
	char prompt[1024];
	int cancel_left;

	snprintf(prompt, sizeof prompt,
	    "It looks like you are %s.  Which Vigor Whizzer would you like to use to help you with this?",
	    wizname);
	strncpy(wiz_task[0], task0, sizeof wiz_task[0] - 1);
	strncpy(wiz_task[1], task1, sizeof wiz_task[1] - 1);
	strncpy(wiz_task[2], task2, sizeof wiz_task[2] - 1);
	strncpy(wiz_body, descr, sizeof wiz_body - 1);
	wiz_task[0][sizeof wiz_task[0] - 1] = '\0';
	wiz_task[1][sizeof wiz_task[1] - 1] = '\0';
	wiz_task[2][sizeof wiz_task[2] - 1] = '\0';
	wiz_body[sizeof wiz_body - 1] = '\0';
	strncpy(pending, wiz_body, sizeof pending - 1);
	pending[sizeof pending - 1] = '\0';
	pending_act = ACT_NONE;
	open_dialog(KIND_WIZARD, prompt, 720);
	if (dlg == NULL)
		return;
	cancel_left = v_random(5) == 0;
	if (cancel_left)
		add_btn("Cancel", ID_CANCEL);
	add_btn(task0, ID_T0);
	add_btn(task1, ID_T1);
	add_btn(task2, ID_T2);
	if (!cancel_left)
		add_btn("Cancel", ID_CANCEL);
	layout_dialog();
	modal();
	dispatch_pending();
}

static const char *
taunt_for(int r)
{
	switch (r) {
	case 1: return ("Don't cry --- it won't help.");
	case 2: return ("Don't try this at home, kids!");
	case 3: return ("Or perhaps you'd prefer Russian Routlette, after all?");
	case 4: return ("You'll understand when you're older, dear.");
	case 5: return ("Want some rye?  'Course ya do!");
	case 6:
	case 7:
	case 8: return ("Your blood pressure just went up a point.");
	case 100: return ("Vigor: A compelling argument for pencil and paper.");
	case 101: return ("Vigor: An inspiring example of form following function... to Hell.");
	case 102: return ("Vigor: Because Hell was full.");
	case 103: return ("Vigor is freely redistributable; void where prohibited by law.");
	case 104: return ("If SIGINT doesn't work, try a tranquilizer.");
	case 105: return ("It's all fun and games, until somebody tries to edit a file.");
	case 106: return ("Lovecraft was an optimist.");
	case 107: return ("Vigor: More than just a pretty face!");
	case 108: return ("Vigor: The only text editor known to get indigestion.");
	case 109: return ("Vigor: Where editing text is like playing Paganini on a glass harmonica.");
	case 110: return ("Vigor: All the problems and twice the bugs.");
	case 111: return ("Vigor: Dissatisfaction guaranteed.");
	case 112: return ("Vigor: Don't get frustrated without it.");
	case 113: return ("Vigor: Even not doing anything would have been better than nothing.");
	case 114: return ("Vigor: Even your dog won't like it.");
	case 115: return ("Vigor: Form follows malfunction.");
	case 116: return ("Vigor: It could be worse, but it'll take time.");
	case 117: return ("Vigor: It could happen to you.");
	case 118: return ("Vigor: Let it get in *your* way.");
	case 119: return ("Vigor: Live the nightmare.");
	case 120: return ("Vigor: Putting new limits on productivity.");
	case 121: return ("Vigor: Some voids are better left unfilled.");
	case 122: return ("Vigor: The problem for your problem.");
	case 150: return ("Vi: It was hard to write; it should be hard to use.");
	case 151: return ("VI VI VI: The editor of the beast");
	case 152: return ("Vigor: Putting the ass in assist");
	case 200: return ("X Windows: A mistake carried out to perfection.");
	case 201: return ("X Windows: A terminal disease.");
	default: return (NULL);
	}
}

static void
show_eula(void)
{
	pending_act = ACT_NONE;
	open_dialog(KIND_EULA, eula_text, 520);
	if (dlg == NULL)
		return;
	add_btn("I Accept", ID_OK);
	layout_dialog();
	modal();
}

static void
start_cp(void)
{
	static const int seq[] = {
		FR_REST, FR_CP4, FR_CP3, FR_CP2, FR_CP1, FR_CP0,
		FR_CP0, FR_CP1, FR_CP2, FR_CP3, FR_CP4, FR_REST
	};

	cp_until = GetTickCount64() + 250 * (sizeof seq / sizeof seq[0]);
	anim_play(seq, (int)(sizeof seq / sizeof seq[0]), 0);
}

static void
vigor_randomness(void)
{
	int r;
	const char *t;

	if (!alive || dlg != NULL || GetTickCount64() < cp_until)
		return;
	r = v_random(4000);
	if (r == 0) {
		confirm_text("Are you sure you wish to have a spontanious pohn fahr?");
		return;
	}
	t = taunt_for(r);
	if (t != NULL) {
		comment_text(t);
		return;
	}
	if (r >= 500 && r <= 529)
		start_cp();
}

static void
timer_tick(void)
{
	ULONGLONG now;

	if (!alive || dlg != NULL)
		return;
	now = GetTickCount64();
	if (now < next_random)
		return;
	next_random = now + 5000;
	vigor_randomness();
}

struct phrase {
	const char *text;
	const char *cp;
	const char *wizname;
	const char *task0;
	const char *task1;
	const char *task2;
	const char *descr;
};

static struct phrase phrases[] = {
	{ "#!", NULL, "starting a program",
	  "write a shell script", "write a C program", "write Tcl code",
	  "You can use a shell script to give a series of commands that are "
	  "implemented by the shell.  You should give commands in shell "
	  "language.  The shell script should also tell the computer which "
	  "shell it should be run under." },
	{ "#include", NULL, "starting a program",
	  "write a C program", "write Tcl code", "write a Lisp program",
	  "You can use a C program to perform a variety of tasks that cannot "
	  "be easily written by shell scripts.  Writing a C program requires "
	  "careful attention to detail.  You will have to compile your C program "
	  "before you can use it." },
	{ "(", NULL, "starting a program",
	  "write a Lisp program", "write Tcl code", "write a shell script",
	  "You can write a Lisp program to perform most tasks you can with a C "
	  "program.  Lisp programs also have the advantage that very few "
	  "programmers know Lisp, so your employer will have to keep you on "
	  "staff to maintain it." },
	{ NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};

void
vigor_pump(void)
{
	if (assistant == NULL || in_pump)
		return;
	in_pump = 1;
	anim_tick();
	poll_hover();
	timer_tick();
	pump_messages();
	in_pump = 0;
}

void
vigor_on_select(void)
{
	if (assistant == NULL || !alive || dlg != NULL)
		return;
	vigor_randomness();
}

int
vigor_init(void *gp)
{
	WNDCLASSA wc;
	RECT r;
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	DWORD ex = WS_EX_TOPMOST;
	static const int intro[] = {
		FR_EMPTY, FR_ENT1, FR_ENT2, FR_ENT3, FR_ENT4, FR_REST
	};
	struct phrase *pp;

	(void)gp;
	if (assistant != NULL)
		return (0);

	white = (HBRUSH)GetStockObject(WHITE_BRUSH);
	font = CreateFontA(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
	    ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
	    DEFAULT_QUALITY, FF_DONTCARE, "Times New Roman");
	load_frames();
	cur_frame = FR_EMPTY;

	memset(&wc, 0, sizeof wc);
	wc.lpfnWndProc = asst_proc;
	wc.hInstance = GetModuleHandleA(NULL);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = white;
	wc.lpszClassName = "VigorAssistant";
	RegisterClassA(&wc);
	wc.lpfnWndProc = dlg_proc;
	wc.lpszClassName = "VigorSays";
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	RegisterClassA(&wc);

	r.left = 0;
	r.top = 0;
	r.right = 148;
	r.bottom = 148;
	AdjustWindowRectEx(&r, style, FALSE, ex);
	aw = r.right - r.left;
	ah = r.bottom - r.top;
	assistant = CreateWindowExA(ex, "VigorAssistant", "Vigor",
	    style, 0, 0, aw, ah, NULL, NULL, GetModuleHandleA(NULL), NULL);
	if (assistant == NULL)
		return (1);
	for (pp = phrases; pp->text != NULL; pp++)
		pp->cp = pp->text;
	corner = 1;
	ShowWindow(assistant, SW_SHOW);
	UpdateWindow(assistant);
	rotate_pos();
	anim_play(intro, 6, 1);
	show_eula();
	comment_text("Hi there!  I'm the Vigor Assistant!  If you have any questions about VI, just ask me!");
	comment_text("If you haven't used Vigor before, be sure to remember that the Vigor Assistant is here to help you!");
	alive = 1;
	next_random = GetTickCount64() + 5000;
	return (0);
}

void
vigor_comment(char *expr, void *gp)
{
	char text[2048];

	(void)gp;
	if (assistant == NULL)
		return;
	unquote(expr, text, sizeof text);
	comment_text(text);
}

void
vigor_hint(char *expr, void *gp)
{
	char text[2048];

	(void)gp;
	if (assistant == NULL)
		return;
	unquote(expr, text, sizeof text);
	confirm_text(text);
}

void
vigor_confirm(char *expr, void *gp)
{
	char text[2048];

	(void)gp;
	if (assistant == NULL)
		return;
	unquote(expr, text, sizeof text);
	confirm_text(text);
}

void
vigor_maybe_confirm(int prob, char *expr, void *gp)
{
	char text[2048];

	(void)gp;
	if (assistant == NULL)
		return;
	if (v_random(100) > prob)
		return;
	unquote(expr, text, sizeof text);
	confirm_text(text);
}

void
vigor_maybe_wizard(int prob, char *wizname, char *task0, char *task1,
    char *task2, int this_task, char *descr, void *gp)
{
	char name[256], t0[256], t1[256], t2[256], body[2048];

	(void)gp;
	(void)this_task;
	if (assistant == NULL)
		return;
	if (v_random(100) > prob)
		return;
	unquote(wizname, name, sizeof name);
	unquote(task0, t0, sizeof t0);
	unquote(task1, t1, sizeof t1);
	unquote(task2, t2, sizeof t2);
	unquote(descr, body, sizeof body);
	wizard_text(name, t0, t1, t2, body);
}

void
vigor_gotchar(char ch)
{
	struct phrase *pp;

	if (assistant == NULL)
		return;
	for (pp = phrases; pp->text != NULL; pp++) {
		if (*pp->cp != ch)
			pp->cp = pp->text;
		else if (!*++pp->cp)
			wizard_text(pp->wizname, pp->task0, pp->task1, pp->task2, pp->descr);
	}
}
