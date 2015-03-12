// $Id: editview.js,v 1.1.2.5 2010/11/10 19:04:30 frodo Exp $

Drupal.behaviors.editview = function (context) {
  $('form', context).each(Editview.formCapture);
};

var Editview = Editview || {
  formCapture: function() {
    var form = this;
    var form_id = $('[name=form_id]', form).val();
    var node_form = form_id && form_id.match(/^editview_node_form_\d+/);
    if (node_form) {
      var options = {
        url: Drupal.settings.editview.submit_url,
        beforeSubmit: function(form_values, form, options) {
          var name;
          var deleted = false;
          for (var i = 0; i < form_values.length; i++) {
            if (form_values[i].name == 'op' && form_values[i].value == Drupal.t('Delete')) {
              deleted = true;
            }
            if (form_values[i].name == 'title') {
              name = form_values[i].value;
            }
          }
          if (deleted && !confirm(Drupal.t('Are you sure you want to delete @name?', {'@name': name}) +"\n"+ Drupal.t('This action cannot be undone.'))) {
            return false;
          }
          var wrapper = $(form).parent();
          wrapper.slideUp('slow');
        },
        success: function(response, status, form) {
          var wrapper = $(form).parent();
          $(wrapper).empty();
          $(wrapper).append(response);
          $(wrapper).slideDown('slow');
          Drupal.attachBehaviors(wrapper);
          //$('.messages', wrapper).fadeOut(6000);
        },
        dataType: 'json',
        type: 'POST'
      }
      $('input[class~=editview-button]',form).click(function(e) {
	form.clk = e.target;
        $(form).ajaxSubmit(options);
        return false;
      })
      $('[name=editview_ids]',form).val(Drupal.settings.editview['ids_' + $('[name=editview_nid]',form).val()])
    }
  }
};
