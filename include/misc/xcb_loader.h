//
// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include <X11/keysym.h>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#define explicit c_explicit             // Avoid mixing up "explicit" with the C++11 explicit
#include <xcb/xkb.h>
#undef explicit

typedef uint32_t uint32;

enum class Result : int32_t
{
#ifndef Success
    Success          = 0x00000000,
#endif
    ErrorUnavailable = -(0x00000002),
};

// symbols from libxcb-keysyms.so
typedef xcb_keysym_t       (*PFN_XcbKeyPressLookupKeysym)  (xcb_key_symbols_t*       syms,
                                                            xcb_key_press_event_t*   event,
                                                            int                      col);
typedef xcb_keysym_t       (*PFN_XcbKeyReleaseLookupKeysym)(xcb_key_symbols_t*       syms,
                                                            xcb_key_release_event_t* event,
                                                            int                      col);
typedef xcb_key_symbols_t* (*PFN_XcbKeySymbolsAlloc)       (xcb_connection_t*        c);

// symbols from libxcb.so
typedef xcb_void_cookie_t        (*PFN_XcbChangeProperty)    (xcb_connection_t*          c,
                                                              uint8_t                    mode,
                                                              xcb_window_t               window,
                                                              xcb_atom_t                 property,
                                                              xcb_atom_t                 type,
                                                              uint8_t                    format,
                                                              uint32_t                   data_len,
                                                              const void*                data);
typedef xcb_connection_t*        (*PFN_XcbConnect)           (const char*                displayname,
                                                              int*                       screenp);
typedef xcb_void_cookie_t        (*PFN_XcbCreateWindow)      (xcb_connection_t*          c,
                                                              uint8_t                    depth,
                                                              xcb_window_t               wid,
                                                              xcb_window_t               parent,
                                                              int16_t                    x,
                                                              int16_t                    y,
                                                              uint16_t                   width,
                                                              uint16_t                   height,
                                                              uint16_t                   border_width,
                                                              uint16_t                   _class,
                                                              xcb_visualid_t             visual,
                                                              uint32_t                   value_mask,
                                                              const uint32_t*            value_list);
typedef xcb_void_cookie_t   (*PFN_XcbChangeWindowAttributes) (xcb_connection_t*          c,
                                                              xcb_window_t               window,
                                                              uint32_t                   value_mask,
                                                              const void*                value_list);
typedef xcb_void_cookie_t        (*PFN_XcbDestroyWindow)     (xcb_connection_t*          c,
                                                              xcb_window_t               window);
typedef void                     (*PFN_XcbDisconnect)        (xcb_connection_t*          c);
typedef int                      (*PFN_XcbFlush)             (xcb_connection_t*          c);
typedef uint32_t                 (*PFN_XcbGenerateId)        (xcb_connection_t*          c);
typedef xcb_get_geometry_cookie_t(*PFN_XcbGetGeometry)       (xcb_connection_t *         connection,
                                                              xcb_drawable_t             drawable);
typedef xcb_get_geometry_reply_t*(*PFN_XcbGetGeometryReply)  (xcb_connection_t*          connection,
                                                              xcb_get_geometry_cookie_t  cookie,
                                                              xcb_generic_error_t**      error);
typedef xcb_setup_t*             (*PFN_XcbGetSetup)          (xcb_connection_t*          c);
typedef xcb_void_cookie_t        (*PFN_XcbWarpPointer)       (xcb_connection_t*          c,
                                                              xcb_window_t               window,
                                                              xcb_window_t               dst_window,
                                                              int16_t                    src_x,
                                                              int16_t                    src_y,
                                                              uint16_t                   src_width,
                                                              uint16_t                   src_height,
                                                              int16_t                    dst_x,
                                                              int16_t                    dst_y);
typedef xcb_intern_atom_cookie_t (*PFN_XcbInternAtom)        (xcb_connection_t*          c,
                                                              uint8_t                    only_if_exists,
                                                              uint16_t                   name_len,
                                                              const char*                name);
typedef xcb_intern_atom_reply_t* (*PFN_XcbInternAtomReply)   (xcb_connection_t*          c,
                                                              xcb_intern_atom_cookie_t   cookie,
                                                              xcb_generic_error_t**      e);
typedef xcb_void_cookie_t        (*PFN_XcbMapWindow)         (xcb_connection_t*          c,
                                                              xcb_window_t               window);
typedef xcb_generic_event_t*     (*PFN_XcbPollForEvent)      (xcb_connection_t*          c);
typedef xcb_generic_event_t*     (*PFN_XcbWaitForEvent)      (xcb_connection_t*          c);
typedef xcb_void_cookie_t        (*PFN_XcbSendEvent)         (xcb_connection_t*          c,
                                                              uint8_t                    propagate,
                                                              xcb_window_t               destination,
                                                              uint32_t                   event_mask,
                                                              const char*                event);
typedef void                     (*PFN_XcbScreenNext)        (xcb_screen_iterator_t*     i);
typedef xcb_screen_iterator_t    (*PFN_XcbSetupRootsIterator)(const xcb_setup_t*         R);
typedef xcb_void_cookie_t        (*PFN_XcbUnmapWindow)       (xcb_connection_t*          c,
                                                              xcb_window_t               window);
typedef xcb_void_cookie_t        (*PFN_XcbCreatePixmap)      (xcb_connection_t*          c,
                                                              uint8_t                    depth,
                                                              xcb_pixmap_t               pid,
                                                              xcb_drawable_t             drawable,
                                                              uint16_t                   width,
                                                              uint16_t                   height);
typedef xcb_void_cookie_t        (*PFN_XcbCreateCursor)      (xcb_connection_t*          c,
                                                              xcb_cursor_t               cid,
                                                              xcb_pixmap_t               source,
                                                              xcb_pixmap_t               mask,
                                                              uint16_t                   fore_red,
                                                              uint16_t                   fore_green,
                                                              uint16_t                   fore_blue,
                                                              uint16_t                   back_red,
                                                              uint16_t                   back_green,
                                                              uint16_t                   back_blue,
                                                              uint16_t                   x,
                                                              uint16_t                   y);
typedef xcb_xkb_use_extension_cookie_t (*PFN_XcbXkbUseExtension)      (xcb_connection_t* c,
                                                                       uint16_t wantedMajor,
                                                                       uint16_t wantedMinor);
typedef xcb_xkb_per_client_flags_cookie_t (*PFN_XcbXkbPerClientFlags) (xcb_connection_t*  	     c,
                                                                       xcb_xkb_device_spec_t  	 deviceSpec,
                                                                       uint32_t  	             change,
                                                                       uint32_t  	             value,
                                                                       uint32_t  	             ctrlsToChange,
                                                                       uint32_t  	             autoCtrls,
                                                                       uint32_t  	             autoCtrlsValues);


enum XCBLoaderLibraries : uint32
{
    XCB_LOADER_LIBRARIES_XCB_KEYSYMS = 0,
    XCB_LOADER_LIBRARIES_XCB         = 1,
    XCB_LOADER_LIBRARIES_XCB_XKB     = 2,

    XCB_LOADER_LIBRARIES_COUNT = 3
};

struct XCBLoaderFuncs
{
    PFN_XcbChangeProperty         pfn_xcbChangeProperty;
    PFN_XcbConnect                pfn_xcbConnect;
    PFN_XcbCreateWindow           pfn_xcbCreateWindow;
    PFN_XcbChangeWindowAttributes pfn_xcbChangeWindowAttributes;
    PFN_XcbDestroyWindow          pfn_xcbDestroyWindow;
    PFN_XcbDisconnect             pfn_xcbDisconnect;
    PFN_XcbFlush                  pfn_xcbFlush;
    PFN_XcbGenerateId             pfn_xcbGenerateId;
    PFN_XcbGetGeometry            pfn_xcbGetGeometry;
    PFN_XcbGetGeometryReply       pfn_xcbGetGeometryReply;
    PFN_XcbGetSetup               pfn_xcbGetSetup;
    PFN_XcbWarpPointer            pfn_xcbWarpPointer;
    PFN_XcbInternAtom             pfn_xcbInternAtom;
    PFN_XcbInternAtomReply        pfn_xcbInternAtomReply;
    PFN_XcbKeyReleaseLookupKeysym pfn_xcbKeyReleaseLookupKeysym;
    PFN_XcbKeyPressLookupKeysym   pfn_xcbKeyPressLookupKeysym;
    PFN_XcbKeySymbolsAlloc        pfn_xcbKeySymbolsAlloc;
    PFN_XcbMapWindow              pfn_xcbMapWindow;
    PFN_XcbPollForEvent           pfn_xcbPollForEvent;
    PFN_XcbWaitForEvent           pfn_xcbWaitForEvent;
    PFN_XcbSendEvent              pfn_xcbSendEvent;
    PFN_XcbScreenNext             pfn_xcbScreenNext;
    PFN_XcbSetupRootsIterator     pfn_xcbSetupRootsIterator;
    PFN_XcbUnmapWindow            pfn_xcbUnmapWindow;
    PFN_XcbXkbUseExtension        pfn_xcbXkbUseExtension;
    PFN_XcbXkbPerClientFlags      pfn_xcbXkbPerClientFlags;
    PFN_XcbCreatePixmap           pfn_xcbCreatePixmap;
    PFN_XcbCreateCursor           pfn_xcbCreateCursor;
};

/*
 * Resolves all external symbols that are going to be needed when using XCB functionality.
 */
class XCBLoader
{
public:
    XCBLoader();
   ~XCBLoader();

    const XCBLoaderFuncs* get_procs_table() const
    {
        return &m_funcs;
    }

    Result init();

    bool is_initialized() const
    {
        return m_initialized;
    }

private:
    XCBLoaderFuncs m_funcs;
    bool           m_initialized;
    void*          m_library_handles[XCB_LOADER_LIBRARIES_COUNT];
};

