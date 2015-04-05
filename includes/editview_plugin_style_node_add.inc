<?php

/**
 * @file
 * Style plug-in file.
 *
 * This is kept in a separate file and included only when needed to prevent
 * missing class errors.
 */

/**
 * Style plugin to add new node form.
 *
 * Fields which are added to the view will be shown.
 */
class editview_plugin_style_node_add extends views_plugin_style {

  /**
   * Implements init().
   */
  function init(&$view, &$display, $options = NULL) {
    $display->handler->set_option('row_plugin', 'editview');
    parent::init($view, $display, $options);
  }

  /**
   * Implements option_definition().
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['editview_node_type'] = array();
    $options['editview_node_position'] = array('default' => EDITVIEW_NEW_ABOVE);
    return $options;
  }

  /**
   * Implements options_form().
   */
  function options_form(&$form, &$form_state) {
    if ($this->view->base_table == 'node') {
      $types = node_type_get_types();
      foreach ($types as $type) {
        $options[$type->name] = $type->name;
      }
      $form['editview_node_type'] = array(
        '#type' => 'select',
        '#title' => t('Node type'),
        '#default_value' => $this->options['editview_node_type'],
        '#options' => $options,
        '#description' => t('Select a node type for the new node form.'),
        '#required' => TRUE,
      );
      $form['editview_node_position'] = array(
        '#type' => 'radios',
        '#title' => t('New node position'),
        '#default_value' => $this->options['editview_node_position'],
        '#options' => array(t('Above'), t('Below'), t('None')),
        '#description' => t('Specify whether a new node form should be above or below existing records or not displayed at all.'),
      );
    }
    if ($this->view->base_table == 'taxonomy_term_data') {
      $types = taxonomy_get_vocabularies();
      foreach ($types as $type) {
        $options[$type->vid] = $type->name;
      }
      $form['editview_vocab_vid'] = array(
        '#type' => 'select',
        '#title' => t('Term type'),
        '#default_value' => $this->options['editview_node_type'],
        '#options' => $options,
        '#description' => t('Select a vocabulary type for the new term form.'),
        '#required' => TRUE,
      );
    }
    parent::options_form($form, $form_state);
  }

  /**
   * Implementation of uses_row_plugin().
   *
   * We want to hard-code the row plug-in, so we return TRUE here instead of
   * specifying that we use a row plug-in in our style definition.
   */
  function uses_row_plugin() {
    return $_GET['q'] != 'system/ajax';
  }

  /**
   * Implements render().
   *
   * Render the view with a new node form.
   */
  function render() {
    $selector = $this->view->base_field;
    // Create empty object.
    $object = new stdClass();
    // Add empty view fields (needed for grouped views).
    foreach ($this->view->field as $key => $field) {
      foreach ($field->aliases as $alias) {
        $object->$alias = NULL;
      }
      $alias = $field->field_alias;
      $object->$alias = ''; // NULL;

      $key = "field_" . $key;
      $object->$key = array();
    }
    $object->$selector = 'new';
    if (!isset($this->view->filter['type']) && $selector == 'nid' && empty($this->options['editview_node_type'])) {
      return 'Please select a type for the new node under Editview Settings.';
    }
    array_unshift($this->view->result, $object);
    return parent::render();
  }

}

