<?php
// $Id: editview.theme.inc,v 1.1.2.4 2011/01/23 20:48:54 frodo Exp $

/**
 * @file
 *
 */

/**
 * @addtogroup editview
 * @{
 */

/**
 * Prepare variables for the editview row template.
 *
 * @param $variables
 *   An array of variables from the theme system.
 */
function template_preprocess_editview_view_row_node_edit(&$variables) {
  if ($variables['row']->nid == 'new') {
    // Create dummy node for new node form.
    $type = isset($row->node_type) ? $row->node_type : $variables['view']->style_options['editview_node_type'];
    $node = array('uid' => $variables['user']->uid, 'name' => (isset($variables['user']->name) ? $variables['user']->name : ''), 'type' => $type, 'language' => '');
  }
  else {
    // Load full node for node edit form.
    $node = node_load($variables['row']->nid);
  }
  // Check user permissions.
  if (isset($node->nid) && node_access('update', $node) || !isset($node->nid) && node_access('create', $type)) {
    /* This function runs once for each row, so prevent page set-up functions
     * from being repeated.
     */
    static $run = TRUE;
    if ($run) {
      drupal_add_js('misc/jquery.form.js');
      drupal_add_js(drupal_get_path('module', 'editview') .'/theme/editview.js');
      drupal_add_js(array('editview' => array('submit_url' => url('editview_js'))), 'setting');
      drupal_add_css(drupal_get_path('module', 'editview') .'/theme/editview.css');
      module_load_include('inc', 'node', 'node.pages');
      $run = FALSE;
    }
    // Add the node form to the variables array.
    $uri = request_uri();
    $view = $variables['view'];
    $id = $variables['id'];
    if ($variables['row']->nid == 'new') {
      return;
    }
    $form = drupal_get_form('editview_node_form_'. $variables['row']->nid, $node, $view, $id, $uri);
    $form = theme('editview_node_form', array('form' => $form, 'view' => $view));
    $variables['form'] = drupal_render_children($form);
  }
}

/**
 * Display selected fields from the node form in a table.
 *
 * @param $form
 *   A node form array.
 * @return
 *   An HTML form.
 * @ingroup themeable
 */
function theme_editview_node_form(&$variables) {
  $view = $variables['view'];
  $header = array();
  $form = $variables['form'];
  $row = array();
  // Add selected fields to table as form fields.
  foreach ($view->field as $field) {
    // If auto_nodetitle module is enabled and the title field is to be overridden don't display it.
    if (module_exists('auto_nodetitle') && $field->real_field == 'title') {
      if (auto_nodetitle_get_setting($form['#node']->type) == AUTO_NODETITLE_ENABLED) {
        continue;
      }
    }

    $header[] = $field->label();
    $classes = array();
    $cell = array('data' => '');
    foreach (module_implements('field_form_render') as $module) {
      $function = $module .'_field_form_render';
      $result = $function($form, $field);
      foreach ($result as $key => $value) {
        switch ($key) {
          case 'data':
            $cell['data'] .= $value;
            break;
          case 'class':
            $classes[] = $value;
            break;
          default:
            $cell[$key] = $value;
            break;
        }
      }
    }
    $cell['class'] = implode(' ', $classes);
    $row[] = $cell;
  }
  $buttons = drupal_render($form['buttons']['submit']);
  $buttons .= drupal_render($form['buttons']['delete']) .'<div style="display: none;">'. drupal_render_children($form) .'</div>'."\n";
  $handler = $view->style_plugin;
  $active = !empty($handler->active) ? $handler->active : '';
  $order = !empty($handler->order) ? $handler->order : 'asc';
  return theme('table', $header, array($row), array('class' => 'editview-row')) . $buttons;
}

/**
 * Implementation of hook_field_form_render().
 */
function editview_field_form_render(&$form, $field) {
  $element = array('class' => 'editview-field', 'valign' => 'top');
  switch ($field->field_alias) {
    case 'og_ancestry_nid':
      $element['data'] = drupal_render($form['og_nodeapi']['visible']['og_groups']);
      break;
    case 'og_og_private':
      $element['data'] = drupal_render($form['og_nodeapi']['visible']['og_public']);
      break;
    default:
      if (isset($field->table_alias) && is_numeric($vocab_id = str_replace('term_node_', '', $field->table_alias))) {
        $element['data'] = drupal_render($form['taxonomy'][$vocab_id]);
      }
      else {
        // If the field is a cck field, send through the right identifier.
        if (isset($field->content_field)) {
          $element['data'] = _editview_form_field($form, $field->content_field['field_name']);
        }
        else {
          $element['data'] = _editview_form_field($form, $field->field);
        }
      }
      break;
  }
  return $element;
}

/**
 * Find fields in form.
 *
 * @param $elements
 *   An array of form elements.
 * @param $field
 *   A field name string.
 * @return
 *   A form element or NULL.
 */
function _editview_form_field(&$elements, $field) {
  if (isset($elements[$field])) {
    if ($elements[$field]['#type'] == 'textarea') {
      $elements[$field]['#resizable'] = FALSE;
      $elements[$field]['#rows'] = 2;
    }

    switch ($field) {
      case 'iid':
        unset($elements['iid']['#description']);
        $elements['image']['#size'] = 20;
        return drupal_render($elements['iid']) . drupal_render($elements['image']);
      default:
        return drupal_render($elements[$field]);
    }
  }
  // Special case for attachments as they don't use consistent naming.
  else if ($field == 'upload_fid') {
    $file_list = '';
    if ($elements['attachments']['wrapper']['files']) {
      $files = array();
      foreach ($elements['attachments']['wrapper']['files'] as $fid => $file) {
        if (is_numeric($fid)) {
          unset($file['description']['#description']);
          $row = array();
          $row[] = drupal_render($file['remove']);
          $row[] = drupal_render($file['list']);
          $row[] = drupal_render($file['description']);
          $rows[] = array('data' => $row);
        }
      }
      $header = array(t('Delete'), t('List'), t('Description'));
      $file_list = theme('table', $header, $rows, array('id' => 'upload-attachments'));

    }
    unset($elements['attachments']['wrapper']['new']['upload']['#description']);
    $elements['attachments']['wrapper']['new']['upload']['#size'] = 30;
    return $file_list . drupal_render($elements['attachments']['wrapper']['new']['upload']) . drupal_render($elements['attachments']['wrapper']['new']['attach']);
  }
  else {
    foreach (element_children($elements) as $key) {
      $output = _editview_form_field($elements[$key], $field);
      if (isset($output)) {
        return $output;
      }
    }
  }
}

/**
 * @} End of "addtogroup editview".
 */
