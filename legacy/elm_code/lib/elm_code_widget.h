#ifndef ELM_CODE_WIDGET_H_
# define ELM_CODE_WIDGET_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file
 * @brief These routines are used for rendering instances of Elm Code.
 */

/**
 * @brief UI Loading functions.
 * @defgroup Init  Creating a widget to render an Elm Code backend
 *
 * @{
 *
 * Functions for UI loading.
 *
 */

EAPI Evas_Object *elm_code_widget_add(Evas_Object *parent, Elm_Code *code);

EAPI void elm_code_widget_font_size_set(Evas_Object *widget, int size);

EAPI void elm_code_widget_fill(Evas_Object *o, Elm_Code *code);

EAPI void elm_code_widget_fill_line_tokens(Evas_Textgrid_Cell *cells, int count, Elm_Code_Line *line);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ELM_CODE_WIDGET_H_ */
