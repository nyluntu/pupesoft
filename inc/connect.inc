<?php

require "salasanat.php";

// $masterlink on aina linkki master-kantaan.
// $slavelink on aina linkki slave-kantaan jos slave kanta on olemassa.
// $link on linkki master- tai slave-kantaan. Jos slave on k�yt�ss� ja aktivoitu $useslave-muuttujalla niin silloin tuo pointtaa siis slave-kantaan.

// $temporarylink on linkki omaan kantaan johon pupe tekee temporary tablet. Jos $tempdbhost:ia ei oo m��ritelty, niin silloin $temporarylink pointaa master-kantaan.

// Setataan muuttujat joilla voidaan ohjata tiettyjen blokkien pupe_query()-funkkarin l�pi menev�t kyselyt joko masteriin tai slaveen.
$pupe_masterdb = 0;
$pupe_slavedb  = 0;

// Katsotaan halutaanko k�ytt�� slavea
if (isset($useslave)) {
  $_useslave = (int) $useslave;
}
else {
  $_useslave = 0;
}

// Default, ei k�ytet� slavea
$useslave = 0;

// Otetaan isoin mahdollinen slave k�ytt��n
for ($_slave_i = $_useslave; $_slave_i > 0; $_slave_i--) {
  if (!empty($slavedb[$_slave_i])) {
    $useslave = $_slave_i;
    break;
  }
}

// Jos Pupeen on konffattu oma database temporary tableja varten
if (isset($tempdbhost) and $tempdbhost != "") {
  $temporarylink = mysql_connect($tempdbhost, $tempdbuser, $tempdbpass) or die ("Ongelma tietokantapalvelimessa $tempdbhost");
  mysql_select_db($tempdbkanta, $temporarylink) or die ("Tietokantaa $tempdbkanta ei l�ydy palvelimelta $tempdbhost! (connect.inc)");
  mysql_set_charset("latin1", $temporarylink);
  mysql_query("set group_concat_max_len=1000000", $temporarylink);
}

// K�ytet��n slave kantaa
if ($useslave > 0) {
  // Otetaan yhteys my�s master-kantaan koska tietyiss� paikoissa t�t� tarvitaan
  $masterlink = mysql_connect($dbhost, $dbuser, $dbpass) or die ("Ongelma tietokantapalvelimessa $dbhost");
  mysql_select_db($dbkanta, $masterlink) or die ("Tietokantaa $dbkanta ei l�ydy palvelimelta $dbhost! (connect.inc)");
  mysql_set_charset("latin1", $masterlink);
  mysql_query("set group_concat_max_len=1000000", $masterlink);

  $link = mysql_connect($slavedb[$useslave], $slaveuser[$useslave], $slavepass[$useslave]) or die ("Ongelma tietokantapalvelimessa: '$slavedb[$useslave]'");
  $slavelink = $link;
}
// K�ytet��n master kantaa
else {
  // Onko slave olemassa? Jos on, niin otetaan linkki siihenkin
  if (isset($slavedb[1]) and $slavedb[1] != "" and isset($slaveuser[1]) and $slaveuser[1] != "" and isset($slavepass[1]) and $slavepass[1] != "") {
    $slavelink = mysql_connect($slavedb[1], $slaveuser[1], $slavepass[1]) or die ("Ongelma tietokantapalvelimessa: '$slavedb[1]'");
    mysql_select_db($dbkanta, $slavelink) or die ("Tietokantaa $dbkanta ei l�ydy palvelimelta $slavedb[1]! (connect.inc)");
    mysql_set_charset("latin1", $slavelink);
    mysql_query("set group_concat_max_len=1000000", $slavelink);
  }

  $link = mysql_connect($dbhost, $dbuser, $dbpass) or die ("Ongelma tietokantapalvelimessa $dbhost");
  $masterlink = $link;
}

mysql_select_db($dbkanta, $link) or die ("Tietokantaa $dbkanta ei l�ydy palvelimelta $dbhost! (connect.inc)");
mysql_set_charset("latin1", $link);
mysql_query("set group_concat_max_len=1000000", $link);

// jos debugataan pienill� arvoilla, ni pakotetaan my�s queryt kuntoon!
if (isset($GLOBALS["pupe_query_debug"]) and $GLOBALS["pupe_query_debug"] > 0 and $GLOBALS["pupe_query_debug"] < 1) {
  mysql_query("set session sql_mode = 'STRICT_ALL_TABLES,ONLY_FULL_GROUP_BY,NO_ZERO_IN_DATE'", $link); // NO_ZERO_DATE ois my�s kova!
}

if (!isset($temporarylink)) $temporarylink = $masterlink;

$pupe_resque = FALSE;

// Otetaan php-resque k�ytt��n jos se on asennettu
if (@include_once "php-resque/lib/Resque.php") {
  $pupe_resque = TRUE;
}
