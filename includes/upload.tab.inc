<?php
/**
 * @file
 * Document/upload tab upload utilities
 */

/**
 * Updates the MODS datastream with the given usage and version.
 *
 * Removes any other usage or version elements.
 *
 * @param AbstractObject $object
 *   The object.
 * @param string $usage
 *   The usage either the 'publisher' or 'author' holds the copyright.
 * @param string $version
 *   The version of the uploaded document either 'Pre-print',
 *  'Published', 'Post-print', 'OTHER'.
 */
function upei_roblib_scholar_customizations_add_usage_and_version_elements_to_mods(AbstractObject $object, $usage, $version) {
  $refuse_msg = t('Refusing to add usage and publication version statements alone');
  if (empty($object['MODS'])) {
    drupal_set_message(t('No MODS in object @pid. %refuse.', array('@pid' => $object->id, '%refuse' => $refuse_msg)), 'error');
    return;
  }
  $namespace = 'http://www.loc.gov/mods/v3';
  $doc = new DOMDocument();
  $doc->loadXML($object['MODS']->content);
  $xpath = new DOMXPath($doc);
  $xpath->registerNamespace('mods', $namespace);
  // Remove all instances of mods:accessCondition and mods:physicalDescription
  // from every mods:mods element instance.
  // we check for displayLabel=License and also for cases where there is not a displaylabel yet. we don't want to touch displayLabel=Permission Statement
  $access_condition_query = '//mods:accessCondition[@type="use and reproduction" and (@displayLabel="License" or not(@displayLabel))]';
  $physical_description_query = '//mods:physicalDescription[@authority="local"]';
  $results = $xpath->query("{$access_condition_query} | {$physical_description_query}");
  foreach ($results as $result) {
    $result->parentNode->removeChild($result);
  }
  // Regardless of the number of mods:mods elements in the document we only
  // add the usage and version to the first one.
  $results = $xpath->query('//mods:mods[1]');
  if ($results->length == 0) {
    drupal_set_message(t('Could not find MODS element in object @pid is empty. %refuse.', array('@pid' => $object->id, '%refuse' => $refuse_msg)), 'error');
    return;
  }
  $mods_element = $results->item(0);
  // Add mods:accessCondition to the first mods:mods element.
  $access_condition_element = $doc->createElementNS($namespace, 'accessCondition');
  $access_condition_element->setAttribute('type', 'use and reproduction');
  $access_condition_element->setAttribute('displayLabel', 'License');
  $access_condition_element->nodeValue = $usage;
  $mods_element->appendChild($access_condition_element);
  // Add mods:physicalDescription to the first mods:mods element.
  $physical_description_element = $doc->createElementNS($namespace, 'physicalDescription');
  $physical_description_element->setAttribute('authority', 'local');
  $physical_description_element->nodeValue = $version;
  $mods_element->appendChild($physical_description_element);
  // Commit changes to datastream.
  $object['MODS']->setContentFromString($doc->saveXML());
}
