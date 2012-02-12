/* Copyright (c) 2012 David Snyder, Benjamin Carr, Braden Wooley
 *
 * rootimg_api.h
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* #ifndef _XTOQ_C_ */
/* #define _XTOQ_C_ */

#include "xtoq.h"
#include "xtoq_internal.h"
#include <string.h>



// aaron key stuff
#define XK_Shift_L                       0xffe1
xcb_key_symbols_t *syms = NULL;
// end aaron key stuff

// This init function needs set the window to be registered for events!
// First one we should handle is damage
xtoq_context_t
xtoq_init(char *screen) {
    xcb_connection_t *conn;
    int conn_screen;
    xcb_screen_t *root_screen;
    xcb_drawable_t root_window;
    uint32_t mask_values[1];
 
    conn = xcb_connect(screen, &conn_screen);
    
    root_screen = xcb_aux_get_screen(conn, conn_screen);
    root_window = root_screen->root;
    
    // Set the mask for the root window so we know when new windows
    // are created on the root. This is where we add masks for the events
    // we care about catching on the root window.
    mask_values[0] = XCB_EVENT_MASK_KEY_PRESS |
                     XCB_EVENT_MASK_BUTTON_PRESS |
                     XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
    xcb_change_window_attributes (conn, root_window,
                                  XCB_CW_EVENT_MASK, mask_values);

	xcb_flush(conn);
    
    // TODO: Creating this reference to the root window so there is a way
    // to get at it from other functions - since we don't have a data
    // structure for the windows yet. This should be deleted later
    // because this is getting VERY HACKY
    root_context = malloc(sizeof(xtoq_context_t));
    root_context->conn = conn;
    root_context->parent = 0;
    root_context->window = root_window;
    
    // Set width, height, x, & y from root_screen into the xtoq_context_t
    root_context->width = root_screen->width_in_pixels;
    root_context->height = root_screen->height_in_pixels;
    root_context->x = 140;
    root_context->y = 42;
    // TODO END
    
    xtoq_context_t init_reply;
    init_reply.conn = conn;
    init_reply.window = root_window;
    
    // Set width, height, x, & y from root_screen into the xtoq_context_t
    init_reply.width = root_screen->width_in_pixels;
    init_reply.height = root_screen->height_in_pixels;
    init_reply.x = 140;
    init_reply.y = 42;    
    
    // TODO: May want to send &init_reply instead of root window
    _xtoq_init_damage(root_context);
    
    _xtoq_init_xfixes(root_context);
    
    // not sure about this error ...
    //_xtoq_add_context_t(init_reply);
    
    return init_reply;
}

xcb_image_t *
xtoq_get_image(xtoq_context_t context) {
    
    xcb_get_geometry_reply_t *geom_reply;
    
    //image_data_t img_data;
    xcb_image_t *image;
    
    geom_reply = _xtoq_get_window_geometry(context.conn, context.window);
    
	//xcb_flush(context.conn);
    /* Get the image of the root window */
    image = xcb_image_get(context.conn,
                          context.window,
                          geom_reply->x,
                          geom_reply->y,
                          geom_reply->width,
                          geom_reply->height,
                          (unsigned int) ~0L,
                          XCB_IMAGE_FORMAT_Z_PIXMAP);
    free(geom_reply);
    return image;
}


xtoq_event_t
dummy_xtoq_wait_for_event(xtoq_context_t context) {
    
    sleep(4);
    xtoq_event_t event;
    xtoq_context_t new_context;
    new_context.window = context.window;
    new_context.conn = context.conn;
    event.context = &new_context;
    event.event_type = XTOQ_DAMAGE;
    
    return event;
}

int 
xtoq_start_event_loop (xtoq_context_t root_context, void *callback)
{
	/* Simply call our internal function to do the actual setup */
	return _xtoq_start_event_loop(root_context.conn, callback);
}

// key code corresponds roughly to a physical key
// while a keysym corresponds to the symbol on the key top
// http://cgit.freedesktop.org/xcb/demo/tree/app/xte/xte.c
uint8_t *
dummy_thing_to_keycode( xcb_connection_t *c, char *thing ) {
    
    xcb_keycode_t *kc;
    xcb_keysym_t ks;
    
    /* For now, assume thing[0] == Latin-1 keysym */
    ks = (uint8_t)thing[0];
    
    kc = xcb_key_symbols_get_keycode( syms, ks );
    
    printf( "String '%s' maps to keysym '%d'\n", thing, ks );
    printf( "String '%s' maps to keycode '%s' - something wrong here\n", thing, kc ); // keycode not correct
    
    return( kc );
}

void
dummy_xtoq_key_press (xtoq_context_t context, int window, unsigned short keyCode, unsigned short aChar, char * charAsCharStar)
{
    // move to setup
        syms = xcb_key_symbols_alloc(context.conn );
   // xcb_generic_error_t **e;
   // xcb_key_symbols_get_reply(syms, e);
    
    //
    xcb_window_t none = { XCB_NONE };
    static xcb_keysym_t shift = { XK_Shift_L };
    
    uint8_t * code;
    uint8_t * wrap_code = NULL;
    
    const char *cap = "~!@#$%^&*()_+{}|:\"<>?";
    if (charAsCharStar[0] >= 'A' && charAsCharStar[0] <= 'Z')
        wrap_code = xcb_key_symbols_get_keycode( syms, shift );
    else if (strchr(cap, charAsCharStar[0]) != NULL)
        wrap_code = xcb_key_symbols_get_keycode( syms, shift );
    
    code = dummy_thing_to_keycode( context.conn, charAsCharStar );
  
    if( wrap_code ){
        xcb_test_fake_input( context.conn, XCB_KEY_PRESS, *wrap_code, 0, none, 0, 0, 0 );  
        printf("wrapcode\n");
    }

    else{
        xcb_test_fake_input( context.conn, XCB_KEY_PRESS, *code, 0, none, 0, 0, 0 );  
        xcb_test_fake_input( context.conn, XCB_KEY_RELEASE, *code, 0, none, 0, 0, 0 );
        //xcb_test_fake_input( context.conn, XCB_KEY_PRESS, *charAsCharStar, 0, context.parent, 0, 0, 0 );  // have to look at xcb_keysyms
        //xcb_test_fake_input( context.conn, XCB_KEY_RELEASE, *charAsCharStar, 0, context.parent, 0, 0, 0 );
    }
    printf("key press received by xtoq.c - xcb keycode '%s',  from Mac keyCode '%i' in Mac window #%i - (ASCII %hu)\n"
           , code, keyCode, window, aChar);
    //printf("key press received by xtoq.c - xcb keycode '%s', wrapcode '%s' from Mac keyCode '%i' in Mac window #%i - (ASCII %hu)\n"
     //      , code, wrap_code, keyCode, window, aChar);
    /*if (wrap_code)
        free(wrap_code);
    if (code)
        free(code);*/
}

void
dummy_xtoq_button_down (xtoq_context_t context, long x, long y, int window)
{
    xcb_window_t none = { XCB_NONE };
    xcb_test_fake_input (context.conn,
                         XCB_BUTTON_PRESS,
                         43,
                         0,
                         none,//context.parent, // remove context to see "UNKNOWN EVENT" message
                         x, // has to be translated (?in the view)
                         y,
                         0 );
    printf("button down received by xtoq.c - (%ld,%ld) in Mac window #%i\n", x, y, window);
}

/* #endif //_XTOQ_C_ */
