<?php

/**
 * @file
 * Contains \Drupal\BehatEditor\BehatEditorRun.
 */

namespace Drupal\BehatEditor;

use Drupal\BehatEditor;

/**
 * Class BehatEditorRun
 * Methods needed to run a test.
 *
 * @params file_object
 *      This is created by \Drupal\BehatEditor\File class
 * @package Drupal\BehatEditor
 *
 * @todo use the File class to create the file object during __construct
 */

class BehatEditorRun {

    public $behat_path = '';
    public $absolute_file_path = '';
    public $output_file = '';
    public $file_full_path = '';
    public $module = '';
    public $relative_path = '';
    public $filename = '';
    public $yml_path = '';
    public $filename_no_ext = '';
    public $file_array;
    public $file_object = array();

    /**
     * File object from FileObject class
     * for now using a function
     * check it comes from the class later one
     */
    public function __construct($file_object) {

        composer_manager_register_autoloader();
        $path = drupal_get_path('module', 'behat_editor');
        $this->yml_path = drupal_realpath($path) . '/behat/behat.yml';
        $this->behat_path = _behat_editor_behat_bin_folder();
        $this->absolute_file_path = '';
        $this->file_full_path = $file_object['absolute_path_with_file'];
        $this->absolute_file_path = $file_object['absolute_path_with_file'];
        $this->relative_path = $file_object['relative_path'];
        $this->filename = $file_object['filename'];
        $this->filename_no_ext = $file_object['filename_no_ext'];
        $this->module = $file_object['module'];
        $this->file_object = $file_object;
        $this->output_file = self::getPath();
    }

    /**
     * Decide what path we should be storing files at
     *
     * @return string
     */
    public function getPath() {
        if(user_access('behat add test') && $this->module != variable_get('behat_editor_default_storage_folder', BEHAT_EDITOR_DEFAULT_STORAGE_FOLDER)) {
            return  self::runFromModuleFolder();
        } else {
            return self::runFromTmpFolder();
        }
    }

    /**
     * If we are running from the ModuleFolder
     * @return string
     * @todo unify this with runFromTmpFolder().
     */
    public function runFromModuleFolder() {
        //Setup folder to store file with test results
        $file_tmp_folder = variable_get('behat_editor_default_folder', BEHAT_EDITOR_DEFAULT_FOLDER);
        $path_results = file_build_uri("/{$file_tmp_folder}/results");
        if (!file_prepare_directory($path_results, FILE_CREATE_DIRECTORY)) {
            drupal_mkdir($path_results);
        }
        $test_id = $this->filename_no_ext;
        $output_file = drupal_realpath($path_results) . '/' . $test_id . '.txt';    //Needed to run bin
        return $output_file;
    }

    /**
     * If we are running from the Tmp folder
     * @return string
     * @todo unify this with runFromModuleFolder().
     */
    public function runFromTmpFolder() {
        $file_tmp_folder = variable_get('behat_editor_default_storage_folder', BEHAT_EDITOR_DEFAULT_STORAGE_FOLDER);
        $path_results = file_build_uri("/{$file_tmp_folder}/results");
        if (!file_prepare_directory($path_results, FILE_CREATE_DIRECTORY)) {
            drupal_mkdir($path_results);
        }
        $test_id = $this->filename_no_ext;
        $output_file = drupal_realpath($path_results) . '/' . $test_id . '.txt';    //Needed to run bin
        return $output_file;
    }

    /**
     * Used to exec the behat command
     *
     * @param bool $javascript
     *   Javascript true will open up a browser locally
     *   if the user is running selenium
     * @return array
     */
    public function exec($javascript = FALSE) {
        if($javascript == TRUE) {
            $tags = '';
        } else {
            $tags = "--tags '~@javascript'";
        }
        $command = "cd $this->behat_path && ./bin/behat --config=\"$this->yml_path\" --no-paths $tags $this->absolute_file_path";
        $context1 = 'behat_run';
        drupal_alter('behat_editor_command', $command, $context1);
        exec($command, $output, $return_var);
        $this->file_array = $output;
        $response = is_array($output) ? 0 : 1;
        self::saveResults($output, $return_var);
        return array('response' => $response, 'output_file' => $this->output_file, 'output_array' => $output);
    }

    /**
     * Used to exec the behat command by drush
     *
     * @param bool $javascript
     *   Javascript true will open up a browser locally
     *   if the user is running selenium
     * @return array
     */
    public function execDrush($javascript = FALSE, $tag_include = FALSE, $profile = 'default') {
        if($javascript == TRUE) {
            $tags_exclude = '';
        } else {
            $tags_exclude = "--tags '~@javascript'";
        }

        if($tag_include) {
            $tag_include = "--tags '" . $tag_include . "'";
        } else {
            $tag_include = '';
        }
        exec("cd $this->behat_path && ./bin/behat --config=\"$this->yml_path\" --format=pretty --no-paths $tag_include --profile=$profile $tags_exclude $this->absolute_file_path", $output, $return_var);
        self::saveResults($output, $return_var);
        return $output;
    }

    /**
     * Return the output on a Pass test
     *
     * @return array
     */
    public function generateReturnPassOutput() {
        $date = format_date(time(), $type = 'medium', $format = '', $timezone = NULL, $langcode = NULL);
        $file_url = l($this->filename, $this->relative_path, $options = array('attributes' => array('target' => '_blank', 'id' => 'test-file')));
        $file_message = array(
            'file' => $this->filename,
            'error' => 0,
            'message' => "$date <br> File $file_url tested."
        );
        $report = self::generateHTMLOutput();
        $output = array('message' => t('@date: <br> Test successful!', array('@date' => $date)), 'file' => $this->filename, 'test_output' => $report, 'error' => FALSE);
        $results = array('file' => $file_message, 'test' => $output, 'error' => 0);
        return $results;
    }

    /**
     * Return the html output on a Test
     *
     * @return array
     */
    public function generateHTMLOutput() {
        $results_message = array_slice($this->file_array, -3);
        $results_message_top = array_slice($this->file_array, 0, -3);
        $output_item_results = theme('item_list', $var = array('title' => 'Summary', 'items' => $results_message));
        $output_item_list = theme('item_list', $var = array('title' => 'All Results', 'items' => $results_message_top));
        return $output_item_results . $output_item_list;
    }

    /**
     * Return the output on a Fail test
     *
     * @return array
     */
    public function generateReturnFailOutput() {
        $date = format_date(time(), $type = 'medium', $format = '', $timezone = NULL, $langcode = NULL);
        $file_url = l($this->filename, $this->relative_path, $options = array('attributes' => array('target' => '_blank', 'id' => 'test-file')));
        $file_message = array(
            'file' => $this->filename,
            'error' => 0,
            'message' => "$date <br> File $file_url tested."
        );
        $message =  t('@date: <br> Error running test @name ', array('@date' => $date, '@name' => $this->filename));
        watchdog('behat_editor', "%date Error Running Test %name", $variables = array('%date' => $date, '%name' => $this->filename), $severity = WATCHDOG_ERROR, $link = $this->absolute_file_path);
        $output = array('message' => $message, 'file' => $this->filename, 'error' => TRUE);
        $results = array('file' => $file_message, 'test' => $output, 'error' => 1, 'message' => $message);
        return $results;
    }

    /**
     * Save the results to the DB
     *
     * @param $output
     *   Test results from exec
     *
     */
    protected function saveResults($output, $return_var = 0) {
        $saveResults = new Results();
        $saveResults->fields['filename'] = $this->filename;
        $saveResults->fields['module'] = $this->module;
        $saveResults->fields['results'] = serialize($output);
        $saveResults->fields['duration'] = array_pop($output);;
        $saveResults->fields['created'] = REQUEST_TIME;
        $saveResults->fields['status'] = $return_var;
        $saveResults->insert();
    }
}