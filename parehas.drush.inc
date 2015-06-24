<?php
/**
 * @file
 * Parehas Drush file.
 */

/**
 * Implements hook_drush_command().
 *
 * @return array
 * @return array
 *   A list of drush commands.
 */
function parehas_drush_command() {
  $items = array();

  $items['deploy-sync'] = array(
    'callback' => '_parehas_sync_data',
    'description' => "Sync content accross environment.",
    'aliases' => array('in-ds'),
    'examples' => array(
      'drush deploy-sync @[site].[env] @[site].[env]' => 'An example command on how to sync DB across environment.',
      'drush deploy-sync @test.prod @test.stage' => 'Sync prod to stage.',
    ),
  );

  return $items;
}


/**
 * Implements hook_drush_help().
 */
function parehas_drush_help($section) {
  switch ($section) {
    case 'drush:deploy-sync':
      return dt('Sync DB across environment.');
  }
}


/**
 * Sync content accross environment.
 *
 * TODO: Make the skip-tables-list configurable from CMS admin to allow the
 * adding/removing of tables if needed.
 */
function _parehas_sync_data() {
  $args = func_get_args();

  if (isset($args[0]) && isset($args[0])) {
    // Run the validation first;
    _parehas_validate_sync($args[0], $args[1]);
  
    // Do the magick here ...

    // Run the sql-sync.
    parehas_sql_sync($args[0], $args[1]);
    drush_log(dt("The database is now synced."), 'notice');
  }
  else {
    drush_log(dt("Missing arguments."), 'error');
  }
}


/**
 * Validation.
 */ 
function parehas_validate_sync($source, $target) {
  $error = FALSE;
  $message = "You are not allowed to update the target ($target) server.";
  
  switch ($target) {
    case '@[site].prod':
      drush_log(dt($message), 'error');
      $error = TRUE;
      break;
      
    case '@[site].test':
      if ($source === '@[site].local' || $source === '@[site].dev') {
        drush_log(dt($message), 'error');
        $error = TRUE;
      }
      break;
      
    case "@[site].dev":
      if ($source === '@[site].local') {
        drush_log(dt($message), 'error');
        $error = TRUE;
      }
      break;
  }
  if ($error) {
    exit();
  }
}


/**
 * Execute the sql-sync command.
 */
function _parehas_sql_sync($source, $target) {
  $schema = drupal_get_schema();
  $cache = '';
  foreach ($schema as $table => $tableschema) {
    if (_startsWith($table, 'cache') || _endsWith($table, 'cache')) {
      $cache .= $table . ',';
    }
  }
  
  drush_invoke_process('@none', 'sql-sync', array($source, $target), array(
    'skip-tables-list' => $cache . "apachesolr_environment ,apachesolr_environment_variable,apachesolr_index_bundles  ,apachesolr_index_entities,apachesolr_index_entities_node,apachesolr_search_page ,semaphore,queue,sessions,captcha_sessions,flood,history,watchdog,users"
  ));
}

/**
 * A helper function to determine the results based on the keyword.
 * 
 * This function will check if the results begins with the pattern.
 */ 
function _startsWith($haystack, $needle) {
  return $needle === "" || strrpos($haystack, $needle, -strlen($haystack)) !== FALSE;
}


/**
 * A helper function to determine the results based on the keyword.
 * 
 * This function will check if the results ends with the pattern.
 */ 
function _endsWith($haystack, $needle) {
  return $needle === "" || (($temp = strlen($haystack) - strlen($needle)) >= 0 && strpos($haystack, $needle, $temp) !== FALSE);
}

