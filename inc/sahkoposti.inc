<?php

//## !!! HUOMHUOM !!! ###
//## T�m� ominaisuus on deprikoitu, k�yt� functions.inc:ss� m��ritelty� pupesoft_sahkoposti -funktiota s�hk�postin l�hett�miseen!! ###

// T�m� ohjelma haluaa $liite muuttujan jossa on tiedostonnimi joka liitet��n
// Ja $kutsu jossa on tiedoston kuvaus
// $content_body on viestin body
// $subject tai $content_subject on poikkeava subject

$varaposti      = '';
$varaposti_trig = FALSE;
if (!isset($sahkoposti_cc)) $sahkoposti_cc = "";
if (!isset($content_body))  $content_body  = "";

if (!isset($subject) or trim($subject) == '') {
  $subject = "$yhtiorow[nimi] - $kutsu";
}

if (isset($content_subject) and trim($content_subject) != "") {
  $subject = $content_subject;
}

if (substr($komento, 0, 12) == 'asiakasemail') {
  $varaposti_trig    = TRUE;
  $varaposti         = $kukarow['eposti'];
  $kukarow['eposti'] = substr($komento, 12);
}

// K�yd��n l�pi liitteet
if (!is_array($liite)) {
  $liite = array(0 => $liite);
  $avaimeksi = 0;
}
else {
  list($avaimeksi) = array_keys($liite);
}

if (!is_array($kutsu)) {
  $kutsu = array($avaimeksi => $kutsu);
}

$mail_liitteet = array();

foreach ($liite as $key => $value) {

  $path_parts = pathinfo($liite[$key]);

  $mail_liitteet[] = array(
    "filename"    => $liite[$key],
    "newfilename" => $kutsu[$key].".{$path_parts["extension"]}",
  );
}

if ($lisaliitteet) {
  foreach ($lisaliitteet as $path => $nimi) {
    $mail_liitteet[] = array("filename" => $path, "newfilename" => $nimi);
  }
}

$content_body_ctype = "TEXT";

if (preg_match("/<[^<]+>/", $content_body)) {
  $content_body_ctype = "HTML";
}

$parametri = array(
  "to"           => $kukarow["eposti"],
  "cc"           => $sahkoposti_cc,
  "subject"      => $subject,
  "ctype"        => $content_body_ctype,
  "body"         => $content_body,
  "attachements" => $mail_liitteet,
);

$boob = pupesoft_sahkoposti($parametri);
if ($boob === FALSE) echo t("S�hk�postin l�hetys ep�onnistui").": $kukarow[eposti]<br>";

//N�� muuttujat voi olla aika isoja joten unsetataan ne
unset($content_body);
unset($content_subject);
unset($ctype);
if (strpos($_SERVER['SCRIPT_NAME'], "tulostakopio.php") === FALSE) unset($komento);
unset($kutsu);
unset($liite);
unset($sahkoposti_cc);
unset($sisalto);
unset($subject);

if ($varaposti_trig) {
  $kukarow['eposti'] = $varaposti;
}
