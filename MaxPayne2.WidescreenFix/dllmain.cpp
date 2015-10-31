#include "..\includes\stdafx.h"

struct Screen
{
	int nWidth;
	int nHeight;
	float fWidth;
	float fHeight;
	float fDynamicScreenFieldOfViewScale;
	float fFieldOfView;
	float fAspectRatio;
	float fGlobalScale;
	float fHudScale;
	float fHudOffset;
	float fHudOffsetWide;
} Screen;

struct TextCoords
{
	float a;
	float b;
	float c;
	float d;
};

union HudPos
{
	DWORD dwPos;
	float fPos;
} HudScaleX, HudScaleY;

float f1_480 = 1.0f / 480.0f;

HMODULE e2mfc, e2_d3d8_driver_mfc;
float fFOVFactor;
bool bComicsMode, bFixHud, bWidescreenHud;
float fWidthScale, fHalfWidthScale, f1_fWidthScale;
float fDouble1_fWidthScale, fHalf1_fWidthScale;
uchar* IsInComicsMode;
DWORD GetScreenRectJmp;
DWORD jmpAddr, jmpAddr2, jmpAddr3, jmpAddr4, jmpAddr5;
DWORD jmpAddr6, jmpAddr7, jmpAddr8, jmpAddr9;
DWORD nCounter;
float* pHudElementPosX; float* pHudElementPosY; TextCoords* pTextElementPosX;
float fMirrorFactor;
float fVisibilityFactor1, fVisibilityFactor2;
float fViewPortSizeX = 640.0f, fViewPortSizeY = 480.0f;
float fFullscreen2DScale;
int nCutsceneBorders;

int __fastcall PDriverGetWidth(int _this)
{
	if (*(DWORD *)(_this + 48))
		Screen.nWidth = *(DWORD *)(_this + 52);
	else
		Screen.nWidth = *(DWORD *)(_this + 4);

	return Screen.nWidth;
}

int __fastcall PDriverGetHeight(int _this)
{
	if (*(DWORD *)(_this + 48))
		Screen.nHeight = *(DWORD *)(_this + 56);
	else
		Screen.nHeight = *(DWORD *)(_this + 8);

	return Screen.nHeight;
}

void __fastcall P_CameraSetFOVHook(int _this, float a2)
{
	if (a2 != *(float *)(_this + 284))
	{
		if (a2 >= 0.0)
		{
			Screen.fFieldOfView = 3.1241393f;
			if (a2 <= 3.1241393)
				Screen.fFieldOfView = a2 * Screen.fDynamicScreenFieldOfViewScale;
			*(BYTE *)(_this + 236) |= 0x40u;
			*(float *)(_this + 284) = Screen.fFieldOfView;
		}
		else
		{
			*(BYTE *)(_this + 236) |= 0x40u;
			*(DWORD *)(_this + 284) = 0;
		}
	}
}

void __fastcall FOVHook(DWORD *_this, float a2)
{
	Screen.fFieldOfView = a2 * Screen.fDynamicScreenFieldOfViewScale;
	*((float *)_this + 334) = a2;
}

void __declspec(naked)ForceEntireViewport()
{
	__asm mov ax, 1
	__asm ret
}

void __declspec(naked)DisableSubViewport()
{
	__asm ret 10h
}

DWORD _EAX, _EBX, _nEDX; float _EDX, fDiffFactor;
void __declspec(naked)PCameraValidateHook()
{
	__asm mov _EAX, eax
	__asm mov _EBX, ebx
	__asm mov edx, [esp + 4]
	__asm mov _EDX, edx
	__asm mov _nEDX, edx
	
	if (_EDX >= 0.7f && _EDX <= 0.8f)
	{
		if ((_EBX == 1 && _EAX == 8) || (_EBX == 0 && _EAX != 8))
		{
			_EDX *= fDiffFactor;
		}
	}
	else
	{
		if (_EAX == 8 && _EDX < 0.7f)
		{
			if (_EBX == 1 && _nEDX != 0x3ED413CD) //0.414213568, it's to avoid comics stretching
				_EDX *= fDiffFactor;
		}
		else
		{
			if ((_EDX > 0.8f/* && _EDX <= 1.5f*/) && _nEDX != 0x3F800000)
			{
				if ((_EBX == 1 && _EAX == 8) || (_EBX == 0 && _EAX != 8))
					_EDX *= fDiffFactor;
			}
		}
	}

	__asm mov edx, _EDX
	__asm mov[esi + 20Ch], edx
	__asm jmp jmpAddr
}

DWORD WINAPI ComicsHandler(LPVOID)
{
	CIniReader iniReader("");
	bComicsMode = iniReader.ReadInteger("MAIN", "ComicsMode", 1) != 0;
	DWORD nComicsModeKey = iniReader.ReadInteger("MAIN", "ComicsModeKey", VK_F2);
	bool bPatched;

	injector::WriteMemory((DWORD)e2mfc + 0x176D4 + 0x2, &fViewPortSizeX, true);
	injector::WriteMemory((DWORD)e2mfc + 0x17719 + 0x2, &fViewPortSizeY, true);
	injector::WriteMemory((DWORD)e2mfc + 0x1779A + 0x2, &fViewPortSizeY, true);

	while (true)
	{
		Sleep(0);

		if ((GetAsyncKeyState(nComicsModeKey) & 1) && (*IsInComicsMode == 0xAE))
		{
			bComicsMode = !bComicsMode;
			bPatched = !bPatched;
			iniReader.WriteInteger("MAIN", "ComicsMode", bComicsMode);
			while ((GetAsyncKeyState(nComicsModeKey) & 0x8000) > 0) { Sleep(0); }
		}

		if (bComicsMode)
		{
			if (*IsInComicsMode == 0xAE)
			{
				if (!bPatched)
				{
					fViewPortSizeX = 480.0f * Screen.fAspectRatio;
					fViewPortSizeY = 480.0f;
					bPatched = true;
				}
			}
			else
			{
				if (bPatched)
				{
					fViewPortSizeX = 640.0f;
					fViewPortSizeY = 480.0f;
					bPatched = false;
				}
			}
		}
		else
		{
			if (*IsInComicsMode == 0xAE)
			{
				if (!bPatched)
				{
					fViewPortSizeX = (480.0f * Screen.fAspectRatio) / 1.17936117936f;
					fViewPortSizeY = 480.0f / 1.17936117936f;
					bPatched = true;
				}
			}
			else
			{
				if (bPatched)
				{
					fViewPortSizeX = 640.0f;
					fViewPortSizeY = 480.0f;
					bPatched = false;
				}
			}
		}
	}
	return 0;
}

float ElementPosX, ElementNewPosX1, ElementNewPosX2;
float ElementPosY, ElementNewPosY1, ElementNewPosY2;
DWORD __EAX, __ESP20, __ESP2C, __ESP30, __ESP3C, __ESP34, __ESP44;
float __ECX;
float* f10042294;
float f10042B58 = 0.003125f;
float f10042B50 = -1.0f;
bool bDoFix;
void __declspec(naked) P_HudPosHook2()
{
	__asm mov __EAX, eax

	__asm mov eax, [esp + 2Ch]
	__asm mov __ESP2C, eax

	__asm mov eax, [esp+3Ch]
	__asm mov __ESP3C, eax

	__asm mov eax, [esp + 44h]
	__asm mov __ESP44, eax

	__asm mov eax, __EAX
		
	bDoFix = false;

	if ((__ESP44 != 0x43F00000 && __ESP2C == 0x459C4400 || __ESP2C == 0x40400000) && (__ESP3C != 0x43000000 && __ESP3C != 0x42800000)) //bullet time fullscreen effect and player's shadow 
	{
		bDoFix = true;
		_asm
		{
			fmul    dword ptr ds : Screen.fHudScale
			jmp	    jmpAddr4
		}
	}

	_asm
	{
		fmul    ds : f10042B58
		jmp	    jmpAddr4
	}
}

void __declspec(naked) P_HudPosHook3()
{
	if (bDoFix) 
	{
		_asm
		{
			fmul    dword ptr ds : Screen.fHudOffset
			jmp	    jmpAddr5
		}
	}

	_asm
	{
		fmul    ds : f10042B50
		jmp	    jmpAddr5
	}
}

void __declspec(naked) P_HudPosHook()
{
	__asm mov __EAX, eax
	__asm mov __ECX, ecx
	__asm mov __ESP20, esp
	__asm mov __ESP30, esp
	__asm mov __ESP34, esp
	__ESP20 += 0x20;
	__ESP30 += 0x30;
	__ESP34 += 0x34;

	ElementPosX = *pHudElementPosX;
	ElementPosY = *pHudElementPosY;
	ElementNewPosX1 = ElementPosX;
	ElementNewPosY1 = ElementPosY;
	ElementNewPosX2 = ElementPosX;
	ElementNewPosY2 = ElementPosY;

	if (bWidescreenHud)
	{
		if (ElementPosX == 7.0f) // bullet time meter
		{
			ElementNewPosX1 = ElementPosX + Screen.fHudOffsetWide;
		}

		if (ElementPosX == 8.0f) // bullet time overlay()
		{
			ElementNewPosX1 = ElementPosX + Screen.fHudOffsetWide;
		}

		if (ElementPosX == 12.0f) // painkillers
		{
			ElementNewPosX1 = ElementPosX + Screen.fHudOffsetWide;
		}

		if (ElementPosX == 22.5f) //health bar and overlay
		{
			ElementNewPosX1 = ElementPosX + Screen.fHudOffsetWide;
		}

		if (ElementPosX == 96.0f) // other weapons name
		{
			ElementNewPosX1 = ElementPosX - Screen.fHudOffsetWide;
		}

		if (ElementPosX == 192.0f) //molotovs/grenades name pos
		{
			ElementNewPosX1 = ElementPosX - Screen.fHudOffsetWide;
		}
	}

	ElementNewPosX2 = ElementNewPosX1;

	if (bDoFix && ElementPosX == 0.0f && ElementPosY == 0.0f && __EAX == 2 && __ECX == 640.0f && *(DWORD*)__ESP20 != 0x40400000 && *(DWORD*)__ESP30 != 0x43800000) // fading, 0x43800000 related to player's shadow
	{
		ElementNewPosX1 = ElementPosX + (Screen.fHudOffsetWide + fFullscreen2DScale);
		ElementNewPosX2 = ElementPosX - (Screen.fHudOffsetWide + fFullscreen2DScale);					  
		ElementNewPosY1 = ElementPosY + (Screen.fHudOffsetWide + fFullscreen2DScale);
		ElementNewPosY2 = ElementPosY - (Screen.fHudOffsetWide + fFullscreen2DScale);
	}
	
	_asm
	{
		fld     ds:f10042294
		mov     eax, [esi+164h]
		fsub    dword ptr[ElementNewPosX1]
		mov     [esp+44h], eax
		sub     esp, 8
		fstp    dword ptr [esp+30h]
		fld     dword ptr [esp+1Ch]
		fsub    dword ptr[ElementNewPosX2]
		fstp    dword ptr [esp+1Ch]
		fld     ds:f10042294
		fsub    dword ptr[ElementNewPosY1]
		fstp    dword ptr [esp+2Ch]
		fld     dword ptr [esp+20h]
		fsub    dword ptr[ElementNewPosY2]
		jmp	    jmpAddr3
	}
}

DWORD pTempPtr;
float TextPosX, TextNewPosX, TextUnkVal;
void __declspec(naked) P_TextPosHook()
{
	__asm mov eax, [esp + 0xC]
	__asm mov eax, [eax + 0x5C]
	__asm mov TextUnkVal, eax

	TextPosX = pTextElementPosX->a;
	TextNewPosX = TextPosX;

	_asm pushad
	_asm pushfd

	if ((pTextElementPosX->a == 0.0f || pTextElementPosX->a == -8.0f || pTextElementPosX->a == -16.0f || pTextElementPosX->a == -24.0f || pTextElementPosX->a == -32.0f) && pTextElementPosX->b == -10.5f && (pTextElementPosX->c == 8.0f || pTextElementPosX->c == 16.0f || pTextElementPosX->c == 24.0f || pTextElementPosX->c == 32.0f) && pTextElementPosX->d == 21) //ammo numbers(position depends on digits amount)
	{
		if (TextUnkVal < 0.0f)
		{
			TextNewPosX = TextPosX - Screen.fHudOffsetWide;
		}
		else
		{
			TextNewPosX = TextPosX + Screen.fHudOffsetWide;
		}
	}

	_asm popfd
	_asm popad

	__asm fld    dword ptr[TextNewPosX]
	__asm jmp    jmpAddr6
}

float __fastcall GetBordersSize(int _this)
{
	if (nCutsceneBorders > 1)
	{
		return *(float *)(*(DWORD *)_this + 4692) * (1.0f / ((4.0f / 3.0f) / Screen.fAspectRatio));
	}
	else
	{
		return 1.0f;
	}
}

void __declspec(naked) CWndCreateExHook()
{
	_asm
	{
		mov     eax, WS_VISIBLE | WS_POPUP
		mov     [ebp - 10h], eax
		jmp     jmpAddr7
	}
}


DWORD WINAPI Thread(LPVOID)
{
	CIniReader iniReader("");
	bool bFixWindowBorder = iniReader.ReadInteger("MAIN", "FixWindowBorder", 1) != 0;
	if (bFixWindowBorder)
	{
		injector::MakeJMP((DWORD)GetModuleHandle("mfc71.dll") + 0x12184, CWndCreateExHook, true); //fullscreen mode border fix for win10
		jmpAddr7 = (DWORD)GetModuleHandle("mfc71.dll") + 0x12184 + 0x6;
	}

	bool bUseGameFolderForSavegames = iniReader.ReadInteger("MAIN", "UseGameFolderForSavegames", 0) != 0;
	if (bUseGameFolderForSavegames)
		injector::WriteMemory<uchar>(0x41D14C, 0x85, true);

	bool bAltTab = iniReader.ReadInteger("MAIN", "AllowAltTabbingWithoutPausing", 0) != 0;
	if (bAltTab)
		injector::MakeNOP(0x404935, 5, true);

	nCutsceneBorders = iniReader.ReadInteger("MAIN", "CutsceneBorders", 1);
	if (nCutsceneBorders)
		injector::MakeCALL(0x488C68, GetBordersSize, true);

	fFOVFactor = iniReader.ReadFloat("MAIN", "FOVFactor", 0.0f);
	bFixHud = iniReader.ReadInteger("MAIN", "FixHud", 1) != 0;
	bWidescreenHud = iniReader.ReadInteger("MAIN", "WidescreenHud", 1) != 0;
	float fWidescreenHudOffset = iniReader.ReadFloat("MAIN", "WidescreenHudOffset", 100.0f);
	fFullscreen2DScale = iniReader.ReadFloat("MAIN", "Fullscreen2DScale", 10.0f);
	if (!fWidescreenHudOffset) { fWidescreenHudOffset = 100.0f; }
	if (!fFullscreen2DScale) { fFullscreen2DScale = 10.0f; }

	do
	{
		Sleep(100);
		e2mfc = GetModuleHandle("e2mfc.dll");
	} while (e2mfc == NULL);

	injector::MakeCALL((DWORD)e2mfc + 0x176AF, PDriverGetWidth, true);
	injector::MakeCALL((DWORD)e2mfc + 0x176F4, PDriverGetHeight, true);

	do
	{
		Sleep(0);
	} while (Screen.nWidth == 0 || Screen.nHeight == 0);

	Screen.fWidth = static_cast<float>(Screen.nWidth);
	Screen.fHeight = static_cast<float>(Screen.nHeight);
	Screen.fAspectRatio = Screen.fWidth / Screen.fHeight;
	Screen.fDynamicScreenFieldOfViewScale = fFOVFactor;
	if (fFOVFactor)
		injector::MakeJMP((DWORD)GetProcAddress(e2mfc, "?setFOV@P_Camera@@QAEXM@Z"), P_CameraSetFOVHook, true);

	IsInComicsMode = (uchar*)((DWORD)e2mfc + 0x61183);

	if (Screen.fAspectRatio > (4.0f / 3.0f))
	{
		fWidthScale = 640.0f / Screen.fAspectRatio;
		fHalfWidthScale = fWidthScale / 2.0f;
		f1_fWidthScale = 1.0f / fWidthScale;
		fDouble1_fWidthScale = f1_fWidthScale * 2.0f;
		fHalf1_fWidthScale = f1_fWidthScale / 2.0f;

		injector::MakeJMP((DWORD)e2mfc + 0x156A0, DisableSubViewport, true);
		injector::MakeJMP((DWORD)e2mfc + 0x14C80, ForceEntireViewport, true);

		fDiffFactor = 1.0f / ((4.0f / 3.0f) / (Screen.fAspectRatio));
		injector::MakeJMP((DWORD)e2mfc + 0x15037, PCameraValidateHook, true);
		jmpAddr = (DWORD)e2mfc + 0x15041;

		//object disappearance fix
		injector::WriteMemory<float>((DWORD)e2mfc + 0x15B87 + 0x6, 0.0f, true);

		do
		{
			Sleep(100);
		} while (GetModuleHandle("X_GameObjectsMFC.dll") == NULL);

		//mirrors fix
		fMirrorFactor = 1.0f * ((4.0f / 3.0f) / (Screen.fAspectRatio));
		injector::WriteMemory((DWORD)GetModuleHandle("X_GameObjectsMFC.dll") + 0x101F39 + 0x2, &fMirrorFactor, true);
		jmpAddr2 = (DWORD)e2mfc + 0x17155 + 0x6;

		//doors graphics fix
		fVisibilityFactor1 = 0.5f;
		fVisibilityFactor2 = 1.5f;
		injector::WriteMemory((DWORD)GetModuleHandle("X_GameObjectsMFC.dll") + 0xA8D0 + 0x4CE + 0x2, &fVisibilityFactor1, true);
		injector::WriteMemory((DWORD)GetModuleHandle("X_GameObjectsMFC.dll") + 0xA8D0 + 0x485 + 0x2, &fVisibilityFactor2, true);

		if (bFixHud)
		{


			Screen.fHudOffset = ((-1.0f / Screen.fAspectRatio) * (4.0f / 3.0f));
			Screen.fHudScale = (1.0f / (480.0f * Screen.fAspectRatio)) * 2.0f;
			injector::MakeJMP((DWORD)e2mfc + 0xF011, P_HudPosHook2, true);
			jmpAddr4 = (DWORD)e2mfc + 0xF011 + 0x6;
			injector::MakeJMP((DWORD)e2mfc + 0xF05E, P_HudPosHook3, true);
			jmpAddr5 = (DWORD)e2mfc + 0xF05E + 0x6;

			injector::WriteMemory<float>((DWORD)e2mfc + 0x42B50, Screen.fHudOffset, true);
			injector::WriteMemory<float>((DWORD)e2mfc + 0x42B58, Screen.fHudScale, true);

			pHudElementPosX = (float*)((DWORD)e2mfc + 0x61134);
			pHudElementPosY = (float*)((DWORD)e2mfc + 0x61138);
			f10042294 = (float*)((DWORD)e2mfc + 0x42294);
			injector::MakeJMP((DWORD)e2mfc + 0xF2B3, P_HudPosHook, true);
			jmpAddr3 = (DWORD)e2mfc + 0xF2B3 + 0x45;

			if (bWidescreenHud)
			{
				Screen.fHudOffsetWide = fWidescreenHudOffset;

				if (Screen.fAspectRatio < (16.0f / 9.0f))
				{
					Screen.fHudOffsetWide = fWidescreenHudOffset / (((16.0f / 9.0f) / (Screen.fAspectRatio)) * 1.5f);
				}

				pTextElementPosX = (TextCoords*)((DWORD)e2mfc + 0x60374);
				injector::MakeJMP((DWORD)e2mfc + 0xAACC, P_TextPosHook, true);
				jmpAddr6 = (DWORD)e2mfc + 0xAACC + 0x6;
			}
		}

		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&ComicsHandler, NULL, 0, NULL);
	}
	return 0;
}



BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD reason, LPVOID /*lpReserved*/)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&Thread, NULL, 0, NULL);
	}
	return TRUE;
}