// GdiPlus.cpp: implementation of the CGdiPlus class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GdiPlus.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////

struct gdix_StartupInput
{
    UINT32 GdiplusVersion;
    void* DebugEventCallback;  /* DebugEventProc */
    BOOL SuppressBackgroundThread;
    BOOL SuppressExternalCodecs;
};

struct gdix_StartupOutput
{
	// not used
};

//////////////////////////////////////////////////////////////////////

typedef gdix_Status (STDAPICALLTYPE* PFNSTARTUP)(ULONG_PTR*, const gdix_StartupInput*, gdix_StartupOutput*);
typedef void		(STDAPICALLTYPE* PFNSHUTDOWN)(ULONG_PTR);

// Graphics management 
typedef gdix_Status (STDAPICALLTYPE *PFNCREATEFROMHDC)(HDC,gdix_Graphics**);
typedef gdix_Status (STDAPICALLTYPE *PFNDELETEGRAPHICS)(gdix_Graphics*);
typedef gdix_Status (STDAPICALLTYPE *PFNSETSMOOTHINGMODE)(gdix_Graphics*,gdix_SmoothingMode);

// Pen management 
typedef gdix_Status (STDAPICALLTYPE *PFNCREATEPEN1)(gdix_ARGB,gdix_Real,gdix_Unit,gdix_Pen**);
typedef gdix_Status (STDAPICALLTYPE *PFNDELETEPEN)(gdix_Pen*);
typedef gdix_Status (STDAPICALLTYPE *PFNSETPENWIDTH)(gdix_Pen*,gdix_Real);
typedef gdix_Status (STDAPICALLTYPE *PFNSETPENCOLOR)(gdix_Pen*,gdix_ARGB);

// Brush management 
typedef gdix_Status (STDAPICALLTYPE *PFNCREATESOLIDFILL)(gdix_ARGB,gdix_SolidFill**);
typedef gdix_Status (STDAPICALLTYPE *PFNDELETEBRUSH)(gdix_Brush*);
typedef gdix_Status (STDAPICALLTYPE *PFNSETSOLIDFILLCOLOR)(gdix_SolidFill*,gdix_ARGB);

// Font management 
typedef gdix_Status (STDAPICALLTYPE *PFNCREATEFONTFROMDC)(HDC,gdix_Font**);
typedef gdix_Status (STDAPICALLTYPE *PFNCREATEFONTFROMLOGFONTW)(HDC,const LOGFONTW*,gdix_Font**);
typedef gdix_Status (STDAPICALLTYPE *PFNDELETEFONT)(gdix_Font*);

// String format management 
typedef gdix_Status (STDAPICALLTYPE *PFNCREATESTRINGFORMAT)(INT,LANGID,gdix_StringFormat**);
typedef gdix_Status (STDAPICALLTYPE *PFNDELETESTRINGFORMAT)(gdix_StringFormat*);
typedef gdix_Status (STDAPICALLTYPE *PFNSETSTRINGFORMATFLAGS)(gdix_StringFormat*,INT);
typedef gdix_Status (STDAPICALLTYPE *PFNSETSTRINGFORMATALIGN)(gdix_StringFormat*,gdix_StringAlignment);

// Path management 
typedef gdix_Status (STDAPICALLTYPE *PFNCREATEPATH)(gdix_FillMode,gdix_Path**);
typedef gdix_Status (STDAPICALLTYPE *PFNDELETEPATH)(gdix_Path*);
typedef gdix_Status (STDAPICALLTYPE *PFNRESETPATH)(gdix_Path*);
typedef gdix_Status (STDAPICALLTYPE *PFNADDPATHARC)(gdix_Path*,gdix_Real,gdix_Real,gdix_Real,gdix_Real,gdix_Real,gdix_Real);
typedef gdix_Status (STDAPICALLTYPE *PFNADDPATHLINE)(gdix_Path*,gdix_Real,gdix_Real,gdix_Real,gdix_Real);
typedef gdix_Status (STDAPICALLTYPE *PFNADDPATHRECTANGLE)(gdix_Path*,gdix_Real,gdix_Real,gdix_Real,gdix_Real);

// Draw methods 
typedef gdix_Status (STDAPICALLTYPE *PFNDRAWLINE)(gdix_Graphics*,gdix_Pen*,gdix_Real,gdix_Real,gdix_Real,gdix_Real);
typedef gdix_Status (STDAPICALLTYPE *PFNDRAWLINES)(gdix_Graphics*,gdix_Pen*,const gdix_PointF*,INT);
typedef gdix_Status (STDAPICALLTYPE *PFNDRAWPIE)(gdix_Graphics*,gdix_Pen*,gdix_Real,gdix_Real,gdix_Real,gdix_Real,gdix_Real,gdix_Real);
typedef gdix_Status (STDAPICALLTYPE *PFNDRAWPOLYGON)(gdix_Graphics*,gdix_Pen*,const gdix_PointF*,INT);

// Fill methods 
typedef gdix_Status (STDAPICALLTYPE *PFNFILLRECTANGLE)(gdix_Graphics*,gdix_Brush*,gdix_Real,gdix_Real,gdix_Real,gdix_Real);
typedef gdix_Status (STDAPICALLTYPE *PFNFILLPOLYGON)(gdix_Graphics*,gdix_Brush*,const gdix_PointF*,INT,gdix_FillMode);
typedef gdix_Status (STDAPICALLTYPE *PFNFILLPOLYGON2)(gdix_Graphics*,gdix_Brush*,const gdix_PointF*,INT);
typedef gdix_Status (STDAPICALLTYPE *PFNFILLELLIPSE)(gdix_Graphics*,gdix_Brush*,gdix_Real,gdix_Real,gdix_Real,gdix_Real);
typedef gdix_Status (STDAPICALLTYPE *PFNFILLPIE)(gdix_Graphics*,gdix_Brush*,gdix_Real,gdix_Real,gdix_Real,gdix_Real,gdix_Real,gdix_Real);
typedef gdix_Status (STDAPICALLTYPE *PFNFILLPATH)(gdix_Graphics*,gdix_Brush*,gdix_Path*);

// String methods 
typedef gdix_Status (STDAPICALLTYPE *PFNDRAWSTRING)(gdix_Graphics*,const WCHAR*,INT,const gdix_Font*,const gdix_RectF*,const gdix_StringFormat*,const gdix_Brush*);
typedef gdix_Status (STDAPICALLTYPE *PFNMEASURESTRING)(gdix_Graphics*,const WCHAR*,INT,const gdix_Font*,const gdix_RectF*,const gdix_StringFormat*,gdix_RectF*,INT*,INT*);

// Bitmap methods
typedef gdix_Status (STDAPICALLTYPE *PFNCREATEBITMAPFROMSTREAM)(IStream*, gdix_Bitmap**);
typedef gdix_Status (STDAPICALLTYPE *PFNCREATEBITMAPFROMFILE)(const WCHAR*, gdix_Bitmap**);
typedef gdix_Status (STDAPICALLTYPE *PFNCREATEHBITMAPFROMBITMAP)(gdix_Bitmap*, HBITMAP*, gdix_ARGB);
typedef gdix_Status (STDAPICALLTYPE *PFNDELETEBITMAP)(gdix_Bitmap*);
typedef gdix_Status (STDAPICALLTYPE *PFNCREATEBITMAPFROMFILE2)(const WCHAR*, gdix_Bitmap**);
typedef gdix_Status (STDAPICALLTYPE *PFNCREATEHBITMAPFROMBITMAP2)(gdix_Bitmap*, HBITMAP*, gdix_ARGB);

//////////////////////////////////////////////////////////////////////

HMODULE   CGdiPlus::s_hGdiPlus = HMODULE(-1);
ULONG_PTR CGdiPlus::s_GdiToken = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BOOL CGdiPlus::Initialize()
{
	if (s_hGdiPlus == HMODULE(-1)) // one time only
	{
		s_hGdiPlus = LoadLibrary(_T("GdiPlus.dll"));

		if (s_hGdiPlus)
		{
		    PFNSTARTUP pFn = (PFNSTARTUP)GetProcAddress(s_hGdiPlus, "GdiplusStartup");

			if (pFn)
			{
			    gdix_StartupInput input = { 0 };

				input.GdiplusVersion = 1;
				input.SuppressExternalCodecs = TRUE;

				if (pFn(&s_GdiToken, &input, NULL) == gdix_Ok) 
				{
					return TRUE;
				}
			}

			// failed
			FreeLibrary(s_hGdiPlus);

			s_hGdiPlus = NULL;
			s_GdiToken = NULL;
		}
	}
	
	return (NULL != s_hGdiPlus);
}

void CGdiPlus::Free()
{
	if (s_hGdiPlus)
	{
		ASSERT(s_GdiToken);

		PFNSHUTDOWN pFn = (PFNSHUTDOWN)GetProcAddress(s_hGdiPlus, "GdiplusShutdown");
		
		if (pFn)
			pFn(s_GdiToken);

		FreeLibrary(s_hGdiPlus);

		s_hGdiPlus = HMODULE(-1);
		s_GdiToken = NULL;
	}
}

//////////////////////////////////////////////////////////////////////

#define GETPROCADDRESS(tdef, fnName)                            \
	if (!Initialize()) return FALSE;                            \
	static tdef pFN = (tdef)GetProcAddress(s_hGdiPlus, fnName); \
	if (!pFN) return FALSE;


//////////////////////////////////////////////////////////////////////

BOOL CGdiPlus::CreateBitmapFromStream(IStream* stream, gdix_Bitmap **bitmap)
{
	GETPROCADDRESS(PFNCREATEBITMAPFROMSTREAM, "GdipCreateBitmapFromStream");
	return (pFN(stream, bitmap) == gdix_Ok);

// 	if (!Initialize())
// 		return FALSE;
// 	
// 	static PFNCREATEBITMAPFROMSTREAM pFN = (PFNCREATEBITMAPFROMSTREAM)GetProcAddress(s_hGdiPlus, "GdipCreateBitmapFromStream");
// 
// 	if (pFN)
// 		return (pFN(stream, bitmap) == gdix_Ok);
// 
// 	// else
// 	return FALSE;
}

BOOL CGdiPlus::CreateBitmapFromFile(const WCHAR* filename, gdix_Bitmap **bitmap)
{
	GETPROCADDRESS(PFNCREATEBITMAPFROMFILE, "GdipCreateBitmapFromFile");
	return (pFN(filename, bitmap) == gdix_Ok);

// 	if (!Initialize())
// 		return FALSE;
// 
// 	static PFNCREATEBITMAPFROMFILE pFN = (PFNCREATEBITMAPFROMFILE)GetProcAddress(s_hGdiPlus, "GdipCreateBitmapFromFile");
// 
// 	if (pFN)
// 		return (pFN(filename, bitmap) == gdix_Ok);
// 
// 	// else
//	return FALSE;
}

BOOL CGdiPlus::CreateHBITMAPFromBitmap(gdix_Bitmap* bitmap, HBITMAP* hbmReturn, gdix_ARGB background)
{
	GETPROCADDRESS(PFNCREATEHBITMAPFROMBITMAP, "GdipCreateHBITMAPFromBitmap");
	return (pFN(bitmap, hbmReturn, background) == gdix_Ok);
// 	if (!Initialize())
// 		return FALSE;
// 
// 	static PFNCREATEHBITMAPFROMBITMAP pFN = (PFNCREATEHBITMAPFROMBITMAP)GetProcAddress(s_hGdiPlus, "GdipCreateHBITMAPFromBitmap");
// 
// 	if (pFN)
// 		return (pFN(bitmap, hbmReturn, background) == gdix_Ok);
// 	
// 	// else
// 	return FALSE;
}

BOOL CGdiPlus::DeleteBitmap(gdix_Bitmap* bitmap)
{
	GETPROCADDRESS(PFNDELETEBITMAP, "GdipDisposeImage");
	return (pFN(bitmap) == gdix_Ok);
// 	if (!Initialize())
// 		return FALSE;
// 
// 	static PFNDELETEBITMAP pFN = (PFNDELETEBITMAP)GetProcAddress(s_hGdiPlus, "GdipDisposeImage");
// 
// 	if (pFN)
// 		return (pFN(bitmap) == gdix_Ok);
// 	
// 	// else
// 	return FALSE;
}

BOOL CGdiPlus::CreatePen(gdix_ARGB color, gdix_Real width, gdix_Pen** pen)
{
	GETPROCADDRESS(PFNCREATEPEN1, "GdipCreatePen1");

	return (pFN(color, width, gdix_UnitPixel, pen) == gdix_Ok);
}

BOOL CGdiPlus::DeletePen(gdix_Pen* pen)
{
	GETPROCADDRESS(PFNDELETEPEN, "GdipDeletePen");

	return (pFN(pen) == gdix_Ok);
}

BOOL CGdiPlus::CreateGraphics(HDC hdc, gdix_Graphics** graphics)
{
	GETPROCADDRESS(PFNCREATEFROMHDC, "GdipCreateFromHDC");

	return (pFN(hdc, graphics) == gdix_Ok);
}

BOOL CGdiPlus::DeleteGraphics(gdix_Graphics* graphics)
{
	GETPROCADDRESS(PFNDELETEGRAPHICS, "GdipDeleteGraphics");

	return (pFN(graphics) == gdix_Ok);
}

BOOL CGdiPlus::SetSmoothingMode(gdix_Graphics* graphics, gdix_SmoothingMode mode)
{
	GETPROCADDRESS(PFNSETSMOOTHINGMODE, "GdipSetSmoothingMode");

	return (pFN(graphics, mode) == gdix_Ok);
}

BOOL CGdiPlus::DrawLine(gdix_Graphics* graphics, gdix_Pen* pen, const gdix_PointF* from, const gdix_PointF* to)
{
	GETPROCADDRESS(PFNDRAWLINE, "GdipDrawLine");

	return (pFN(graphics, pen, from->x, from->y, to->x, to->y) == gdix_Ok);
}

BOOL CGdiPlus::DrawPolygon(gdix_Graphics* graphics, gdix_Pen* pen, const gdix_PointF* points, int count)
{
	GETPROCADDRESS(PFNDRAWPOLYGON, "GdipDrawPolygon");

	return (pFN(graphics, pen, points, count) == gdix_Ok);
}
