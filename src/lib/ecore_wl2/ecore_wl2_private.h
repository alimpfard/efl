#ifndef _ECORE_WL2_PRIVATE_H
# define _ECORE_WL2_PRIVATE_H

# include <unistd.h>
# include "Ecore_Wl2.h"
# include "Ecore_Input.h"

/* NB: Test if subsurface protocol is part of wayland code, if not then
 * include our own copy */
# ifndef WL_SUBSURFACE_ERROR_ENUM
#  include "subsurface-client-protocol.h"
# endif

# include "xdg-shell-client-protocol.h"
# define XDG_VERSION 5

extern int _ecore_wl2_log_dom;

# ifdef ECORE_WL2_DEFAULT_LOG_COLOR
#  undef ECORE_WL2_DEFAULT_LOG_COLOR
# endif
# define ECORE_WL2_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

# ifdef ERR
#  undef ERR
# endif
# define ERR(...) EINA_LOG_DOM_ERR(_ecore_wl2_log_dom, __VA_ARGS__)

# ifdef DBG
#  undef DBG
# endif
# define DBG(...) EINA_LOG_DOM_DBG(_ecore_wl2_log_dom, __VA_ARGS__)

# ifdef INF
#  undef INF
# endif
# define INF(...) EINA_LOG_DOM_INFO(_ecore_wl2_log_dom, __VA_ARGS__)

# ifdef WRN
#  undef WRN
# endif
# define WRN(...) EINA_LOG_DOM_WARN(_ecore_wl2_log_dom, __VA_ARGS__)

# ifdef CRI
#  undef CRI
# endif
# define CRI(...) EINA_LOG_DOM_CRIT(_ecore_wl2_log_dom, __VA_ARGS__)

struct _Ecore_Wl2_Display
{
   int refs;
   char *name;
   pid_t pid;

   struct
     {
        struct wl_display *display;
        struct wl_registry *registry;
        struct wl_compositor *compositor;
        struct wl_subcompositor *subcompositor;
        struct wl_data_device_manager *data_device_manager;
        struct wl_shm *shm;
        struct wl_shell *wl_shell;
        struct xdg_shell *xdg_shell;
     } wl;

   uint32_t serial;

   struct xkb_context *xkb_context;

   Ecore_Idle_Enterer *idle_enterer;
   Ecore_Fd_Handler *fd_hdl;

   Eina_Hash *globals;

   Eina_Inlist *windows;
   Eina_Inlist *outputs;
   Eina_Inlist *inputs;

   Eina_Bool sync_done : 1;
};

struct _Ecore_Wl2_Subsurface
{
   EINA_INLIST;

   int x, y;

   Ecore_Wl2_Window *parent;

   struct
     {
        struct wl_surface *surface;
        struct wl_subsurface *subsurface;
     } wl;

   Eina_Bool sync : 1;
};

struct _Ecore_Wl2_Window
{
   EINA_INLIST;

   Ecore_Wl2_Display *display;
   Ecore_Wl2_Input *input;

   Ecore_Wl2_Window *parent;

   int id, rotation, surface_id;
   const char *title;
   const char *class;
   const char *cursor;

   struct wl_surface *surface;
   struct wl_shell_surface *wl_shell_surface;
   struct xdg_surface *xdg_surface;
   struct xdg_popup *xdg_popup;

   Eina_Rectangle saved;
   Eina_Rectangle geometry;
   Eina_Rectangle opaque;
   Eina_Rectangle input_rect;

   Ecore_Wl2_Window_Type type;

   Eina_Inlist *subsurfs;

   Eina_Bool moving : 1;
   Eina_Bool minimized : 1;
   Eina_Bool maximized : 1;
   Eina_Bool fullscreen : 1;
   Eina_Bool focused : 1;
   Eina_Bool resizing : 1;
   Eina_Bool alpha : 1;
   Eina_Bool transparent : 1;
};

struct _Ecore_Wl2_Output
{
   EINA_INLIST;

   Ecore_Wl2_Display *display;
   struct wl_output *wl_output;

   int mw, mh, transform;
   const char *make, *model;
   Eina_Rectangle geometry;
};

typedef struct _Ecore_Wl2_Dnd_Source
{
   Ecore_Wl2_Input *input;

   int refcount, fd, x, y;

   struct wl_data_offer *offer;
   struct wl_array types;
} Ecore_Wl2_Dnd_Source;

struct _Ecore_Wl2_Input
{
   EINA_INLIST;

   Ecore_Wl2_Display *display;

   unsigned int timestamp;

   struct
     {
        struct wl_seat *seat;
        struct wl_pointer *pointer;
        struct wl_keyboard *keyboard;
        struct wl_touch *touch;
     } wl;

   struct
     {
        struct wl_data_device *device;
        struct wl_data_source *source;
        struct wl_array types;
     } data;

   struct
     {
        const char *name, *theme_name;
        unsigned int index, size;
        struct wl_cursor *wl_cursor;
        struct wl_cursor_theme *theme;
        struct wl_surface *surface;
        struct wl_callback *frame_cb;
        Ecore_Timer *timer;
     } cursor;

   struct
     {
        double sx, sy;
        unsigned int button;
        unsigned int enter_serial;
     } pointer;

   struct
     {
        unsigned int modifiers;
     } keyboard;

   struct
     {
        Ecore_Wl2_Window *pointer;
        Ecore_Wl2_Window *keyboard;
        Ecore_Wl2_Window *touch;
     } focus;

   struct
     {
        unsigned int button, count, timestamp;
        Ecore_Wl2_Window *window;
     } grab;

   struct
     {
        struct xkb_keymap *keymap;
        struct xkb_state *state;
        xkb_mod_mask_t control_mask;
        xkb_mod_mask_t alt_mask;
        xkb_mod_mask_t shift_mask;
        xkb_mod_mask_t win_mask;
        xkb_mod_mask_t scroll_mask;
        xkb_mod_mask_t num_mask;
        xkb_mod_mask_t caps_mask;
        xkb_mod_mask_t altgr_mask;
        unsigned int mods_depressed;
        unsigned int mods_latched;
        unsigned int mods_locked;
        unsigned int mods_group;
     } xkb;

   struct
     {
        Ecore_Timer *timer;
        unsigned int sym, key, time;
        double rate, delay;
        Eina_Bool enabled : 1;
     } repeat;

   struct
     {
        Ecore_Wl2_Dnd_Source *source;
     } drag, selection;
};

Ecore_Wl2_Window *_ecore_wl2_display_window_surface_find(Ecore_Wl2_Display *display, struct wl_surface *wl_surface);

void _ecore_wl2_output_add(Ecore_Wl2_Display *display, unsigned int id);
void _ecore_wl2_output_del(Ecore_Wl2_Output *output);

void _ecore_wl2_input_add(Ecore_Wl2_Display *display, unsigned int id);
void _ecore_wl2_input_del(Ecore_Wl2_Input *input);

void _ecore_wl2_input_ungrab(Ecore_Wl2_Input *input);
void _ecore_wl2_input_grab(Ecore_Wl2_Input *input, Ecore_Wl2_Window *window, unsigned int button);

void _ecore_wl2_input_cursor_set(Ecore_Wl2_Input *input, const char *cursor);
void _ecore_wl2_input_cursor_update_stop(Ecore_Wl2_Input *input);

void _ecore_wl2_dnd_add(Ecore_Wl2_Input *input, struct wl_data_offer *offer);
void _ecore_wl2_dnd_enter(Ecore_Wl2_Input *input, struct wl_data_offer *offer, struct wl_surface *surface, int x, int y, unsigned int timestamp);
void _ecore_wl2_dnd_leave(Ecore_Wl2_Input *input);
void _ecore_wl2_dnd_motion(Ecore_Wl2_Input *input, int x, int y, unsigned int timestamp);
void _ecore_wl2_dnd_drop(Ecore_Wl2_Input *input);
void _ecore_wl2_dnd_selection(Ecore_Wl2_Input *input, struct wl_data_offer *offer);
void _ecore_wl2_dnd_del(Ecore_Wl2_Dnd_Source *source);

void _ecore_wl2_subsurf_free(Ecore_Wl2_Subsurface *subsurf);

#endif
