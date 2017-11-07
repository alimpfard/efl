// g++ -g `pkg-config --cflags --libs elementary-cxx efl-cxx eina-cxx eo-cxx ecore-cxx evas-cxx edje-cxx` slider_cxx_example.cc -o slider_cxx_example

#include <Elementary.hh>

using efl::eo::instantiate;

static void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   efl::ui::Win win(instantiate);
   win.text_set("Slider example");
   win.autohide_set(true);

   efl::ui::Box bx(instantiate, win);
   win.content_set(bx);

   efl::ui::Slider sl(instantiate, win);
   sl.hint_align_set(EFL_GFX_SIZE_HINT_FILL, 0.5);
   bx.pack_end(sl);

   efl::ui::Slider sl2(instantiate, win);
   sl2.text_set("Counter");
   efl::ui::Image ic(instantiate, win);
   ic.icon_set("home");
   ic.scalable_set(false, false);
   sl2.content_set(ic);

   efl::ui::Image ic2(instantiate, win);
   ic2.icon_set("folder");
   ic2.scalable_set(false, false);
   // FIXME: C++ part API needs special reference handling! This will show ERR!
   efl::eo::downcast<efl::Container>(sl2.part("elm.swallow.end"))
         .content_set(ic2);

   sl2.hint_align_set(EFL_GFX_SIZE_HINT_FILL, 0.5);
   bx.pack_end(sl2);

   efl::ui::Slider sl3(instantiate, win);
   sl3.range_value_set(1);
   sl3.hint_min_set({220, 0});
   sl3.hint_align_set(EFL_GFX_SIZE_HINT_FILL, 0.5);
   bx.pack_end(sl3);

   efl::ui::Slider sl4(instantiate, win);
   sl4.format_string_set("%1.0f units");
   sl4.range_min_max_set(0, 100);
   sl4.hint_align_set(EFL_GFX_SIZE_HINT_FILL, 0.5);
   bx.pack_end(sl4);

   efl::ui::Slider sl5(instantiate, win);

   // FIXME: C++ part API needs special reference handling! This will show ERR!
   efl::eo::downcast<efl::ui::slider::Part>(sl5.part("indicator"))
         .format_string_set("%1.2f");

   sl5.direction_set(EFL_UI_DIR_UP);
   sl5.hint_align_set(EFL_GFX_SIZE_HINT_FILL, 0.5);
   bx.pack_end(sl5);

   efl::ui::Slider sl6(instantiate, win);
   sl4.direction_set(EFL_UI_DIR_HORIZONTAL);

// FIXME
//   auto indicator_format = [] (double val) {
//                                            char *indicator = new char[32];
//                                            snprintf(indicator, 32, "%1.2f u", val);
//                                            return indicator;
//                                           };
//   auto indicator_free = [] (char *obj) {delete obj;} ;
//   sl6.indicator_format_function_set(indicator_format, indicator_free);

   sl6.hint_align_set(0.5, EFL_GFX_SIZE_HINT_FILL);
   sl6.hint_weight_set(0, EFL_GFX_SIZE_HINT_EXPAND);
   bx.pack_end(sl6);

   efl::ui::Slider sl7(instantiate, win);
   sl7.format_string_set("%1.3f units");
//   sl7.indicator_format_function_set(indicator_format, indicator_free);
   sl7.hint_align_set(EFL_GFX_SIZE_HINT_FILL, 0.5);
   bx.pack_end(sl7);

   auto changed = std::bind ( [] (efl::ui::Slider obj)
   { std::cout << "Changed to " << obj.range_value_get() << std::endl; }
         , std::placeholders::_1);

   auto delay =  std::bind ( [] (efl::ui::Slider obj)
   { std::cout << "Delay changed to " << obj.range_value_get() << std::endl; }
         , std::placeholders::_1);

   efl::eolian::event_add(sl7.changed_event, sl7, changed);
   efl::eolian::event_add(sl7.delay_changed_event, sl7, delay);
}
EFL_MAIN()
