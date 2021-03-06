<?php

$ulos = "";
$jatko = 1; // oletetaan normaali k�sittely
$tyyppi = 1; // oletetaan rivin n�kyvyys

if ($i == 0) { //yhti�t� ei n�ytet�
  $tyyppi = 0;
}

if (mysql_field_name($result, $i) == "abs_pvm" or
  mysql_field_name($result, $i) == "kassa_abspvm") {
  $tyyppi = 1.5;
}

if ((mysql_field_name($result, $i) == "jv")
  or (mysql_field_name($result, $i) == "suoraveloitus")
  or (mysql_field_name($result, $i) == "erapvmkasin")
  or (mysql_field_name($result, $i) == "itsetulostus")
  or (mysql_field_name($result, $i) == "jaksotettu")) {

  $ulos  = "<td><select name='$nimi'>";

  $chk1 = '';
  $chk2 = '';

  if ($trow[$i] == "") {
    $chk1 = 'selected';
  }
  else {
    $chk2 = 'selected';
  }

  $ulos .= "<option value=''  $chk1>".t("Ei")."</option>";
  $ulos .= "<option value='o' $chk2>".t("Kyll�")."</option>";

  $ulos .= "</select></td>";
  $jatko = 0;
}

if (mysql_field_name($result, $i) == "kaytossa") {

  $chk1 = '';
  $chk2 = '';

  if ($trow[$i] == "") {
    $chk1 = 'selected';
  }
  else {
    $chk2 = 'selected';
  }

  if ($kaytossa_disabled != "") {
    $ulos  = "<td>";

    if ($chk1 != "") $ulos .= t("K�yt�ss�");
    else $ulos .= t("Poistettu / Ei k�yt�ss�");

    $ulos .= "<input type='hidden' name='$nimi' value='$trow[$i]'></td>";
  }
  else {

    $ulos  = "<td><select name='$nimi' $kaytossa_disabled>";

    $ulos .= "<option value=''  $chk1>".t("K�yt�ss�")."</option>";
    $ulos .= "<option value='E' $chk2>".t("Poistettu / Ei k�yt�ss�")."</option>";

    $ulos .= "</select></td>";
  }

  $jatko = 0;
}

if (mysql_field_name($result, $i) == "kateinen") {

  $sel1 = '';
  $sel2 = '';
  $sel3 = '';

  if (strtoupper($trow[$i]) == '') {
    $sel1 = "SELECTED";
  }
  if (strtoupper($trow[$i]) == 'O') {
    $sel2 = "SELECTED";
  }
  if (strtoupper($trow[$i]) == 'P') {
    $sel3 = "SELECTED";
  }
  if (strtoupper($trow[$i]) == 'N') {
    $sel4 = "SELECTED";
  }

  $ulos  = "<td><select name='$nimi'>";
  $ulos .= "<option value=''  $sel1> ".t("T�m� ei ole k�teismaksuehto.")."</option>";
  $ulos .= "<option value='n' $sel4> ".t("K�teismaksuehto, Pankkikortti ei py�ristet�.")."</option>";
  $ulos .= "<option value='o' $sel2> ".t("K�teismaksuehto, Luottokortti ei py�ristet�.")."</option>";
  $ulos .= "<option value='p' $sel3> ".t("K�teismaksuehto, K�teinen py�ristet��n.")."</option>";

  $ulos .= "</select></td>";

  $jatko = 0;
}

if (mysql_field_name($result, $i) == "factoring") {

  $query = "SELECT distinct factoringyhtio
            FROM factoring
            WHERE yhtio = '$kukarow[yhtio]'
            ORDER BY factoringyhtio";
  $vresult = pupe_query($query);

  $ulos = "<td><select name='$nimi'>";
  $ulos .= "<option value = ''>".t("Ei factoroida");

  while ($vrow=mysql_fetch_array($vresult)) {
    $sel="";
    if ($trow[$i] == $vrow[0]) {
      $sel = "selected";
    }
    $ulos .= "<option value = '$vrow[0]' $sel>$vrow[0]";
  }

  $ulos .= "</select></td>";
  $jatko = 0;
}


if (mysql_field_name($result, $i) == "osamaksuehto1" or mysql_field_name($result, $i) == "osamaksuehto2") {
  $query = "SELECT *
            FROM maksuehto
            WHERE yhtio = '$kukarow[yhtio]'
            ORDER BY jarjestys, teksti";
  $vresult = pupe_query($query);

  $ulos = "<td><select name='$nimi'>";
  $ulos .= "<option value = ''>".t("T�m� ei ole moniehto-maksuehto");

  while ($vrow=mysql_fetch_array($vresult)) {
    $sel="";
    if ($trow[$i] == $vrow["tunnus"]) {
      $sel = "selected";
    }
    $ulos .= "<option value = '$vrow[tunnus]' $sel>".t_tunnus_avainsanat($vrow, "teksti", "MAKSUEHTOKV")."</option>";
  }
  $ulos .= "</select></td>";
  $jatko = 0;
}

if (mysql_field_name($result, $i) == "pankkiyhteystiedot") {
  $query = "SELECT *
            FROM pankkiyhteystiedot
            WHERE yhtio = '$kukarow[yhtio]'
            ORDER BY nimitys";
  $vresult = pupe_query($query);

  $ulos = "<td><select name='$nimi'>";
  $ulos .= "<option value='0'>".t("K�yt� yrityksen oletuksia");

  while ($vrow = mysql_fetch_array($vresult)) {
    $sel = "";
    if ($trow[$i] == $vrow["tunnus"]) {
      $sel = "selected";
    }
    $ulos .= "<option value='$vrow[tunnus]' $sel>$vrow[nimitys]</option>";
  }
  $ulos .= "</select></td>";
  $jatko = 0;
}
