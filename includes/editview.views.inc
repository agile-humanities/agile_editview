<?php
// $Id: editview.views.inc,v 1.1.2.8 2011/01/23 20:48:54 frodo Exp $

/**
 * @file
 * Views functions.
 */

/**
 * @addtogroup editview
 * @{
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
 * Implementation of hook_views_plugins().
 */
function editview_views_plugins() {
  return array(
    'style' => array(
      'editview' => array(
        'title' => t('Editview'),
        'help' => t('Displays a new node form.'),
        'handler' => 'editview_plugin_style_node_add',
        'path' => drupal_get_path('module', 'editview') .'/includes',
        'theme' => 'views_view_unformatted',
        'theme path' => drupal_get_path('module', 'views') .'/theme',
        'theme file' => 'theme.inc',
        'base' => array('node'),
        'uses options' => TRUE,
        'even empty' => TRUE,
        'type' => 'normal',
      ),
    ),
    'row' => array(
      'editview' => array(
        'title' => t('Editview'),
        'help' => t('Displays a node edit form.'),
        'handler' => 'editview_plugin_row_node_edit',
        'path' => drupal_get_path('module', 'editview') .'/includes',
        'theme' => 'editview_view_row_node_edit',
        'theme path' => drupal_get_path('module', 'editview') .'/theme',
        'theme file' => 'editview.theme.inc',
        'base' => array('node'),
        'uses fields' => TRUE,
        'type' => 'normal',
      ),
    ),
  );
}

/**
 * Menu callback; allow JavaScript to update node in view.
 */
function editview_js() {
  if (isset($_POST['form_build_id']) && isset($_POST['editview_view'])) {
    // We are going to need to load nodes
    module_load_include('inc', 'node', 'node.pages');

    // Build the view so all necessary classes will be defined.
    $view = views_get_view($_POST['editview_view']);
    $view->build();

    // Fast path is to get the form out of the cache. But if minimum cache
    // lifetimes have been specified, on post of a form all cached forms
    // except the posted one get invalidated. In that case we need to rebuild
    // the form from scratch.
    $form_build_id = $_POST['form_build_id'];
    $form_state = array('storage' => NULL, 'submitted' => FALSE);
    $form = form_get_cache($form_build_id, $form_state);
    if (empty($form)) {
      $node = node_load($_POST["editview_nid"]);
      $form = drupal_retrieve_form($_POST['form_id'], $form_state, $node, $view, $_POST["editview_order"], $_POST["editview_uri"]);
      $form['#build_id'] = $form_build_id;
      drupal_prepare_form($form_id, $form, $form_state);
    }

    // Submit node.
    $args = $form['#parameters'];
    $form_id = array_shift($args);
    $form_state['post'] = $form['#post'] = $_POST;
    drupal_process_form($form_id, $form, $form_state);

    // Load fresh copy of the node.
    unset($form_state['node']);
    $args[1] = node_load($form_state['values']['nid'], NULL, TRUE);
    if (!empty($args[1])) {
      $form = drupal_rebuild_form($form_id, $form_state, $args, $form_build_id);
      $response = drupal_render($form);
    }

    // Return form to user.
    drupal_json(theme('status_messages') . (isset($response) ? $response : ''));
  }
}

/**
 * Prepare a node form for the view.
 *
 * @param $form_state
 *   The form state array.
 * @param $node
 *   The node object.
 * @param $view
 *   The view object.
 * @return
 *   A node form array.
 * @ingroup forms
 * @see editview_node_form_submit()
 * @see editview_node_form_delete_submit()
 */
function editview_node_form($form, &$form_state, $node, $view, $order, $uri) {
  // Get the node form
  $debug = "goes here";
  if (!is_object($node)) {
    return array();
  }

  $form = node_form($form, $form_state, $node);
  // Let modules alter the node form.
  $form['#method'] = 'post';
  $type = is_array($node) ? $node['type'] : $node->type;
  $form['__drupal_alter_by_ref'] = array(&$form_state);
  drupal_alter($type .'_node_form', $form);
  $form['__drupal_alter_by_ref'] = array(&$form_state);
  #drupal_alter('form', $form, $type .'_node_form');
  $var1 = 'form';
  $var3 = $type . '_node_form';
  drupal_alter($var1, $form, $var3);

  // Change save and delete functions.
  $form['buttons']['submit']['#submit'] = array('editview_node_form_submit');
  $form['buttons']['delete']['#submit'] = array('editview_node_form_delete_submit');

  // Mark the buttons for Javascript handling
  $form['buttons']['submit']['#attributes']['class'] .= " editview-button";
  $form['buttons']['delete']['#attributes']['class'] .= " editview-button";

  // We need some hidden variables to reconstruct all form options in
  // editview_js, especially in the case the form could not be recovered
  // from the form cache. Also, we add a hidden field which will be filled
  // with all ids used in this form. Javascript will get them from the
  // Drupal.settings.
  $form['editview_view'] = array('#type' => 'hidden', '#value' => $view->name);
  $form['editview_nid'] = array('#type' => 'hidden', '#value' => $node->nid);
  $form['editview_order'] = array('#type' => 'hidden', '#value' => $order);
  $form['editview_uri'] = array('#type' => 'hidden', '#value' => $uri);
  $form['editview_ids'] = array('#type' => 'hidden', '#default_value' => "");

  // Other form data. Note the editview_form_after_build callback which
  // will collect all form ids and put them into Drupal.settings.
  $form['#theme'] = 'editview_node_form';
  $form['#cache'] = TRUE;
  $form['#action'] = $uri;
  $form['#after_build'][] = 'editview_form_after_build_proxy';

  return $form;
}

/**
 * Callback, triggered whenever an editview form has been built.
 *
 * Collects all form ids and puts them into Drupal.settings.
 */
function editview_form_after_build($form, &$form_state) {
  static $ids;

  // Check if we are in a Javascript callback
  if (array_key_exists('HTTP_X_REQUESTED_WITH', $_SERVER) && $_SERVER['HTTP_X_REQUESTED_WITH'] == 'XMLHttpRequest') {
    // This is a Javascript callback, so we try to restore all ids
    if (! isset($ids)) {
      if (array_key_exists('values', $form_state) && array_key_exists('editview_ids', $form_state['values']) && $form_state['values']['editview_ids'] != "") {
        $ids = unserialize(gzuncompress(base64_decode($form_state['values']['editview_ids'])));
      }
    }
    if (isset($ids)) {
      _editview_patch_ids($form, $ids);
    }
  } else {
    // Not javascript, so we save the ids to be picked up
    $ids = array();
    // Collect all ids
    _editview_collect_ids($form, $ids);
    // Put them into Drupal.settings.editview.ids_NID, but compress them
    // first to save on bandwidth
    $key = "ids_". $form['editview_nid']['#value'];
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
  node_form_submit($form, $form_state);
  if ($uri == $form['#parameters'][5] || !isset($form['#parameters'][2]->nid)) {
    global $base_root;
    $form_state['redirect'] = $base_root . $uri;
  }
  else {
    $form_state['redirect'] = FALSE;
  }
}

/**
 * Override the node delete redirect.
 */
function editview_node_form_delete_submit($form, &$form_state) {
  $uri = request_uri();
  if ($uri == $form['#parameters'][5]) {
    node_form_delete_submit($form, $form_state);
    $form_state['redirect'] = array('node/'. $form_state['values']['nid'] .'/delete', 'destination='. urlencode($_GET['q']));
  }
  else {
    node_delete($form_state['values']['nid']);
    $form_state['redirect'] = FALSE;
  }
}

/**
 * Collect all form ids and put them into the $ids array.
 * Each key is constructed as .[/FORMFIELD1[/FORMFIELD2[/FORMFIELD3...]]]
 * Note that this is a recursively called function.
 */
function _editview_collect_ids($form, &$ids, $prefix=".") {
  foreach ($form as $key => $value) {
    if (substr($key, 0, 1) != '#' and is_array($value)) {
      _editview_collect_ids($value, $ids, $prefix ."/". $key);
    }
    elseif ($key == "#id") {
      $ids[$prefix] = $value;
    }
  }
}

/**
 * Patch a form to use the old ids as given in the $ids array.
 * Both the ids themselves as the ahah referrals are changed
 */
function _editview_patch_ids(&$form, $ids) {
  $translate = array();
  $form = _editview_change_ids($form, $ids, ".", $translate);
  $form = _editview_change_ahah($form, $translate);
}

/**
 * Patch the ids in a form.
 * Note that this function is recursively called.
 */
function _editview_change_ids($form, $ids, $prefix, &$translate) {
  foreach ($form as $form_el_name => $form_el_val) {
    if (substr($form_el_name, 0, 1) != '#' and is_array($form_el_val)) {
      $form[$form_el_name] = _editview_change_ids($form_el_val, $ids, $prefix ."/". $form_el_name, $translate);
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
 * Note that this function is recursively called.
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
 * @} End of "addtogroup editview".
 */
