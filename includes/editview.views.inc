<?php

/**
 * @file
 * Views functions.
 */
/**
 * New node form position: above view results.
 */
define('EDITVIEW_NEW_ABOVE', 0);

/**
 * New node form position: below view results.
 */
define('EDITVIEW_NEW_BELOW', 1);

/**
 * New node form position: invisible
 */
define('EDITVIEW_NEW_NONE', 2);

/**
 * Implements hook_views_plugins().
 */
function editview_views_plugins() {
  return array(
    'style' => array(
      'editview' => array(
        'title' => t('Editview'),
        'help' => t('Displays a new node form.'),
        'handler' => 'editview_plugin_style_node_add',
        'path' => drupal_get_path('module', 'editview') . '/includes',
        'theme' => 'views_view_unformatted',
        'theme path' => drupal_get_path('module', 'views') . '/theme',
        'theme file' => 'theme.inc',
        'uses options' => TRUE,
        'even empty' => TRUE,
        'type' => 'normal',
      ),
    ),
    'row' => array(
      'editview' => array(
        'title' => t('Editview'),
        'help' => t('Displays a node edit form.'),
        'handler' => 'Editview_plugin_row_node_edit',
        'path' => drupal_get_path('module', 'editview') . '/includes',
        'theme' => 'editview_view_row_node_edit',
        'theme path' => drupal_get_path('module', 'editview') . '/theme',
        'theme file' => 'editview.theme.inc',
        'uses fields' => TRUE,
        'type' => 'normal',
      ),
    ),
  );
}

/**
 * Prepare a node form for the view.
 *
 * @param array $form
 *   The form  array.
 * @param array $form_state
 *   The form state array.
 * @param stClass $object
 *   The object.
 * @param View $view
 *   The view object.
 *
 * @return array
 *   A node form array.
 */
function editview_node_form($form, &$form_state, $object, $view, $order, $uri, $selector) {
  module_load_include('inc', 'node', 'node.pages');
  module_load_include('inc', 'taxonomy', 'taxonomy.admin');
  module_load_include('inc', 'user', 'user.pages');
  module_load_include('inc', 'editview', 'includes/editview.views');
  form_load_include($form_state, 'inc', 'editview', 'includes/editview.views');
  // Get the node form.
  $identifier = NULL;
  if ($selector == 'nid') {
    $identifier = $object->nid;
    if ($object->nid != 'new') {
      $object = node_load($object->nid);
    }

    $form = node_form($form, $form_state, $object);
    $form["node_nid"] = array(
      '#type' => 'hidden',
      '#value' => $object->nid,
    );
  }
  if ($selector == 'tid') {
    if (isset($object->tid)) {
      $identifier = $object->tid;
    }
    $form = taxonomy_form_term($form, $form_state, $object);
  }
  if ($selector == 'uid') {
    $form = user_profile_form($form, $form_state, $object);
    $uid = $form_state['user']->uid;
    $form['rid'] = editview_role_element($uid);
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#weight' => 15,
    );
  }
  // Let modules alter the node form.
  $form['#method'] = 'post';
  if (isset($object->type)) {
    $type = $object->type;
  }

  // Change save and delete functions.
  $form['actions']['submit']['#submit'] = array('editview_node_form_submit');
  $form['actions']['delete']['#submit'] = array('editview_node_form_delete_submit');

  // Mark the buttons for Javascript handling.
  if (!isset($form['actions']['submit']['#attributes']['class'])) {
    $form['actions']['submit']['#attributes']['class'] = array();
  }
  if (!isset($form['actions']['delete']['#attributes']['class'])) {
    $form['actions']['delete']['#attributes']['class'] = array();
  }
  $form['actions']['submit']['#attributes']['class'][] = "editview-button";
  $form['actions']['delete']['#attributes']['class'][] = "editview-button";

  $ajax = array(
    'wrapper' => 'editview-node-form-' . $form[$selector]['#value'],
    'callback' => 'editview_js',
    'effect' => 'fade',
    'speed' => 2000,
  );
  $form['actions']['submit']['#ajax'] = $ajax;
  $form['actions']['delete']['#ajax'] = $ajax;

  // We need some hidden variables to reconstruct all form options in
  // editview_js, especially in the case the form could not be recovered
  // from the form cache. Also, we add a hidden field which will be filled
  // with all ids used in this form. Javascript will get them from the
  // Drupal.settings.
  $form['editview_view'] = array(
    '#type' => 'hidden',
    '#value' => $view->name,
  );
  $form['editview_nid'] = array(
    '#type' => 'hidden',
    '#value' => $object->$selector,
  );
  $form['editview_order'] = array(
    '#type' => 'hidden',
    '#value' => $order,
  );
  $form['editview_uri'] = array(
    '#type' => 'hidden',
    '#value' => $uri,
  );
  $form['editview_ids'] = array(
    '#type' => 'hidden',
    '#default_value' => "",
  );

  // Other form data. Note the editview_form_after_build callback which
  // will collect all form ids and put them into Drupal.settings.
  $form['#theme'] = 'editview_custom_node_form';
  $form['#cache'] = TRUE;
  $form['#action'] = $uri;
  $form['#after_build'][] = 'editview_form_after_build_proxy';

  // Add the view into the form so it can be accessed by the theme function.
  $form['#storage']['view'] = $form_state['build_info']['args'][1];
  // And add the object.
  $form['#storage']['entity'] = $object;

  // Add files to find node form.
  if (empty($form_state['build_info']['files']) || !in_array('modules/node/node.pages.inc', $form_state['build_info']['files'])) {
    form_load_include($form_state, 'inc', 'node', 'node.pages');
  }
  if ($identifier == 'new') {
    unset($form['actions']['delete']);
  }
  return $form;
}

/**
 * Callback, triggered whenever an editview form has been built.
 *
 * Collects all form ids and puts them into Drupal.settings.
 */
function editview_form_after_build($form, &$form_state) {
  static $ids;

  // Check if we are in a Javascript callback.
  if (array_key_exists('HTTP_X_REQUESTED_WITH', $_SERVER) && $_SERVER['HTTP_X_REQUESTED_WITH'] == 'XMLHttpRequest') {
    // This is a Javascript callback, so we try to restore all ids.
    if (!isset($ids)) {
      if (array_key_exists('values', $form_state) && array_key_exists('editview_ids', $form_state['values']) && $form_state['values']['editview_ids'] != "") {
        $ids = unserialize(gzuncompress(base64_decode($form_state['values']['editview_ids'])));
      }
    }
    if (isset($ids)) {
      _editview_patch_ids($form, $ids);
    }
  }
  else {
    // Not javascript, so we save the ids to be picked up.
    $ids = array();
    // Collect all ids.
    _editview_collect_ids($form, $ids);
    // Put them into Drupal.settings.editview.ids_NID, but compress them
    // first to save on bandwidth.
    $key = "ids_" . $form['editview_nid']['#value'];
    drupal_add_js(array('editview' => array($key => base64_encode(gzcompress(serialize($ids))))), "setting");
  }
  return $form;
}

/**
 * Override the node submit redirect.
 *
 * If the form was submitted by JavaScript, do not redirect. Otherwise, redirect
 * to the requesting page.
 */
function editview_node_form_submit($form, &$form_state) {
  $uri = request_uri();
  if ($form['#entity_type'] == 'taxonomy_term') {
    module_load_include('inc', 'taxonomy', 'taxonomy.admin');
    taxonomy_form_term_submit($form, $form_state);
  }
  if ($form['#entity_type'] == 'node') {
    node_form_submit($form, $form_state);
  }
  if ($form['#entity_type'] == 'user') {
    $rids = $form_state['values']['rid'];
    $roles = $form_state['values']['roles'];
    foreach ($rids as $index => $value) {
      $form_state['values']['roles'][$index] = $rids[$value];
    }
    user_profile_form_submit($form, $form_state);
  }
  if ($uri == $form_state['build_info']['args'][3] || !isset($form_state['build_info']['args'][0]->nid)) {
    global $base_root;
    $form_state['redirect'] = url($uri);
  }
  else {
    $form_state['redirect'] = FALSE;
  }
}

/**
 * Override the node delete redirect.
 */
function editview_node_form_delete_submit($form, &$form_state) {

  if (isset($form_state['values']['nid'])) {
    node_delete($form_state['values']['nid']);
  }
  if (isset($form_state['values']['tid'])) {
    taxonomy_term_delete($form_state['values']['tid']);
  }
  if (isset($form_state['values']['rid'])) {
    user_delete($form_state['user']->uid);
  }
  $form_state['redirect'] = FALSE;
}

/**
 * Collect all form ids and put them into the $ids array.
 */
function _editview_collect_ids($form, &$ids, $prefix = ".") {

  foreach ($form as $key => $value) {
    if (substr($key, 0, 1) != '#' and is_array($value)) {
      _editview_collect_ids($value, $ids, $prefix . "/" . $key);
    }
    elseif ($key == "#id") {
      $ids[$prefix] = $value;
    }
  }
}

/**
 * Patch a form to use the old ids as given in the $ids array.
 */
function _editview_patch_ids(&$form, $ids) {
  $translate = array();
  $form = _editview_change_ids($form, $ids, ".", $translate);
  $form = _editview_change_ahah($form, $translate);
}

/**
 * Patch the ids in a form.
 */
function _editview_change_ids($form, $ids, $prefix, &$translate) {
  foreach ($form as $form_el_name => $form_el_val) {
    if (substr($form_el_name, 0, 1) != '#' and is_array($form_el_val)) {
      $form[$form_el_name] = _editview_change_ids($form_el_val, $ids, $prefix . "/" . $form_el_name, $translate);
    }
    elseif ($form_el_name == "#id") {
      if (array_key_exists($prefix, $ids)) {
        $translate[$form_el_val] = $ids[$prefix];
        $form[$form_el_name] = $ids[$prefix];
      }
    }
  }
  return $form;
}

/**
 * Patch the ahah wrapper references in a form based on a translation table.
 */
function _editview_change_ahah($form, $translate) {
  foreach ($form as $form_el_name => $form_el_val) {
    if (substr($form_el_name, 0, 1) != '#' and is_array($form_el_val)) {
      $form[$form_el_name] = _editview_change_ahah($form_el_val, $translate);
    }
    elseif (($form_el_name == '#ahah') and is_array($form_el_val) and array_key_exists('wrapper', $form_el_val)) {
      if (array_key_exists($form_el_val['wrapper'], $translate)) {
        $form[$form_el_name]['wrapper'] .= $translate[$form_el_val['wrapper']];
      }
    }
  }
  return $form;
}

/**
 * Adds role selection.
 *
 * @param string $uid
 *   uid definign user
 *
 * @return array
 *   Drupal form element
 */
function editview_role_element($uid) {
  $roles = user_roles(TRUE);
  $account = user_load($uid);
  $current_roles = array_keys($account->roles);

  $form['roles'] = array(
    '#title' => t("User's roles"),
    '#type' => 'checkboxes',
    '#multiple' => TRUE,
    '#options' => $roles,
    '#default_value' => $current_roles,
  );
  return $form['roles'];
}
