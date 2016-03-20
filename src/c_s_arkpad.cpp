/* Nintendulator - Win32 NES emulator written in C++
 * Copyright (C) 2002-2011 QMT Productions
 *
 * $URL$
 * $Id$
 */

#include "stdafx.h"
#include "Nintendulator.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"
#include "GFX.h"

namespace Controllers
{
#include <pshpack1.h>
struct StdPort_ArkanoidPaddle_State
{
	unsigned char Bits;
	unsigned short Pos;
	unsigned char BitPtr;
	unsigned char Strobe;
	unsigned char Button;
	unsigned char NewBits;
};
#include <poppack.h>
int	StdPort_ArkanoidPaddle::Save (FILE *out)
{
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}
int	StdPort_ArkanoidPaddle::Load (FILE *in, int version_id)
{
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}
void	StdPort_ArkanoidPaddle::Frame (unsigned char mode)
{
	int x, i, bits;
	if (mode & MOV_PLAY)
	{
		State->Pos = MovData[0] | ((MovData[1] << 8) & 0x7F);
		State->Button = MovData[1] >> 7;
	}
	else
	{
		State->Button = IsPressed(Buttons[0]);
		State->Pos += MouseState.lX;
		if (State->Pos < 196)
			State->Pos = 196;
		if (State->Pos > 484)
			State->Pos = 484;
	}
	if (mode & MOV_RECORD)
	{
		MovData[0] = (unsigned char)(State->Pos & 0xFF);
		MovData[1] = (unsigned char)((State->Pos >> 8) | (State->Button << 7));
	}
	bits = 0;
	x = ~State->Pos;
	for (i = 0; i < 9; i++)
	{
		bits <<= 1;
		bits |= x & 1;
		x >>= 1;
	}
	State->NewBits = bits;
}
unsigned char	StdPort_ArkanoidPaddle::Read (void)
{
	unsigned char result;
	if (State->BitPtr < 8)
		result = (char)((State->Bits >> State->BitPtr++) & 1) << 4;
	else	result = 0x10;
	if (State->Button)
		result |= 0x08;
	return result;
}
void	StdPort_ArkanoidPaddle::Write (unsigned char Val)
{
	if ((!State->Strobe) && (Val & 1))
	{
		State->Bits = State->NewBits;
		State->BitPtr = 0;
	}
	State->Strobe = Val & 1;
}
INT_PTR	CALLBACK	StdPort_ArkanoidPaddle_ConfigProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int dlgLists[1] = {IDC_CONT_D0};
	int dlgButtons[1] = {IDC_CONT_K0};
	StdPort *Cont;
	if (uMsg == WM_INITDIALOG)
	{
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		Cont = (StdPort *)lParam;
	}
	else	Cont = (StdPort *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	return ParseConfigMessages(hDlg, 1, dlgLists, dlgButtons, Cont ? Cont->Buttons : NULL, uMsg, wParam, lParam);
}
void	StdPort_ArkanoidPaddle::Config (HWND hWnd)
{
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_STDPORT_ARKANOIDPADDLE), hWnd, StdPort_ArkanoidPaddle_ConfigProc, (LPARAM)this);
}
void	StdPort_ArkanoidPaddle::SetMasks (void)
{
	MaskMouse = TRUE;
}
StdPort_ArkanoidPaddle::~StdPort_ArkanoidPaddle (void)
{
	delete State;
	delete[] MovData;
}
StdPort_ArkanoidPaddle::StdPort_ArkanoidPaddle (DWORD *buttons)
{
	Type = STD_ARKANOIDPADDLE;
	NumButtons = 1;
	Buttons = buttons;
	State = new StdPort_ArkanoidPaddle_State;
	MovLen = 2;
	MovData = new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->Bits = 0;
	State->Pos = 340;
	State->BitPtr = 0;
	State->Strobe = 0;
	State->Button = 0;
	State->NewBits = 0;
}
} // namespace Controllers