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
// fctc/tracking/sent-received                             /
//---------------------------------------------------------/
/**
 * Callback for fctc/tracking/sent-received.
 * 
 * @return $build
 *  Render array.
 *
 * @todo This relies of data being cached in a local 
 *   sparql endpoint. If prototype requires larger scale or higher
 *   volume of data, make this configureable to point to a remote 
 *   sparql endpoint.
 */
function fctc_tracking_sent_received_page() {
  global $base_url; 
  $endpoint = $base_url . '/sparql';
  $build = array('#theme' => 'fctc_tracking_sent_received');

  // Total number of shipments sent.
  // Count the total number of unique identifiers, not the number of shipments
  // sent. Otherwise we will double count shipments every time they are sent
  // while they are still legitimately in transit.
  $query = 'SELECT DISTINCT ?p ?o WHERE {
             GRAPH ?g { ?s ?p ?o . } . FILTER regex(?p, "id")
           }';
  $sparql_results = sparql_request($query, $endpoint);
  $total_sent = count($sparql_results['result']['rows']);
  $build['total_sent'] = $total_sent;

  // Total number of shipments received in final destination. 
  $query = "SELECT * WHERE {
              GRAPH ?g { ?s ?p ?o . } 
              . FILTER regex(?p, 'status') 
              . FILTER regex(?o, '1')
            }";
  $sparql_results = sparql_request($query, $endpoint);
  $total_received = count($sparql_results['result']['rows']);
  $build['total_received'] = $total_received;

  // Report bad data. Every shipment can be sent and received multiple times
  // en route to its final destination. But every shipment only has one 
  // final destination. Check to see if any IDs are reported as arriving at
  // a final destination more than once.
  // @TODO

  return $build;
}

/**
 * Callback for theming tracking/sent-received page.
 */
function theme_fctc_tracking_sent_received($elements) {
  $output = '';
  $tracking_data = $elements['tracking data'];
  
  $shipments_sent = t('Shipments Sent');
  $shipments_received = t('Shipments Received');
  $total_sent = $tracking_data['total_sent'];
  $total_received = $tracking_data['total_received'];

  $output .= "<h1>$shipments_sent: $total_sent</h1>";
  $output .= "<h1>$shipments_received: $total_received</h1>";
  
  return $output;
}

//---------------------------------------------------------/
// fctc/tracing                                            /
//---------------------------------------------------------/

/**
 * @return $form
 *  Form array.
 */
function fctc_tracing_form($form, $form_state) {
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
function fctc_tracing_form_submit($form, &$form_state) {
  $id = check_plain($form_state['values']['fctc_id']);
  $path = "fctc/tracing/$id";
  $form_state['redirect'] = array($path);

}

//------------------------------------------------------/
// Helper functions                                     /
//------------------------------------------------------/
/**
 * Get all cached RDF data and store it in one big array.
 * Use this for processing data for track and trace examples.
 *
 * Make these values: 
 *   s, subject, e.g. http://localhost/countryx/node/20
 *   p, predicate, e.g. http://this.doesnt.really.get.checked.com/id
 *   o, object, e.g. 12345
 * Look like this:  
 *   $data['subject']['predicate'] = 'object';
 * 
 * @todo This is an RDF/SPARQL hack but it will make prototyping a lot faster
 * without getting bogged down in SPARQL syntax.
 * 
 * @return $data
 *  Array of sanitized shipment data in the following format:  
 *  $data['subject']['predicate'] = 'object';
 */
function fctc_get_data($short_predicate = TRUE) {
  // Get RDF data from SPARQL endpoint.
  global $base_url; 
  $endpoint = $base_url . '/sparql';
  $query = 'SELECT DISTINCT * WHERE { '
         . '  GRAPH ?g { ?s ?p ?o . } '
         . '}';
  $sparql_results = sparql_request($query, $endpoint);

  // Convert RDF/SPARQL data to an array.
  $data = array();
  foreach ($sparql_results['result']['rows'] as $row) {
    // Sanitize data. 
    $subject = check_plain($row['s']);
    $predicate = check_plain($row['p']);
    $object = check_plain($row['o']);
    // Shorten predicate from this: http://this.doesnt.really.get.checked.com/id
    // to this: id.
    if ($short_predicate) {
      $position = strripos($predicate, '/') + 1;
      $predicate = substr($predicate, $position);
    }
    $data[$subject][$predicate] = $object;
  }

  return $data;
}
