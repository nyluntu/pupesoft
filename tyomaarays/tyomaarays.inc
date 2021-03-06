<?php

if ($tee ==  "VALMIS" or $tee == "LEPAA") {
  //tulostetaan tilaus kun se on valmis
  $otunnus = $laskurow["tunnus"];

  if ($toim != "REKLAMAATIO") {
    $tulostimet[0] = "Ty�m��r�ys";
    require_once "tyomaarays/tulosta_tyomaarays.inc";
  }
  else {
    if ((isset($tilaustyyppi) and $tilaustyyppi == 'U') or $laskurow['tilaustyyppi'] == 'U') {
      // Katsotaan onko avainsanoihin m��ritelty varaston toimipaikan l�heteprintteri� takuul�hetett� varten laajassa toimipaikkak�sittelyss�
      $onkolaajattoimipaikat = ($yhtiorow['toimipaikkakasittely'] == "L" and $toimipaikkares = hae_yhtion_toimipaikat($kukarow['yhtio']) and mysql_num_rows($toimipaikkares) > 0) ? TRUE : FALSE;

      if (!empty($laskurow['yhtio_toimipaikka']) and $onkolaajattoimipaikat) {
        $avainsana_where = " and avainsana.selite       = '{$laskurow['varasto']}'
                             and avainsana.selitetark   = '{$laskurow['yhtio_toimipaikka']}'
                             and avainsana.selitetark_2 = 'printteri1'";

        $tp_tulostin = t_avainsana("VARTOIMTULOSTIN", '', $avainsana_where, '', '', "selitetark_3");

        // k�sinvalittu printteri
        if (!empty($tp_tulostin)) {
          $apuprintteri = $valittu_tulostin = $tp_tulostin;
          $kappaleet = "";
          //haetaan l�hetteen tulostuskomento
          $query = "SELECT komento
                    FROM kirjoittimet
                    WHERE yhtio = '$kukarow[yhtio]'
                    AND tunnus  = '$apuprintteri'";
          $kirres = pupe_query($query);
          $kirrow = mysql_fetch_assoc($kirres);

          $komento = $kirrow['komento'];
        }
        else {
          $tulostimet[0] = "Takuu";
        }
      }
      else {
        $tulostimet[0] = "Takuu";
      }
    }
    else {
      $tulostimet[0] = "Reklamaatio";
    }

    require_once "tilauskasittely/tulosta_lahete_kerayslista.inc";
  }

  if (count($komento) == 0) {

    if (strtolower($tulostimet[0]) == 'takuu' and (isset($tilaustyyppi) and $tilaustyyppi == 'U') or $laskurow['tilaustyyppi'] == 'U') {

      $tilaustyyppi = 'U';

      echo "<font class='head'>", t("Takuu"), ":</font><hr>";
    }
    else {
      echo "<font class='head'>", t($tulostimet[0]), ":</font><hr>";
    }

    require "inc/valitse_tulostin.inc";
  }
  elseif ($kappaleet > 0) {
    if ($toim != "REKLAMAATIO" and $komento["Ty�m��r�ys"] != "email" and $tyomtyyppi != 'Z') {
      $komento["Ty�m��r�ys"] .= " -# $kappaleet ";
    }
    elseif (isset($komento["Reklamaatio"]) and $komento["Reklamaatio"] != "email") {
      $komento["Reklamaatio"] .= " -# $kappaleet ";
    }
    elseif (isset($komento["Takuu"]) and $komento["Takuu"] != "email") {
      $komento["Takuu"] .= " -# $kappaleet ";
    }
  }

  if ($laskurow["tila"] == 'U') {
    $where = " uusiotunnus='$laskurow[tunnus]' ";
  }
  else {
    $where = " otunnus='$laskurow[tunnus]' ";
  }

  // Haetaan asiakkaan tiedot
  $query = "SELECT luokka,
            puhelin,
            if (asiakasnro!='', asiakasnro, ytunnus) asiakasnro,
            asiakasnro as asiakasnro_aito,
            kerayserat,
            kieli
            FROM asiakas
            WHERE tunnus='$laskurow[liitostunnus]' and yhtio='$kukarow[yhtio]'";
  $result = pupe_query($query);
  $asrow = mysql_fetch_array($result);

  // katotaan miten halutaan sortattavan
  if ($toim == "REKLAMAATIO") {
    $sorttauskentta = generoi_sorttauskentta($yhtiorow["kerayslistan_jarjestys"]);
    $order_sorttaus = $yhtiorow["kerayslistan_jarjestys_suunta"];

    if ($yhtiorow["kerayslistan_palvelutjatuottet"] == "E") $pjat_sortlisa = "tuotetyyppi,";
    else $pjat_sortlisa = "";

  }
  else {
    $sorttauskentta = generoi_sorttauskentta($yhtiorow["tyomaarayksen_jarjestys"]);
    $order_sorttaus = $yhtiorow["tyomaarayksen_jarjestys_suunta"];

    if ($yhtiorow["tyomaarayksen_palvelutjatuottet"] == "E") $pjat_sortlisa = "tuotetyyppi,";
    else $pjat_sortlisa = "";
  }

  $query_ale_lisa = generoi_alekentta('M');

  //ty�m��r�yksen rivit
  $query = "SELECT tilausrivi.*,
            round(tilausrivi.hinta * (tilausrivi.varattu+tilausrivi.jt+tilausrivi.kpl) * {$query_ale_lisa},'$yhtiorow[hintapyoristys]') rivihinta,
            tuote.sarjanumeroseuranta,
            $sorttauskentta,
            if (tuote.tuotetyyppi='K','2 Ty�t','1 Muut') tuotetyyppi,
            if (tuote.myyntihinta_maara=0, 1, tuote.myyntihinta_maara) myyntihinta_maara,
            tuote.sarjanumeroseuranta,
            tuote.eankoodi,
            abs(tilausrivin_lisatiedot.asiakkaan_positio) asiakkaan_positio
            FROM tilausrivi
            LEFT JOIN tilausrivin_lisatiedot ON tilausrivi.yhtio = tilausrivin_lisatiedot.yhtio and tilausrivi.tunnus = tilausrivin_lisatiedot.tilausrivitunnus
            JOIN tuote ON tilausrivi.yhtio = tuote.yhtio and tilausrivi.tuoteno = tuote.tuoteno
            JOIN lasku ON tilausrivi.yhtio = lasku.yhtio and tilausrivi.otunnus = lasku.tunnus
            WHERE tilausrivi.otunnus  = '$otunnus'
            and tilausrivi.yhtio      = '$kukarow[yhtio]'
            and tilausrivi.tyyppi    != 'D'
            and tilausrivi.yhtio      = tuote.yhtio
            and tilausrivi.tuoteno    = tuote.tuoteno
            ORDER BY $pjat_sortlisa sorttauskentta $order_sorttaus, tilausrivi.tunnus";
  $riresult = pupe_query($query);

  $tilausnumeroita = $otunnus;

  //generoidaan rivinumerot
  $rivinumerot = array();

  $kal = 1;

  while ($row = mysql_fetch_array($riresult)) {
    $rivinumerot[$row["tunnus"]] = $kal;
    $kal++;
  }

  mysql_data_seek($riresult, 0);

  // Aloitellaan l�hetteen teko
  if ($toim != "REKLAMAATIO") {

    if ($tyomtyyppi == 'Z') {
      tulosta_tyomaaraystarra_zebra($laskurow, $komento[$tulostimet[0]], $kappaleet);
    }
    else {
      if ($toim == "SIIRTOTYOMAARAYS") {
        $tyyppi = "SISAINEN";
      }
      elseif ((isset($tyomtyyppi) and $tyomtyyppi == "O") or $kukarow['hinnat'] != 0) {
        $tyyppi = "O";
      }
      elseif (isset($tyomtyyppi) and $tyomtyyppi == "P") {
        $tyyppi = "P";
      }
      elseif (isset($tyomtyyppi) and $tyomtyyppi == "A") {
        $tyyppi = "";
      }
      elseif (isset($tyomtyyppi) and $tyomtyyppi == "Q") {
        $tyyppi = "Q";
      }
      else {
        $tyyppi = $yhtiorow["tyomaaraystyyppi"];
      }

      $params_tyomaarays = array( "asrow"           => $asrow,
        "boldi"           => $boldi,
        "edtuotetyyppi"   => "",
        "iso"             => $iso,
        "kala"            => 0,
        "kieli"           => $kieli,
        "komento"      => $komento[$tulostimet[0]],
        "laskurow"        => $laskurow,
        "lineparam"       => $lineparam,
        "norm"            => $norm,
        "page"            => NULL,
        "pdf"             => NULL,
        "perheid"         => 0,
        "perheid2"        => 0,
        "pieni"           => $pieni,
        "pieni_boldi"     => $pieni_boldi,
        "rectparam"       => $rectparam,
        "returnvalue"     => 0,
        "rivinkorkeus"    => $rivinkorkeus,
        "rivinumerot"     => $rivinumerot,
        "row"             => NULL,
        "sivu"            => 1,
        "tee"             => $tee,
        "thispage"      => NULL,
        "toim"            => $toim,
        "tots"        => 0,
        "tyyppi"          => $tyyppi, );

      // Aloitellaan lomakkeen teko
      $params_tyomaarays = tyomaarays_alku($params_tyomaarays);

      if ($yhtiorow["tyomaarayksen_palvelutjatuottet"] == "") {
        // Ekan sivun otsikot
        $params_tyomaarays['kala'] -= $params_tyomaarays['rivinkorkeus']*3;
        $params_tyomaarays = tyomaarays_rivi_otsikot($params_tyomaarays);
      }

      while ($row = mysql_fetch_assoc($riresult)) {
        $params_tyomaarays["row"] = $row;
        $params_tyomaarays = tyomaarays_rivi($params_tyomaarays);
      }

      if (($toim == 'TYOMAARAYS_ASENTAJA' or $toim == 'TYOMAARAYS') and $yhtiorow['tyomaarays_tulostus_lisarivit'] == 'L') {
        $params_tyomaarays["tots"] = 1;
        $params_tyomaarays = tyomaarays_loppu_lisarivit($params_tyomaarays);
      }
      else {
        $params_tyomaarays["tots"] = 1;
        $params_tyomaarays = tyomaarays_loppu($params_tyomaarays);
      }

      //tulostetaan sivu
      tyomaarays_print_pdf($params_tyomaarays);
    }
  }
  else {

    if (strtolower($tulostimet[0]) == 'takuu') {
      $tyyppi = "TAKUU";
    }
    else {
      $tyyppi = "REKLAMAATIO";
    }

    $params_kerayslista = array(
      'asrow'             => $asrow,
      'boldi'             => $boldi,
      'iso'               => $iso,
      'kala'              => 0,
      'kieli'             => $kieli,
      'komento'      => '',
      'laskurow'          => $laskurow,
      'norm'              => $norm,
      'page'              => '',
      'paino'             => 0,
      'pdf'               => NULL,
      'perheid'           => 0,
      'perheid2'          => 0,
      'pieni'             => $pieni,
      'pieni_boldi'       => $pieni_boldi,
      'rectparam'         => $rectparam,
      'rivinkorkeus'      => $rivinkorkeus,
      'rivinumerot'      => $rivinumerot,
      'row'               => NULL,
      'sivu'              => 1,
      'tee'               => $tee,
      'thispage'      => NULL,
      'tilausnumeroita'   => $tilausnumeroita,
      'toim'              => $toim,
      'tots'              => 0,
      'tyyppi'        => $tyyppi,
      'kerayslistatyyppi'  => '');


    // Aloitellaan ker�yslistan teko
    $params_kerayslista = alku_kerayslista($params_kerayslista);

    while ($row = mysql_fetch_assoc($riresult)) {
      $params_kerayslista["row"] = $row;
      $params_kerayslista = rivi_kerayslista($params_kerayslista);
    }

    $params_kerayslista["tots"] = 1;
    $params_kerayslista = loppu_kerayslista($params_kerayslista);

    //tulostetaan sivu
    $params_kerayslista["komento"] = $komento[$tulostimet[0]];
    print_pdf_kerayslista($params_kerayslista);
  }
}

if (($toim == "TYOMAARAYS" or $toim == "TYOMAARAYS_ASENTAJA" or $toim == "REKLAMAATIO") and $tee == 'LEPAA') {
  $query = "UPDATE kuka SET kesken=0 WHERE session='$session'";
  $result = pupe_query($query);

  $tee        = '';
  $tilausnumero    = '';
  $laskurow      = '';
  $kukarow['kesken']  = '';

  if ($lopetus != '') {
    lopetus($lopetus, "META");
  }
}

if (($toim == "TYOMAARAYS" or $toim == "TYOMAARAYS_ASENTAJA") and $tee == 'VALMIS') {
  $query  = "UPDATE lasku
             SET tila='L'
             WHERE tunnus='$laskurow[tunnus]'
             and tila='A'
             and yhtio='$kukarow[yhtio]'";
  $result = pupe_query($query);

  //Katotaan l�ytyyk� tilausrivej�
  $query = "SELECT *
            from tilausrivi
            where yhtio  = '$kukarow[yhtio]'
            and otunnus  = '$laskurow[tunnus]'
            and tyyppi  != 'D'";
  $keres = pupe_query($query);

  //Yht��n rivi� ei ole
  if (mysql_num_rows($keres) == 0) {
    $query = "UPDATE lasku SET alatila='X' where yhtio='$kukarow[yhtio]' and tunnus='$kukarow[kesken]'";
    $result = pupe_query($query);

    // tilaus ei en�� kesken...
    $query  = "UPDATE kuka set kesken=0 where yhtio='$kukarow[yhtio]' and kuka='$kukarow[kuka]'";
    $result = pupe_query($query);
  }
  else {
    require "tilauskasittely/tilaus-valmis.inc";
  }

  // L�hetet��n tekstiviesti asiakkaalle ty�m��r�yksen valmistumisesta jo t�ss� vaiheessa,
  // jos ker�tt�vi� tuotteita ei ole, muutoin viestin l�hetys tapahtuu manuaalisesti
  $ei_kerattavia_riveja = (isset($kerattavia) and $kerattavia == 0);

  if ($yhtiorow['vahvistusviesti_asiakkaalle'] == "Y" and $ei_kerattavia_riveja) {
    require "inc/jt_ja_tyomaarays_valmis_viesti.inc";
    laheta_vahvistusviesti($zoner_tunnarit["username"],
      $zoner_tunnarit["salasana"],
      $tilausnumero,
      true);
  }

  $tee = "";
}

if ($toim == "REKLAMAATIO" and $tee == 'VALMIS') {

  if (!tee_palautustilaus($laskurow)) {
    $query = "UPDATE lasku SET alatila='X' where yhtio='$kukarow[yhtio]' and tunnus='$kukarow[kesken]'";
    $result = pupe_query($query);

    // tilaus ei en�� kesken...
    $query  = "UPDATE kuka set kesken=0 where yhtio='$kukarow[yhtio]' and kuka='$kukarow[kuka]'";
    $result = pupe_query($query);
  }
  else {
    $query  = "UPDATE lasku
               SET tila = 'L'
               WHERE tunnus = '$laskurow[tunnus]'
               and tila     = 'C'
               and yhtio    = '$kukarow[yhtio]'";
    $result = pupe_query($query);

    require "tilauskasittely/tilaus-valmis.inc";

    $tee = "";
  }
}

if ($toim == "SIIRTOTYOMAARAYS" and $tee == 'VALMIS') {
  $query  = "UPDATE lasku
             SET alatila = 'J'
             WHERE tunnus = '$laskurow[tunnus]'
             and tila     = 'S'
             and yhtio='$kukarow[yhtio]'";
  $result = pupe_query($query);

  //Katotaan l�ytyyk� tilausrivej�
  $query = "SELECT *
            from tilausrivi
            where yhtio  = '$kukarow[yhtio]'
            and otunnus  = '$laskurow[tunnus]'
            and tyyppi  != 'D'";
  $keres = pupe_query($query);

  //Yht��n rivi� ei ole
  if (mysql_num_rows($keres) == 0) {
    $query = "UPDATE lasku SET alatila='X' where yhtio='$kukarow[yhtio]' and tunnus='$kukarow[kesken]'";
    $result = pupe_query($query);

    // tilaus ei en�� kesken...
    $query  = "UPDATE kuka set kesken=0 where yhtio='$kukarow[yhtio]' and kuka='$kukarow[kuka]'";
    $result = pupe_query($query);
  }
  else {
    require "tilauskasittely/tilaus-valmis-siirtolista.inc";
  }

  $tee = "";
}
