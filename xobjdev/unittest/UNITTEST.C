
#include <windows.h>
#include "XObject.h"

#define DEBUG 0

/*
------------------------------------------------------------------------------
Method Function Prototypes:
------------------------------------------------------------------------------
*/

short __far __pascal __export LibMain(
    HANDLE hndInstance, WORD wDataSeg, WORD cbHeapSize, DWORD ignore);
//DJ20jul93int __export __far __pascal WEP(int nParameter);
long __far __pascal __export _unittest_mNew(
    LxProcTablePtr xtbl, LxMemHandle hUnitTest);
long __far __pascal __export _unittest_mDispose(
    LxProcTablePtr xtbl, LxMemHandle hUnitTest); 
long __far __pascal __export _unittest_mIsRealDirector(
    LxProcTablePtr xtbl, LxMemHandle hUnitTest);
long __far __pascal __export _unittest_mScreenshot(
    LxMemHandle hPath, LxProcTablePtr xtbl, LxMemHandle hUnitTest);
long __far __pascal __export _unitTest_mMoveMouse(
    long x, long y,  LxProcTablePtr xtbl, LxMemHandle hUnitTest);
long __far __pascal __export _unitTest_mLeftMouseDown(
    LxProcTablePtr xtbl, LxMemHandle hUnitTest);
 long __far __pascal __export _unitTest_mLeftMouseUp(
    LxProcTablePtr xtbl, LxMemHandle hUnitTest);

/*
------------------------------------------------------------------------------
Constants:
------------------------------------------------------------------------------
*/

#define WIDGET_SUCCESS          0       // Successful return code
#define WIDGET_MEM_ALLOC        1       // Memory allocation error

/*
------------------------------------------------------------------------------
Types:
------------------------------------------------------------------------------
*/

typedef struct
{
    LxXObjHeader        head;           // Required XObject header
    LxMemHandle         hErrMsg;        // Sample instance data: handle
    short               sJunk;          // Sample instance data: short
}   UnitTestType, FAR *pUnitTestType;


/*
------------------------------------------------------------------------------
Variables:
------------------------------------------------------------------------------
*/

long    lUnitTestError = WIDGET_SUCCESS;
HANDLE  hInst;


/*
------------------------------------------------------------------------------
FUNCTION        ||      LibMain()

Windows Entry Procedure for DLLs.  Called by LibEntry routine.
------------------------------------------------------------------------------
*/

short __far __pascal __export LibMain(
    HANDLE hndInstance, WORD wDataSeg, WORD cbHeapSize, DWORD ignore)
{
    if (cbHeapSize != 0)
    {
        if (! LocalInit( (UINT)wDataSeg, (UINT)NULL, (UINT)cbHeapSize))
        {
            return (0);
        }
    }
    hInst = hndInstance;
    return (1);
}


//DJ20jul93/*
//DJ20jul93------------------------------------------------------------------------------
//DJ20jul93FUNCTION        ||      WEP()
//DJ20jul93
//DJ20jul93Windows Exit Procedure for DLLs.
//DJ20jul93------------------------------------------------------------------------------
//DJ20jul93*/
//DJ20jul93int __export __far __pascal WEP(int nParameter)
//DJ20jul93{
//DJ20jul93    return 1;
//DJ20jul93}


/*
------------------------------------------------------------------------------
Internal Functions:
------------------------------------------------------------------------------
*/

// Log some useful debug info to message window
static void LogMessage( LxProcTablePtr xtbl, const char *format, ...)
{
    char    buf[250];               // !!@ No error check on size.

    wvsprintf( buf, format, ((char *)&format)+sizeof(char *) );
    xtbl->showMsg( buf );
}


/*
------------------------------------------------------------------------------
Macros:
------------------------------------------------------------------------------
*/


/*
------------------------------------------------------------------------------
FUNCTION        ||      _unittest_mNew

Create a new instance of the UnitTest XObject.  Initialize instance variables.
------------------------------------------------------------------------------
*/

long __far __pascal __export _unittest_mNew(
    LxProcTablePtr xtbl, LxMemHandle hUnitTest)
{
    pUnitTestType pUnitTest;
    
    lUnitTestError = WIDGET_SUCCESS;

    if ( xtbl->mem_SetSize(hUnitTest, (long) sizeof(UnitTestType) )  != NULL)
    {
        // Initialize instance data
        pUnitTest = xtbl->mem_Lock(hUnitTest);
        pUnitTest->hErrMsg = NULL;
        pUnitTest->sJunk = -1;
        xtbl->mem_Unlock(hUnitTest);
    }
    else
    {
        lUnitTestError = WIDGET_MEM_ALLOC;
    }
    return (lUnitTestError);
}

/*
-----------------------------------------------------------------------------
FUNCTION       ||      _unittest_mDispose

Dispose of a UnitTest XObject instance.  Free instance data (if any).
-----------------------------------------------------------------------------
*/
long __far __pascal __export _unittest_mDispose(
    LxProcTablePtr xtbl, LxMemHandle hUnitTest)
{
    pUnitTestType pUnitTest;

    lUnitTestError = WIDGET_SUCCESS;
    pUnitTest = xtbl->mem_Lock(hUnitTest);
    if (pUnitTest->hErrMsg != NULL)
    {
        // Dispose of saved error message:
        xtbl->mem_Dispose(pUnitTest->hErrMsg);
    }
    xtbl->mem_Unlock(hUnitTest);
    xtbl->xobj_Dispose(hUnitTest);
    return (lUnitTestError);
}

long __far __pascal __export _unittest_mIsRealDirector(
    LxProcTablePtr xtbl, LxMemHandle hUnitTest)
{
    return 1;
}

long __far __pascal __export _unittest_mScreenshot(
    LxMemHandle hPath, LxProcTablePtr xtbl, LxMemHandle hUnitTest)
{
    RECT rect;
    int width = 0;
    int height = 0;
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bmInfo;
    HWND hWnd;
    HDC hWindowDC;
    HDC hMemDC;
    int iBits = 0;
    LPPALETTEENTRY lppColors[256];
    LPSTR pPath;
    HBITMAP hbWindow;
    HGDIOBJ hgPrev;
    DWORD sizeDIB;
    HANDLE hDIB;
    LPBITMAPINFOHEADER lpBitmap;
    LPSTR lpBits;
    int response = 0;

    HFILE hFile;

    hWnd = xtbl->getStageWindow();
    GetClientRect(hWnd, &rect);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    hWindowDC = GetDC(hWnd);
    hMemDC = CreateCompatibleDC(hWindowDC);
    SetStretchBltMode(hMemDC, COLORONCOLOR);
    iBits = GetDeviceCaps(hWindowDC, BITSPIXEL) * GetDeviceCaps(hWindowDC, PLANES);
    GetSystemPaletteEntries(hWindowDC, 0, 256, &lppColors);
#if DEBUG
    LogMessage( xtbl, "Creating a bitmap - width: %d, height: %d, bpp: %d\n", width, height, iBits );
#endif

    bmInfo.biSize = sizeof(BITMAPINFOHEADER);
    bmInfo.biWidth = width;
    bmInfo.biHeight = height; 
    bmInfo.biPlanes = 1;
    bmInfo.biBitCount = iBits <= 8 ? 8 : 24;
    bmInfo.biCompression = BI_RGB;
    bmInfo.biSizeImage = 0;
    bmInfo.biXPelsPerMeter = 0;
    bmInfo.biYPelsPerMeter = 0;
    bmInfo.biClrUsed = iBits <= 8 ? 256 : 0;
    bmInfo.biClrImportant = 0;
    bmInfo.biSizeImage = ((((DWORD)width * (DWORD)bmInfo.biBitCount + 31) & ~31) >> 3) * (DWORD)height;
    hbWindow = CreateCompatibleBitmap(hWindowDC, width, height);
#if DEBUG
    LogMessage( xtbl, "CreateCompatibleBitmap: %d\n", hbWindow );
    LogMessage( xtbl, "Bitmap image data size: %ld bytes\n", (DWORD)bmInfo.biSizeImage);
#endif
    hgPrev = SelectObject(hMemDC, hbWindow);
    sizeDIB = (DWORD)sizeof(BITMAPINFOHEADER) + (DWORD)bmInfo.biSizeImage;
    if (iBits == 8) {
        sizeDIB += (DWORD)sizeof(lppColors);
    }
#if DEBUG
    LogMessage( xtbl, "Bitmap data size: %ld bytes\n", sizeDIB);
#endif
    hDIB = GlobalAlloc(GHND, sizeDIB);
    lpBitmap = (LPBITMAPINFOHEADER)GlobalLock(hDIB);
    *lpBitmap = bmInfo;
    lpBits = (LPSTR)lpBitmap + *(LPDWORD)lpBitmap;
    if (iBits == 8) {
#if DEBUG
        LogMessage( xtbl, "Writing %d bytes of palette data\n", sizeof(lppColors));
#endif
        memcpy(lpBits, lppColors, sizeof(lppColors));
        lpBits += sizeof(lppColors);
    }

    // copy from the window device context to the memory device context
    response = BitBlt(hMemDC, 0, 0, width, height, hWindowDC, 0, 0, SRCCOPY);
#if DEBUG
    LogMessage( xtbl, "BitBlt response: %d\n", response );
#endif

    SelectObject(hMemDC, hgPrev);
    response = GetDIBits(hWindowDC, hbWindow, 0, height, lpBits, (LPBITMAPINFO) lpBitmap, DIB_RGB_COLORS);
#if DEBUG
    LogMessage( xtbl, "GetDIBits response: %ld %d\n", lpBitmap, response );
#endif

    // avoid memory leak
    DeleteObject(hbWindow);
    DeleteDC(hMemDC);
    ReleaseDC(hWnd, hWindowDC);
    
    pPath = xtbl->mem_Lock(hPath);

    // A file is created, this is where we will save the screen capture.
#if DEBUG
    LogMessage( xtbl, "Opening file %s\n", pPath );
#endif
    hFile = _lcreat(pPath, OF_WRITE);
    
    xtbl->mem_Unlock(hPath);

    
    // Offset to where the actual bitmap bits start.
    bf.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
    if (iBits <= 8) {
        bf.bfOffBits += 4*256;
    }
   
    // total file size, including headers
    bf.bfSize = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeDIB;

    // bfType must always be BM for Bitmaps.
    bf.bfType = 0x4D42; // BM.
    bf.bfReserved1 = 0;
    bf.bfReserved2 = 0;

    _hwrite(hFile, (LPSTR)&bf, sizeof(BITMAPFILEHEADER));
    _hwrite(hFile, (LPSTR)lpBitmap, sizeDIB);

    // Unlock and Free the DIB from the heap.
    GlobalUnlock(hDIB);
    GlobalFree(hDIB);

    // Close the handle for the file that was created.
    _lclose(hFile);

    return 0;
}

long __far __pascal __export _unitTest_mMoveMouse(
    long x, long y, LxProcTablePtr xtbl, LxMemHandle hUnitTest) {
    HWND hWnd;
    POINT ptCursor;
    hWnd = xtbl->getStageWindow();
    ptCursor.x = x;
    ptCursor.y = y;
    ClientToScreen(hWnd, &ptCursor);
    SetCursorPos(ptCursor.x, ptCursor.y);
    return 0;
};

long __far __pascal __export _unitTest_mLeftMouseDown(
    LxProcTablePtr xtbl, LxMemHandle hUnitTest) {
    HWND hWnd;
    POINT ptCursor;
    hWnd = xtbl->getStageWindow();
    GetCursorPos(&ptCursor);
    ScreenToClient(hWnd, &ptCursor);
    SendMessage(hWnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(ptCursor.x, ptCursor.y));
    return 0;
};

long __far __pascal __export _unitTest_mLeftMouseUp(
    LxProcTablePtr xtbl, LxMemHandle hUnitTest) {
    HWND hWnd;
    POINT ptCursor;
    hWnd = xtbl->getStageWindow();
    GetCursorPos(&ptCursor);
    ScreenToClient(hWnd, &ptCursor);
    SendMessage(hWnd, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(ptCursor.x, ptCursor.y));
    return 0;
};
