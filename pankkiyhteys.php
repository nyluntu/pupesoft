<?php

require("inc/parametrit.inc");
require("inc/pankkiyhteys_functions.inc");

echo "<font class='head'>" . t('SEPA-pankkiyhteys') . "</font>";
echo "<hr>";

if (!isset($_SERVER["HTTPS"]) or $_SERVER["HTTPS"] != 'on') {
  echo "<font class='error'>";
  echo t("Voit k�ytt�� pankkiyhteytt� vain salatulla yhteydell�!");
  echo "</font>";
  exit;
}

if (!isset($sepa_pankkiyhteys_token)) {
  echo "<font class='error'>";
  echo t("SEPA-palvelua ei ole aktivoitu.");
  echo "</font>";
  exit;
}

$tee = empty($tee) ? '' : $tee;

// Oikellisuustarkistukset
if ($tee == "laheta" and !($formi_kunnossa = formi_kunnossa())) {
  $tee = "";
}

// Tiliotteiden haku
if ($tee == "laheta" and $hae_tiliotteet == "on") {
  $params = array(
    "tiedostotyyppi"      => "TITO",
    "pankkiyhteys_tunnus" => $pankkiyhteys_tunnus,
    "salasana"            => $salasana
  );

  $tiedostot = sepa_lataa_kaikki_uudet_tiedostot($params);

  if ($tiedostot) {
    viesti("Ladatut tiliotteet:");
    tiedostot_table($tiedostot);
  }
  else {
    viesti("Ladattavia tiliotteita ei ollut saatavilla");
  }
}

// Viitteiden haku
if ($tee == "laheta" and $hae_viitteet == "on") {
  $params = array(
    "tiedostotyyppi"      => "KTL",
    "pankkiyhteys_tunnus" => $pankkiyhteys_tunnus,
    "salasana"            => $salasana
  );

  $tiedostot = sepa_lataa_kaikki_uudet_tiedostot($params);

  if ($tiedostot) {
    viesti("Ladatut viitteet:");
    tiedostot_table($tiedostot);
  }
  else {
    viesti("Ladattavia viitteit� ei ollut saatavilla");
  }
}

// Maksuaineiston oikeellisuustarkistus
if ($tee == "laheta" and $laheta_maksuaineisto == "on") {
  $maksuaineisto = file_get_contents($_FILES["maksuaineisto"]["tmp_name"]);

  if (!$maksuaineisto) {
    virhe("Valitsit maksuaineiston l�hetyksen, mutta et valinnut maksuaineistoa");
    $tee = "";
  }
}

// Maksuaineiston l�hetys
if ($tee == "laheta" and $laheta_maksuaineisto == "on") {
  $pankkiyhteys = hae_pankkiyhteys_ja_pura_salaus($pankkiyhteys_tunnus, $salasana);

  $params = array(
    "bank" => $pankkiyhteys["pankki_lyhyt_nimi"],
    "customer_id" => $pankkiyhteys["customer_id"],
    "target_id" => $pankkiyhteys["target_id"],
    "certificate" => $pankkiyhteys["certificate"],
    "private_key" => $pankkiyhteys["private_key"],
    "file_type" => "NDCORPAYS",
    "maksuaineisto" => $maksuaineisto
  );

  $vastaus = sepa_upload_file($params);

  if ($vastaus) {
    viesti("Maksuaineisto l�hetetty, vastaus pankista:");

    echo "<br/>";

    echo "<table>";
    echo "<tbody>";

    foreach ($vastaus as $key => $value) {
      echo "<tr>";
      echo "<td>{$key}</td>";
      echo "<td>{$value}</td>";
      echo "</tr>";
    }

    echo "</tbody>";
    echo "</table>";
    echo "<br/><br/>";
  }

  $tee = "";
}

if ($formi_kunnossa) {
  $_POST["hae_tiliotteet"] = "";
  $_POST["hae_viitteet"] = "";
  $_POST["laheta_maksuaineisto"] = "";
}

// K�ytt�liittym�
formi();