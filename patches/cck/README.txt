You will experience problems when you have a multi-value CCK field with
editview: the Add More-button will, especially when used a second time,
do strange and unexpected things (like adding fields to an unrelated node)
and drag-and-drop will not work at all.

The patch files in this directory will solve this issue. Apply them to
your CCK module.

This patch has been submitted as CCK issue 1037460: 
  http://drupal.org/node/1037460

If you are using a CCK version older than 2.8, please upgrade CCK first.
For CCK 2.8 or 2.9, use patch file cck-2.8.diff
For newer CCK versions, check the above issue first.
