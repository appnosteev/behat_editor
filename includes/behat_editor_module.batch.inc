<?php

use Drupal\BehatEditor;

function behat_editor_bulk() {
    $output = drupal_get_form('behat_editor_bulk_form_modules');
    return $output;
}

function behat_editor_bulk_form_modules($form, &$form_state) {
    composer_manager_register_autoloader();

    $build = array();
    $path = drupal_get_path('module', 'behat_editor');
    $build['#attached']['js']['behat_editor_sids'] = $path . '/js/behat_editor_sids.js';

    $build['overview'] = array(
        '#prefix' => '<div>',
        '#markup' => t("Choose which modules to run the tests from."),
        '#suffix' => '</div>',
    );

    $header = array(
        'machinename' => array('data' => t('Module Name')),
        'nicename' => array('data' => t('Nice Name')),
        'folder' => array('data' => t('Folder')),
        'last_run' => array('data' => t('Last Run')),
        'batch_status' => array('data' => t('Status')),
        'pass_fail' => array('data' => t('Pass / Fail')),
        'report' => array(t('Report Link')),
    );

    $rows = buildRowsModules();

    $build['dashboard'] = array(
        '#type' => 'tableselect',
        '#header' => $header,
        '#options' => $rows,
        '#empty' => t('No Tests Files'),
    );
    BehatEditor\BehatSettingsFormHelper::behatSettingsFields($build);
    $options = array(
        'gui' => t('Run in Webbrowser'),
//        'stop' => t('Stop'),
//        'background' => t('Run in background'),
    );
    $build['operations']['operation'] = array(
        '#type' => 'select',
        '#title' => t('Operation'),
        '#title_display' => 'invisible',
        '#options' => $options,
    );
    $build['operations']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Execute'),
        '#validate' => array('bulk_editor_run_module_based_validate'),
        '#submit' => array('bulk_editor_run_module_based_submit'),
    );

    return $build;
}

/**
 * Build rows for table
 * @return array
 */
function buildRowsModules() {
    //So I can pull out the nice name
    $files = new BehatEditor\FileModel();
    $modules = $files->getAllModuleFiles();
    $rows = array();
    $modules_info = system_rebuild_module_data();
    foreach($modules as $key => $value) {
        //Only need one sample from the module
        $done = '';
        $first_test = array_values($value);
        $first_test = $first_test[0];
        $module = $key;
        $last_run = 'n/a';
        $status = 'n/a';
        $pass_fail = 'n/a';
        $folder = (isset($first_test['subpath'])) ? $first_test['subpath'] : BEHAT_EDITOR_FOLDER;
        $array_key = $key;
        $args = array();
        $view = 'n/a';
        //@todo refactor this to go to the results table based on the array_key to get results.
        $results = BehatEditor\ResultsBatch::getResultsForByModule($module);
        if(!empty($results['results'])) {
            $results = $results['results'][0];
            $args = implode('/', $results['results']);
            $view = l('view', 'admin/behat/report/' . $args, array('attributes' => array('target' => '_blank')));
            $last_run = format_date($results['created'], 'medium');
            $status = BehatEditor\ResultsBatch::getBatchRunningStatus($results['batch_status']);
            $pass_fail = BehatEditor\ResultsBatch::getResultsPassFail($results['pass_fail']);
        }
        $row['machinename'] = $array_key;
        $row['nicename'] = $modules_info[$key]->info['name'];
        $row['folder'] = $folder;
        $row['last_run'] = $last_run;
        $row['batch_status'] = $status;
        $row['pass_fail'] = $pass_fail;
        $row['report'] = $view;
        $rows["$module"] = $row;
    }
    return $rows;
}

/**
 * Validate callback for the dashboard form.
 */
function bulk_editor_run_module_based_validate($form, &$form_state) {
    // Error if there are no items to select.
    if (!is_array($form_state['values']['dashboard']) || !count(array_filter($form_state['values']['dashboard']))) {
        form_set_error('', t('No items selected.'));
    }
}

/**
 * Submit callback for the dashboard form.
 */
function bulk_editor_run_module_based_submit($form, &$form_state) {
    $settings = array(
        'base_url_usid' => $form_state['values']['users'],
        'base_url_gsid' => $form_state['values']['group']
    );

    $operation = $form_state['values']['operation'];
    $machine_name_and_subfolder = array_filter($form_state['values']['dashboard']);

    if($operation == 'gui') {
        $start = new BehatEditor\BehatEditorBatchRunGui();
        $batch = $start->batchSubmit('module', $machine_name_and_subfolder, 'non_background', $settings);
        batch_set($batch->getBatch());
    }
}

function bulk_editor_batch_run_module($module, $subfolder, $rid, $settings){
    composer_manager_register_autoloader();
    $run = new BehatEditor\BehatEditorBatchTypeModule();
    $run->batchRun(array('module' => $module, 'subfolder' => $subfolder, 'rid' => $rid, 'settings' => $settings));
    $run->batchItemDone(array('item' => $module));
    $context['message'] = $run->message;
}

function bulk_editor_batch_module_done($success, $results, $operations, $duration){
    composer_manager_register_autoloader();
    $message = "All tests have been completed";
    $done = new BehatEditor\BehatEditorBatchTypeModule();
    $message = $done->batchDone($success, $results, $operations, $message);
    drupal_set_message($message);
}
