<?php
// $Id: editview.php,v 1.1 2009/02/17 00:39:02 agileware Exp $

/**
 * @defgroup editview Editview
 * Allows users to add and edit nodes in a view.
 */

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
 * @param $form
 *   A node form array.
 * @param $field
 *   A Views field object.
 * @return
 *   A table cell array. See theme_table() for more information.
 *
 * @ingroup hooks
 */
function hook_field_form_render(&$form, $field) {
  $element = array('class' => 'editview-field', 'valign' => 'top');
  switch ($field->field_alias) {
    default:
      $element['data'] = _editview_form_field($form, $field->field);
      break;
  }
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
