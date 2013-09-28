<?php

/**
 * @file
 * Drush hook implementations for BehatEditor
 */

/**
 * Implements hook_drush_command().
 */
function behat_editor_drush_command() {
    $items = array();

    $items['behat-run'] = array(
        'description' => 'Pass a file to run as a test',
        'allow-additional-options' => TRUE,
        'aliases' => array('br'),
        'examples' => array(
            'drush run module_name view.feature' => 'This will use the path of the module_name to run that test'
        ),
        'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
        'arguments' => array(
            'module' => 'The module name that the test file is in',
            'filename' => 'The file name to test. Should be in present folder',
        ),
    );

    $items['behat-run-folder'] = array(
        'description' => 'Pass a folder to run all tests and decide to turn on Javascript',
        'allow-additional-options' => TRUE,
        'aliases' => array('brf'),
        'examples' => array(
            'drush run module_name 1' => 'This will use the path of the module_name to run that test and 1 means you want to include Javascript'
        ),
        'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
        'arguments' => array(
            'module' => 'The module name that the test files are in',
            'javascript' => '1 if you want to run javascript based tests 0 if not. Javascript means you are running this locally.',
        ),
    );

    return $items;
}


function drush_behat_editor_behat_run() {
    $module = func_get_arg(0);
    $filename = func_get_arg(1);
    //@todo validate args
    composer_manager_register_autoloader();
    module_load_include('inc', 'behat_editor', 'helpers/behat_helpers_app');
    //@todo get composer to load this
    $path = drupal_get_path('module', 'behat_editor');
    $path = $path . '/src/Drupal/BehatEditor/BehatEditorRun.php';
    $path = drupal_realpath($path);
    require_once("$path");
    $file_object = _behat_get_file_info($module, $filename);
    $run = new Drupal\BehatEditor\BehatEditorRun($file_object);
    print "Running Tests...\n";
    $output = $run->execDrush();
    $results = implode("\n\r", $output);
    return $results;
}

function drush_behat_editor_behat_run_folder() {
    //@todo validate args
    $module = func_get_arg(0);

    if(func_num_args() < 1) {
        $javascript = func_get_arg(1);
    } else {
        $javascript = 0;
    }

    $folder = drupal_get_path('module', $module);
    $full_path = drupal_realpath($folder);

    composer_manager_register_autoloader();
    module_load_include('inc', 'behat_editor', 'helpers/behat_helpers_app');
    //@todo get composer to load this
    $path = drupal_get_path('module', 'behat_editor');
    $path = $path . '/src/Drupal/BehatEditor/BehatEditorRun.php';
    $path = drupal_realpath($path);
    require_once("$path");
    $file_object = array();
    $run = new Drupal\BehatEditor\BehatEditorRun($file_object);
    print "Running Tests...\n";
    $output = $run->execDrushAll($full_path, $javascript);
    $results = implode("\n\r", $output);
    return $results;
}
