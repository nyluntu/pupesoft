<?php

/**
 * L�hett�� parametrina annetun viestin sms-viestin� parametrina annetun asiakkaan gms-numeroon,
 * jos sellainen l�ytyy. Toistaiseksi ainut tuettu palvelu on Zoner. L�hett�� my�s s�hk�postin, jos
 * asiakkaalta sellainen l�ytyy
 *
 * @param string  $username     Zonerin k�ytt�j�tunnus
 * @param string  $salasana     Zonerin salasana
 * @param int     $tilausnumero
 * @param bool    $kerattu      Onko kaikki tilauksen rivit jo ker�tty
 *
 * @return bool Jos tekstiviestin tai s�hk�postin l�hetys onnistui, palauttaa true
 */


function laheta_vahvistusviesti($username, $salasana, $tilausnumero, $kerattu = false) {
  global $yhtiorow;

  $lahetetty = false;

  $tilaus  = hae_tilaus($tilausnumero);
  $asiakas = hae_asiakas_vahvistusta_varten($tilaus['liitostunnus']);

  if (loytyy_jt_riveja($tilausnumero)) {
    $otsikko = t("Tilaus noudettavissa");

    $viesti =
      t("Kaikki tilauksen %s osat ovat noudettavissa, terveisin", "", $tilausnumero) .
      " " .
      $yhtiorow["nimi"];
  }
  elseif (loytyy_tyomaarays($tilausnumero) or $kerattu) {
    $otsikko = t("Huoltoty� valmis");
    $viesti  = t("Huoltoty�si on valmis, terveisin") . " " . $yhtiorow["nimi"];
  }

  if (empty($viesti)) {
    return false;
  }

  $gsm   = empty($tilaus["toim_puh"]) ? $asiakas["gsm"] : $tilaus["toim_puh"];
  $email = empty($tilaus["toim_email"]) ? $asiakas["email"] : $tilaus["toim_email"];

  if (!empty($gsm)) {
    $parameters = array(
      "method"   => "POST",
      "data"     => array(
        "username"   => $username,
        "password"   => $salasana,
        "numberto"   => $gsm,
        "numberfrom" => $yhtiorow['nimi'],
        "message"    => $viesti
      ),
      "url"      => "https://sms.zoner.fi/sms.php",
      "headers"  => array(),
      "posttype" => "form"
    );

    $vastaus = pupesoft_rest($parameters);
    $vastaus = $vastaus[1];
    $vastaus = explode(" ", $vastaus);
    $vastaus = $vastaus[0];

    if ($vastaus == "OK") {
      $lahetetty = true;
    }
  }

  if (!empty($email)) {
    $parametrit = array(
      "to"      => $email,
      "subject" => $otsikko,
      "body"    => $viesti
    );

    if (pupesoft_sahkoposti($parametrit)) {
      $lahetetty = true;
    }
  }

  return $lahetetty;
}

function hae_asiakas_vahvistusta_varten($tunnus) {
  global $kukarow;

  $query  = "SELECT gsm,
             email
             FROM  asiakas
             WHERE yhtio  = '{$kukarow['yhtio']}'
             AND   tunnus = '{$tunnus}'";
  $result = pupe_query($query);

  $asiakas = mysql_fetch_assoc($result);

  return $asiakas;
}

function hae_tilaus($tilausnumero) {
  global $kukarow;

  $query  = "SELECT clearing,
             liitostunnus,
             toim_puh,
             toim_email
             FROM  lasku
             WHERE yhtio  = '{$kukarow['yhtio']}'
             AND   tunnus = '{$tilausnumero}'";
  $result = pupe_query($query);

  return mysql_fetch_assoc($result);
}

function loytyy_tyomaarays($tilausnumero) {
  global $kukarow;

  $query  = "SELECT COUNT(*) AS maara
             FROM  tyomaarays
             WHERE yhtio   = '{$kukarow['yhtio']}'
             AND   otunnus = '{$tilausnumero}'";
  $result = pupe_query($query);
  $result = mysql_fetch_assoc($result);

  return (int) $result["maara"] > 0;
}

function loytyy_jt_riveja($tilausnumero) {
  global $kukarow;

  $query = "SELECT tilausrivin_lisatiedot.vanha_otunnus
            FROM tilausrivi
            INNER JOIN tilausrivin_lisatiedot ON (tilausrivin_lisatiedot.yhtio = tilausrivi.yhtio
              AND tilausrivi.tunnus = tilausrivin_lisatiedot.tilausrivitunnus)
            WHERE tilausrivi.yhtio  = '{$kukarow['yhtio']}'
            AND tilausrivi.otunnus  = '{$tilausnumero}'";

  $result = pupe_query($query);

  while ($rivi = mysql_fetch_assoc($result)) {
    $query = "SELECT count(*) AS jt_rivien_maara
              FROM tilausrivi
              WHERE yhtio = '{$kukarow['yhtio']}'
              AND otunnus = '{$rivi['vanha_otunnus']}'
              AND     var = 'J'";

    $result2 = pupe_query($query);
    $result2 = mysql_fetch_assoc($result2);

    if ((int) $result2['jt_rivien_maara'] > 0) {
      return true;
    }
  }

  return false;
}