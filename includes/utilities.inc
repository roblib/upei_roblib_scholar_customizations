<?php
/**
 * @file
 * Implementation of robertson library scholar utility functions.
 */
const SOLR_MADS_FACULTY_IDENTIFIER_TEXT = 'MADS_u1_mt';
const SOLR_MADS_FACULTY_IDENTIFIER_STRING = 'MADS_u1_ms';
const SOLAR_MADS_DEPARTMENT_STRING = 'MADS_department_ms';

/**
 * Update the PDF datastream label to = the objects PID + -PDF.
 *
 * This makes for a somewhat more meaningful PDF file download file name
 *
 * @param $object
 */
function upei_roblib_scholar_customizations_update_pdf_label($object) {
  $cmodels = array(
    'ir:thesisCModel',
    'ir:citationCModel',
  );
  if (count(array_intersect($object->models, $cmodels)) > 0 && !empty($object['PDF'])) {
    $object['PDF']->label = str_replace(':', '-', $object->id) . '-PDF';
  }
}

/**
 * Returns the value of the MODS or MADS identifier[@type=u1] element.
 *
 * @param object $object
 *   An islandora object
 * @param string $dsid
 *   A datastream id either MADS or MODS
 * @return string
 *   The value of the identifier[@type=u1]
 * @throws \Exception
 *   if the datastream does not exist, is empty, can't be loaded as xml or the identifier does not exist.
 */
function upei_roblib_scholar_customizations_get_u1($object, $dsid){
  if(empty($object[$dsid]->content)) {
    throw new Exception("No content found for datastream $dsid");
  }
  $xml = $object[$dsid]->content;
  $doc = @DOMDocument::loadXML($xml);
  if (empty($doc)) {
    // couldn't parse the mods
    throw new Exception('Could not parse XML');
  }
  $xpath = new DOMXPath($doc);
  $namespace = ($dsid === 'MADS') ? 'http://www.loc.gov/mads/v2' : 'http://www.loc.gov/mods/v3';
  $xpath->registerNamespace('ns', $namespace);
  $results = $xpath->query('//ns:identifier[@type="u1" and normalize-space(text())]');
  if ($results->length > 0) {
    // take the first u1 should really only be one
    $u1 = $results->item(0)->textContent;
  }
  if (empty($u1)) {
    // no issn to use in romeo query
    throw new Exception("U1 identifier not found in $dsid datastream.");
  }
  return $u1;
}

/**
 * Callback for faculty identifiers autocomplete
 */
function upei_roblib_scholar_customizations_u1_callback() {
  $qualifier = SOLR_MADS_FACULTY_IDENTIFIER_TEXT . ':*';
  $path = request_path();
  if ($path) {
    $path_array = explode('/', $path);
    $stub = end($path_array);
    $qualifier = SOLR_MADS_FACULTY_IDENTIFIER_TEXT . ':' . $stub . '*';
  }
  $params = array(
    'fl' => array('' . SOLR_MADS_FACULTY_IDENTIFIER_TEXT . '', 'PID'),
  );
  $query = "$qualifier AND " . 'RELS_EXT_hasModel_uri_mt:"islandora:personCModel"';
  $solr = upei_roblib_scholar_customizations_get_solr();
  try {
    $results = $solr->search($query, 0, 1000, $params);
    $json = json_decode($results->getRawResponse(), TRUE);
  }
  catch (Exception $e) {
    watchdog_exception('UPEI Scholar Customizations', $e, 'Got an exception while searching faculty identifiers for callback.', array(), WATCHDOG_ERROR);
  }
  $list = array();
  foreach ($json['response']['docs'] as $choice) {
    if (isset($choice[SOLR_MADS_FACULTY_IDENTIFIER_TEXT])) {
      $list[$choice[SOLR_MADS_FACULTY_IDENTIFIER_TEXT][0]] = $choice[SOLR_MADS_FACULTY_IDENTIFIER_TEXT][0];
    }
  }

  drupal_json_output($list);
}

/**
 * Validation for elements on various citation, thesis and scholar forms.  We use this one function for many forms.
 *
 * We want to make sure Department or Faculty id fields populated via a type ahead driven by solr actually match the solr values.
 *
 * @param $form
 * @param $form_state
 * @param $form_field_name
 * @param $cmodel
 * @param $solr_field
 * @param null $form_element
 */
function _upei_roblib_scholar_customizations_fb_validate($form, $form_state, $form_field_name, $cmodel, $solr_field, $form_element = NULL) {
  $form_values = $form_state['values'];
  if(empty($form_element)) {
    $form_element = empty($form_values[$form_field_name]) ? array() : $form_values[$form_field_name];
  }
  $type = ($cmodel === 'islandora\:personCModel') ? 'Faculty ID' : 'Department ID';
  $solr = upei_roblib_scholar_customizations_get_solr();
  $params = array(
    'fl' => array('PID'),
  );
  foreach ($form_element as $element) {
    unset($element['add-tabpanel']);
    unset($element['delete-tabpanel']);
    foreach ($element as $value) {
      if (!empty($value)) {
        // TODO we should be checking namespaces here as well
        $query = "$solr_field:\"$value\" AND RELS_EXT_hasModel_uri_mt:$cmodel";
        try {
          $results = $solr->search($query, 0, 1000, $params);
          $json = json_decode($results->getRawResponse(), TRUE);
          if ($json['response']['numFound'] < 1) {
            form_set_error($form_field_name, t("value %value is not a valid %type", array(
              '%value' => $value,
              '%type' => $type,
            )));
          }
        }
        catch (Exception $e) {
          watchdog_exception('Roblib Scholar Customizations', $e, "Error while trying to validate input.", array(), WATCHDOG_ERROR);
        }
      }
    }
  }
}

/**
 * Return a solr service object.
 *
 * @return \Apache_Solr_Service
 */
function upei_roblib_scholar_customizations_get_solr() {
  $url = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr'));
  $solr = new Apache_Solr_Service($url['host'], $url['port'], $url['path'] . '/');
  $solr->setCreateDocuments(FALSE);
  return $solr;
}