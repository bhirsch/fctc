<?php
/**
 * @file fctc.pages.inc
 * 
 * Callbacks and theme functions for pages declared by hook_menu.
 */

//---------------------------------------------------------/
// path fctc/tracking/in-transit                           /
//---------------------------------------------------------/
/**
 * Callback from fctc/tracking/in-transit.
 * 
 * List IDs of shipments currently in transit with links
 * to tracing screen.
 * 
 * @return $build
 *  Renderable array.
 */
function fctc_tracking_in_transit() {
  $build = array();
  $build = array('#theme' => 'fctc_tracking_in_transit');

  // Get shipments in transit. That is, shipments that have been sent or received, but
  // have not arrived at their final destination.
  $data = fctc_get_data();
  $in_transit = array();
  foreach ($data as $shipment) {
    if (isset($shipment['id']) && ($shipment['status'] == 0 
                                   || empty($shipment['status']))) {
      $id = $shipment['id'];
      $date = $shipment['date'];
      // Drupal path to trace shipment.
      $trace_path = 'fctc/tracing/' . $id;
      // Shipments may be sent and received multiple times en route to final
      // destination. We only want the first shipment sent date.
      if (!isset($transit[$id]) || empty($shipment['date sent'])) {
        $in_transit[$id] = array(
          'id' => $id, 
          'date sent' => $date, 
          'trace path' => $trace_path
        );
      }
      else if ($date < $in_transit[$id]['date sent']){
        $in_transit[$id]['date sent'] = $date;
      }
    }
  }
  $build['in transit'] = $in_transit;

  return $build;
}

/**
 * @param $element
 *  Render array.
 * 
 * @return 
 *  HTML
 */
function theme_fctc_tracking_in_transit($element) {
  $header = array(t('#'), t('Shipment ID'), t('First Ship Date') );
  $rows = array();
  $in_transit = $element['tracking data']['in transit'];

  // Order by date. 
  $order = array();
  foreach ($in_transit as $shipment) {
    $order[$shipment['id']] = $shipment['date sent'];  
  }
  arsort($order);

  $i = 0;
  foreach ($order as $id => $date) {
    $i++;
    $rows[] = array(
      // Cell 1.
      $i,
      // Cell 2.
      array('data' => l($id, $in_transit[$id]['trace path'])),
      // Cell 3. 
      array('data' => date('j-n-Y', $in_transit[$id]['date sent'])),
    );
  }

  $variables['header'] = $header;
  $variables['rows'] = $rows;
  $output = theme('table', $variables);

  return $output;
}

//---------------------------------------------------------/
// fctc/tracing                                            /
//---------------------------------------------------------/

/**
 * @return $form
 *  Form array.
 */
function fctc_trace($form, $form_state) {
  if ($id = arg(2)) {
    $data = fctc_get_data();
  }
  else {
    $id = '';  
  }
  // Basic ID look-up form.
  $form['fctc_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Unique Identifier'),
    '#description' => t("Search for a shipment by entering its unique "
                      . "identifier here."),
    '#default_value' =>  $id,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Search'),
  );
  // Display data.
  if (isset($data)) {
    $i = 0;
    foreach ($data as $url => $shipment) {
      if (isset($shipment['id']) && $shipment['id'] == $id) {
        $key = $id . '-' . $i;
        $url = url($url);
        $text = t('Information about shipment !id is available here: !here', 
          array(
            '!id' => l($id, $url),
            '!here' => l($url, $url),
          ) 
        );
        $form[$key] = array(
          '#type' => 'item',
          '#title' => $text,
        );
        $i++;
      }
    }
  }

  return $form;
}

/**
 * Submit handler for fctc_tracing_form.
 */
function fctc_trace_submit($form, &$form_state) {
  $id = check_plain($form_state['values']['fctc_id']);
  $path = "fctc/tracing/$id";
  $form_state['redirect'] = array($path);

}
