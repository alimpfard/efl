#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "vg_common.h"
#include <Evas.h>

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

   lottie_animation_prepare_frame(lot_anim, frame_num, vfd->view_box.w, vfd->view_box.h);
   int size = lottie_animation_get_node_count(lot_anim);

   //Construct vg tree
   for (int i = 0; i < size; i++)
     {
        const LOTNode *p = lottie_animation_get_node(lot_anim, i);
        if (!p) continue;

#ifdef PRINT_LOTTIE_INFO
        ERR("%d[%d] node(%p)", frame_num, i, p);
        ERR("\t Flag - None(%d), Path(%d), Paint(%d), All(%d)", p->mFlag&ChangeFlagNone, p->mFlag&ChangeFlagPath, p->mFlag&ChangeFlagPaint, p->mFlag&ChangeFlagAll);
        ERR("\t BrushType - mType(%d)", p->mBrushType);
        ERR("\t FillRule - mFillRule(%d)", p->mFillRule);
        ERR("\t PathData - ptPtr(%p), ptCount(%d), elmPtr(%p), elmCount(%d)", p->mPath.ptPtr, p->mPath.ptCount, p->mPath.elmPtr, p->mPath.elmCount);
        ERR("\t Color - r(%d) g(%d) b(%d) a(%d)", p->mColor.r, p->mColor.g, p->mColor.b, p->mColor.a);
        ERR("\t Stroke - enable(%d), width(%d), cap(%d), join(%d), meterLimit(%d), dashArray(%p), dashArraySize(%d)", p->mStroke.enable, p->mStroke.width, p->mStroke.cap, p->mStroke.join, p->mStroke.meterLimit, p->mStroke.dashArray, p->mStroke.dashArraySize);
        ERR("\t Gradient - type(%d), stopCnt(%d) start(%f %f), end(%f %f), center(%f %f), focal(%f %f), center_radius(%f) focal_radius(%f)", p->mGradient.type, p->mGradient.stopCount, p->mGradient.start.x, p->mGradient.start.y, p->mGradient.end.x, p->mGradient.end.y, p->mGradient.center.x, p->mGradient.center.y, p->mGradient.focal.x, p->mGradient.focal.y, p->mGradient.cradius, p->mGradient.fradius);
        ERR("\n");
#endif

        Efl_VG* shape = evas_vg_shape_add(root);
        if (!shape) continue;

        //0: Path
        const float *data = p->mPath.ptPtr;
        if (!data) continue;

        for (int i = 0; i < p->mPath.elmCount; i++)
          {
             switch (p->mPath.elmPtr[i])
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
                   ERR("No reserved path type = %d", p->mPath.elmPtr[i]);
               }
          }

        //1: Stroke
        if (p->mStroke.enable)
          {
             //Stroke Width
             evas_vg_shape_stroke_width_set(shape, p->mStroke.width);

             //Stroke Cap
             Efl_Gfx_Cap cap;
             switch (p->mStroke.cap)
               {
                case CapFlat: cap = EFL_GFX_CAP_BUTT; break;
                case CapSquare: cap = EFL_GFX_CAP_SQUARE; break;
                case CapRound: cap = EFL_GFX_CAP_ROUND; break;
                default: cap = EFL_GFX_CAP_BUTT; break;
               }
             evas_vg_shape_stroke_cap_set(shape, cap);

             //Stroke Join
             Efl_Gfx_Join join;
             switch (p->mStroke.join)
               {
                case JoinMiter: join = EFL_GFX_JOIN_MITER; break;
                case JoinBevel: join = EFL_GFX_JOIN_BEVEL; break;
                case JoinRound: join = EFL_GFX_JOIN_ROUND; break;
                default: join = EFL_GFX_JOIN_MITER; break;
               }
             evas_vg_shape_stroke_join_set(shape, join);
          }

        //2: Fill Method
        switch (p->mBrushType)
          {
           case BrushSolid:
             {
                float pa = ((float)p->mColor.a) / 255;
                int r = (int)(((float) p->mColor.r) * pa);
                int g = (int)(((float) p->mColor.g) * pa);
                int b = (int)(((float) p->mColor.b) * pa);
                int a = p->mColor.a;
                if (p->mStroke.enable)
                  evas_vg_shape_stroke_color_set(shape, r, g, b, a);
                else
                  evas_vg_node_color_set(shape, r, g, b, a);
             }
             break;
           case BrushGradient:
             {
                if (p->mGradient.type == GradientLinear)
                  {
                     Efl_VG* grad = evas_vg_gradient_linear_add(root);
                     evas_vg_gradient_linear_start_set(grad, p->mGradient.start.x, p->mGradient.start.y);
                     evas_vg_gradient_linear_end_set(grad, p->mGradient.end.x, p->mGradient.end.y);

                     //Gradient Stop
                     Efl_Gfx_Gradient_Stop* stops = malloc(sizeof(Efl_Gfx_Gradient_Stop) * p->mGradient.stopCount);
                     if (stops)
                       {
                          for (unsigned int i = 0; i < p->mGradient.stopCount; i++)
                            {
                               stops[i].offset = p->mGradient.stopPtr[i].pos;
                               float pa = ((float)p->mGradient.stopPtr[i].a) / 255;
                               stops[i].r = (int)(((float)p->mGradient.stopPtr[i].r) * pa);
                               stops[i].g = (int)(((float)p->mGradient.stopPtr[i].g) * pa);
                               stops[i].b = (int)(((float)p->mGradient.stopPtr[i].b) * pa);
                               stops[i].a = p->mGradient.stopPtr[i].a;
                            }
                          evas_vg_gradient_stop_set(grad, stops, p->mGradient.stopCount);
                          free(stops);
                       }
                     if (p->mStroke.enable)
                       evas_vg_shape_stroke_fill_set(shape, grad);
                     else
                       evas_vg_shape_fill_set(shape, grad);
                  }
                else if (p->mGradient.type == GradientRadial)
                  {
                     //TODO:
                     ERR("Radial Gradient Type isn't implemented yet.");
                  }
                else
                  ERR("No reserved gradient type = %d", p->mGradient.type);
             }
             break;
           default:
              ERR("No reserved brush type = %d", p->mBrushType);
          }

        //3: Fill Rule
        if (p->mFillRule == FillEvenOdd)
          efl_gfx_shape_fill_rule_set(shape, EFL_GFX_FILL_RULE_ODD_EVEN);
        else if (p->mFillRule == FillWinding)
          efl_gfx_shape_fill_rule_set(shape, EFL_GFX_FILL_RULE_WINDING);
     }
#else
   return EINA_FALSE;
#endif

   return EINA_TRUE;
}
