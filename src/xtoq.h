/* Copyright (c) 2012 David Snyder
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


#ifndef _XTOQ_H_
#define _XTOQ_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xcb/xcb_aux.h>
#include <xcb/damage.h>
#include <xcb/xtest.h>
#include <xcb/xcb_keysyms.h> //aaron
#include "context_list.h"
#include "util.h"

#define XTOQ_DAMAGE 0
#define XTOQ_EXPOSE 1
#define XTOQ_CREATE 2
#define XTOQ_DESTROY 3

typedef struct xtoq_context_t {
    xcb_connection_t *conn;
    xcb_drawable_t window;
    xcb_window_t parent;
    int x;
    int y;
    int width;
    int height;
    void *local_data;   // Area for data client cares about
} xtoq_context_t;

typedef struct xtoq_event_t {
    xtoq_context_t *context;
    int event_type;
} xtoq_event_t;

/**
 * Strucuture used to pass nesessary data to xtoq_start_event_loop.
 */
typedef struct xtoq_event_connetion {
	xcb_connection *conn;		/* Connection to listen to events on */
	void * event_callback;		/* Fuction to call when event caught */
} xtoq_event_connection;

/**
 * Sets up the connection and grabs the root window from the specified screen
 * @param screen The screen that we wish to connect to
 */
xtoq_context_t
xtoq_init(char *screen);

xcb_query_extension_reply_t * 
_xtoq_init_extension(xcb_connection_t *conn, char *extension_name);

void 
_xtoq_init_damage(xtoq_context_t contxt);

xcb_image_t *
xtoq_get_image(xtoq_context_t context);

int
dummy_xtoq_get_image(xtoq_context_t context);

xtoq_event_t
dummy_xtoq_wait_for_event(xtoq_context_t context);

/**
 * Starts the event loop and listens on the connection specified in
 * the given xtoq_context_t. Uses callback as the function to call
 * when an event of interest is received. Callback must be able to
 * take an xtoq_event_t as its one and only parameter.
 * @param root_context The context containing the connection to listen
 * for events on.
 * @param callback The function to call when an event of interest is
 * received.
 * @return Uses the return value of the call to pthread_create as 
 * the return value.
 */
int
xtoq_start_event_loop (xtoq_context_t root_context, void *callback);

/**
 * Event loop that returns X events. Designed to be called by
 * a seprate thread in of the client application so it does
 * not block.
 * @param xtoq_context_t The context whose connection will be
 * used to listen for X events.
 * @returns The event caught.
 */
xtoq_event_t
xtoq_wait_for_event (xtoq_context_t context);

/**
 * Testing function
 * @param context xtoq_context_t 
 * @param window The window that the key press was made in.
 * @param keyCode The key pressed.
 */
void
dummy_xtoq_key_press (xtoq_context_t context, int window, unsigned short keyCode, unsigned short aChar, char *);
uint8_t *
dummy_thing_to_keycode( xcb_connection_t *c, char *thing );
/**
 * Testing function
 * @param context xtoq_context_t 
 * @param x - x coordinate
 * @param y - y coordinate
 * @param window The window that the key press was made in.
 */
void
dummy_xtoq_button_down (xtoq_context_t context, long x, long y, int window);

#endif // _XTOQ_H_
