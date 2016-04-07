<?php
/**
 * @file
 * Implementation of robertson library scholar xacml customizations.
 */

/**
 * Wrapper for upei_roblib_scholar_customizations_add_romeo with error reporting.
 *
 * @param AbstractObject $object
 *   An islandora object
 */
function _upei_roblib_scholar_customizations_add_romeo($object, $redirect = TRUE) {
  // we only want to update objects with a citaton cmodel
  $cmodels = array(
    'ir:citationCModel',
  );
  if (count(array_intersect($object->models, $cmodels)) > 0) {
    try {
      $result = upei_roblib_scholar_customizations_add_romeo($object);
    }
    catch (Exception $e) {
      watchdog('upei_roblib_scholar_customizations', 'Could not add/modify romeo data for object %pid, %error ',
        array(
          '%pid' => $object->id,
          '%error' => $e->getMessage()
        ), WATCHDOG_INFO);
      drupal_set_message(t('Could not add/modify romeo data for object %pid, %error ',
        array('%pid' => $object->id, '%error' => $e->getMessage())), 'warning', FALSE);
      if($redirect) {
        drupal_goto("/islandora/object/$object->id");
      }
    }
    if ($result === 0) {
      drupal_set_message(t('Successfully added/updated the ROMEO datastream for object %pid',
        array('%pid' => $object->id)), 'status', FALSE);
    }
    elseif ($result < 0) {
      watchdog('upei_roblib_scholar_customizations', 'No Romeo record found for the object %pid',
        array('%pid' => $object->id), WATCHDOG_INFO);
      drupal_set_message(t('No Romeo record found for the object %pid',
        array('%pid' => $object->id)), 'warning', FALSE);
    }
  }
  if($redirect) {
    drupal_goto("/islandora/object/$object->id");
  }
}

/**
 * Add a Romeo datastream to an object.
 *
 * @param AbstractObject $object
 *   The islandora object to add the datastream too.
 * @return int
 *   Returns 0 on success or a negative int if the datastream was not added.
 * @throws \Exception
 *   Throws an exception on error, ie can't parse or load datastreams or error communication with romeo api
 * param $string /null $key
 *   The romeo api key.
 * param $string /null $base_url
 *   The romeo api base url.
 *
 */
function upei_roblib_scholar_customizations_add_romeo($object, $key = NULL, $base_url = NULL) {
  $dsid = 'ROMEO';
  $key = empty($key) ? variable_get('islandora_scholar_romeo_key', '') : $key;
  $base_url = empty($base_url) ? variable_get('islandora_scholar_romeo_url', 'http://www.sherpa.ac.uk/romeo/api29.php') : $base_url;
  if (empty($object['MODS'])) {
    // we can't get an issn from the mods
    throw new Exception('No MODS datastream.');
  }
  $mods_content = $object['MODS']->content;
  if (empty($mods_content)) {
    // mods is empty still can't get an issn
    throw new Exception('Empty MODS datastream');
  }
  $mods_doc = @DOMDocument::loadXML($mods_content);
  if (empty($mods_doc)) {
    // couldn't parse the mods
    throw new Exception('Could not parse the MODS datastream');
  }
  $mods_xpath = new DOMXPath($mods_doc);
  $mods_xpath->registerNamespace('mods', 'http://www.loc.gov/mods/v3');
  $issn_results = $mods_xpath->query('//mods:identifier[@type="issn" and normalize-space(text())]');
  if ($issn_results->length > 0) {
    // take the first issn
    $issn = $issn_results->item(0)->textContent;
  }
  if (empty($issn)) {
    // no issn to use in romeo query
    throw new Exception('ISSN not found in MODS datastream.');
  }
  $base_url .= "?issn=$issn&ak=$key";
  $options = array(
    'method' => 'GET',
  );
  $result = drupal_http_request($base_url, $options);
  // romeo returns 200 even if it can't find the issn so we need to parse the xml
  $romeo_content = $result->data;
  $romeo_doc = @DOMDocument::loadXML($romeo_content);
  if (empty($romeo_doc)) {
    // romeo response could not be loaded as xml
    throw new Exception('Error parsing romeo api response.');
  }
  $romeo_xpath = new DOMXpath($romeo_doc);
  $publisher_results = $romeo_xpath->query('/romeoapi/publishers[1]/publisher[@id]');
  if ($publisher_results->length > 0) {
    if (isset($object[$dsid]) && $object[$dsid]->controlGroup == 'E') {
      // we can't update a datastream with controlGroup of E with xml so we need to delete it
      $object->purgeDatastream($dsid);
    }
    if (isset($object[$dsid])) {
      $object[$dsid]->content = $romeo_content;
    }
    else {
      $datastream = $object->constructDatastream($dsid);
      $datastream->versionable = TRUE;
      $datastream->label = 'ROMEO';
      $datastream->mimetype = 'application/xml';
      $datastream->content = $romeo_content;
      $object->ingestDatastream($datastream);
    }
    return 0;
  }
  else {
    return -2;
  }
}