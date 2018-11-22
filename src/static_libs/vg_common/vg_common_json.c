#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "vg_common.h"
#include <Evas.h>

#ifdef BUILD_VG_LOADER_JSON
static void
_construct_drawable_nodes(Efl_VG *root, const LOTLayerNode *layer)
{
   for (unsigned int i = 0; i < layer->mNodeList.size; i++)
     {
        const LOTNode *node = layer->mNodeList.ptr[i];
        if (!node) continue;

        Efl_VG* shape = evas_vg_shape_add(root);
        if (!shape) continue;

        //0: Path
        const float *data = node->mPath.ptPtr;
        if (!data) continue;

        for (int i = 0; i < node->mPath.elmCount; i++)
          {
             switch (node->mPath.elmPtr[i])
               {
                case 0:
                   evas_vg_shape_append_move_to(shape, data[0], data[1]);
                   data += 2;
                   break;
                case 1:
                   evas_vg_shape_append_line_to(shape, data[0], data[1]);
                   data += 2;
                   break;
                case 2:
                   evas_vg_shape_append_cubic_to(shape, data[0], data[1], data[2], data[3], data[4], data[5]);
                   data += 6;
                   break;
                case 3:
                   evas_vg_shape_append_close(shape);
                   break;
                default:
                   ERR("No reserved path type = %d", node->mPath.elmPtr[i]);
               }
          }

        //1: Stroke
        if (node->mStroke.enable)
          {
             //Stroke Width
             evas_vg_shape_stroke_width_set(shape, node->mStroke.width);

             //Stroke Cap
             Efl_Gfx_Cap cap;
             switch (node->mStroke.cap)
               {
                case CapFlat: cap = EFL_GFX_CAP_BUTT; break;
                case CapSquare: cap = EFL_GFX_CAP_SQUARE; break;
                case CapRound: cap = EFL_GFX_CAP_ROUND; break;
                default: cap = EFL_GFX_CAP_BUTT; break;
               }
             evas_vg_shape_stroke_cap_set(shape, cap);

             //Stroke Join
             Efl_Gfx_Join join;
             switch (node->mStroke.join)
               {
                case JoinMiter: join = EFL_GFX_JOIN_MITER; break;
                case JoinBevel: join = EFL_GFX_JOIN_BEVEL; break;
                case JoinRound: join = EFL_GFX_JOIN_ROUND; break;
                default: join = EFL_GFX_JOIN_MITER; break;
               }
             evas_vg_shape_stroke_join_set(shape, join);

             //Stroke Dash
             if (node->mStroke.dashArraySize > 0)
               {
                  int size = (node->mStroke.dashArraySize / 2);
                  Efl_Gfx_Dash *dash = malloc(sizeof(Efl_Gfx_Dash) * size);
                  if (dash)
                    {
                       for (int i = 0; i <= size; i+=2)
                         {
                            dash[i].length = node->mStroke.dashArray[i];
                            dash[i].gap = node->mStroke.dashArray[i + 1];
                         }
                       evas_vg_shape_stroke_dash_set(shape, dash, size);
                       free(dash);
                    }
               }
          }

        //2: Fill Method
        switch (node->mBrushType)
          {
           case BrushSolid:
             {
                float pa = ((float)node->mColor.a) / 255;
                int r = (int)(((float) node->mColor.r) * pa);
                int g = (int)(((float) node->mColor.g) * pa);
                int b = (int)(((float) node->mColor.b) * pa);
                int a = node->mColor.a;
                if (node->mStroke.enable)
                  evas_vg_shape_stroke_color_set(shape, r, g, b, a);
                else
                  evas_vg_node_color_set(shape, r, g, b, a);
             }
             break;
           case BrushGradient:
             {
                Efl_VG* grad = NULL;

                if (node->mGradient.type == GradientLinear)
                  {
                     grad = evas_vg_gradient_linear_add(root);
                     evas_vg_gradient_linear_start_set(grad, node->mGradient.start.x, node->mGradient.start.y);
                     evas_vg_gradient_linear_end_set(grad, node->mGradient.end.x, node->mGradient.end.y);

                  }
                else if (node->mGradient.type == GradientRadial)
                  {
                     grad = evas_vg_gradient_radial_add(root);
                     evas_vg_gradient_radial_center_set(grad, node->mGradient.center.x, node->mGradient.center.y);
                     evas_vg_gradient_radial_focal_set(grad, node->mGradient.focal.x, node->mGradient.focal.y);
                     evas_vg_gradient_radial_radius_set(grad, node->mGradient.cradius);
                  }
                else
                  ERR("No reserved gradient type = %d", node->mGradient.type);

                if (grad)
                  {
                     //Gradient Stop
                     Efl_Gfx_Gradient_Stop* stops = malloc(sizeof(Efl_Gfx_Gradient_Stop) * node->mGradient.stopCount);
                     if (stops)
                       {
                          for (unsigned int i = 0; i < node->mGradient.stopCount; i++)
                            {
                               stops[i].offset = node->mGradient.stopPtr[i].pos;
                               float pa = ((float)node->mGradient.stopPtr[i].a) / 255;
                               stops[i].r = (int)(((float)node->mGradient.stopPtr[i].r) * pa);
                               stops[i].g = (int)(((float)node->mGradient.stopPtr[i].g) * pa);
                               stops[i].b = (int)(((float)node->mGradient.stopPtr[i].b) * pa);
                               stops[i].a = node->mGradient.stopPtr[i].a;
                            }
                          evas_vg_gradient_stop_set(grad, stops, node->mGradient.stopCount);
                          free(stops);
                       }
                     if (node->mStroke.enable)
                       evas_vg_shape_stroke_fill_set(shape, grad);
                     else
                       evas_vg_shape_fill_set(shape, grad);
                  }
             }
             break;
           default:
              ERR("No reserved brush type = %d", node->mBrushType);
          }

        //3: Fill Rule
        if (node->mFillRule == FillEvenOdd)
          efl_gfx_shape_fill_rule_set(shape, EFL_GFX_FILL_RULE_ODD_EVEN);
        else if (node->mFillRule == FillWinding)
          efl_gfx_shape_fill_rule_set(shape, EFL_GFX_FILL_RULE_WINDING);
     }
}

static void
_update_vg_tree(Efl_VG *root, const LOTLayerNode *layer, int idx)
{
   if (!layer || !layer->mVisible) return;

   //Note: We assume that if req_matte is true, next layer must be a matte layer.
   Eina_Bool req_matte = EINA_FALSE;

   ERR("%d: Layer Size = %d, MatteType = %d Mask = %d", idx, layer->mLayerList.size, layer->mMatte, layer->mMaskList.size);

   //Is this layer a container layer?
   for (unsigned int i = 0; i < layer->mLayerList.size; i++)
     {
        //This layer must be a mastte.
        if (req_matte)
          {
             req_matte = EINA_FALSE;
             continue;
          }

        if (layer->mLayerList.ptr[i]->mMatte != MatteNone)
          {
             ERR("%d: in layer[%d], Matte = %d, SKIP", idx, i, layer->mLayerList.ptr[i]->mMatte);
             req_matte = EINA_TRUE;
          }

        ERR("%d: in layer[%d], Matte = %d", idx, i, layer->mLayerList.ptr[i]->mMatte);
        _update_vg_tree(root, layer->mLayerList.ptr[i], idx + 1);
        ERR("%d: out layer[%d]", idx, i);
     }

   //Construct drawable nodes.
   if (layer->mNodeList.size > 0)
     {
        ERR("%d: Draw Nodes = %d", idx, layer->mNodeList.size);
        _construct_drawable_nodes(root, layer);
     }
}
#endif

Eina_Bool
vg_common_json_create_vg_node(Vg_File_Data *vfd)
{
#ifdef BUILD_VG_LOADER_JSON
   Lottie_Animation *lot_anim = (Lottie_Animation *) vfd->loader_data;
   if (!lot_anim) return EINA_FALSE;

   //Root node
   if (vfd->root) efl_unref(vfd->root);
   vfd->root = efl_add_ref(EFL_CANVAS_VG_CONTAINER_CLASS, NULL);
   Efl_VG *root = vfd->root;
   if (!root) return EINA_FALSE;

   unsigned int frame_num = (vfd->anim_data) ? vfd->anim_data->frame_num : 0;
   const LOTLayerNode *tree =
      lottie_animation_render_tree(lot_anim, frame_num, vfd->view_box.w, vfd->view_box.h);
   _update_vg_tree(root, tree, 0);
#else
   return EINA_FALSE;
#endif
   return EINA_TRUE;
}
