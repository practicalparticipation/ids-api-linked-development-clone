<?php
/*
 * @author Tim Davies <tim@practicalparticipation.co.uk>
 * @created 23 Jan 2013
 * @version 0.1
 *
 * This is a very rough-and-ready example of how a wrapper around the IDS API (http://api.ids.ac.uk/) can be re-implemented using
 * SPARQL against a virtuoso triple store, drawing on data stored using the AGRIS data model.
 *
 * Currently implemented are the search and get functions.
 *
 * Other functions to be implemented as required...
 *
 * 
 * 
 */

/*
 * Step 1: Include the config, Graphite and ARC (required by Graphite)
 */ 
$include_prefix = '';
include($include_prefix . 'config.inc');
include($include_prefix . 'ldapi.wrapper.inc');
include_once($include_prefix . 'arc/ARC2.php');
include($include_prefix . 'Graphite.php');

//Step 2: Instantiate a copy of the API object
$ldapi = new LinkedDevelopmentWrapper();

/*
 * Step 3: Search for documents, you can currently add filters for:
 * q = - free text query (description and title)
 * country =
 * region =
 * theme =
 *
 * The second paramter selects whether we are searching eldis, r4d, or all available sets of data.
 *
 */
$response = $ldapi->search('documents', 'all',null, 'short',10,0, array('q'=>'Kenya'),null,'date');

// Once we have articles, grab an array of them, with IDs and titles.
$articles = $response->getArrayTitles();



// Now loop through and fetch full details on these from the server...
foreach($articles as $id => $title) {
    $article = $ldapi->get('documents','all',null,full,$id,'eldis');
    print_r($article);
}










?>