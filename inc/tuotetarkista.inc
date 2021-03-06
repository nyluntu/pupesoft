<?php

if (!function_exists("tuotetarkista")) {
  function tuotetarkista(&$t, $i, $result, $tunnus, &$virhe, $trow) {
    global $kukarow, $yhtiorow, $alias_set, $laji, $poistolukko, $luedata_toiminto;

    static $tem_tuoteno, $tem_tullinimike1, $tem_eisaldoa, $tem_kehahin, $tem_kehahin_ind, $tem_tuotetyyppi, $tem_status, $tem_status_vanha, $tem_try, $tem_try_vanha;

    $fieldname = mysql_field_name($result, $i);

    if ($fieldname == "tuoteno") {
      if (trim($t[$i]) == '') {
        $virhe[$i] = t("Tieto puuttuu");
      }
      else {
        $temp_ekakirjain = (substr(trim($t[$i]), 0, 1));

        if (($temp_ekakirjain == '*') or ($temp_ekakirjain == '@') or ($temp_ekakirjain == '?')) {
          $virhe[$i] = t("Tuotenumero ei voi alkaa merkeill� *, @ tai ?");
        }
        else {
          if (substr(trim($t[$i]), -1) == '*') {
            $virhe[$i] = t("Tuotenumero ei voi loppua merkkiin *");
          }
          else {
            if ($tunnus == "") {
              $query  = "SELECT yhtio
                         FROM tuote
                         WHERE yhtio = '$kukarow[yhtio]'
                         and tuoteno = '".trim($t[$i])."'";
              $chkressiresult = pupe_query($query);

              if (mysql_num_rows($chkressiresult) > 0) {
                $virhe[$i] = t("Tuotenumero on jo perustettu j�rjestelm��n");
              }
            }
          }
        }
      }

      $tem_tuoteno = $t[$i];

      // Onko tuotteella tapahtumia
      if ($poistolukko != "" and ($poistolukko != "LUEDATA" or ($poistolukko == "LUEDATA" and strtoupper($luedata_toiminto) == "POISTA"))) {
        onkotapahtumajariveja($tem_tuoteno);

        if ($poistolukko != "" and strtoupper($luedata_toiminto) == "POISTA") {
          $virhe[$i] = t("Tuotetta ei saa poistaa, koska tuotteella on tilausrivej�/tapahtumia")."!";
        }
      }
    }

    if (($fieldname == "projekti" or $fieldname == "kohde" or $fieldname == "kustp") and (int) trim($t[$i]) != 0) {

      if ($fieldname == "projekti") $kptyyppi = "P";
      if ($fieldname == "kohde")    $kptyyppi = "O";
      if ($fieldname == "kustp")    $kptyyppi = "K";

      $tpque = "SELECT tunnus
                from kustannuspaikka
                where yhtio   = '$kukarow[yhtio]'
                and tyyppi    = '$kptyyppi'
                and kaytossa != 'E'
                and tunnus    = '$t[$i]'";
      $tpres = pupe_query($tpque);

      if (mysql_num_rows($tpres) == 0) {
        $virhe[$i] .= t("Kustannuspaikkaa ei l�ydy")."!";
      }
    }

    if ($fieldname == "osasto"
      or $fieldname == "try"
      or $fieldname == "tuotemerkki"
      or $fieldname == "malli"
      or $fieldname == "mallitarkenne") {

      // Ei tehd� tarkistuksia jos tietoa ei oo sy�tetty
      if (empty($t[$i])) {
        return;
      }

      $t[$i] = trim($t[$i]);

      $tpque = "SELECT tunnus
                FROM avainsana
                WHERE yhtio = '$kukarow[yhtio]'
                and LAJI    = '$fieldname'";
      $tpres = pupe_query($tpque);

      if (mysql_num_rows($tpres) > 0) {

        $tpque = "SELECT tunnus
                  FROM avainsana
                  WHERE yhtio = '$kukarow[yhtio]'
                  AND LAJI    = '$fieldname'
                  AND SELITE  = '$t[$i]'";
        $tpres = pupe_query($tpque);

        if (mysql_num_rows($tpres) == 0) {
          $virhe[$i] .= t("Avainsanaa ei l�ydy")."!";
          return;
        }

        if ($fieldname == "try") {
          $tem_try = $t[$i];
          if (!empty($trow)) {
            $tem_try_vanha = $trow[$i];
            $tem_status_vanha = $trow['status'];
          }
          else {
            $tem_try_vanha = '';
            $tem_status_vanha = '';
          }
        }

        if ($fieldname == "try" and !empty($trow) and $tem_try_vanha != $tem_try) {
          list($saldo, $hyllyssa, $myytavissa) = saldo_myytavissa($tem_tuoteno);

          if ($saldo == 0) {

            $query = "SELECT tuotepaikat.*,
                      concat_ws('-', hyllyalue, hyllynro, hyllyvali, hyllytaso) as hylly
                      FROM tuotepaikat
                      WHERE yhtio = '{$kukarow['yhtio']}'
                      AND tuoteno = '{$tem_tuoteno}'
                      ORDER BY oletus ASC";
            $tuotepaikat_result = pupe_query($query);
            $poista = array();

            while ($tuotepaikka = mysql_fetch_assoc($tuotepaikat_result)) {
              $poista[] = $tuotepaikka;
            }

            $tee = "MUUTA";
            $tuoteno = $tem_tuoteno;
            $halyraja2 = array();
            $tilausmaara2 = array();
            $kutsuja = "tuotetarkista.inc";
            require "muuvarastopaikka.php";
          }
        }
      }
    }

    if ($fieldname == "kehahin" and $yhtiorow["palvelutuotteiden_kehahinnat"] == "K") {
      $tem_kehahin = $t[$i];
      $tem_kehahin_ind = $i;
    }

    if ($fieldname == "nimitys") {
      if (trim($t[$i]) == '') {
        $virhe[$i] .= t("Nimitys puuttuu")."!";
      }
    }

    if (($fieldname == "tilino" or
        $fieldname == "tilino_eu" or
        $fieldname == "tilino_ei_eu" or
        $fieldname == "tilino_marginaali" or
        $fieldname == "tilino_osto_marginaali") and $t[$i] != "") {

      $query = "SELECT yhtio
                FROM tili
                WHERE yhtio = '$kukarow[yhtio]'
                and tilino  = '$t[$i]'";
      $sresult = pupe_query($query);

      if (mysql_num_rows($sresult) != 1) {
        $virhe[$i] = t("Tili� ei l�ydy");
      }
    }

    if ($fieldname == "tilino" and trim($t[$i]) == "" and $laji == "V") {
      $virhe[$i] = t("Viranomaistuotteella on oltava tili")."!";
    }

    if (($fieldname == "tullinimike1") and ($t[$i] != 0)) {
      $query = "SELECT cn
                FROM tullinimike
                WHERE cn = '$t[$i]' and kieli = '$yhtiorow[kieli]'";
      $sresult = pupe_query($query);

      if (mysql_num_rows($sresult) != 1) {
        $virhe[$i] = t("Virheellinen tullinimike");
      }

      $tem_tullinimike1 = $t[$i];
    }

    if ($fieldname == "tullinimike2" and $tem_tullinimike1 != "") {
      // jos tyhj�� laitetaan nollaksi
      if ($t[$i] == "") $t[$i] = "00";
    }

    if ($fieldname == "tuotetyyppi") {
      if (isset($t[$i])) {
        // Tyyppi� ei saa vaihtaa, jos tuotteella on yksikin tapahtuma
        if (trim(strtoupper($trow["tuotetyyppi"])) != trim(strtoupper($t[$i]))) {

          // Onko tuotteella tapahtumia
          if ($poistolukko != "" and $poistolukko != "TSEKATTU") {
            onkotapahtumajariveja($tem_tuoteno);
          }

          if (trim(strtoupper($trow["tuotetyyppi"])) == "") {
            if ($poistolukko != "" and ($t[$i] == 'R' or $t[$i] == 'K' or $t[$i] == 'M')) {
              $virhe[$i] = t("Tuotetyyppi� ei voida muuttaa, koska tuotteella on tapahtumia")."!";
            }
          }
          elseif ($poistolukko != "") {
            $virhe[$i] = t("Tuotetyyppi� ei voida muuttaa, koska tuotteella on tapahtumia")."!";
          }
        }

        $tem_tuotetyyppi = $t[$i];
      }
    }

    if ($fieldname == "sarjanumeroseuranta") {

      // Onko tuotteella tapahtumia
      if ($poistolukko != "" and $poistolukko != "TSEKATTU") {
        onkotapahtumajariveja($tem_tuoteno);
      }

      if ($poistolukko != "" and $t[$i] != $trow["sarjanumeroseuranta"]) {
        $query = "SELECT count(*) saldo
                  FROM tuotepaikat
                  WHERE yhtio  = '$kukarow[yhtio]'
                  and tuoteno  = '$tem_tuoteno'
                  and saldo   != 0";
        $vresult = pupe_query($query);
        $vrow = mysql_fetch_array($vresult);

        // Tietyt vaihdot sallitaan!!!, huomaa NOT iffiss�, helpompi kirjoittaa n�in p�in
        if (!(($t[$i] == "S" and $vrow["saldo"] == 0 and in_array($trow["sarjanumeroseuranta"], array("S", "U")))
            or ($t[$i] == "U" and $vrow["saldo"] == 0 and in_array($trow["sarjanumeroseuranta"], array("S", "U")))
            or ($t[$i] == ""  and $vrow["saldo"] == 0 and in_array($trow["sarjanumeroseuranta"], array("", "T", "V", "E", "F")))
            or ($t[$i] == "T" and $vrow["saldo"] == 0 and in_array($trow["sarjanumeroseuranta"], array("", "T", "V", "E", "F")))
            or ($t[$i] == "V" and $vrow["saldo"] == 0 and in_array($trow["sarjanumeroseuranta"], array("", "T", "V", "E", "F")))
            or ($t[$i] == "E" and (in_array($trow["sarjanumeroseuranta"], array("E", "F")) or ($vrow["saldo"] == 0 and in_array($trow["sarjanumeroseuranta"], array("", "T", "V", "E", "F")))))
            or ($t[$i] == "F" and (in_array($trow["sarjanumeroseuranta"], array("E", "F")) or ($vrow["saldo"] == 0 and in_array($trow["sarjanumeroseuranta"], array("", "T", "V", "E", "F"))))))) {
          $virhe[$i] = t("Seurantatyyppi� ei voida muuttaa, koska tuotteella on tapahtumia")."!";
        }
      }
    }

    if ($fieldname == "ei_saldoa") {
      if (isset($t[$i])) {
        $query = "SELECT *
                  FROM tuotepaikat
                  WHERE tuoteno = '$tem_tuoteno'
                  AND yhtio     = '$kukarow[yhtio]'";
        $sresult = pupe_query($query);

        if ($t[$i] != '' and mysql_num_rows($sresult) > 0) {
          $virhe[$i] = t("Tuotteella on varastopaikkoja. Poista ne ensin!");
        }

        // Onko tuotteella tapahtumia
        if ($poistolukko != "" and $poistolukko != "TSEKATTU") {
          onkotapahtumajariveja($tem_tuoteno);
        }

        if ($poistolukko != "" and isset($trow['ei_saldoa']) and $t[$i] != $trow["ei_saldoa"]) {
          $virhe[$i] = t("Tuotteella on tapahtumia. Saldottomuutta ei voi vaihtaa")."!";
        }

        $tem_eisaldoa = $t[$i];
      }
    }

    // Viranomaistuotteita ei tsekata...
    if ($fieldname == "vienti" and $laji != "V" and $tem_tuotetyyppi != "A" and $tem_tuotetyyppi != "B") {

      // Katsotaan, ettei ole sekaisin plussia ja miinuksia
      if ($t[$i] != '' and strpos($t[$i], "+") !== FALSE and strpos($t[$i], "-") !== FALSE) {
        $virhe[$i] = t("Laita samanaikaisesti vain + tai - maakoodeja!")."<br>";
      }
      elseif ($t[$i] != '') {

        // K�ytt�j�t voi erotella my�s maat pilkuilla, joten vaihdetaan ne t�ss� spaceiksi, trimmataan ja uppercasetaan
        $t[$i] = str_replace(",", " ", strtoupper(trim($t[$i])));

        // Poistetaan ylim��r�iset spacet (kaksi tai enemm�n spaceja yhdeksi spaceksi)
        $t[$i] = preg_replace("/ {2,}/", " ", $t[$i]);

        // K�yd��n l�pi sy�tetyt arvot, splitataan v�lily�nnist�
        $apu_vientikiellot = explode(" ", $t[$i]);

        foreach ($apu_vientikiellot as $apu_vientikielto) {

          // varmistetaan, ett� l�ytyy plus tai miinusmerkki
          if (strpos($apu_vientikielto, "+") === FALSE and strpos($apu_vientikielto, "-") === FALSE) {
            $virhe[$i] .= t("Maakoodi virheellisess� muodossa").": '$apu_vientikielto'<br>";
          }
          // varmistetaan, ett� plus tai miinus on eka merkki
          elseif (strpos($apu_vientikielto, "+") != 0 or strpos($apu_vientikielto, "-") != 0) {
            $virhe[$i] .= t("Maakoodi virheellisess� muodossa").": '$apu_vientikielto'<br>";
          }
          else {
            // Otetaan eka +/- pois
            $apu_vientikielto = substr($apu_vientikielto, 1);

            // Katsotaan l�ytyyk� annettu maakoodi
            $query = "SELECT distinct koodi
                      FROM maat
                      WHERE koodi = '$apu_vientikielto'";
            $vientikielto_result = pupe_query($query);

            if (mysql_num_rows($vientikielto_result) == 0) {
              $virhe[$i] .= "Virheellinen maakoodi $apu_vientikielto!<br>";
            }
          }
        }
      }

      if ($virhe[$i] != "") {
        $virhe[$i] .= t("Sy�t� tieto muodossa +MAAKOODI tai -MAAKOODI ja erottele maat v�lily�nnill�.");
      }
    }

    if ($fieldname == "eankoodi") {
      if (trim($t[$i]) != 0 and trim($t[$i] != '')) {
        $query  = "SELECT eankoodi
                   FROM tuote
                   WHERE yhtio   = '$kukarow[yhtio]'
                   and eankoodi  = '".trim($t[$i])."'
                   and tuoteno  != '$tem_tuoteno'";
        $chkressiresult = pupe_query($query);

        if (mysql_num_rows($chkressiresult) > 0) {
          $virhe[$i] = t("EAN-koodi on jo perustettu j�rjestelm��n");
        }

        if ($yhtiorow["tarkista_eankoodi"] == "K" and tarkista_ean13(trim($t[$i])) === FALSE) {
          $virhe[$i] = t("EAN-koodi on virheellinen");
        }
      }
    }

    if (mysql_field_name($result, $i) == "valmistusaika_sekunneissa") {
      $valmistusaika_sekunneissa = trim($t[$i]);

      if ($valmistusaika_sekunneissa != "" and (!is_numeric($valmistusaika_sekunneissa) or $valmistusaika_sekunneissa < 0)) {
        $virhe[$i] = t("Valmistusaika pit�� olla suurempi tai yht�suuri kuin 0");
      }
    }

    if ($fieldname == 'status') {
      $tem_status = $t[$i];
      $tem_status_vanha = $trow[$i];

      if (!empty($trow) and $tem_status_vanha != $tem_status) {
        //jos tuotteella status muutetaan ja tuotteella on vain oletuspaikka ja saldo on nolla,
        //niin tuotteen muokkauksen yhteydess� mit�t�id��n nykyinen tuotepaikka + tapahtumiin j�lki ja perustetaan m��ritellyin ehdoin uudet tuotepaikat.
        list($saldo, $hyllyssa, $myytavissa) = saldo_myytavissa($tem_tuoteno);

        if ($saldo == 0) {

          $query = "SELECT tuotepaikat.*,
                    concat_ws('-', hyllyalue, hyllynro, hyllyvali, hyllytaso) as hylly
                    FROM tuotepaikat
                    WHERE yhtio = '{$kukarow['yhtio']}'
                    AND tuoteno = '{$tem_tuoteno}'
                    ORDER BY oletus ASC";
          $tuotepaikat_result = pupe_query($query);
          $poista = array();
          while ($tuotepaikka = mysql_fetch_assoc($tuotepaikat_result)) {
            $poista[] = $tuotepaikka;
          }

          $tee = "MUUTA";
          $tuoteno = $tem_tuoteno;
          $halyraja2 = array();
          $tilausmaara2 = array();
          $kutsuja = "tuotetarkista.inc";
          require "muuvarastopaikka.php";
        }
      }
      // Jos kyseess� tuotteen poisto, tarkistetaan l�ytyyk� tuotetta avoimilta ostotilauksilta, jos l�ytyy niin poistoa ei sallita.
      if ($tem_status == 'P') {
        $query = "SELECT tuoteno
                  FROM tilausrivi
                  WHERE yhtio  = '$kukarow[yhtio]'
                  AND tyyppi   = 'O'
                  AND tuoteno  = '$tem_tuoteno'
                  AND kpl      = 0
                  AND varattu != 0";
        $result = pupe_query($query);

        if (mysql_num_rows($result) > 0) {
          $virhe[$i] = t("Tuotetta l�ytyy avoimelta ostotilaukselta, ei voida merkata poistetuksi");
        }
      }
    }

    // ei piirret� ellei yhti�parametri ohjeista katsomaan t��lt�
    if ($fieldname == "suoratoimitus") {
      if (in_array($yhtiorow["tee_osto_myyntitilaukselta"], array('V', 'W', 'I', 'J'))) {
        if (!in_array($t[$i], array('S', 'U', 'X', ''))) {
          $virhe[$i] = t("Suoratoimitustieto virheellinen");
        }
      }
    }

    //tehd��n n�� jutut t�ss� koska t�m� on aina viimeinen sarake
    if ($fieldname == "tunnus") {

      if ($tem_kehahin != ""
        and $tem_kehahin != $trow["kehahin"]
        and ((isset($tem_eisaldoa)
            and $tem_eisaldoa == "")
          or (!isset($tem_eisaldoa)
            and $trow["ei_saldoa"] == ""))) {
        // Pit�� aina setata my�s "t�m�n" kent�n virhe, muuten luedata ei toimi
        $virhe[$i] = $virhe[$tem_kehahin_ind] = t("Saldollisen tuotteen hankintahintaa ei voi muuttaa")."!";
      }

      if ($tem_kehahin != ""
        and $tem_kehahin != $trow["kehahin"]
        and count($virhe) == 0
        and $yhtiorow["palvelutuotteiden_kehahinnat"] == "K"
        and ((isset($tem_eisaldoa)
            and $tem_eisaldoa == "")
          or (!isset($tem_eisaldoa)
            and $trow["ei_saldoa"] == ""))) {

        $tem_kehahin = sprintf("%.6f", $tem_kehahin);

        $query = "INSERT into tapahtuma set
                  yhtio     = '$kukarow[yhtio]',
                  tuoteno   = '$tem_tuoteno',
                  kpl       = '0',
                  kplhinta  = '$tem_kehahin',
                  hinta     = '$tem_kehahin',
                  laji      = 'tulo',
                  hyllyalue = '',
                  hyllynro  = '',
                  hyllyvali = '',
                  hyllytaso = '',
                  selite    = '".t("Tuotteen kehahinta vaihdettiin")." $trow[kehahin] --> $tem_kehahin',
                  laatija   = '$kukarow[kuka]',
                  laadittu  = now()";
        $chkressires = pupe_query($query);
      }

      if (count($virhe) == 0
        and $yhtiorow["tuotteen_oletuspaikka"] != ""
        and ((isset($tem_eisaldoa)
            and $tem_eisaldoa == "")
          or (!isset($tem_eisaldoa)
            and $trow["ei_saldoa"] == ""))) {

        $query = "SELECT *
                  FROM tuotepaikat
                  WHERE tuoteno = '$tem_tuoteno'
                  and yhtio     = '$kukarow[yhtio]'";
        $sresult = pupe_query($query);

        if (mysql_num_rows($sresult) == 0) {
          $kaikki_oletuspaikat = hae_kaikki_oletuspaikat_try_tai_status($tem_try, $tem_status);
          $_counter = 0;

          foreach ($kaikki_oletuspaikat as $_tuotepaikka) {
            list($hyllyalue, $hyllynro, $hyllyvali, $hyllytaso) = explode("-", trim($_tuotepaikka));

            if ($hyllyalue == "" or !isset($hyllyalue)) {
              $hyllyalue = 0;
            }
            if ($hyllynro == "" or !isset($hyllynro)) {
              $hyllynro = 0;
            }
            if ($hyllyvali == "" or !isset($hyllyvali)) {
              $hyllyvali = 0;
            }
            if ($hyllytaso == "" or !isset($hyllytaso)) {
              $hyllytaso = 0;
            }

            $oletus_x = $_counter == 0 ? 'X' : '';

            $query = "SELECT *
                      FROM tuotepaikat
                      WHERE yhtio   = '{$kukarow['yhtio']}'
                      AND tuoteno   = '{$tem_tuoteno}'
                      AND hyllyalue = '{$hyllyalue}'
                      AND hyllynro  = '{$hyllynro}'
                      AND hyllytaso = '{$hyllytaso}'
                      AND hyllyvali = '{$hyllyvali}'";
            $paikka_chk_res = pupe_query($query);

            if (mysql_num_rows($paikka_chk_res) == 0) {
              lisaa_tuotepaikka($tem_tuoteno, $hyllyalue, $hyllynro, $hyllyvali, $hyllytaso, "Tuoteyll�pidossa", "", 0, 0, 0);

              if ($trow["luedata_from"] == "LUEDATA") {
                lue_data_echo("<font class='message'>".t("Lis�ttiin varastopaikka")." {$_tuotepaikka}</font></br>");
              }
              else {
                echo "<font class='message'>".t("Lis�ttiin varastopaikka")." {$_tuotepaikka}</font></br>";
              }

              $_counter++;
            }
          }
        }
      }
    }
    //tarkistetaan ettei yritet� laittaa ostoehdotus sarakkeeseen v��ri� kirjaimia! sallitut "E" = ei ja "" = kyll�
    if ($fieldname == "ostoehdotus") {
      if ($t[$i] != "E" and $t[$i] != "") {
        $virhe[$i] = t("Ostoehdotus arvo tulee olla joko E tai tyhj�! (E = ei n�ytet� ja tyhj� = n�ytet��n)");
      }
    }
  }
}
