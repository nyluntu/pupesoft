<?php

if (!function_exists("vaihtoehtoiset_verkkolaskutunnuksettarkista")) {

  function vaihtoehtoiset_verkkolaskutunnuksettarkista(&$t, $i, $result, $tunnus, &$virhe, $trow) {
    global $kukarow, $yhtiorow, $alias_set, $alasveto, $asiakasid;
    static $tem_kohdesarake;

    if (mysql_field_name($result, $i) == "toimi_tunnus") {
      $toimittaja = hae_toimittaja($t[$i]);

      if (empty($toimittaja)) {
        $virhe[$i] = t("Toimittaja puuttuu tai sit� ei l�ydy");
      }
    }

    if (mysql_field_name($result, $i) == "kohde_sarake") {
      $kohde_sarakkeet = array(
        'nimi',
        'ytunnus',
        'ovt_tunnus',
      );

      if (!in_array($t[$i], $kohde_sarakkeet)) {
        $virhe[$i] = t("Kohde sarake ei ole sallittujen listalla").'(nimi, ytunnus, ovt_tunnus)';
      }

      $tem_kohdesarake = $t[$i];
    }

    if (mysql_field_name($result, $i) == "arvo") {

      if (empty($t[$i])) {
        $virhe[$i] = t("Arvo ei voi olla tyhj�");
      }
      else {
        //tarkistetaan, ett� kyseist� arvoa ei ole jo kannassa
        $query = "SELECT *
                  FROM vaihtoehtoiset_verkkolaskutunnukset
                  WHERE yhtio      = '{$kukarow['yhtio']}'
                  AND kohde_sarake = '$tem_kohdesarake'
                  AND arvo         = '{$t[$i]}'";
        $arvo_result = pupe_query($query);
        $vaihtoehtoinen_verkkolaskutunnus = mysql_fetch_assoc($arvo_result);

        if (!empty($vaihtoehtoinen_verkkolaskutunnus) and $vaihtoehtoinen_verkkolaskutunnus['toimi_tunnus'] != $t['toimi_tunnus']) {
          $virhe[$i] = t("Arvo l�ytyy jo toiselta toimittajalta tietokannasta");
        }
      }
    }
  }
}
