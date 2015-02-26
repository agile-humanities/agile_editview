<?php

/**
 * @file
 * Editview API documentation.
 */

/**
 * @addtogroup editview
 * @{
 */

/**
 * Get a node form field for a View field.
 *
 * @param array $form
 *   A node form array.
 * @param string $field
 *   Field name
 *
 * @return array
 *   A table cell array. See theme_table() for more information.
 */
function hook_field_form_render(&$form, $field) {
  $element = array('class' => 'editview-field', 'valign' => 'top');
  $element['data'] = _editview_form_field($form, $field->field);

  return $element;
}

/**
 * Placeholder for template file used to theme rows.
 *
 * @ingroup themeable
 */
function theme_editview_view_row_node_edit() {

}

/**
 * @} End of "addtogroup editview".
 */
