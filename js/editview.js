(function ($) {

    Drupal.behaviors.editview_ajax = {

        attach: function (context, settings) {

            // Fully declare element_settings, as Drupal's ajax.js seems not to merge defaults correctly.
            var ajax_element_settings_defaults = {
                url: settings.editview_ajax.url + '/',
                event: 'mousedown',
                prevent: 'click',
                keypress: false,
                selector: '#',
                effect: 'none',
                speed: 'none',
                method: 'replaceWith',
                wrapper: '',
                progress: {
                    type: 'throbber',
                    message: ''
                },
                submit: {
                    'js': true
                }
            };


            // If module set a 'reload' setting, then retrieve the Webform with AJAX.
            if (setting = settings.editview_ajax.reload) {
                var element_settings = ajax_element_settings_defaults;
                element_settings.url += setting.nid + '/' + setting.wrapper_id;
                element_settings.selector += setting.wrapper_id;
                element_settings.wrapper += setting.wrapper_id;

                button = $('#' + setting.button_id, context);

                Drupal.ajax[setting.wrapper_id] = new Drupal.ajax(setting.wrapper_id, button, element_settings);
                Drupal.ajax[setting.wrapper_id].eventResponse(button, null);

                // Remove the reload setting to avoid looping.
                delete settings.editview_ajax.reload;
            }

        }

    };

}(jQuery));
