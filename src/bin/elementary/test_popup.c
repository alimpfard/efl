#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_POPUP_ANCHOR_BETA
#include <Elementary.h>

#define POPUP_POINT_MAX 8

typedef struct
{
   double x;
   double y;
} Evas_Rel_Coord_Point;

static Evas_Object *g_popup = NULL;
static int times = 0;
static Eina_Bool is_popup_scroll;
static Evas_Rel_Coord_Point _popup_point[POPUP_POINT_MAX] =
{
   { 0.01, 0.01 },
   { 0.2, 0.2 },
   { 0.5, 0.5 },
   { 0.99, 0.01 },
   { 0.01, 0.99 },
   { 0.99, 0.99 },
   { 0.0, 0.0 },
   { 1.5, 1.5 }
};

static void
_response_cb(void *data EINA_UNUSED, Evas_Object *obj,
             void *event_info EINA_UNUSED)
{
   evas_object_del(obj);
}

static void
_popup_close_cb(void *data, Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   evas_object_del(data);
}

static void
_popup_dismiss_btn_cb(void *data, Evas_Object *obj EINA_UNUSED,
                      void *event_info EINA_UNUSED)
{
   elm_popup_dismiss(data);
}

static void
_popup_align_cb(void *data, Evas_Object *obj EINA_UNUSED,
               void *event_info EINA_UNUSED)
{
   static int k = 0;
   double h = -1, v = -1;

   elm_popup_align_set(data, _popup_point[k].x, _popup_point[k].y);
   elm_popup_align_get(data, &h, &v);

   printf("elm_popup_align_get :: Aligned: %lf %lf\n", h, v);

   k++;
   if (k >= POPUP_POINT_MAX)
     k = 0;
}

static void
_g_popup_response_cb(void *data, Evas_Object *obj EINA_UNUSED,
             void *event_info EINA_UNUSED)
{
   evas_object_hide(data);
}

static void
_restack_block_clicked_cb(void *data EINA_UNUSED, Evas_Object *obj,
                          void *event_info EINA_UNUSED)
{
   // remove restacked image if there is any
   Evas_Object *popup_data = evas_object_data_get(obj, "im");
   if (popup_data) evas_object_del(popup_data);
   evas_object_del(obj);
}

static void
_restack_popup_close_cb(void *data, Evas_Object *obj EINA_UNUSED,
                        void *event_info EINA_UNUSED)
{
   Evas_Object *popup_data = evas_object_data_get(data, "im");
   if (popup_data) evas_object_del(popup_data);
   evas_object_del(data);
}

static void
_block_clicked_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   if (data)
     evas_object_del(data);
   evas_object_del(obj);
}

static void
_item_selected_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                  void *event_info)
{
   printf("popup item selected: %s\n", elm_object_item_text_get(event_info));
}

static void
_list_click(void *data EINA_UNUSED, Evas_Object *obj,
            void *event_info EINA_UNUSED)
{
   Elm_Object_Item *it = elm_list_selected_item_get(obj);
   if (!it) return;
   elm_list_item_selected_set(it, EINA_FALSE);
}

static void
_popup_key_down_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED,
                   Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;

   printf("Key: %s\n", ev->keyname);
}

static void
_popup_center_text_cb(void *data, Evas_Object *obj EINA_UNUSED,
                      void *event_info EINA_UNUSED)
{
   Evas_Object *popup;

   popup = elm_popup_add(data);
   elm_popup_scrollable_set(popup, is_popup_scroll);
   elm_object_text_set(popup, "This Popup has content area and "
                       "timeout value is 3 seconds");
   elm_popup_timeout_set(popup, 3.0);
   evas_object_smart_callback_add(popup, "dismissed", _response_cb, popup);
   evas_object_event_callback_add(popup, EVAS_CALLBACK_KEY_DOWN,
                                  _popup_key_down_cb, NULL);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_popup_center_text_1button_cb(void *data, Evas_Object *obj EINA_UNUSED,
                              void *event_info EINA_UNUSED)
{
   Evas_Object *popup;
   Evas_Object *btn;

   popup = elm_popup_add(data);
   elm_popup_scrollable_set(popup, is_popup_scroll);
   elm_object_text_set(popup, "This Popup has content area and "
                       "action area set, action area has one button Close");

   // popup buttons
   btn = elm_button_add(popup);
   elm_object_text_set(btn, "Close");
   elm_object_part_content_set(popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _popup_close_cb, popup);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_popup_center_title_text_1button_cb(void *data, Evas_Object *obj EINA_UNUSED,
                                    void *event_info EINA_UNUSED)
{
   Evas_Object *popup;
   Evas_Object *btn;

   popup = elm_popup_add(data);
   elm_popup_scrollable_set(popup, is_popup_scroll);

   // popup text
   elm_object_text_set(popup, "This Popup has title area, content area and "
                       "action area set, action area has one button Close");
   // popup title
   elm_object_part_text_set(popup, "title,text", "Title");

   // popup buttons
   btn = elm_button_add(popup);
   elm_object_text_set(btn, "Close");
   elm_object_part_content_set(popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _popup_close_cb, popup);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_popup_center_title_text_block_clicked_event_cb(void *data,
                                                Evas_Object *obj EINA_UNUSED,
                                                void *event_info EINA_UNUSED)
{
   Evas_Object *popup;

   popup = elm_popup_add(data);
   elm_popup_scrollable_set(popup, is_popup_scroll);
   evas_object_smart_callback_add(popup, "block,clicked", _block_clicked_cb,
                                  NULL);

   // popup text
   elm_object_text_set(popup, "This Popup has title area and content area. "
                       "When clicked on blocked event region, popup gets "
                       "deleted");
   // popup title
   elm_object_part_text_set(popup, "title,text", "Title");

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_popup_center_title_text_block_clicked_event_with_parent_cb(void *data,
                                                            Evas_Object *obj EINA_UNUSED,
                                                            void *event_info EINA_UNUSED)
{
   Evas_Object *bg, *popup;

   bg = elm_bg_add(data);
   elm_bg_color_set(bg, 255, 0, 0);
   evas_object_resize(bg, 350, 200);
   evas_object_move(bg, 100, 100);
   evas_object_show(bg);

   popup = elm_popup_add(bg);
   elm_popup_scrollable_set(popup, is_popup_scroll);
   evas_object_smart_callback_add(popup, "block,clicked",
                                  _block_clicked_cb, bg);

   // popup text
   elm_object_text_set(popup, "This Popup has title area and content area. "
                       "Its blocked event region is a small rectangle. "
                       "When clicked on blocked event region, popup gets "
                       "deleted");
   // popup title
   elm_object_part_text_set(popup, "title,text", "Title");

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_popup_bottom_title_text_3button_cb(void *data, Evas_Object *obj EINA_UNUSED,
                                    void *event_info EINA_UNUSED)
{
   Evas_Object *popup;
   Evas_Object *icon, *btn1, *btn2, *btn3;
   char buf[256];

   popup = elm_popup_add(data);
   elm_popup_scrollable_set(popup, is_popup_scroll);
   elm_popup_orient_set(popup, ELM_POPUP_ORIENT_BOTTOM);

   // popup text
   elm_object_text_set(popup, "This Popup has title area, content area and "
                       "action area set with content being character wrapped. "
                       "action area has three buttons OK, Cancel and Close");
   elm_popup_content_text_wrap_type_set(popup, ELM_WRAP_CHAR);

   // popup title
   elm_object_part_text_set(popup, "title,text", "Title");

   // popup title icon
   icon = elm_icon_add(popup);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png",
            elm_app_data_dir_get());
   elm_image_file_set(icon, buf, NULL);
   elm_object_part_content_set(popup, "title,icon", icon);

   // popup buttons
   btn1 = elm_button_add(popup);
   elm_object_text_set(btn1, "OK");
   elm_object_part_content_set(popup, "button1", btn1);
   evas_object_smart_callback_add(btn1, "clicked", _popup_close_cb, popup);

   btn2 = elm_button_add(popup);
   elm_object_text_set(btn2, "Cancel");
   elm_object_part_content_set(popup, "button2", btn2);
   evas_object_smart_callback_add(btn2, "clicked", _popup_close_cb, popup);

   btn3 = elm_button_add(popup);
   elm_object_text_set(btn3, "Close");
   elm_object_part_content_set(popup, "button3", btn3);
   evas_object_smart_callback_add(btn3, "clicked", _popup_close_cb, popup);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_popup_center_title_content_3button_cb(void *data, Evas_Object *obj EINA_UNUSED,
                                       void *event_info EINA_UNUSED)
{
   Evas_Object *popup;
   Evas_Object *icon, *btn, *btn1, *btn2, *btn3;
   char buf[256];

   popup = elm_popup_add(data);
   elm_popup_scrollable_set(popup, is_popup_scroll);

   // popup title
   elm_object_part_text_set(popup, "title,text", "Title");

   // popup content
   btn = elm_button_add(popup);
   elm_object_text_set(btn, "Content");
   icon = elm_icon_add(btn);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png",
            elm_app_data_dir_get());
   elm_image_file_set(icon, buf, NULL);
   elm_object_content_set(btn, icon);
   elm_object_content_set(popup, btn);

   // popup buttons
   btn1 = elm_button_add(popup);
   elm_object_text_set(btn1, "OK");
   elm_object_part_content_set(popup, "button1", btn1);
   evas_object_smart_callback_add(btn1, "clicked", _popup_close_cb, popup);

   btn2 = elm_button_add(popup);
   elm_object_text_set(btn2, "Cancel");
   elm_object_part_content_set(popup, "button2", btn2);
   evas_object_smart_callback_add(btn2, "clicked", _popup_close_cb, popup);

   btn3 = elm_button_add(popup);
   elm_object_text_set(btn3, "Close");
   elm_object_part_content_set(popup, "button3", btn3);
   evas_object_smart_callback_add(btn3, "clicked", _popup_close_cb, popup);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_item_focused_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Elm_Object_Item *it = event_info;

   printf("item,focused:%p\n", it);
}

static void
_item_unfocused_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Elm_Object_Item *it = event_info;

   printf("item,unfocused:%p\n", it);
}

static void
_popup_center_title_item_3button_cb(void *data, Evas_Object *obj EINA_UNUSED,
                                    void *event_info EINA_UNUSED)
{
   char buf[256];
   unsigned int i;
   Evas_Object *popup, *icon1, *btn1, *btn2, *btn3;

   popup = elm_popup_add(data);
   elm_popup_scrollable_set(popup, is_popup_scroll);

   // popup title
   elm_object_part_text_set(popup, "title,text", "Title");

   // popup items
   icon1 = elm_icon_add(popup);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png",
            elm_app_data_dir_get());
   elm_image_file_set(icon1, buf, NULL);
   for (i = 0; i < 20; i++)
     {
        snprintf(buf, sizeof(buf), "Item%u", i+1);
        if (i == 3)
          elm_popup_item_append(popup, buf, icon1, _item_selected_cb, NULL);
        else
          elm_popup_item_append(popup, buf, NULL, _item_selected_cb, NULL);
     }

   // popup buttons
   btn1 = elm_button_add(popup);
   elm_object_text_set(btn1, "OK");
   elm_object_part_content_set(popup, "button1", btn1);
   evas_object_smart_callback_add(btn1, "clicked", _popup_close_cb, popup);

   btn2 = elm_button_add(popup);
   elm_object_text_set(btn2, "Cancel");
   elm_object_part_content_set(popup, "button2", btn2);
   evas_object_smart_callback_add(btn2, "clicked", _popup_close_cb, popup);

   btn3 = elm_button_add(popup);
   elm_object_text_set(btn3, "Close");
   elm_object_part_content_set(popup, "button3", btn3);
   evas_object_smart_callback_add(btn3, "clicked", _popup_close_cb, popup);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
   evas_object_smart_callback_add(popup, "item,focused", _item_focused_cb, NULL);
   evas_object_smart_callback_add(popup, "item,unfocused", _item_unfocused_cb, NULL);
}

static void
_restack_btn_clicked(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *im;
   char buf[PATH_MAX];
   void *popup_data;

   popup_data = evas_object_data_get(data, "im");
   if (popup_data) return;

   im = evas_object_image_filled_add(evas_object_evas_get(obj));
   snprintf(buf, sizeof(buf), "%s/images/%s",
            elm_app_data_dir_get(), "twofish.jpg");
   evas_object_image_file_set(im, buf, NULL);
   evas_object_move(im, 40, 40);
   evas_object_resize(im, 320, 320);
   evas_object_show(im);
   evas_object_data_set((Evas_Object *)data, "im", im);

   evas_object_raise((Evas_Object *)data);
}

static void
_popup_center_title_text_2button_restack_cb(void *data, Evas_Object *obj EINA_UNUSED,
                                    void *event_info EINA_UNUSED)
{
   Evas_Object *popup;
   Evas_Object *btn, *btn2;

   popup = elm_popup_add(data);
   elm_popup_scrollable_set(popup, is_popup_scroll);
   evas_object_smart_callback_add(popup, "block,clicked",
                                  _restack_block_clicked_cb, NULL);

   // popup text
   elm_object_text_set(popup, "When you click the 'Restack' button, "
                       "an image will be located under this popup");
   // popup title
   elm_object_part_text_set(popup, "title,text", "Title");

   // popup buttons
   btn = elm_button_add(popup);
   elm_object_text_set(btn, "Restack");
   elm_object_part_content_set(popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _restack_btn_clicked, popup);

   btn2 = elm_button_add(popup);
   elm_object_text_set(btn2, "Close");
   elm_object_part_content_set(popup, "button2", btn2);
   evas_object_smart_callback_add(btn2, "clicked",
                                  _restack_popup_close_cb, popup);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_popup_center_text_1button_hide_show_cb(void *data, Evas_Object *obj EINA_UNUSED,
                              void *event_info EINA_UNUSED)
{
   Evas_Object *btn;
   char str[128];

   times++;
   if (g_popup)
     {
        sprintf(str, "You have checked this popup %d times.", times);
        elm_object_text_set(g_popup, str);
        evas_object_show(g_popup);
        return;
     }

   g_popup = elm_popup_add(data);
   elm_popup_scrollable_set(g_popup, is_popup_scroll);
   elm_object_text_set(g_popup, "Hide this popup by using the button."
                       "When you click list item again, you can see this popup.");

   // popup buttons
   btn = elm_button_add(g_popup);
   elm_object_text_set(btn, "Hide");
   elm_object_part_content_set(g_popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _g_popup_response_cb, g_popup);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(g_popup);
}

static void
_toggle_button_cb(void *data,
                  Evas_Object *obj,
                  void *event_info EINA_UNUSED)
{
   Evas_Object *btn = data;
   char buf[] = "button0";
   int i;

   i = (int)(uintptr_t)evas_object_data_get(btn, "index");

   buf[6] = '0' + i + 1;
   if (evas_object_visible_get(btn))
     {
        elm_object_part_content_unset(obj, buf);
        evas_object_hide(btn);
     }
   else
     elm_object_part_content_set(obj, buf, btn);
}

static void
_popup_center_text_3button_add_remove_button_cb(void *data,
                                                Evas_Object *obj EINA_UNUSED,
                                                void *event_info EINA_UNUSED)
{
   Evas_Object *popup;
   Evas_Object *btns[3];

   char buf[256];
   int i;

   popup = elm_popup_add(data);
   elm_popup_scrollable_set(popup, is_popup_scroll);

   // popup title
   elm_object_part_text_set(popup, "title,text",
                            "Click the item to toggle button");

   // popup buttons
   for (i = 0; i < 3; ++i)
     {
        snprintf(buf, sizeof(buf), "Btn #%d", i + 1);
        btns[i] = elm_button_add(popup);
        evas_object_data_set(btns[i], "index", (void*)(uintptr_t)i);
        elm_object_text_set(btns[i], buf);

        elm_popup_item_append(popup, buf, NULL, _toggle_button_cb, btns[i]);

        snprintf(buf, sizeof(buf), "button%d", i + 1);
        elm_object_part_content_set(popup, buf, btns[i]);
        evas_object_smart_callback_add(btns[i], "clicked",
                                       _popup_close_cb, popup);
     }

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_popup_transparent_cb(void *data, Evas_Object *obj EINA_UNUSED,
                      void *event_info EINA_UNUSED)
{
   Evas_Object *popup;
   Evas_Object *btn;

   popup = elm_popup_add(data);
   elm_popup_scrollable_set(popup, is_popup_scroll);
   elm_object_style_set(popup, "transparent");
   elm_object_text_set(popup, "This Popup has transparent background");

   // popup buttons
   btn = elm_button_add(popup);
   elm_object_text_set(btn, "Close");
   elm_object_part_content_set(popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _popup_close_cb, popup);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_popup_transparent_align_cb(void *data, Evas_Object *obj EINA_UNUSED,
                            void *event_info EINA_UNUSED)
{
   Evas_Object *popup;
   Evas_Object *btn, *btn1;

   popup = elm_popup_add(data);
   elm_popup_scrollable_set(popup, is_popup_scroll);
   elm_object_style_set(popup, "transparent");
   elm_object_text_set(popup, "This Popup has transparent background");

   // popup buttons
   btn = elm_button_add(popup);
   elm_object_text_set(btn, "Move");
   elm_object_part_content_set(popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _popup_align_cb, popup);

   btn1 = elm_button_add(popup);
   elm_object_text_set(btn1, "Close");
   elm_object_part_content_set(popup, "button2", btn1);
   evas_object_smart_callback_add(btn1, "clicked", _popup_close_cb, popup);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_list_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   evas_object_del(data);
}

char *
gl_popup_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   char buf[256];
   snprintf(buf, sizeof(buf), "Item # %i", (int)(uintptr_t)data);
   return strdup(buf);
}

static void
_delete_btn_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_object_content_set(data, NULL);
}

static void
_popup_center_title_list_content_2button_cb(void *data, Evas_Object *obj EINA_UNUSED,
                                            void *event_info EINA_UNUSED)
{
   Evas_Object *popup, *list;
   Evas_Object *btn;

   popup = elm_popup_add(data);
   elm_popup_scrollable_set(popup, is_popup_scroll);
   elm_object_part_text_set(popup, "title,text", "Title");

   // list as a popup content
   list = elm_list_add(popup);
   elm_list_mode_set(list, ELM_LIST_EXPAND);
   elm_list_item_append(list, "List Item #1", NULL, NULL, _list_cb, popup);
   elm_list_item_append(list, "List Item #2", NULL, NULL, _list_cb, popup);
   elm_list_item_append(list, "List Item #3", NULL, NULL, _list_cb, popup);
   elm_object_content_set(popup, list);

   // popup buttons
   btn = elm_button_add(popup);
   elm_object_text_set(btn, "OK");
   elm_object_part_content_set(popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _popup_close_cb, popup);

   btn = elm_button_add(popup);
   elm_object_text_set(btn, "Delete");
   elm_object_part_content_set(popup, "button2", btn);
   evas_object_smart_callback_add(btn, "clicked", _delete_btn_clicked, popup);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_popup_center_title_genlist_content_1button_cb(void *data, Evas_Object *obj EINA_UNUSED,
                                            void *event_info EINA_UNUSED)
{
   Evas_Object *popup, *genlist;
   Evas_Object *btn;
   int i;

   popup = elm_popup_add(data);
   elm_popup_scrollable_set(popup, is_popup_scroll);
   elm_object_part_text_set(popup, "title,text", "Title");

   Elm_Genlist_Item_Class *itc1 = elm_genlist_item_class_new();
   itc1->item_style = "default";
   itc1->func.text_get = gl_popup_text_get;
   itc1->func.content_get = NULL;
   itc1->func.state_get = NULL;
   itc1->func.del = NULL;

   // genlist as a popup content
   genlist = elm_genlist_add(popup);
   elm_genlist_mode_set(genlist, ELM_LIST_EXPAND);
   elm_scroller_content_min_limit(genlist, EINA_FALSE, EINA_TRUE);

   for (i = 0; i < 10; i++)
     {
        elm_genlist_item_append(genlist,
                                itc1,
                                (void *)(uintptr_t)i/* item data */,
                                NULL/* parent */,
                                ELM_GENLIST_ITEM_NONE,
                                NULL,
                                NULL);
     }
   elm_genlist_item_class_free(itc1);
   elm_object_content_set(popup, genlist);

   // popup buttons
   btn = elm_button_add(popup);
   elm_object_text_set(btn, "OK");
   elm_object_part_content_set(popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _popup_close_cb, popup);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_subpopup_cb(void *data, Evas_Object *obj EINA_UNUSED,
             void *event_info EINA_UNUSED)
{
   Evas_Object *popup;
   Evas_Object *btn, *btnclose;

   popup = elm_popup_add(data);
   elm_popup_scrollable_set(popup, is_popup_scroll);
   elm_object_style_set(popup, "subpopup");
   elm_object_part_text_set(popup, "title,text", "Title");

   // button as a popup content
   btn = elm_button_add(popup);
   elm_object_text_set(btn, "content");
   elm_object_content_set(popup, btn);

   // popup buttons
   btn = elm_button_add(popup);
   elm_object_text_set(btn, "OK");
   elm_object_part_content_set(popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _popup_close_cb, popup);

   //popup-base close button
   btnclose = elm_button_add(popup);
   //TODO: write a X style button theme
   elm_object_text_set(btnclose, "x");
   elm_object_part_content_set(popup, "elm.swallow.closebtn", btnclose);
   evas_object_smart_callback_add(btnclose, "clicked", _popup_close_cb, popup);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_popup_content_only_cb(void *data, Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED)
{
   Evas_Object *popup, *bx, *hbx, *lbl, *btn;
   Evas_Object *win = data;

   popup = elm_popup_add(win);
   elm_popup_scrollable_set(popup, is_popup_scroll);

   bx = elm_box_add(popup);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.0);
   elm_object_content_set(popup, bx);

   lbl = elm_label_add(popup);
   elm_object_text_set(lbl,
                       "<align=center>This popup contains content area only.<br>"
                       "The buttons below are the part of content area, not action area");
   evas_object_size_hint_weight_set(lbl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(lbl, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(bx, lbl);
   evas_object_show(lbl);

   hbx = elm_box_add(bx);
   elm_box_horizontal_set(hbx, EINA_TRUE);
   evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(hbx, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(bx, hbx);
   evas_object_show(hbx);

   btn = elm_button_add(hbx);
   elm_object_text_set(btn, "Save");
   evas_object_smart_callback_add(btn, "clicked", _popup_close_cb, popup);
   elm_box_pack_end(hbx, btn);
   evas_object_show(btn);

   btn = elm_button_add(hbx);
   elm_object_text_set(btn, "Discard");
   evas_object_smart_callback_add(btn, "clicked", _popup_close_cb, popup);
   elm_box_pack_end(hbx, btn);
   evas_object_show(btn);

   btn = elm_button_add(hbx);
   elm_object_text_set(btn, "Cancel");
   evas_object_smart_callback_add(btn, "clicked", _popup_close_cb, popup);
   elm_box_pack_end(hbx, btn);
   evas_object_show(btn);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
   elm_object_focus_set(btn, EINA_TRUE);
}

static void
_popup_center_title_text_1button_hide_effect_cb(void *data, Evas_Object *obj EINA_UNUSED,
                                                void *event_info EINA_UNUSED)
{
   Evas_Object *popup;
   Evas_Object *btn;

   popup = elm_popup_add(data);
   elm_popup_scrollable_set(popup, is_popup_scroll);

   // popup text
   elm_object_text_set(popup, "This Popup has title area, content area and "
                       "action area set, action area has one button Close");
   // popup title
   elm_object_part_text_set(popup, "title,text", "Title");

   // popup buttons
   btn = elm_button_add(popup);
   elm_object_text_set(btn, "Close");
   elm_object_part_content_set(popup, "button1", btn);
   evas_object_smart_callback_add(popup, "dismissed", _response_cb, NULL);
   evas_object_smart_callback_add(btn, "clicked", _popup_dismiss_btn_cb, popup);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_focus_changed_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Eina_Bool check = elm_check_state_get(obj);
   elm_win_focus_highlight_enabled_set(data, check);
   elm_win_focus_highlight_animate_set(data, check);
}

static void
_popup_scroll_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   is_popup_scroll = elm_check_state_get(obj);
}

void
test_popup(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *list, *check;

   win = elm_win_util_standard_add("popup", "Popup");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   list = elm_list_add(box);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, list);
   elm_list_mode_set(list, ELM_LIST_LIMIT);
   evas_object_smart_callback_add(list, "selected", _list_click, NULL);

   elm_list_item_append(list, "popup-center-text", NULL, NULL,
                        _popup_center_text_cb, win);
   elm_list_item_append(list, "popup-center-text + 1 button", NULL, NULL,
                        _popup_center_text_1button_cb, win);
   elm_list_item_append(list, "popup-center-title + text + 1 button", NULL,
                        NULL, _popup_center_title_text_1button_cb, win);

   elm_list_item_append(list,
                        "popup-center-title + text (block,clicked handling)",
                        NULL, NULL,
                        _popup_center_title_text_block_clicked_event_cb, win);
   elm_list_item_append(list,
                        "popup-center-title + text (block,clicked handling with parent)",
                        NULL, NULL,
                        _popup_center_title_text_block_clicked_event_with_parent_cb, win);
   elm_list_item_append(list, "popup-bottom-title-icon + text + 3 buttons", NULL,
                        NULL, _popup_bottom_title_text_3button_cb, win);
   elm_list_item_append(list, "popup-center-title + content + 3 buttons", NULL,
                        NULL, _popup_center_title_content_3button_cb, win);
   elm_list_item_append(list, "popup-center-title + items + 3 buttons", NULL,
                        NULL, _popup_center_title_item_3button_cb, win);
   elm_list_item_append(list, "popup-center-title + text + 2 buttons (check restacking)", NULL, NULL,
                        _popup_center_title_text_2button_restack_cb, win);
   elm_list_item_append(list, "popup-center-text + 1 button (check hide, show)", NULL, NULL,
                        _popup_center_text_1button_hide_show_cb, win);
   elm_list_item_append(list, "popup-center-text + 3 button (check add, remove buttons)", NULL, NULL,
                        _popup_center_text_3button_add_remove_button_cb, win);
   elm_list_item_append(list, "popup-transparent", NULL, NULL,
                        _popup_transparent_cb, win);
   elm_list_item_append(list, "popup-transparent-align", NULL, NULL,
                        _popup_transparent_align_cb, win);
   elm_list_item_append(list, "popup-center-title + list content + 2 button (with delete content)",
                        NULL, NULL, _popup_center_title_list_content_2button_cb,
                        win);
   elm_list_item_append(list, "popup-center-title + genlist content + 1 button",
                        NULL, NULL, _popup_center_title_genlist_content_1button_cb,
                        win);
   elm_list_item_append(list, "subpopup + X button",
                        NULL, NULL, _subpopup_cb,
                        win);
   elm_list_item_append(list, "popup with content only",
                        NULL, NULL, _popup_content_only_cb,
                        win);
   elm_list_item_append(list, "popup-center-title + text + 1 button + hide effect", NULL,
                        NULL, _popup_center_title_text_1button_hide_effect_cb, win);
   elm_list_go(list);
   evas_object_show(list);

   check = elm_check_add(box);
   elm_object_text_set(check, "Enable Focus Highlight");
   evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, check);
   evas_object_show(check);
   evas_object_smart_callback_add(check, "changed", _focus_changed_cb, win);

   check = elm_check_add(box);
   elm_object_text_set(check, "Enable popup scroll");
   evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, check);
   evas_object_show(check);
   evas_object_smart_callback_add(check, "changed", _popup_scroll_cb, NULL);

   if (elm_config_popup_scrollable_get())
     {
        elm_check_state_set(check, EINA_TRUE);
        is_popup_scroll = EINA_TRUE;
     }

   evas_object_resize(win, 480, 400);
   evas_object_show(win);
}

static void
_bg_clicked(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                      void *event_info EINA_UNUSED)
{
   printf("Popup's background is clicked.\n");
}

static void
_image_change_btn_cb(void *data, Evas_Object *obj EINA_UNUSED,
                      void *event_info EINA_UNUSED)
{
   static int k = 0;

   if (k == 0)
     {
        char buf[PATH_MAX];
        snprintf(buf, sizeof(buf), "%s/images/logo.png", elm_app_data_dir_get());
        efl_ui_popup_bg_set(data, buf, NULL);
        efl_ui_popup_bg_repeat_events_set(data, EINA_TRUE);
     }
   else
     {
        efl_ui_popup_bg_set(data, NULL, NULL);
        efl_ui_popup_bg_repeat_events_set(data, EINA_FALSE);
     }
   k = !k;
}

static void
_center_align_cb(void *data, Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   efl_ui_popup_align_set(data, EFL_UI_POPUP_ALIGN_CENTER);
}

static void
_left_align_cb(void *data, Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   efl_ui_popup_align_set(data, EFL_UI_POPUP_ALIGN_LEFT);
}

static void
_right_align_cb(void *data, Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   efl_ui_popup_align_set(data, EFL_UI_POPUP_ALIGN_RIGHT);
}

static void
_top_align_cb(void *data, Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   efl_ui_popup_align_set(data, EFL_UI_POPUP_ALIGN_TOP);
}

static void
_bottom_align_cb(void *data, Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   efl_ui_popup_align_set(data, EFL_UI_POPUP_ALIGN_BOTTOM);
}

static void
_position_set_cb(void *data, Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   evas_object_move(data, 0, 0);
}

static void
_timeout_set_cb(void *data, Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   efl_ui_popup_timeout_set(data, 3);
   printf("timemout is set to 3 seconds\n");
}

void
test_efl_ui_popup(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *btn;

   win = elm_win_util_standard_add("Efl UI Popup", "Efl UI Popup");
   elm_win_autodel_set(win, EINA_TRUE);

   evas_object_resize(win, 500, 500);
   evas_object_show(win);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "bg repeat events test");
   evas_object_move(btn, 0, 0);
   evas_object_resize(btn, 150, 150);
   evas_object_show(btn);

   Evas_Object *efl_ui_popup= efl_add(EFL_UI_POPUP_CLASS, win);

   evas_object_smart_callback_add(efl_ui_popup, "bg,clicked", _bg_clicked, NULL);
   evas_object_resize(efl_ui_popup, 160, 160);
   evas_object_show(efl_ui_popup);

   int x, y, w, h;
   evas_object_geometry_get(efl_ui_popup, &x, &y, &w, &h);
   printf("efl_ui_popup: %d %d %d %d\n", x, y, w, h);

   btn = elm_button_add(efl_ui_popup);
   elm_object_text_set(btn, "Efl.Ui.Popup content");
   evas_object_smart_callback_add(btn, "clicked", _image_change_btn_cb, efl_ui_popup);
   evas_object_show(btn);

   Evas_Object *center_btn;
   center_btn = elm_button_add(win);
   elm_object_text_set(center_btn, "Center Align");
   evas_object_move(center_btn, 0, 300);
   evas_object_resize(center_btn, 100, 50);
   evas_object_show(center_btn);
   evas_object_smart_callback_add(center_btn, "clicked", _center_align_cb, efl_ui_popup);

   Evas_Object *left_btn;
   left_btn = elm_button_add(win);
   elm_object_text_set(left_btn, "Left Align");
   evas_object_move(left_btn, 100, 300);
   evas_object_resize(left_btn, 100, 50);
   evas_object_show(left_btn);
   evas_object_smart_callback_add(left_btn, "clicked", _left_align_cb, efl_ui_popup);

   Evas_Object *right_btn;
   right_btn = elm_button_add(win);
   elm_object_text_set(right_btn, "Right Align");
   evas_object_move(right_btn, 200, 300);
   evas_object_resize(right_btn, 100, 50);
   evas_object_show(right_btn);
   evas_object_smart_callback_add(right_btn, "clicked", _right_align_cb, efl_ui_popup);

   Evas_Object *top_btn;
   top_btn = elm_button_add(win);
   elm_object_text_set(top_btn, "Top Align");
   evas_object_move(top_btn, 0, 350);
   evas_object_resize(top_btn, 100, 50);
   evas_object_show(top_btn);
   evas_object_smart_callback_add(top_btn, "clicked", _top_align_cb, efl_ui_popup);

   Evas_Object *bottom_btn;
   bottom_btn = elm_button_add(win);
   elm_object_text_set(bottom_btn, "Bottom Align");
   evas_object_move(bottom_btn, 100, 350);
   evas_object_resize(bottom_btn, 100, 50);
   evas_object_show(bottom_btn);
   evas_object_smart_callback_add(bottom_btn, "clicked", _bottom_align_cb, efl_ui_popup);

   Evas_Object *position_btn;
   position_btn = elm_button_add(win);
   elm_object_text_set(position_btn, "Position Set");
   evas_object_move(position_btn, 200, 350);
   evas_object_resize(position_btn, 100, 50);
   evas_object_show(position_btn);
   evas_object_smart_callback_add(position_btn, "clicked", _position_set_cb, efl_ui_popup);

   Evas_Object *timeout_btn;
   timeout_btn = elm_button_add(win);
   elm_object_text_set(timeout_btn, "Timeout Set");
   evas_object_move(timeout_btn, 0, 400);
   evas_object_resize(timeout_btn, 100, 50);
   evas_object_show(timeout_btn);
   evas_object_smart_callback_add(timeout_btn, "clicked", _timeout_set_cb, efl_ui_popup);

   efl_content_set(efl_ui_popup, btn);
}

static void
efl_ui_popup_alert_clicked_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
    Efl_Ui_Popup_Alert_Button type = (Efl_Ui_Popup_Alert_Button)ev->info;
    if (type == EFL_UI_POPUP_ALERT_BUTTON_POSITIVE)
      printf("Positive Button is clicked\n");
    else if(type == EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE)
      printf("Negative Button is clicked\n");
    else if(type == EFL_UI_POPUP_ALERT_BUTTON_USER)
      printf("User Button is clicked\n");
}

void
test_efl_ui_popup_alert(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("Efl UI Popup Alert", "Efl UI Popup Alert");
   elm_win_autodel_set(win, EINA_TRUE);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);

   Evas_Object *efl_ui_popup= efl_add(EFL_UI_POPUP_ALERT_CLASS, win);

   efl_text_set(efl_part(efl_ui_popup, "title"), "title");

   evas_object_resize(efl_ui_popup, 160, 160);
   evas_object_show(efl_ui_popup);

   Evas_Object *layout = elm_layout_add(efl_ui_popup);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
   elm_layout_file_set(layout, buf, "efl_ui_popup_scroll_content");
   evas_object_show(layout);

   efl_content_set(efl_ui_popup, layout);
   efl_ui_popup_alert_button_set(efl_ui_popup, EFL_UI_POPUP_ALERT_BUTTON_POSITIVE, "Yes");
   efl_ui_popup_alert_button_set(efl_ui_popup, EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE, "No");
   efl_ui_popup_alert_button_set(efl_ui_popup, EFL_UI_POPUP_ALERT_BUTTON_USER, "Cancel");

   efl_event_callback_add(efl_ui_popup, EFL_UI_POPUP_ALERT_EVENT_CLICKED, efl_ui_popup_alert_clicked_cb, NULL);
}

void
test_efl_ui_popup_alert_scroll(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("Efl UI Popup Alert Scroll", "Efl UI Popup Alert Scroll");
   elm_win_autodel_set(win, EINA_TRUE);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);

   Evas_Object *efl_ui_popup= efl_add(EFL_UI_POPUP_ALERT_SCROLL_CLASS, win);

   efl_text_set(efl_part(efl_ui_popup, "title"), "title");

   evas_object_resize(efl_ui_popup, 160, 160);
   evas_object_show(efl_ui_popup);

   Evas_Object *layout = elm_layout_add(efl_ui_popup);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
   elm_layout_file_set(layout, buf, "efl_ui_popup_scroll_content");
   evas_object_show(layout);

   efl_content_set(efl_ui_popup, layout);

   efl_ui_popup_alert_button_set(efl_ui_popup, EFL_UI_POPUP_ALERT_BUTTON_POSITIVE, "Yes");
   efl_ui_popup_alert_button_set(efl_ui_popup, EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE, "No");
   efl_ui_popup_alert_button_set(efl_ui_popup, EFL_UI_POPUP_ALERT_BUTTON_USER, "Cancel");

   efl_ui_popup_alert_scroll_expandable_set(efl_ui_popup, EINA_TRUE, EINA_TRUE);
   efl_gfx_size_hint_max_set(efl_ui_popup, EINA_SIZE2D(300, 180));

   efl_event_callback_add(efl_ui_popup, EFL_UI_POPUP_ALERT_EVENT_CLICKED, efl_ui_popup_alert_clicked_cb, NULL);
}

void
test_efl_ui_popup_alert_text(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win;

   win = elm_win_util_standard_add("Efl UI Popup Alert Text", "Efl UI Popup Alert Text");
   elm_win_autodel_set(win, EINA_TRUE);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);

   Evas_Object *efl_ui_popup= efl_add(EFL_UI_POPUP_ALERT_TEXT_CLASS, win);

   efl_text_set(efl_part(efl_ui_popup, "title"), "title");

   evas_object_resize(efl_ui_popup, 160, 160);
   evas_object_show(efl_ui_popup);

   efl_text_set(efl_ui_popup,
                "This is Text Popup This is Text Popup This is Text Popup This is Text Popup This is Text Popup This is Text Popup This is Text Popup"
                "This is Text Popup This is Text Popup This is Text Popup This is Text Popup This is Text Popup This is Text Popup This is Text Popup"
                "This is Text Popup This is Text Popup This is Text Popup This is Text Popup This is Text Popup This is Text Popup This is Text Popup");

   efl_ui_popup_alert_button_set(efl_ui_popup, EFL_UI_POPUP_ALERT_BUTTON_POSITIVE, "Yes");
   efl_ui_popup_alert_button_set(efl_ui_popup, EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE, "No");
   efl_ui_popup_alert_button_set(efl_ui_popup, EFL_UI_POPUP_ALERT_BUTTON_USER, "Cancel");

   efl_ui_popup_alert_text_expandable_set(efl_ui_popup, EINA_TRUE);
   efl_gfx_size_hint_max_set(efl_ui_popup, EINA_SIZE2D(100, 180));

   efl_event_callback_add(efl_ui_popup, EFL_UI_POPUP_ALERT_EVENT_CLICKED, efl_ui_popup_alert_clicked_cb, NULL);
}

static void
_anchor_set_cb(void *data, Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   efl_ui_popup_anchor_set(data, obj);
}

static void
_anchor_unset_cb(void *data, Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   efl_ui_popup_anchor_set(data, NULL);
}

void
test_efl_ui_popup_anchor(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *layout, *tbl, *btn;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("Efl UI Popup Anchor", "Efl UI Popup Anchor");
   elm_win_autodel_set(win, EINA_TRUE);

   evas_object_resize(win, 500, 500);
   evas_object_show(win);

   layout = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
   elm_layout_file_set(layout, buf, "efl_ui_popup_anchor_layout");
   evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, layout);
   evas_object_show(layout);

   Evas_Object *efl_ui_popup= efl_add(EFL_UI_POPUP_ANCHOR_CLASS, win);

   efl_ui_popup_bg_repeat_events_set(efl_ui_popup, EINA_TRUE);

   //Default align priority order is top, left, right, bottom, center.
   efl_ui_popup_anchor_align_priority_set(efl_ui_popup, EFL_UI_POPUP_ALIGN_TOP,
                                          EFL_UI_POPUP_ALIGN_BOTTOM,
                                          EFL_UI_POPUP_ALIGN_LEFT,
                                          EFL_UI_POPUP_ALIGN_RIGHT,
                                          EFL_UI_POPUP_ALIGN_CENTER);

   evas_object_move(efl_ui_popup, 80, 80);
   evas_object_resize(efl_ui_popup, 160, 120);
   evas_object_show(efl_ui_popup);

   for (int i = 0; i < 6; i++)
     {
        btn = elm_button_add(win);
        elm_object_text_set(btn, "anchor");
        evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_smart_callback_add(btn, "clicked", _anchor_set_cb, efl_ui_popup);

        snprintf(buf, sizeof(buf), "anchor%d", i+1);
        elm_object_part_content_set(layout, buf, btn);
     }

   btn = elm_button_add(win);
   elm_object_text_set(btn, "anchor none");
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_smart_callback_add(btn, "clicked", _anchor_unset_cb, efl_ui_popup);
   elm_object_part_content_set(layout, "anchor_none", btn);

   tbl = elm_table_add(efl_ui_popup);
   evas_object_size_hint_weight_set(tbl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tbl, EVAS_HINT_FILL, EVAS_HINT_FILL);

   btn = elm_button_add(efl_ui_popup);
   elm_object_text_set(btn, "Center Align");
   evas_object_size_hint_min_set(btn, 70, 35);
   evas_object_smart_callback_add(btn, "clicked", _center_align_cb, efl_ui_popup);
   elm_table_pack(tbl, btn, 0, 0, 1, 1);
   evas_object_show(btn);

   btn = elm_button_add(efl_ui_popup);
   elm_object_text_set(btn, "Left Align");
   evas_object_size_hint_min_set(btn, 70, 35);
   evas_object_smart_callback_add(btn, "clicked", _left_align_cb, efl_ui_popup);
   elm_table_pack(tbl, btn, 1, 0, 1, 1);
   evas_object_show(btn);

   btn = elm_button_add(efl_ui_popup);
   elm_object_text_set(btn, "Right Align");
   evas_object_size_hint_min_set(btn, 70, 35);
   evas_object_smart_callback_add(btn, "clicked", _right_align_cb, efl_ui_popup);
   elm_table_pack(tbl, btn, 2, 0, 1, 1);
   evas_object_show(btn);

   btn = elm_button_add(efl_ui_popup);
   elm_object_text_set(btn, "Top Align");
   evas_object_size_hint_min_set(btn, 70, 35);
   evas_object_smart_callback_add(btn, "clicked", _top_align_cb, efl_ui_popup);
   elm_table_pack(tbl, btn, 0, 1, 1, 1);
   evas_object_show(btn);

   btn = elm_button_add(efl_ui_popup);
   elm_object_text_set(btn, "Bottom Align");
   evas_object_size_hint_min_set(btn, 70, 35);
   evas_object_smart_callback_add(btn, "clicked", _bottom_align_cb, efl_ui_popup);
   elm_table_pack(tbl, btn, 1, 1, 1, 1);
   evas_object_show(btn);

   btn = elm_button_add(efl_ui_popup);
   elm_object_text_set(btn, "Position Set");
   evas_object_size_hint_min_set(btn, 70, 35);
   evas_object_smart_callback_add(btn, "clicked", _position_set_cb, efl_ui_popup);
   elm_table_pack(tbl, btn, 2, 1, 1, 1);
   evas_object_show(btn);

   efl_content_set(efl_ui_popup, tbl);
}
