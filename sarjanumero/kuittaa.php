<?php

$_GET['ohje'] = 'off';
$_GET["no_css"] = 'yes';

if (@include_once "../inc/parametrit.inc");
elseif (@include_once "inc/parametrit.inc");

echo "<meta name='viewport' content='width=device-width, maximum-scale=1.0' />\n";
echo "<link rel='stylesheet' type='text/css' href='ipad.css' />\n";
echo "<body>";

require 'generoi_edifact.inc';

if (!isset($errors)) $errors = array();

// Jos haulla ei l�ytyny mit��n, ollaan palattu t�lle sivulle virheparametrilla.
if (isset($virhe)) {
  $errors[] = t("Ei l�ytynyt. Hae uudestaan.");
}

if (isset($submit)) {
  switch ($submit) {
  case 'rahtikirjanumero':
  case 'sarjanumero':

    if (empty($rahtikirjanumero) and empty($sarjanumero)) {
      $errors[] = t("Sy�t� rahtikirjanumero tai sarjanumero");
      break;
    }
    elseif (empty($rahtikirjanumero)) {
      $parametrit = kuittaus_parametrit($sarjanumero, 'S');
    }
    else {
      $parametrit = kuittaus_parametrit($rahtikirjanumero, 'R');
    }

    if (is_string($parametrit)) {
      $errors[] = t("Rahtikirja: {$parametrit} on jo kuitattu.");
    }
    elseif ($parametrit) {
      $sanoma = laadi_edifact_sanoma($parametrit);
    }
    else{
      $errors[] = t("Rahtikirjaa ei l�ytynyt!");
    }
    if ($sanoma) {
      $lahetys = 'X';
      if (laheta_sanoma($sanoma)) {
        $lahetys = 'OK';
      }
      else{
        $errors[] = t("L�hetys ei onnistunut");
      }
    }
    else{
      $errors[] = t("Ei sanomaa");
    }
    break;
  default:
    $errors[] = t("Yll�tt�v� virhe");
    break;
  }
}

echo "
<div class='header'>
  <h1>", t("Rahdin kuittaus vastaanotetuksi"), "</h1>
</div>";

echo "<div class='error' style='text-align:center'>";
foreach ($errors as $error) {
  echo $error."<br>";
}
echo "</div>";

if ($lahetys == 'OK') {
  echo "<div style='text-align:center;'>";
  echo "Rahti vastaanotettu ja sanoma l�hetetty!";
  echo "</div>";
}

echo "
<form method='post' action=''>
  <div style='text-align:center;padding:10px;'>
    <label for='rahtikirjanumero'>", t("Sy�t� rahtikirjanumero"), "</label><br>
    <input type='text' id='rahtikirjanumero' name='rahtikirjanumero' style='margin:10px;' />
    <br>
    <button name='submit' value='rahtikirjanumero' onclick='submit();' class='button'>", t("OK"), "</button>
  </div>
</form>

<form method='post' action=''>
  <div style='text-align:center;padding:10px;'>
    <label for='sarjanumero'>", t("Tai lue jokin rahdin sarjanumero"), "</label><br>
    <input type='text' id='sarjanumero' name='sarjanumero' style='margin:10px;' />
    <br>
    <button name='submit' value='sarjanumero' onclick='submit();' class='button'>", t("OK"), "</button>
  </div>
</form>

<script type='text/javascript'>
  $(document).on('touchstart', function(){
    $('#sarjanumero').focus();
  });
</script>";

require 'inc/footer.inc';