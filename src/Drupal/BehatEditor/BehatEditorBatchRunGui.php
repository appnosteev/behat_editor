<?php

/**
 * @file
 * Contains \Drupal\BehatEditor\BehatEditorBatchRun.
 */

namespace Drupal\BehatEditor;

use Drupal\BehatEditor;


class BehatEditorBatchRunGui {
    public $operations;
    public $rid;
    public $test_results;

    public function __construct(){
        composer_manager_register_autoloader();

    }

    function batchSubmit($method, $args, $type, $settings) {
        $batch = BehatEditorBatchType::type($method);
        $batch->setUp($method, $args, $type, $settings);
        return $batch;
    }

}
