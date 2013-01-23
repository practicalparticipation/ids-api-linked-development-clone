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
 

/**
 * Method used to retrieve a set of items in the IDS collection (by a 'search' request to the API).
 *
 * @param string $type - type of object(s) requested (documents, [organisations, themes, regions, countries - NOT YET IMPLEMENTED])
 * @param string $set - dataset from where to retrieve the information (eldis, bridge, r4d, etc.). 'all' for any available set.
 * @param string $api_key - IF REQUIRED
 * @param string $format - format of the responses (short, full) (default 'full').
 * @param int $num_requested - the number of results requested (default 0, means all available results).
 * @param int $age_results - only results published in this last number of days are retrieved (default 0, means no limit).
 * @param array $params - unencoded filters to be added to the request (eg. array('publisher' => 'UNDP', 'country' => 'Costa Rica|Nicaragua')).
 * @param array $extra_fields - extra fields to be returned (in short responses). (eg. array('headline', 'category_theme_array')).
 * @param array $order_by - what to order by - defaults to publication date. Can be 'title', 'set'
 * @return response object - response object holding the data retrieved by the API call.
 */
class LinkedDevelopmentWrapper {

    public $graph;
    private $context;

    //Set up a Graphite object when first initialised. 
    //ToDo: error handling if Graphite library not included
    function __construct() {
        $this->graph = new Graphite();
        $this->graph->cacheDir(RDF_CACHE);
    }
    
    //Reset
    private function reset() {
        //ToDo: Empty the graph of any of the defined type of thing when called.
    }

    public function sparql($query) {
        return $this->graph->load(ENDPOINT."?query=".urlencode($query)."&format=".urlencode("application/rdf+xml") );
    }
    
    public function search($type, $set = 'all', $api_key, $format = 'short', $num_requested = 20, $age_results = 0, $params = array(), $extra_fields = array(),$order_by) {    
        
        $query = SPARQL_PREFIX . "\n\n";
        
        //Generate different SPARQL depending on the type of document...
        switch($type) {
            case "documents":
                if($format == 'full') {
                    echo "Full format not yet implemented";
                    //ToDo: implement full format
                } else {
                    $construct = "?article dcterms:title ?title. \n"
                                ."?article rdf:type bibo:Article.\n";
                    $where = "?article a bibo:Article.\n"
                            ."?article dcterms:title ?title. \n";
                }
                
                $this->context = 'documents';
            break;
            default:
                return null; //Better error handling needed here.
        }
        
        //Filter based on requested set. For now we assume that set name is in the URI
        //ToDo: Find better way of defining available sets and then filtering from them?
        //Should we use named graphs? 
        if(!($set == 'all')) {
            $filter = "FILTER regex(?article,'$set')";
        }
        
        //If we have paramters to deal with then we need to add these to our query...
        if(is_array($params)) {
          ksort($params); //Makes sure 'q' parameters are handled last...
          $included = array(); //Set up array to log what is in the query
          
          foreach($params as $parameter => $value) {
              switch($parameter) {
                  case "country":
                        $included['country'] = TRUE;
                        $where .= "?article dcterms:coverage ?countryObj.\n"
                                  ."{?countryObj rdfs:label ?country. } UNION {?countryObj faogeo:nameList ?country. }\n"; 
                        //Country Name
                        $filter .= "\nFILTER regex(?country,'^$value')\n";
                        
                        //ISO 2-digit code
                        
                        
                  break;
                  case "region":
                        $included['region'] = TRUE;
                        $where .= "?article dcterms:coverage ?regionObj.\n"
                                 ."{?regionObj rdfs:label ?region. } UNION {?regionObj faogeo:nameList ?region. }\n";
                        //Region name
                        $filter .= "\nFILTER regex(?region,'^$value')\n";
                        
                        //FAO region identifier
                  break;
                  case "theme":
                        $included['theme'] = TRUE;
                        //Theme name
                        $where .= "?article dcterms:subject ?subjectObj.\n"
                                 ."{?subjectObj rdfs:label ?subject. } UNION {?subjectObj skos:prefLabel ?subject. }";
                        $filter .= "\nFILTER regex(?subject,'^$value','i')"; //Case insensitive for the moment...
                        //Agrovoc ID
                        
                  break;
                  case "keyword":
                    echo "Keywords not yet implemented";                  
                  break;
                  case "q":
                        $where .= "OPTIONAL {?article dcterms:description ?description. }";
                        
                        $filter .= "\n FILTER (regex(?title,'$value','i') || regex(?description,'$value','i'))\n";
                        echo "Full text query not yet implemented";
                  break;
                  default:
                    echo "Unknown parameter: not yet implemented in general API";
                  
              }
              
          }
        }
        
        //Set num requested to 0 to get unlimited results
        if($num_requested) {
            $limit = "LIMIT ".$num_requested;
        }
        
        //Currently the idsapi does not implement an order by, so we just use date... newest first...
        switch($order_by) {
            case "title":
                $order = "ORDER BY ?title";
            break;
            case "set":
                $order = "ORDER BY ?article";
            break;
            default:
                $where .= "?article dcterms:date ?date.";
                $order = "ORDER BY DESC(?date)";
            
        }
        
        $query .= "CONSTRUCT { $construct } WHERE { $where \n $filter } $order $limit";
        
        echo $query;
        
        $this->sparql($query);
        return $this;

    }
    
    /**
    * Return an array with an array of retrieved non-archived object's titles.
    * @return array - titles of retrieved items indexed by object_id.
    */
    function getArrayTitles($add_padding = FALSE, $padding_str = '',$context = null) {
      $array_objects = array();
      if(!$context) {
          $context = $this->context;
      }
      switch($context) {
          case "organisations":
            //ToDo: implement
          break;
          case "themes":
            //ToDo: implement
          break;
          case "regions":
            //ToDo: implement
          break;
          case "countries":
            //ToDo: implement
          break;
          default: //Case is documents
            $items = $this->graph->allOfType("bibo:Article");
            $titleProperty= "dcterms:title";
      }
      
      foreach ($items as $object) {
        //ToDo: Original idsapi code includes a check for whether a document is archived. Consider adding this check for ELDIS
        //Original code includes padding code for when we are fetching concept trees - re-implement with SKOS query?
        /*
        if ((!isset($object->archived)) || ($object->archived !== 'true')) {
          $value = $object->title;
          if ($add_padding) {
            if (isset($object->level)) {
              $level = $object->level;
              $padding = strlen($value) + strlen($padding_str)*($level - 1);
              $value = str_pad($object->title, $padding, $padding_str, STR_PAD_LEFT);
            } 
          }
          */
          
          $array_objects[$object->toString()] = $object->getString($titleProperty);
        //}
      }
      return $array_objects;
    }
    
    
    /**
    * Method used to retrieve one item in the collection (by a 'get' request to the API).
    * @param string $type - type of object(s) requested (assets, documents, organisations, themes, regions, countries).
    * @param string $set - dataset from where to retrieve the information (eldis, bridge).
    * @param string $api_key - IDS API token guid.
    * @param string $format - format of the responses (short, full) (default 'full').
    * @param string $object_id - object_id value of the object to be retrieved - should be a URI in the ldapi...
    * @param string $format - what format should be applied to the return? eldis structure? or fao agris structure?
    * @return formatted response object holding the data retrieved by the API call.
    */
    
    public function get($type = 'documents', $set, $api_key, $format = 'full', $object_id, $format = 'eldis') {
      $query = 'DEFINE sql:describe-mode "SPO"'; //Needed on Virtuoso store...
      $query .= <<<DESCRIBE
          DESCRIBE * WHERE {
             ?article a bibo:Article.
             OPTIONAL {?article dcterms:subject ?subject.}
             OPTIONAL {?article dcterms:coverage ?coverage.}
             OPTIONAL {?article dcterms:creator ?creator.}
             
             FILTER(?article = <$object_id>)
          }
DESCRIBE;
      
      
      $this->sparql($query);
      $resource = $this->graph->resource($object_id);
//      print_r($resource);
      if($format == 'eldis') {
          return $this->eldis_format($type,$resource);
      } else {
          return $resource;
      }
      
    }
    
    //Format a resource according to the eldis format. 
    private function eldis_format($type = 'documents',$resource) {
        switch($type) {
            case 'documents':
                $document = (object) array();
                
                //Authors
                $authors = array();
                foreach($resource->all("dcterms:creator") as $creator) {
                    //Cope with directly addressed, and blank node authors...
                    if($creator->has("foaf:name")) {
                        $authors[] = $creator->get("foaf:name")->toString();
                    } else {
                        $authors[] = $creator->toString();
                    }
                }
                $document->authors = $authors;
                
                //Title
                $document->title = $resource->get("dcterms:title")->toString();

                //URIs
                $uris = array();
                foreach($resource->all("bibo:uri") as $uri) {
                    //Cope with directly addressed, and blank node authors...
                    $uris[] = $uri->toString();
                }
                $document->urls = $uris;
                
                //Publication date
                $document->publication_date = $resource->get("dcterms:date")->toString();
                
                //Abstract
                if($resource->has("dcterms:description")) {
                    $document->description = $resource->get("dcterms:description")->toString();                    
                } else {//Try dcterms:abstract 
                    $document->description = $resource->get("dcterms:abstract")->toString();                                        
                }

                //It is hard right now to get the coverage quite right due to the data model. We need to pull in details on whether something is a country or region...
                foreach($resource->all("dcterms:coverage") as $coverage) {
                        
                }

                
                //It is hard right now to get the coverage quite right due to the data model. We need to pull in details on whether something is a country or region...
                $coverage = array();
                foreach($resource->all("dcterms:coverage") as $location) {
                        $coverage[] = (object) array("name"=>$location->label());
                }
                //ToDo: Work out which is country, which is region. Right now we duplicate
                $document->category_region_array = $coverage;
                $document->country_focus_array = $coverage;

                $subjects = array();
                //ToDo: More advanced information on subjects...
                foreach($resource->all("dcterms:subject") as $subject) {
                    $subjects[] = (object) array("name"=>$subject->label(),"metadata_url"=>$subject->toString());
                }
                $document->category_theme_array = $subjects;
                
                return $document;
            break;
            default:
                print "No default eldis formatter implemented";
        }
    }
    
}
