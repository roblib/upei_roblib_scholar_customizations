<?php
/**
 * @file
 * Implementation of robertson library scholar utility functions.
 */

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