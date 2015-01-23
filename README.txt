$Id: README.txt,v 1.5.2.5 2011/01/26 20:51:20 frodo Exp $

Editview is a Views plug-in that allows you to create views in which nodes are
editable and new nodes can be created. Editview works with or without JavaScript
enabled.


Installation
------------

  1. Ensure that Views 2 is installed.

  2. Unpack the editview tarball in your sites/all/modules directory.

  3. Enable Editview at Administer >> Site building >> Modules.

  4. Check the patches directory for patches to other modules to make them
     work with editview and apply them as needed.


Usage
-----

Editable node views are created just like any other type of view, with a few
caveats:
   1. At this moment, editview can only be used on node views (View Type must
      be set to Node when creating the view).
   2. If you want to be able to add new nodes in the view, set the style to
      Editview in Basic settings.
        * You must now indicate what type of node will be added through the
          new node form and where it will appear.
        * Make sure all required fields are included in the view, or new nodes
          will fail validation.
        * It may be a good idea to provide a Sort criteria of Node: Post date
          so that when adding new nodes, they appear where users expect them to
          appear.
   3. If you do not want to add new nodes, select any other style that allows
      you to set a row style and set the row style to Editview.
      Alternatively, you can now select not to display the new node form when
      using the Edtiview style.


Known issues
------------

If you are not asked for confirmation when deleting an issue in an editview
view and you are using the locale module and download method 'private', you
might have come across a Drupal core bug:
  http://drupal.org/node/250451
A patch can be found in patches/locale

If you have trouble with re-ordering multi-value CCK fields or adding
additional values to them, you need to patch your CCK module:
  http://drupal.org/node/1037460
A patch can be found in patches/cck


Cookbook
--------

A few example views follow. If you find another useful way to use an Editview,
write a quick description and it will be added it here.

Make a view that allows you to edit and create pages on your site
-----------------------------------------------------------------

This Editview will show an editable table of page nodes. You can edit their
title and body fields, save your changes, delete pages (with confirmation), and
create new pages which are then added to the view.

Steps:

   1. Create a new node view, giving it a name and setting its View type to
      node
   2. In the default display, add Node: title and Node: body fields
   3. In the default display, add a sort criterium on Node: Post date and set
      it to descending
   4. In the default display, add a filter on Node: type being equal to Page.
   5. Add a Page display
   6. For this page, set within Page Settings the Path to something easy to
      remember
   7. For this page, set within Basic Settings the Style to Editview. Remember
      to use the Override button. Set Node Type to Page.
   8. For this page, set within Basic Settings the Access to something
      sensible. Remember to use the Override button.
   9. Save the view and visit its url.


View, edit, and add child nodes on a parent node page
-----------------------------------------------------

This view uses cck, views, and editview to add the ability to see and edit a
table of child nodes on a parent node page. We will relate children to parents
using a cck node reference field to the child node type.

Steps:

   1. Create the parent and child node types. In this example we will make
      organisation the parent type, and employee the child type:
         1. Create organisation as a cck node type, giving it whatever fields
            you like.
         2. Create employee as a ckk node type. One of its fields should be a
            node reference which you should limit to only be able to access
            nodes of type organisation. Make sure the node reference is a
            select list, not an auto-complete text field. We will be relying
            on not having to enter the reference manually.
         3. For the node reference field, specify the following code for its
            default value:

            if (arg(0) == 'node' && is_numeric(arg(1))) {
              $organisation = node_load(arg(1));
              if ($organisation->type == 'organisation') {
                return array(0 => array('nid' => $organisation->nid));
              }
            }
            return array(array());

         4. At this point you may want to create a few test organisations and
            assign each one a few employees.
   2. Create a new node view, giving it a name and setting its View type to
      node:
         1. In the default display, add all employee fields except the node
            reference fields
         2. In the default display, add a filter on Node: type being equal to
            employee.
         3. In the default display, add an argument on the node reference
            field. Select Provide default argument and use Node ID from URL.
         4. Add a Block display and set its name to View Block and its title
            to Employees.
         5. Add another Block display and set its name to Edit Block and its
            title to Employees.
         6. For the edit block set within Basic Settings the Style to Editview.
            Remember to use the Override button. Set Node Type to Employee.
         7. For both blocks, set within Basic Settings the Access to something
            sensible. Remember to use the Override button.
   3. Go to the blocks admin page and enable both blocks:
         1. Select a region for the view block. Configure it and select
            Show if the following PHP code returns TRUE with the following code:

            <?php
              if (arg(0) == 'node' && is_numeric(arg(1)) && arg(2) == '\ {
                $node = node_load(arg(1));
                if ($node->type == 'organisation') {
                  return true;
                }
              }
              return false;
              ?>

            This will only enable the block if you are viewing an organisation.
         2. Select a region for the edit block. Configure it and select
            Show if the following PHP code returns TRUE with the following code:

            <?php
              if (arg(0) == 'node' && is_numeric(arg(1)) && arg(2) == 'edit') {
                $node = node_load(arg(1));
                if ($node->type == 'organisation') {
                  return true;
                }
              }
              return false;
            ?>

            This will only enable the block if you are editing an organisation.

Now when you go to an organisation node, you will see a table listing all of that organisation's employees, and when you click on its edit tab, the list of employees will become editable as well.


About Editview
--------------

Editview was developed  by Agileware Pty Ltd (http://www.agileware.net) and
co-sponsored by Xu Media Solutions (http://xumedia.org) and Agileware. The
current active maintainer is Frodo Looijaard <drupal@frodo.looijaard.name>
