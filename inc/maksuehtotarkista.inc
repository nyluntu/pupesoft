<?php

if (!function_exists("maksuehtotarkista")) {
  function maksuehtotarkista(&$t, $i, $result, $tunnus, &$virhe, $trow) {
    global $kukarow, $yhtiorow, $alias_set;

    static $prossa, $moniehto, $kassa_txt, $kassa_pros;
    if (mysql_field_name($result, $i) == "summanjakoprososa2") {

      $prossa = (int) $t[$i];

      if ($yhtiorow["maksuehto_tuotenumero"] == "" and $prossa != 0) {
        $virhe[$i] = t("Et voi tehd� moniehto-maksuehtoa koska yhti�lt� puuttuu maksuehto_tuotenumero!");
      }
      if ($prossa!= 0) {
        $moniehto = 1;
      }
    }

    if (mysql_field_name($result, $i) == "jv") {
      if (trim($t[$i]) != "" and $moniehto != 0) {
        $virhe[$i] = t("Et voi tehd� t�st� jv-maksuehtoa koska t�m� on moniehto-maksuehto!");
      }
    }

    if (mysql_field_name($result, $i) == "kateinen") {
      if (trim($t[$i]) != "" and $moniehto != 0) {
        $virhe[$i] = t("Et voi tehd� t�st� k�teismaksuehtoa koska t�m� on moniehto-maksuehto!");
      }
    }
  }
}
