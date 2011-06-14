<?php

/**
 * @file
 *   drush integration for fctc.
 */

/**
 * Implementation of hook_drush_command().
 *
 * In this hook, you specify which commands your
 * drush module makes available, what it does and
 * description.
 *
 * Notice how this structure closely resembles how
 * you define menu hooks.
 *
 * @See drush_parse_command() for a list of recognized keys.
 *
 * @return
 *   An associative array describing your command(s).
 */
function fctc_drush_command() {
  $items = array();

  $items['fctc-cache-data'] = array(
    'callback' => 'fctc_drush_cache_data',
    'description' => dt('Pull in available data from avaliable SPARQL end points (national central points).'),
    'aliases' => array('fctcc'),
    'arguments' => array(
      'path' => dt('Optional. A path to the fctc module. If omitted Drush will use the default location.'),
    ),
  );
  /*
  $items['fctc-add-end-point'] = array(
    'callback' => 'fctc_drush_get_data',
    'description' => dt('Add SPARQL endpoint.'),
    'aliases' => array('fctca'),
    'arguments' => array(
      'path' => dt('Optional. A path to the fctc module. If omitted Drush will use the default location.'),
    ),
  );
  // */

  return $items;
}

/**
 * Implementation of hook_drush_help().
 *
 * This function is called whenever a drush user calls
 * 'drush help <name-of-your-command>'
 *
 * @param
 *   A string with the help section (prepend with 'drush:')
 *
 * @return
 *   A string with the help text for your command.
 */
function fctc_drush_help($section) {
  switch ($section) {
    case 'drush:rdf-download':
      return dt("Downloads the required ARC2 library from http://github.com/semsol/arc2. Include the optional path.");
  }
}

/**
 * Example drush command callback.
 *
 * This is where the action takes place.
 *
 * In this function, all of Drupals API is (usually) available, including
 * any functions you have added in your own modules/themes.
 *
 * To print something to the terminal window, use drush_print().
 *
 */
function fctc_drush_cache_data() {
  // Instantiate the ARC2 local store.
  $store = sparql_endpoint_initialize();
  //$store = sparql_ep_initialize();

  // Emtpy the local store.
  $store->reset();

  // Load data.
  drush_print('Getting data from sparql endpoints: Country X and Country Y');
  // TODO Each site (sparql endpoint) should maintain own data dump.
  // TODO Get from GUI.
  $domains = array(
    'localhost/countryx', // Country X
    'localhost/countryy', // Country Y
  );
  foreach ($domains as $domain) {
    _fctc_rebuild_rdf_index($domain);
    $q = 'LOAD <http://' . $domain . '/sparql?query=CONSTRUCT+{%0D%0A++%3Fs+%3Fp+%3Fo%0D%0A}+WHERE+{%3Fs+%3Fp+%3Fo+.}&output=&jsonp=&key=>';
drush_print($q);
    $rs = $store->query($q);
  }

  drush_print('Complete. Check here to see data: http://localhost/globalfocalpoint/sparql?query=SELECT+*+WHERE+{%0D%0A++GRAPH+%3Fg+{+%3Fs+%3Fp+%3Fo+.+}%0D%0A}%0D%0A&output=htmltab&jsonp=&key=');
}

/** 
 * Rebuild RDF index at endpoint before bringing in endpoint's data.
 * 
 * @param $domain
 *  String, domain of endpoint.
 */
function _fctc_rebuild_rdf_index($domain) {
  $url = 'http://' . $domain . '/sparql_endpoint_index';
  drush_print("Refreshing RDF index: $url");
}
