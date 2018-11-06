#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#define MY_CLASS EFL_UI_TREE_FACTORY_CLASS
#define MY_CLASS_NAME "Efl.Ui.Tree_Factory"

typedef struct _Efl_Ui_Tree_Factory_Data
{
    Eina_Hash *connects;
    Eina_Hash *factory_connects;
    Eina_Stringshare *klass;
    Eina_Stringshare *group;
    Eina_Stringshare *def_style;
    Eina_Stringshare *exp_style;
} Efl_Ui_Tree_Factory_Data;

static Eina_Bool
_model_connect(const Eina_Hash *hash EINA_UNUSED, const void *key, void *data, void *fdata)
{
   Eo *layout = fdata;
   Eina_Stringshare *name = key;
   Eina_Stringshare *property = data;

   efl_ui_model_connect(layout, name, property);
   return EINA_TRUE;
}

static Eina_Bool
_factory_model_connect(const Eina_Hash *hash EINA_UNUSED, const void *key, void *data, void *fdata)
{
   Eo *layout = fdata;
   Eina_Stringshare *name = key;
   Efl_Ui_Factory *factory = data;

   efl_ui_factory_model_connect(layout, name, factory);
   return EINA_TRUE;
}

EOLIAN static Eo *
_efl_ui_tree_factory_efl_object_constructor(Eo *obj, Efl_Ui_Tree_Factory_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   pd->klass = NULL;
   pd->group = NULL;
   pd->def_style = NULL;
   pd->exp_style = NULL;
   pd->connects = eina_hash_stringshared_new(EINA_FREE_CB(eina_stringshare_del));
   pd->factory_connects = eina_hash_stringshared_new(EINA_FREE_CB(efl_del));

   return obj;
}

EOLIAN static void
_efl_ui_tree_factory_efl_object_destructor(Eo *obj, Efl_Ui_Tree_Factory_Data *pd)
{
   eina_stringshare_del(pd->klass);
   eina_stringshare_del(pd->group);
   eina_stringshare_del(pd->def_style);
   eina_stringshare_del(pd->exp_style);

   eina_hash_free(pd->connects);
   eina_hash_free(pd->factory_connects);

   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Gfx_Entity *
_efl_ui_tree_factory_efl_ui_factory_create(Eo *obj EINA_UNUSED, Efl_Ui_Tree_Factory_Data *pd
                                                        , Efl_Model *model, Efl_Gfx_Entity *parent)
{
   Efl_Gfx_Entity *layout;
   Efl_Ui_Tree_Factory_Item_Event evt;
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   Eina_Stringshare *style = pd->def_style;
   evt.model = model;
   evt.expandable = EINA_FALSE;
   efl_event_callback_call(obj, EFL_UI_TREE_FACTORY_EVENT_ITEM_CREATE, &evt);

   if (pd->exp_style && (evt.expandable || efl_model_children_count_get(model)))
     style = pd->exp_style;

   layout = efl_add(EFL_UI_LAYOUT_OBJECT_CLASS, parent,
                    efl_ui_view_model_set(efl_added, model),
                    efl_ui_layout_object_theme_set(efl_added, pd->klass, pd->group, style));

   eina_hash_foreach(pd->connects, _model_connect, layout);
   eina_hash_foreach(pd->factory_connects, _factory_model_connect, layout);

   evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

   return layout;
}

EOLIAN static void
_efl_ui_tree_factory_efl_ui_factory_release(Eo *obj EINA_UNUSED, Efl_Ui_Tree_Factory_Data *pd EINA_UNUSED, Efl_Gfx_Entity *layout)
{
   efl_ui_view_model_set(layout, NULL);
   efl_parent_set(layout, NULL);
}

EOLIAN static void
_efl_ui_tree_factory_efl_ui_factory_model_connect(Eo *obj EINA_UNUSED, Efl_Ui_Tree_Factory_Data *pd
                                                                        , const char *name, Efl_Ui_Factory *factory)
{
   Eina_Stringshare *ss_name;
   ss_name = eina_stringshare_add(name);

   if (factory == NULL)
     {
        eina_hash_del(pd->factory_connects, ss_name, NULL);
        return;
     }

   eina_stringshare_del(eina_hash_set(pd->factory_connects, ss_name, factory));
}

EOLIAN static void
_efl_ui_tree_factory_efl_ui_model_connect_connect(Eo *obj EINA_UNUSED, Efl_Ui_Tree_Factory_Data *pd
                                                                        , const char *name, const char *property)
{
   Eina_Stringshare *ss_name, *ss_prop;
   ss_name = eina_stringshare_add(name);

   if (property == NULL)
     {
        eina_hash_del(pd->connects, ss_name, NULL);
        return;
     }

   ss_prop = eina_stringshare_add(property);
   eina_stringshare_del(eina_hash_set(pd->connects, ss_name, ss_prop));
}


EOLIAN static void
_efl_ui_tree_factory_theme_config(Eo *obj EINA_UNUSED, Efl_Ui_Tree_Factory_Data *pd
                                        , const char *klass, const char *group, const char *style)
{
   eina_stringshare_replace(&pd->klass, klass);
   eina_stringshare_replace(&pd->group, group);
   eina_stringshare_replace(&pd->def_style, style);
}

EOLIAN static void
_efl_ui_tree_factory_expandable_style_set(Eo *obj EINA_UNUSED, Efl_Ui_Tree_Factory_Data *pd, const char *style)
{
   eina_stringshare_replace(&pd->exp_style, style);
}

#include "efl_ui_tree_factory.eo.c"
