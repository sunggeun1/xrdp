/*
Copyright 2005-2013 Jay Sorg

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

misc draw calls

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* this should be before all X11 .h files */
#include <xorg-server.h>

/* all driver need this */
#include <xf86.h>
#include <xf86_OSproc.h>

#include <mipointer.h>
#include <fb.h>
#include <micmap.h>
#include <mi.h>

#include "rdp.h"

/******************************************************************************/
#define LOG_LEVEL 1
#define LLOGLN(_level, _args) \
    do { if (_level < LOG_LEVEL) { ErrorF _args ; ErrorF("\n"); } } while (0)

/*****************************************************************************/
PixmapPtr
rdpCreatePixmap(ScreenPtr pScreen, int width, int height, int depth,
                unsigned usage_hint)
{
    ScrnInfoPtr pScrn;
    rdpPtr dev;
    PixmapPtr rv;

    LLOGLN(10, ("rdpCreatePixmap: width %d height %d depth %d",
           width, height, depth));
    pScrn = xf86Screens[pScreen->myNum];
    dev = XRDPPTR(pScrn);
    pScreen->CreatePixmap = dev->CreatePixmap;
    rv = pScreen->CreatePixmap(pScreen, width, height, depth, usage_hint);
    pScreen->CreatePixmap = rdpCreatePixmap;
    return rv;
}

/******************************************************************************/
Bool
rdpDestroyPixmap(PixmapPtr pPixmap)
{
    Bool rv;
    ScreenPtr pScreen;
    rdpPtr dev;
    ScrnInfoPtr pScrn;

    LLOGLN(10, ("rdpDestroyPixmap: refcnt %d", pPixmap->refcnt));
    pScreen = pPixmap->drawable.pScreen;
    pScrn = xf86Screens[pScreen->myNum];
    dev = XRDPPTR(pScrn);
    pScreen->DestroyPixmap = dev->DestroyPixmap;
    rv = pScreen->DestroyPixmap(pPixmap);
    pScreen->DestroyPixmap = rdpDestroyPixmap;
    return rv;
}

/******************************************************************************/
Bool
rdpModifyPixmapHeader(PixmapPtr pPixmap, int width, int height, int depth,
                      int bitsPerPixel, int devKind, pointer pPixData)
{
    Bool rv;
    ScreenPtr pScreen;
    rdpPtr dev;
    ScrnInfoPtr pScrn;

    LLOGLN(10, ("rdpModifyPixmapHeader:"));
    pScreen = pPixmap->drawable.pScreen;
    pScrn = xf86Screens[pScreen->myNum];
    dev = XRDPPTR(pScrn);
    pScreen->ModifyPixmapHeader = dev->ModifyPixmapHeader;
    rv = pScreen->ModifyPixmapHeader(pPixmap, width, height, depth, bitsPerPixel,
                                     devKind, pPixData);
    pScreen->ModifyPixmapHeader = rdpModifyPixmapHeader;
    return rv;
}

/*****************************************************************************/
void
rdpCopyWindow(WindowPtr pWin, DDXPointRec ptOldOrg, RegionPtr pOldRegion)
{
    ScrnInfoPtr pScrn;
    ScreenPtr pScreen;
    rdpPtr dev;

    pScreen = pWin->drawable.pScreen;
    pScrn = xf86Screens[pScreen->myNum];
    dev = XRDPPTR(pScrn);
    dev->pScreen->CopyWindow = dev->CopyWindow;
    dev->pScreen->CopyWindow(pWin, ptOldOrg, pOldRegion);
    dev->pScreen->CopyWindow = rdpCopyWindow;
}

/*****************************************************************************/
Bool
rdpCloseScreen(int index, ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn;
    rdpPtr dev;
    Bool rv;

    LLOGLN(0, ("rdpCloseScreen:"));
    pScrn = xf86Screens[pScreen->myNum];
    dev = XRDPPTR(pScrn);
    dev->pScreen->CloseScreen = dev->CloseScreen;
    rv = dev->pScreen->CloseScreen(index, pScreen);
    dev->pScreen->CloseScreen = rdpCloseScreen;
    return rv;
}

/******************************************************************************/
WindowPtr
rdpGetRootWindowPtr(ScreenPtr pScreen)
{
#if XORG_VERSION_CURRENT < (((1) * 10000000) + ((9) * 100000) + ((0) * 1000) + 0)
    return WindowTable[pScreen->myNum]; /* in globals.c */
#else
    return pScreen->root;
#endif
}

/******************************************************************************/
int
rdpBitsPerPixel(int depth)
{
    if (depth == 1)
    {
        return 1;
    }
    else if (depth <= 8)
    {
        return 8;
    }
    else if (depth <= 16)
    {
        return 16;
    }
    else
    {
        return 32;
    }
}