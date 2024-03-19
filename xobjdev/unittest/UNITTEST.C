
#include <windows.h>
#include "XObject.h"

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
long __far __pascal __export _unittest_mScreenshot(
    LxMemHandle hPath, LxProcTablePtr xtbl, LxMemHandle hUnitTest);


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

long __far __pascal __export _unittest_mScreenshot(
    LxMemHandle hPath, LxProcTablePtr xtbl, LxMemHandle hUnitTest)
{
    RECT rect;
    int width = 0;
    int height = 0;
    BITMAPFILEHEADER bf;
    BITMAPINFO bi = {0};
    HWND hWnd;
    HDC hWindowDC;
    HDC hMemDC;
    int iBits;
    LPPALETTEENTRY lppColors[256];
    LPSTR pPath;
    HBITMAP hbWindow;
    DWORD dwBmpSize = 0;
    HANDLE hDIB;
    LPVOID lpBitmap;
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
    LogMessage( xtbl, "Creating a bitmap - width: %d, height: %d, bpp: %d\n", width, height, iBits );

    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = width;
    bi.bmiHeader.biHeight = -height;  //this is the line that makes it draw upside down or not
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = iBits <= 8 ? 8 : 24;
    bi.bmiHeader.biCompression = BI_RGB;
    bi.bmiHeader.biSizeImage = 0;
    bi.bmiHeader.biXPelsPerMeter = 0;
    bi.bmiHeader.biYPelsPerMeter = 0;
    bi.bmiHeader.biClrUsed = iBits <= 8 ? 256 : 0;
    bi.bmiHeader.biClrImportant = 0;
    
    hbWindow = CreateCompatibleBitmap(hWindowDC, width, height);
    SelectObject(hMemDC, hbWindow);

    dwBmpSize = ((width * bi.bmiHeader.biBitCount + 31) / 32) * 4 * height;
    hDIB = GlobalAlloc(GHND, dwBmpSize);
    lpBitmap = (LPVOID)GlobalLock(hDIB);
    LogMessage( xtbl, "Bitmap data size: %ld bytes\n", dwBmpSize );

    // copy from the window device context to the bitmap device context
    //StretchBlt(hMemDC, 0, 0, width, height, hWindowDC, rect.left, rect.top, width, height, SRCCOPY);
    response = BitBlt(hMemDC, 0, 0, width, height, hWindowDC, 0, 0, SRCCOPY);
    LogMessage( xtbl, "BitBlt response: %d\n", response );
    response = GetDIBits(hWindowDC, hbWindow, 0, height, lpBitmap, &bi, DIB_RGB_COLORS);
    LogMessage( xtbl, "GetDIBits response: %ld %d\n", lpBitmap, response );

    // avoid memory leak
    DeleteObject(hbWindow);
    DeleteDC(hMemDC);
    ReleaseDC(hWnd, hWindowDC);
    
    pPath = xtbl->mem_Lock(hPath);

    // A file is created, this is where we will save the screen capture.
    LogMessage( xtbl, "Opening file %s\n", pPath );
    hFile = _lcreat(pPath, OF_WRITE);
    
    xtbl->mem_Unlock(hPath);

    
    // Offset to where the actual bitmap bits start.
    bf.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFO);
    if (iBits <= 8) {
        bf.bfOffBits += 4*256;
    }
    
    // Add the size of the headers to the size of the bitmap to get the total file size.
    bf.bfSize = dwBmpSize + bf.bfOffBits;

    // bfType must always be BM for Bitmaps.
    bf.bfType = 0x4D42; // BM.

    _lwrite(hFile, (LPSTR)&bf, sizeof(BITMAPFILEHEADER));
    _lwrite(hFile, (LPSTR)&bi, sizeof(BITMAPINFO));
    if (iBits <= 8) {
        _lwrite(hFile, (LPSTR)lppColors, 4*256);
    }
    _lwrite(hFile, (LPSTR)lpBitmap, dwBmpSize);

    // Unlock and Free the DIB from the heap.
    GlobalUnlock(hDIB);
    GlobalFree(hDIB);

    // Close the handle for the file that was created.
    _lclose(hFile);

    return 0;
}




