<?php
/**
 * @file
 * file used to create the PDF upload step for upei scholar customizations.
 */

/**
 * Get a list of document versions for PDF upload.
 *
 * @return array
 *   document versions to choose from during document ingest
 */
function upei_roblib_scholar_customizations_get_document_versions() {
  $document_versions = array(
    'PRE-PRINT' => 'Pre-print - The version of the document you submitted to the publisher prior to peer-review.',
    'POST-PRINT' => 'Post-print - The version of the document after it has been peer-reviewed, but before it has gone to typesetting.',
    'PUBLISHED' => 'Published PDF - The published version of the document.',
    'OTHER' => 'Other/Unpublished - A document that has not been published or is otherwise considered unpublished.',
  );
  return $document_versions;
}

/**
 * Get a list of use permissions for PDF upload.
 *
 * @return array
 *   use permissions to choose from during document ingest
 */
function upei_roblib_scholar_customizations_get_use_permissions() {
  $use_permissions = array(
    'Contact Author' => 'Contact Author (I hold the copyright and wish to retain all rights)',
    'Contact Publisher' => 'Contact Publisher (I do not hold the copyright)',
    'CC-BY' => '<a rel="license" title="Creative Commons Attribution" 
   href="http://creativecommons.org/licenses/by/3.0/">
  <img alt="Creative Commons License" style="border-width:0" src="http://i.creativecommons.org/l/by/3.0/80x15.png" />
  </a> This work is licensed under a <a rel="license" 
   title="Creative Commons Attribution 3.0 Unported License" 
   href="http://creativecommons.org/licenses/by/3.0/">Creative Commons
   Attribution </a>',

    'CC-BY-SA' => '<a rel="license" title="Creative Commons Attribution-ShareAlike"
   href="http://creativecommons.org/licenses/by-sa/3.0/">
  <img alt="Creative Commons License" style="border-width:0" src="http://i.creativecommons.org/l/by-sa/3.0/80x15.png" />
  </a> This work is licensed under a <a rel="license" 
   title="Creative Commons Attribution-ShareAlike" 
   href="http://creativecommons.org/licenses/by-sa/3.0/">Creative Commons Attribution-ShareAlike License</a>',

    'CC-BY-NC' => '<a rel="license" title="Creative Commons Attribution-NonCommercial" 
   href="http://creativecommons.org/licenses/by-nc/3.0/">
  <img alt="Creative Commons License" style="border-width:0" src="http://i.creativecommons.org/l/by-nc/3.0/80x15.png" />
  </a> This work is licensed under a <a rel="license" 
   title="Creative Commons Attribution-NonCommercial 3.0 Unported License" 
   href="http://creativecommons.org/licenses/by-nc/3.0/">Creative Commons
   Attribution-NonCommercial License</a>',

    'CC-BY-NC-SA' => '<a rel="license" title="Creative Commons Attribution-NonCommercial-ShareAlike " 
   href="http://creativecommons.org/licenses/by-nc-sa/3.0/">
  <img alt="Creative Commons License" style="border-width:0" src="http://i.creativecommons.org/l/by-nc-sa/3.0/80x15.png" />
  </a> This work is licensed under a <a rel="license" 
   title="Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License" 
   href="http://creativecommons.org/licenses/by-nc-sa/3.0/">Creative Commons Attribution-NonCommercial-ShareAlike License</a>',

    'CC-BY-ND' => '<a rel="license" title="Creative Commons Attribution-NoDerivatives"
   href="http://creativecommons.org/licenses/by-nd/3.0/">
  <img alt="Creative Commons License" style="border-width:0" src="http://i.creativecommons.org/l/by-nd/3.0/80x15.png" />
  </a> This work is licensed under a <a rel="license" 
   title="Creative Commons Attribution-NoDerivatives" 
   href="http://creativecommons.org/licenses/by-nd/3.0/">Creative Commons Attribution-NoDerivatives License</a>',

    'CC-BY-NC-ND' => '<a rel="license" title="Creative Commons Attribution-NonCommercial-NoDerivatives" 
   href="http://creativecommons.org/licenses/by-nc-nd/3.0/">
  <img alt="Creative Commons License" style="border-width:0" src="http://i.creativecommons.org/l/by-nc-nd/3.0/80x15.png" />
  </a> This work is licensed under a <a rel="license" 
   title="Creative Commons Attribution-NonCommercial-NoDerivatives 3.0 Unported License" 
   href="http://creativecommons.org/licenses/by-nc-nd/3.0/">Creative Commons Attribution-NonCommercial-NoDerivatives License</a>',

    'CC0' => '<a rel="license" title="Creative Commons - No Rights Reserved"
   href="https://creativecommons.org/about/cc0/">
  <img alt="Creative Commons License" style="border-width:0" src="https://licensebuttons.net/p/zero/1.0/80x15.png" />
  </a> This work is licensed under a <a rel="license" 
   title="Creative Commons Attribution-NoDerivatives" 
   href="https://creativecommons.org/about/cc0/">Creative Commons - No Rights Reserved</a>',

    'Public Domain' => 'Public Domain',
  );
  return $use_permissions;
}

/**
 * Gets the upload form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function upei_roblib_scholar_customizations_pdf_upload_form($form, &$form_state) {
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  // Pull document versions from the module settings if available.
  $document_versions = upei_roblib_scholar_customizations_get_document_versions();
  $use_permissions = upei_roblib_scholar_customizations_get_use_permissions();
  $form['upload_pdf_checkbox'] = array(
    '#type' => 'checkbox',
    '#title' => t('Would you like to include a PDF document for this citation?'),
  );
  $form['upload_document'] = array(
    '#type' => 'fieldset',
    '#title' => t('Upload Document'),
    '#states' => array(
      'visible' => array(
        ':input[name="upload_pdf_checkbox"]' => array('checked' => TRUE),
      ),
    ),
  );
  $form['upload_document']['file'] = array(
    '#type' => 'managed_file',
    '#title' => 'PDF file',
    '#upload_location' => 'temporary://',
    '#upload_validators' => array(
      'file_validate_extensions' => array('pdf'),
      'file_validate_size' => array($upload_size * 1024 * 1024),
    ),
  );
  $form['upload_document']['version'] = array(
    '#type' => 'radios',
    '#title' => t('Document Version'),
    '#options' => $document_versions,
  );
  $form['upload_document']['usage'] = array(
    '#type' => 'radios',
    '#title' => t('Use Permission'),
    '#options' => $use_permissions,
  );
  $form['upload_document']['certifying'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Certify'),
    '#options' => array(
      'certify' => t('I certify that I have the right to upload this version.'),
    ),
  );
  // Call our submit handler not scholars We are using different attributes for usage etc.
  return $form;
}



/**
 * Validation for the Islandora Scholar PDF upload form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function upei_roblib_scholar_customizations_pdf_upload_form_validate($form, $form_state) {
  if ($form_state['values']['upload_pdf_checkbox']) {
    if (empty($form_state['values']['file'])) {
      form_set_error('file', t('A file must be uploaded!'));
    }
    if (empty($form_state['values']['version'])) {
      form_set_error('version', t('A document version must be selected!'));
    }
    if (empty($form_state['values']['usage'])) {
      form_set_error('usage', t('A usage permission must be selected!'));
    }
    if (empty($form_state['values']['certifying']['certify'])) {
      form_set_error('certifying', t('You must certify that you have the right to upload this PDF!'));
    }
  }
}


/**
 * Upload form Submit handler, adds PDF datastream.
 *
 * Also updates the MODS usage and version for the PDF datastream.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function upei_roblib_scholar_customizations_pdf_upload_form_submit(array $form, array &$form_state) {
  if ($form_state['values']['upload_pdf_checkbox']) {
    $object = islandora_ingest_form_get_object($form_state);
    $file = file_load($form_state['values']['file']);
    $datastream = $object->constructDatastream('PDF', 'M');
    $datastream->label = $file->filename;
    $datastream->mimetype = 'application/pdf';
    $datastream->setContentFromFile($file->uri, FALSE);
    $object->ingestDatastream($datastream);
    module_load_include('inc', 'upei_roblib_scholar_customizations', 'includes/upload.tab');
    upei_roblib_scholar_customizations_add_usage_and_version_elements_to_mods($object, $form_state['values']['usage'], $form_state['values']['version']);
  }
}
