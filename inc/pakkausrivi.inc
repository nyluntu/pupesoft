<?php

$ulos = "";
$jatko = 1; // oletetaan normaali k�sittely
$tyyppi = 1; // oletetaan rivin n�kyvyys

if (mysql_field_name($result, $i) == "tunnus" or
  mysql_field_name($result, $i) == "yhtio") {
  $tyyppi = 0;
}

if (mysql_field_name($result, $i) == "erikoispakkaus" or mysql_field_name($result, $i) == "yksin_eraan") {
  $sel = $trow[$i] == 'K' ? ' selected' : '';

  $ulos = "<td><select name='{$nimi}'>";
  $ulos .= "<option value=''>".t("Ei")."</option>";
  $ulos .= "<option value='K' {$sel}>".t("Kyll�")."</option>";
  $ulos .= "</select></td>";

  $jatko = 0;
}
