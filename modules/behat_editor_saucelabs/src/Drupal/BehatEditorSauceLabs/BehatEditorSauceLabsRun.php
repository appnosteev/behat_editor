<?php

namespace Drupal\BehatEditorSauceLabs;
use Drupal\BehatEditor\BehatEditorRun;

class BehatEditorSauceLabsRun extends BehatEditorRun {

    public function __construct($file_object) {
        parent::__construct($file_object);
    }

    public function exec() {
        $response = exec("cd $this->behat_path && ./bin/behat --config=\"$this->yml_path\" --no-paths --out $this->output_file  --profile=Selenium-saucelabs2 $this->absolute_file_path && echo $?");
        return array('response' => $response, 'output_file' => $this->output_file);
    }
}