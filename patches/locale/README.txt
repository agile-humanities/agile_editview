When using the locale module and Download method private, the JavaScript
Drupal.t function does not work. Symptom: you are not asked for confirmation
when deleting a node in an editview.

The patch files in this directory will solve this issue. Apply them to
your locale module.

This patch has been submitted as locale issue #250451, in comment #47: 
  http://drupal.org/node/250451#comment-1813626

If you are using a Drupal version older than 6.13, please upgrade Drupal first.
For Drupal 6.13 upto 6.20, use patch file locale-6.13.diff
For newer Drupal versions, check the above issue first.
