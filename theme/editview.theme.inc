<?php

/**
 * @file
 * Editview theme functions.
 */

/**
 * Prepare variables for the editview row template.
 *
 * @param array $variables
 *   An array of variables from the theme system.
 */
function template_preprocess_editview_view_row_node_edit(&$variables) {
  global $user;
  $selector = $variables['view']->base_field;
  $valid = FALSE;
  if ($selector == 'nid') {
    // Set a default node type to 'article' to avoid ajax errors when creating a new node view.
    if (!isset($variables['view']->style_options['editview_node_type'])) {
      $variables['view']->style_options['editview_node_type'] = 'article';
    }
    $type = empty($variables['row']->node_type) ? $variables['view']->style_options['editview_node_type'] : $variables['row']->node_type;
    $type = strtolower($type);
    if (!$type) {
      $type = 'article';
    }
    $nid = $variables['row']->nid;
    $object = editview_build_node($nid, $type);
    if (isset($object->nid) && node_access('update', $object) || !isset($object->nid) && node_access('create', $type)) {
      $valid = TRUE;
    }
  }
  if ($selector == 'tid') {
    $tid = $variables['row']->tid;
    if (!isset($variables['view']->style_options['editview_vocab_vid'])) {
      $variables['view']->style_options['editview_vocab_vid'] = 1;
    }
    $name = $variables['row']->taxonomy_term_data_name;
    $vid = $tid == 'new' ? $variables['view']->style_options['editview_vocab_vid'] : $variables['row']->taxonomy_term_data_vid;
    $object = editview_build_term($name, $vid, $tid);
    $valid = TRUE;
  }

  if ($selector == 'uid') {
    $uid = $variables['row']->uid;
    if ($uid == 'new') {
      return;
    }
    $object = user_load($uid);
    $valid = TRUE;
  }
  // Check user permissions.
  if ($valid) {
    /* This function runs once for each row, so prevent page set-up functions
     * from being repeated.
     */
    static $run = TRUE;
    if ($run) {
      drupal_add_js('misc/jquery.form.js');
      drupal_add_css(drupal_get_path('module', 'editview') . '/theme/editview.css');
      module_load_include('inc', 'node', 'node.pages');
      $run = FALSE;
    }
    // Add the node form to the variables array.
    $uri = request_uri();
    $view = $variables['view'];
    $id = $variables['id'];
    $form = drupal_get_form('editview_node_form_' . $variables['row']->$selector, $object, $view, $id, $uri, $selector);
    if (module_exists('auto_nodetitle')) {
      if (auto_nodetitle_get_setting($form['#node']->type) == AUTO_NODETITLE_ENABLED) {
        unset($form['title']);
      }
    }
    $form = drupal_render($form);
    $variables['form'] = $form;
  }
}

/**
 * Display selected fields from the node form in a table.
 *
 * @param array $variables
 *   Inputs
 *
 * @return array
 *   An HTML form.
 */
function theme_editview_custom_node_form(&$variables) {
  $form = $variables['form'];
  $view = $form['#storage']['view'];
  $header = array();
  $row = array();
  // Add selected fields to table as form fields.
  foreach ($view->field as $field) {
    // Don't display title field if overidden by autonodetitle.
    if (module_exists('auto_nodetitle') && $field->real_field == 'title') {
      if (auto_nodetitle_get_setting($form['#node']->type) == AUTO_NODETITLE_ENABLED) {
        continue;
      }
    }
    $label = $field->label();
    if (!$label) {
      $label = t('Title');
    }
    $header[] = $label;

    $classes = array();
    $cell = array('data' => '');
    foreach (module_implements('field_form_render') as $module) {
      $function = $module . '_field_form_render';
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
    $cell['class'] = array(implode(' ', $classes));
    $row[] = $cell;
  }
  $buttons = drupal_render($form['actions']['submit']);
  $buttons .= drupal_render($form['actions']['delete']) . '<div style="display: none;">' . drupal_render_children($form) . '</div>' . "\n";
  $handler = $view->style_plugin;
  $active = !empty($handler->active) ? $handler->active : '';
  $order = !empty($handler->order) ? $handler->order : 'asc';
  return theme('table', array(
        'header' => $header,
        'rows' => array($row),
        'attributes' => array('class' => array('editview-row')))) . $buttons;
}

/**
 * Implements hook_field_form_render().
 */
function editview_field_form_render(&$form, $field) {

  $element = array('class' => 'editview-field', 'valign' => 'top');
  $debug_field = $field->field_alias;
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
          if ($field->field == "body") {
            $form[$field->field][LANGUAGE_NONE]['0']['format']['#access'] = FALSE;
          }
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
 * @param array $elements
 *   An array of form elements.
 * @param string $field
 *   A field name string.
 *
 * @return mixed
 *   A form element or NULL.
 */
function _editview_form_field(&$elements, $field) {

  if (isset($elements[$field])) {
    if (isset($elements[$field]['#type']) && $elements[$field]['#type'] == 'textarea') {
      $elements[$field]['#resizable'] = FALSE;
      $elements[$field]['#rows'] = 2;
    }

    switch ($field) {
      case 'iid':
        unset($elements['iid']['#description']);
        $elements['image']['#size'] = 20;
        return drupal_render($elements['iid']) . drupal_render($elements['image']);

      default:
        $return = drupal_render($elements[$field]);
        if ($field == 'body' || $field == 'description') {
          if ($return) {
            $dom = new DOMDocument("1.0", "ISO-8859-15");
            $dom->loadHTML($return);
            $fieldsets = $dom->getElementsByTagName('fieldset');
            foreach ($fieldsets as $fieldset) {
              $fieldset->parentNode->removeChild($fieldset);
            }
            $return = $dom->saveHTML();
          }
        }
        if (!$return && isset($elements[$field]['#value'])) {
          $return = $elements[$field]['#value'];
        }
        return $return;
    }
  }
  // Special case for attachments as they don't use consistent naming.
  elseif ($field == 'upload_fid') {
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
    $children = element_children($elements);
    foreach ($children as $key) {
      $output = _editview_form_field($elements[$key], $field);
      if (isset($output)) {
        return $output;
      }
    }
  }
  if (isset($elements['#storage']['entity'])) {
    $object = $elements['#storage']['entity'];
    if (isset($object->$field)) {
      return $object->$field;
    }
  }
}

/**
 * Builds node.
 *
 * @param int $nid
 *   Node identifier
 * @param string $type
 *   Node type
 *
 * @return Node
 *   New or existing node
 */
function editview_build_node($nid, $type) {
  global $user;
  $name = isset($user->name) ? $user->name : 'anonymous';
  if ($nid == 'new') {
    // Create dummy node for new node form.
    $values = array(
      'nid' => $nid,
      'type' => $type,
      'uid' => $user->uid,
      'name' => $name,
      'status' => 1,
      'comment' => 1,
      'bundle' => $type,
      'promote' => 0,
      'created' => time(),
    );
    $node = entity_create('node', $values);
  }
  else {
    // Load full node for node edit form.
    $node = node_load($nid);
  }
  return $node;
}

/**
 * Builds term.
 *
 * @param string $name
 *   Term name
 * @param int $vid
 *   Vocabulary identifier
 * @param int $tid
 *   Term identifier
 *
 * @return term
 *   New or existing term
 */
function editview_build_term($name, $vid, $tid) {
  if ($tid == 'new') {
    $term = array();
    $term['name'] = $name;
    $term['vid'] = $vid;
    $term = entity_create('taxonomy_term', $term);
  }
  else {
    $term = taxonomy_term_load($tid);
  }
  return $term;
}

