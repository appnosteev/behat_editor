<?php

use Drupal\BehatEditor;

/**
 * @file functions needed to Edit a file
 */


/**
 * Implements hook_form().
 *
 * Form to take a file and turn it into editable html
 * You can see the related js file as well in the js folder
 *
 * @param module
 *   this should be the machine_name
 * @param filename
 *   this should be the filename.feature extension
 * @return array
 *
 * @todo my "controller" seems to have a ton of view related work?
 */
function behat_editor_edit_form($form, $form_state, $module, $filename) {
    module_load_include('inc', 'behat_editor', 'helpers/behat_helpers_app');
    module_load_include('inc', 'behat_editor', 'tpl/behat_shared_views');
    drupal_add_library('system', 'ui.sortable');
    drupal_add_library('system', 'ui.autocomplete');
    drupal_add_library('behat_editor', 'tag_it') ;
    composer_manager_register_autoloader();
    $file = new Drupal\BehatEditor\File(array(), $module, $filename, FALSE);
    $file_info = $file->get_file_info();
    //Product the HTML from the File in a format it will have all the markup needed to
    //  build / edit a test
    $file_to_html = theme('behat_editor_file_to_html_edit', $vars = array('file_text' => $file_info['scenario']));

    $path = drupal_get_path('module', 'behat_editor');
    $form['#attached']['js']['behat_editor_tags'] = $path . '/js/jquery.behat_editor.apply_tags.js';
    $form['#attached']['js']['behat_editor_close'] = $path . '/js/jquery.behat_editor.close_button.js';
    $form['#attached']['js']['behat_editor_app'] = $path . '/js/behat_editor_app.js';
    $form['#attached']['js']['behat_editor_run'] = $path . '/js/behat_editor_run.js';
    $form['#attached']['js']['behat_editor_add'] = $path . '/js/behat_editor_edit.js';
    $form['#attached']['js']['behat_editor_save'] = $path . '/js/behat_editor_save.js';
    $form['#attached']['js']['behat_editor_delete'] = $path . '/js/behat_editor_delete.js';
    $form['#attached']['js']['behat_editor_sbuilder'] = $path . '/js/behat_editor_scenario_builder.js';

    $form['#attached']['css']['behat_app_css'] = $path . '/css/behat_editor_app.css';
    $form['#attached']['css']['behat_add_css'] = $path . '/css/behat_editor_edit.css';
    $form['#attributes'] = array('class' => array('scenario-builder'));
    $form['header'] = array(
        '#markup' => '<h3>Editing: ' . $filename . '</h3>'
    );

    $form['actions'] = array(
        '#type' => 'container',
        '#attributes' => array('class' => array('col-md-12', 'well'))
    );

    $form['actions']['save_test'] = update_test_array($file_info['filename_no_ext'], $module);

    $form['actions']['run_test'] = run_test_array($file_info['filename_no_ext'], $module, 'create_run');

    if(user_access('behat delete test')) {
        $form['actions']['delete_test'] = delete_test_array($file_info['filename_no_ext'], $module);
    }

    $form['actions']['admin_area'] = admin_button_array();

    $form['left-side'] = array(
        '#type' => 'container',
        '#attributes' => array('class' => array('col-md-6'))
    );

    $form['left-side']['intro'] = array(
        '#theme' => 'behat_editor_questions_area_intro',
        '#prefix' => '<div class="nothing-here">',
        '#suffix' => '</div>',
    );

    $form['results'] = array(
        '#markup' => theme('behat_editor_results_area', $args = array('params' => array()))
    );

    $form['left-side']['existing_test'] = array(
        '#prefix' => '<div class="well">',
        '#suffix' => '</div>',
        '#markup' => theme('behat_editor_view_questions_area', array('questions' => $file_to_html['file_text'], 'actions' => array()))
    );

    $form['left-side']['questions'] = _behat_editor_questions_form();

    $title =  "Confirm Delete";
    $body = "The file {$filename} will be deleted.";
    $buttons = array(
                   'save' => array('title' => "Confirm", 'class' => array('btn', 'btn-danger', 'confirm-delete')),
                   'delete' => array('title' => 'Cancel', 'class' => array('btn', 'btn-success'), 'data' => array('dismiss' => "modal"))
               );
    $form['message']['#markup'] = behat_editor_modal($title, $body, $buttons);
    $form['filename'] = array('#type' => 'hidden', '#default_value' => $file_info['filename_no_ext']);
    $form['module'] = array('#type' => 'hidden', '#default_value' => $module);

    return $form;
}
