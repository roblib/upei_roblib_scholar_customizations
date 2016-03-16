<?php
/**
 * @file
 * Implementation of robertson library scholar xacml customizations.
 */

/**
 * Adds an xacml policy to objects.
 *
 * We check for MODS or MADS and use the identifier type = u1 field if it exists
 * to add usernames to the management rules.  Also adds the object owner and scholar editor
 * roles to the management rules.
 *
 * @param AbstractObject $object
 *   An Islandora object
 */
function upei_roblib_scholar_customizations_add_xacml($object){
  module_load_include('inc', 'islandora_xacml_api', 'includes/xacml');
  $cmodels = array(
    'ir:thesisCModel',
    'ir:citationCModel',
    'islandora:personCModel',
    'islandora:eventCModel',
    'islandora:organizationCModel',
  );
  if (count(array_intersect($object->models,$cmodels)) < 1) {
    return;
  }
  // start with a fresh policy (empty)
  $xacml = new IslandoraXacml($object, "");
  $xml = new DOMDocument();
  if (!empty($object['MODS'])) {
    $test = $xml->loadXML($object['MODS']->getContent(''));
    if (!empty($test)) {
      $query = "//mods:identifier[@type='u1']";
      $xpath = new DOMXPath($xml);
      $xpath->registerNamespace('mods', 'http://www.loc.gov/mods/v3');
      $results = $xpath->query($query, $xml);
      foreach ($results as $element) {
        // add xacml rules for u1 mods element if any exist
        $xacml->managementRule->addUser(array($element->nodeValue));
      }
    }
  }
  if (!empty($object['MADS'])) {
    $test = $xml->loadXML($object['MODS']->getContent(''));
    if (!empty($test)) {
      $query = "//mads:identifier[@type='u1']";
      $xpath = new DOMXPath($xml);
      $xpath->registerNamespace('mads', 'http://www.loc.gov/mads/v2');
      $results = $xpath->query($query, $xml);
      foreach ($results as $element) {
        // add xacml rules for u1 mods element if any exist
        $xacml->managementRule->addUser(array($element->nodeValue));
      }
    }
  }
  if (!empty($object->owner)) {
    $xacml->managementRule->addUser(array($object->owner, 'admin'));
  }
  else {
    $xacml->managementRule->addUser(array('admin'));
  }
  $xacml->managementRule->addRole(array('administrator', 'scholar editor'));
  $xacml->writeBackToFedora();
}