<?php
/**
 * @file
 * Editview row template.
 *
 * Available variables:
 * - $view: View object.
 * - $row: Row data object.
 * - $zebra: Even or odd.
 * - $id: Row number.
 * - $is_admin: True if the user is a site administrator.
 * - $is_front: True if this is the site front page.
 * - $logged_in: True if user is logged in.
 * - $user: Current user object.
 * - $form: Themed node form.
 *
 * @see template_preprocess_editview_view_row_node_edit()
 */
if (isset($row->nid)) {
  $selector = 'nid';
}
if (isset($row->tid)) {
  $selector = 'tid';
}
if (isset($row->uid)) {
  $selector = 'uid';
}
?>
<?php if (isset($form)): ?>
  <div id="editview-node-form-<?php print $row->$selector; ?>">
    <?php print $form; ?>
  </div>
<?php endif; ?>
