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
 * @param stdClass $object
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
  form_load_include($form_state, 'inc', 'editview', 'includes/editview.views');
  form_load_include($form_state, 'inc', 'user', 'user.pages');
  form_load_include($form_state, 'inc', 'field', 'field.form');
  // Get the node form.

  if ($selector == 'nid') {
    if ($object->nid != 'new') {
      $object = node_load($object->nid);
    }

    $form = node_form($form, $form_state, $object);
    $form_id = $form_state['build_info']['form_id'];
    if (module_exists('auto_nodetitle')) {
      auto_nodetitle_form_node_form_alter($form, $form_state, $form_id);
    }

    $form["node_nid"] = array(
      '#type' => 'hidden',
      '#value' => $object->nid,
    );
  }
  if ($selector == 'tid') {
    if (isset($object->tid)) {

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
  $form['actions']['submit']['#validate'] = array('editview_node_form_validate');
  //$form['actions']['delete']['#validate'] = array('editview_node_form_validate');
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
    'wrapper' => 'editview-node-form-' . $object->$selector,
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
  if (isset($object->is_new) && $object->is_new) {
    unset($form['actions']['delete']);
    unset($form['actions']['submit']['#ajax']);
  }
  return $form;
}

/**
 * Callback, triggered whenever an editview form has been built.
 *
 * Collects all form ids and puts them into Drupal.settings.
 */
function editview_form_after_build($form, &$form_state) {
  // If the item is 'new' and has just been submitted, remove the nid.
  if (isset($form_state['triggering_element']) && $form_state['triggering_element']['#type'] == 'submit' && $form_state['executed'] == FALSE) {
    $selector = $form_state['build_info']['args'][4];
    if (isset($form_state['build_info']['args'][0]->is_new) && ($form_state['build_info']['args'][0]->is_new)) {
      unset($form_state['values'][$selector]);
    }
  }

  return $form;
}

function editview_node_form_validate($form, &$form_state) {
  if ($form['#entity_type'] == 'node') {
    $nid = $form['#node']->nid;
    if ($nid != 'new') {
      $form_state['values']['changed'] = node_last_changed($nid);
    }
  }
}

/**
 * Override the node submit redirect.
 *
 * If the form was submitted by JavaScript, do not redirect. Otherwise, redirect
 * to the requesting page.
 */
function editview_node_form_submit($form, &$form_state) {
  $selector = 'nid';
  if ($form['#entity_type'] == 'taxonomy_term') {
    module_load_include('inc', 'taxonomy', 'taxonomy.admin');
    taxonomy_form_term_submit($form, $form_state);
    $selector = 'tid';
  }
  if ($form['#entity_type'] == 'node') {
    if ($form_state['node']->nid == 'new') {
      unset($form_state['node']->nid);
    }
    node_form_submit($form, $form_state);
    $selector = 'nid';
  }
  if ($form['#entity_type'] == 'user') {
    $rids = $form_state['values']['rid'];
    $selector = 'uid';
    foreach ($rids as $index => $value) {
      $form_state['values']['roles'][$index] = $value;
    }
    $form_state['values']['signature'] = NULL;
    user_profile_form_submit($form, $form_state);
  }

  $uri = request_uri();
  $path = request_path();

  if ($uri == $form_state['build_info']['args'][3] || $form_state['build_info']['args'][0]->$selector == 'new') {
    global $base_root;
    $form_state['redirect'] = $path;
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
