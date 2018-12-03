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
#include <include/misc/window_xcb.h>
#include "misc/window_xcb.h"

/* Copy-pasted from xcb-icccm.h header, as there's no really good reason to include
 * a new dependency on the icccm library other than this struct and a bunch of const ints.
 */
typedef struct
{
    uint32_t flags;
    int32_t x, y;
    int32_t width, height;
    int32_t min_width, min_height;
    int32_t max_width, max_height;
    int32_t width_inc, height_inc;
    int32_t min_aspect_num, min_aspect_den;
    int32_t max_aspect_num, max_aspect_den;
    int32_t base_width, base_height;
    uint32_t win_gravity;
} xcb_size_hints_t;


/** Please see header for specification */
Anvil::WindowUniquePtr Anvil::WindowXcb::create(const std::string&             in_title,
                                                unsigned int                   in_width,
                                                unsigned int                   in_height,
                                                bool                           in_closable,
                                                Anvil::PresentCallbackFunction in_present_callback_func,
                                                bool                           in_visible)
{
    WindowUniquePtr result_ptr(nullptr,
                               std::default_delete<Window>() );

    result_ptr.reset(
        new Anvil::WindowXcb(in_title,
                             in_width,
                             in_height,
                             in_closable,
                             in_present_callback_func)
    );

    if (result_ptr)
    {
        if (!dynamic_cast<Anvil::WindowXcb*>(result_ptr.get() )->init(in_visible) )
        {
            result_ptr.reset();
        }
    }

    return result_ptr;
}

Anvil::WindowUniquePtr Anvil::WindowXcb::create(xcb_connection_t* in_connection_ptr,
                                                WindowHandle      in_window_handle)
{
    WindowUniquePtr result_ptr(nullptr,
                               std::default_delete<Window>() );

    result_ptr.reset(
        new Anvil::WindowXcb(in_connection_ptr,
                             in_window_handle)
    );

    if (result_ptr)
    {
        if (!dynamic_cast<WindowXcb*>(result_ptr.get() )->init(true /* in_visible */) )
        {
            result_ptr.reset();
        }
    }

    return result_ptr;
}

/** Please see header for specification */
Anvil::WindowXcb::WindowXcb(const std::string&             in_title,
                            unsigned int                   in_width,
                            unsigned int                   in_height,
                            bool                           in_closable,
                            Anvil::PresentCallbackFunction in_present_callback_func)
    :Window(in_title,
            in_width,
            in_height,
            in_closable,
            in_present_callback_func)
{
    m_connection_ptr = nullptr;
    m_window_owned   = true;
}

/** Please see header for specification */
Anvil::WindowXcb::WindowXcb(xcb_connection_t*  in_connection_ptr,
                            WindowHandle       in_window_handle)
    :Window("",
            0,        /* in_width                 */
            0,        /* in_height                */
            true,     /* in_closable              */
            nullptr)  /* in_present_callback_func */
{
    /* NOTE: Window title/size info is extracted at init time */
    m_connection_ptr = in_connection_ptr;
    m_window         = in_window_handle;
    m_window_owned   = false;
}

Anvil::WindowXcb::~WindowXcb()
{
    if (m_window_owned)
    {
        const XCBLoaderFuncs* xcb_procs_ptr = m_xcb_loader.get_procs_table();

        xcb_procs_ptr->pfn_xcbUnmapWindow(static_cast<xcb_connection_t*>(m_connection_ptr),
                                          m_window);

        xcb_procs_ptr->pfn_xcbDestroyWindow(static_cast<xcb_connection_t*>(m_connection_ptr),
                                            m_window);

        xcb_procs_ptr->pfn_xcbDisconnect(static_cast<xcb_connection_t*>(m_connection_ptr));
    }
}

/** Please see header for specification */
void Anvil::WindowXcb::close()
{
    anvil_assert(m_window_owned);

    if (!m_window_should_close)
    {
        OnWindowAboutToCloseCallbackArgument callback_argument(this);

        callback(WINDOW_CALLBACK_ID_ABOUT_TO_CLOSE,
                &callback_argument);

        m_window_should_close = true;

        free(m_key_symbols);
        m_key_symbols = nullptr;

        free(m_atom_wm_delete_window_ptr);
        m_atom_wm_delete_window_ptr = nullptr;
    }
}

/** Creates a new system window and prepares it for usage. */
bool Anvil::WindowXcb::init(const bool& in_visible)
{
    bool                  result        = false;
    const XCBLoaderFuncs* xcb_procs_ptr = nullptr;

    m_xcb_loader.init();

    if (m_xcb_loader.is_initialized() == false)
    {
        anvil_assert(m_xcb_loader.is_initialized() == true);

        goto end;
    }

    xcb_procs_ptr = m_xcb_loader.get_procs_table();
    anvil_assert(xcb_procs_ptr != nullptr);

    if (m_window_owned)
    {
        xcb_intern_atom_reply_t* atom_reply_ptr                   = nullptr;
        xcb_intern_atom_reply_t* atom_wm_delete_window_ptr        = nullptr;
        xcb_intern_atom_cookie_t cookie;
        xcb_intern_atom_cookie_t cookie2;
        xcb_size_hints_t         size_hints;
        const uint32_t           value_mask                       = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
        uint32_t                 value_list[sizeof(uint32_t) * 8];
        const uint32_t           window_size[2]                   =
        {
            m_width,
            m_height
        };

        if (!init_connection() )
        {
            goto end;
        }

        // Make input messages more WINAPI-like
        xcb_procs_ptr->pfn_xcbXkbUseExtension(m_connection_ptr, XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);

        xcb_procs_ptr->pfn_xcbXkbPerClientFlags(m_connection_ptr,
                                                XCB_XKB_ID_USE_CORE_KBD,
                                                XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT,
                                                XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT,
                                                0,0,0);

        value_list[0] = m_screen_ptr->black_pixel;
        value_list[1] = XCB_EVENT_MASK_POINTER_MOTION|
                        XCB_EVENT_MASK_BUTTON_PRESS  | XCB_EVENT_MASK_BUTTON_RELEASE    |
                        XCB_EVENT_MASK_KEY_PRESS     | XCB_EVENT_MASK_KEY_RELEASE       |
                        XCB_EVENT_MASK_EXPOSURE      | XCB_EVENT_MASK_STRUCTURE_NOTIFY;

        m_window = xcb_procs_ptr->pfn_xcbGenerateId(m_connection_ptr);

        xcb_procs_ptr->pfn_xcbCreateWindow(m_connection_ptr,
                                           XCB_COPY_FROM_PARENT,
                                           m_window,
                                           m_screen_ptr->root,
                                           0,
                                           0,
                                           window_size[0],
                                           window_size[1],
                                           0,
                                           XCB_WINDOW_CLASS_INPUT_OUTPUT,
                                           m_screen_ptr->root_visual,
                                           value_mask,
                                           value_list);

        // Hide cursor hack
        xcb_cursor_t cur = xcb_procs_ptr->pfn_xcbGenerateId(m_connection_ptr);
        xcb_pixmap_t pix = xcb_procs_ptr->pfn_xcbGenerateId(m_connection_ptr);

        xcb_procs_ptr->pfn_xcbCreatePixmap(m_connection_ptr, 1, pix, m_window, 1, 1);
        xcb_procs_ptr->pfn_xcbCreateCursor(m_connection_ptr, cur, pix, pix, 0, 0, 0, 0, 0, 0, 1, 1);

        xcb_procs_ptr->pfn_xcbChangeWindowAttributes(m_connection_ptr,
                                                     m_window,
                                                     XCB_CW_CURSOR,
                                                     &cur);

        xcb_procs_ptr->pfn_xcbFlush(m_connection_ptr);

        /* Anvil does not currently support run-time swapchain resizing so make sure WMs are aware
         * the window we created is not supposed to be resizable.
         *
         * The constants have been copy-pasted from xcb_icccm.h.
         */
        static const auto XCB_SIZE_HINT_P_MAX_SIZE = 1 << 5;
        static const auto XCB_SIZE_HINT_P_MIN_SIZE = 1 << 4;
        static const auto XCB_SIZE_HINT_P_SIZE     = 1 << 3;
        static const auto XCB_SIZE_HINT_US_SIZE    = 1 << 1;

        memset(&size_hints,
               0,
               sizeof(size_hints) );

        size_hints.flags      = XCB_SIZE_HINT_P_MAX_SIZE | XCB_SIZE_HINT_P_MIN_SIZE | XCB_SIZE_HINT_P_SIZE | XCB_SIZE_HINT_US_SIZE;
        size_hints.max_width  = window_size[0];
        size_hints.max_height = window_size[1];
        size_hints.min_width  = window_size[0];
        size_hints.min_height = window_size[1];

        xcb_procs_ptr->pfn_xcbChangeProperty(m_connection_ptr,
                                             XCB_PROP_MODE_REPLACE,
                                             m_window,
                                             XCB_ATOM_WM_NORMAL_HINTS,
                                             XCB_ATOM_WM_SIZE_HINTS,
                                             32,
                                             sizeof(size_hints) >> 2,
                                            &size_hints);

        /* Magic code that will send notification when window is destroyed */
        cookie  = xcb_procs_ptr->pfn_xcbInternAtom(m_connection_ptr,
                                                   1,
                                                   12,
                                                   "WM_PROTOCOLS");
        cookie2 = xcb_procs_ptr->pfn_xcbInternAtom(m_connection_ptr,
                                                   0,
                                                   16,
                                                   "WM_DELETE_WINDOW");

        atom_reply_ptr            = xcb_procs_ptr->pfn_xcbInternAtomReply(m_connection_ptr,
                                                                          cookie,
                                                                          0);
        atom_wm_delete_window_ptr = xcb_procs_ptr->pfn_xcbInternAtomReply(m_connection_ptr,
                                                                          cookie2,
                                                                          0);

        xcb_procs_ptr->pfn_xcbChangeProperty(m_connection_ptr,
                                             XCB_PROP_MODE_REPLACE,
                                             m_window,
                                             atom_reply_ptr->atom,
                                             4,
                                             32,
                                             1,
                                             &atom_wm_delete_window_ptr->atom);

        xcb_procs_ptr->pfn_xcbChangeProperty(m_connection_ptr,
                                             XCB_PROP_MODE_REPLACE,
                                             m_window,
                                             XCB_ATOM_WM_NAME,
                                             XCB_ATOM_STRING,
                                             8,
                                             m_title.size(),
                                             m_title.c_str() );

        free(atom_reply_ptr);

        if (in_visible)
        {
            xcb_procs_ptr->pfn_xcbMapWindow(m_connection_ptr,
                                            m_window);
        }

        xcb_procs_ptr->pfn_xcbFlush(m_connection_ptr);

        m_atom_wm_delete_window_ptr = atom_wm_delete_window_ptr;
        m_key_symbols               = xcb_procs_ptr->pfn_xcbKeySymbolsAlloc(m_connection_ptr);
    }
    else
    {
        xcb_get_geometry_cookie_t cookie;
        xcb_get_geometry_reply_t* reply_ptr = nullptr;

        cookie    = xcb_procs_ptr->pfn_xcbGetGeometry     (m_connection_ptr,
                                                           m_window);
        reply_ptr = xcb_procs_ptr->pfn_xcbGetGeometryReply(m_connection_ptr,
                                                           cookie,
                                                           nullptr);

        if (reply_ptr == nullptr)
        {
            anvil_assert(!(reply_ptr == nullptr) );

            goto end;
        }

        m_height = reply_ptr->height;
        m_width  = reply_ptr->width;

        free(reply_ptr);
    }

    result = true;
end:
    return result;
}

/** Initializes a XCB connection */
bool Anvil::WindowXcb::init_connection()
{
    xcb_screen_iterator_t iter;
    bool                  result;
    int32_t               scr;
    const xcb_setup_t*    setup_ptr     = nullptr;
    const XCBLoaderFuncs* xcb_procs_ptr = m_xcb_loader.get_procs_table();

    anvil_assert(xcb_procs_ptr != NULL);

    m_connection_ptr = xcb_procs_ptr->pfn_xcbConnect(nullptr,
                                                     &scr);

    if (m_connection_ptr == nullptr)
    {
        goto end;
    }

    setup_ptr = xcb_procs_ptr->pfn_xcbGetSetup          (m_connection_ptr);
    iter      = xcb_procs_ptr->pfn_xcbSetupRootsIterator(setup_ptr);

    while (scr-- > 0)
    {
        xcb_procs_ptr->pfn_xcbScreenNext(&iter);
    }

    m_screen_ptr = iter.data;
    result       = true;
end:
    return result;
}

/* Please see header for specification */
void Anvil::WindowXcb::run()
{
    bool running = true;

    anvil_assert(m_window_owned);

    while (running                &&
           !m_window_should_close)
    {
        xcb_generic_event_t* event_ptr = m_xcb_loader.get_procs_table()->pfn_xcbPollForEvent(m_connection_ptr);

        if (event_ptr)
        {
            running = msg_callback(event_ptr);
        }
        else
        {
            if (m_present_callback_func != nullptr)
            {
                m_present_callback_func();
            }

            running = !m_window_should_close;
        }
    }

    close();

    m_window_close_finished = true;
}

bool Anvil::WindowXcb::msg_callback(xcb_generic_event_t* event_ptr)
{
    bool keepRunning = true;
    switch (event_ptr->response_type & 0x7f)
    {
        case XCB_CLIENT_MESSAGE:
            if (!((reinterpret_cast<xcb_client_message_event_t*>(event_ptr)->data.data32[0] == m_atom_wm_delete_window_ptr->atom) &&
                m_closable))
                break;

        case XCB_DESTROY_NOTIFY:
         {
            OnWindowAboutToCloseCallbackArgument callback_argument(this);

            callback(WINDOW_CALLBACK_ID_CLOSE_EVENT,
                     &callback_argument);

            keepRunning = false;
            break;
         }


        case XCB_MOTION_NOTIFY:
        {
            const xcb_motion_notify_event_t* key_ptr = reinterpret_cast<const xcb_motion_notify_event_t*>(event_ptr);

            const int32_t resetXPOS = 256;
            const int32_t resetYPOS = 256;

            if((mouseLastPos.xPos != -1 && mouseLastPos.yPos != -1) && (key_ptr->event_x != resetXPOS || key_ptr->event_y != resetYPOS))
            {
                OnMouseMovementCallbackArgument callback_argument(this,
                                                                  key_ptr->event_x - mouseLastPos.xPos,
                                                                  key_ptr->event_y - mouseLastPos.yPos);

                callback(WINDOW_CALLBACK_ID_MOUSE_MOVED,
                         &callback_argument);

                m_xcb_loader.get_procs_table() -> pfn_xcbWarpPointer(m_connection_ptr,
                                                                     m_window,
                                                                     m_window,
                                                                     0,0,0,0,
                                                                     resetXPOS,
                                                                     resetYPOS);
            }

            mouseLastPos.xPos = key_ptr->event_x;
            mouseLastPos.yPos = key_ptr->event_y;
            break;
        }

        case XCB_KEY_RELEASE:
        {
            const xcb_key_release_event_t* key_ptr = reinterpret_cast<const xcb_key_release_event_t*>(event_ptr);
            xcb_keysym_t                   sym;

            sym = m_xcb_loader.get_procs_table()->pfn_xcbKeyReleaseLookupKeysym(m_key_symbols,
                                                                                const_cast<xcb_key_release_event_t*>(key_ptr),
                                                                                0);
            if(sym >= 'a' && sym <='z')
                sym -= 'a' - 'A';

            OnKeypressReleasedCallbackArgument callback_argument(this,
                                                                 static_cast<Anvil::KeyID>(sym));

            callback(WINDOW_CALLBACK_ID_KEYPRESS_RELEASED,
                     &callback_argument);

            break;
        }

        case XCB_KEY_PRESS:
        {
            const xcb_key_press_event_t* key_ptr = reinterpret_cast<const xcb_key_press_event_t*>(event_ptr);
            xcb_keysym_t                   sym;

            sym = m_xcb_loader.get_procs_table()->pfn_xcbKeyPressLookupKeysym(m_key_symbols,
                                                                              const_cast<xcb_key_press_event_t*>(key_ptr),
                                                                              0);

            if(sym >= 'a' && sym <='z')
                sym -= 'a' - 'A';

            OnKeypressPressedWasUpCallbackArgument callback_argument(this,
                                                                     static_cast<Anvil::KeyID>(sym));

            callback(WINDOW_CALLBACK_ID_KEYPRESS_PRESSED_WAS_UP,
                     &callback_argument);

            break;
        }

        case XCB_BUTTON_RELEASE:
        {
            const xcb_button_release_event_t* key_ptr = reinterpret_cast<const xcb_button_release_event_t*>(event_ptr);
            if(key_ptr->detail == 1)
            {
                OnKeypressReleasedCallbackArgument callback_argument(this, KEY_ID_LBUTTON);
                callback(WINDOW_CALLBACK_ID_KEYPRESS_RELEASED, &callback_argument);
            }
            if(key_ptr->detail == 2)
            {
                OnKeypressReleasedCallbackArgument callback_argument(this, KEY_ID_MBUTTON);
                callback(WINDOW_CALLBACK_ID_KEYPRESS_RELEASED, &callback_argument);
            }
            if(key_ptr->detail == 3)
            {
                OnKeypressReleasedCallbackArgument callback_argument(this, KEY_ID_RBUTTON);
                callback(WINDOW_CALLBACK_ID_KEYPRESS_RELEASED, &callback_argument);
            }
            last_button_release_timestamp = key_ptr->time;
        }

        case XCB_BUTTON_PRESS:
        {
            const xcb_button_press_event_t* key_ptr = reinterpret_cast<const xcb_button_press_event_t*>(event_ptr);
            if(key_ptr->time != last_button_release_timestamp)
            {
                if(key_ptr->detail == 1)
                {
                    OnKeypressPressedWasUpCallbackArgument callback_argument(this, KEY_ID_LBUTTON);
                    callback(WINDOW_CALLBACK_ID_KEYPRESS_PRESSED_WAS_UP, &callback_argument);
                }
                if(key_ptr->detail == 2)
                {
                    OnKeypressPressedWasUpCallbackArgument callback_argument(this, KEY_ID_MBUTTON);
                    callback(WINDOW_CALLBACK_ID_KEYPRESS_PRESSED_WAS_UP, &callback_argument);
                }
                if(key_ptr->detail == 3)
                {
                    OnKeypressPressedWasUpCallbackArgument callback_argument(this, KEY_ID_RBUTTON);
                    callback(WINDOW_CALLBACK_ID_KEYPRESS_PRESSED_WAS_UP, &callback_argument);
                }
            }
        }

        case XCB_EXPOSE:
        {
            if (m_present_callback_func != nullptr)
            {
                m_present_callback_func();
            }

            keepRunning = !m_window_should_close;
            break;
        }

        default:
        {
            break;
        }
    }

    free(event_ptr);

    return keepRunning;
}