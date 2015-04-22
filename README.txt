
Editview is a Views plug-in that allows you to create views in which nodes are
editable and new nodes can be created. Editview works with or without JavaScript
enabled.

Install
-------
Installing the Editview module:
1) Copy the Editview folder to the sites/all/modules folder in your
   installation.
2) Enable the module using Administer -> Modules (admin/modules)

Dependencies
------------

Views
Entities

Usage
-----

Editable node views are created just like any other type of view, with a few
caveats:
   1. Editview can only be used on node, taxonomy and user views, but new users cannot be created from views.
   2. If you want to be able to add new nodes in the view, set the style to
      Editview in Basic settings.
        * You must now indicate what type of node will be added through the
          new node form and where it will appear.
        * Make sure all required fields are included in the view, or new nodes
          will fail validation.



About Editview
--------------

Editview was developed  by Agileware Pty Ltd (http://www.agileware.net) and
co-sponsored by Xu Media Solutions (http://xumedia.org) and Agileware. The
current active maintainer is Frodo Looijaard <drupal@frodo.looijaard.name>
The port to 7 was done by Agile Humanities
