<?php
# Tuotepaikka ja varmistuskoodi talteen
if ($_GET['tee'] == 'laske' and isset($_POST['tuotepaikka']) and isset($_POST['varmistuskoodi'])) {
	setcookie("_tuotepaikka", $_POST['tuotepaikka']);
	setcookie("_varmistuskoodi", $_POST['varmistuskoodi']);
}
# Alkuvalikossa nollataan tuotepaikka ja varmistuskoodi
if ((!isset($_GET['tee']) and !isset($_POST['tee'])) and isset($_COOKIE['_tuotepaikka']) and isset($_COOKIE['_varmistuskoodi'])) {
	setcookie("_tuotepaikka", "", time()-3600);
	setcookie("_varmistuskoodi", "", time()-3600);
}

$_GET['ohje'] = 'off';
$_GET['no_css'] = 'yes';
$mobile = true;

if (@include_once("../inc/parametrit.inc"));
elseif (@include_once("inc/parametrit.inc"));

$errors = array();

# Haetaan tuotteita
function hae($viivakoodi='', $tuoteno='', $tuotepaikka='') {
	global $kukarow;

	$hylly = explode('-', $tuotepaikka);

	# Hakuehdot
	if ($viivakoodi != '')	$params[] = "tuote.eankoodi = '{$viivakoodi}'";
	if ($tuoteno != '')		$params[] = "tuote.tuoteno = '{$tuoteno}'";
	if ($tuotepaikka != '')	$params[] = "tuotepaikat.hyllyalue='{$hylly[0]}'
										AND tuotepaikat.hyllynro='{$hylly[1]}'
										AND tuotepaikat.hyllyvali='{$hylly[2]}'
										AND tuotepaikat.hyllytaso='{$hylly[3]}'";
	$haku_ehto = implode($params, " AND ");

	$osumat = array();
	if (!empty($haku_ehto)) {
		$query = "	SELECT
					tuote.tuoteno,
					tuotepaikat.inventointilista,
					tuotepaikat.inventointilista_aika,
					concat(	lpad(upper(tuotepaikat.hyllyalue), 5, '0'),
							lpad(upper(tuotepaikat.hyllynro), 5, '0'),
							lpad(upper(tuotepaikat.hyllyvali), 5, '0'),
							lpad(upper(tuotepaikat.hyllytaso), 5, '0')) as sorttauskentta,
					concat_ws('-',tuotepaikat.hyllyalue, tuotepaikat.hyllynro,
								tuotepaikat.hyllyvali, tuotepaikat.hyllytaso) tuotepaikka
					FROM tuotepaikat
					JOIN tuote on (tuote.yhtio=tuotepaikat.yhtio and tuote.tuoteno=tuotepaikat.tuoteno)
					WHERE tuotepaikat.yhtio = '{$kukarow['yhtio']}'
					AND $haku_ehto
					LIMIT 200";
		$result = pupe_query($query);

		while($row = mysql_fetch_assoc($result)) {
			$osumat[] = $row;
		}
	}

	return $osumat;
}

# Tarkistetaan koodi
function tarkista_varmistuskoodi($tuotepaikka, $varmistuskoodi) {
	$hylly = explode('-', $tuotepaikka);

	# Jos ei annettu varmistuskoodia ja keksiss� on koodi, k�ytet��n keksin koodia
	if ($varmistuskoodi == '' and isset($_COOKIE['_varmistuskoodi']) and $tuotepaikka == $_COOKIE['_tuotepaikka']) {
		$varmistuskoodi = $_COOKIE['_varmistuskoodi'];
	}

	if ($varmistuskoodi == '') {
		return false;
	}else {
		$options = array('varmistuskoodi' => $varmistuskoodi);
		return tarkista_varaston_hyllypaikka($hylly[0], $hylly[1], $hylly[2], $hylly[3], $options);
	}
}

if (!isset($tee)) $tee = '';

# Valikko
if ($tee == '') {
	$title = t("Inventointi");
	include('views/inventointi/index.php');
}

# Haku
if ($tee == 'haku') {

	$title = t("Vapaa Inventointi");
	# Haettu jollain
	if (isset($viivakoodi) or isset($tuoteno) or isset($tuotepaikka)) {
		$tuotteet = hae($viivakoodi,$tuoteno,$tuotepaikka);
		if(count($tuotteet) == 0) $errors[] = "Ei l�ytynyt";
	}

	$haku_tuotepaikalla = ($viivakoodi=='' and $tuoteno=='' and $tuotepaikka != '') ? 'true' : '';

	# Vain yksi osuma
	if (isset($tuotteet) and count($tuotteet) == 1) {
		# Suoraan varmistuskoodiin
		$url = http_build_query(array('tee' => 'laske',
								'tuotepaikka' => $tuotteet[0]['tuotepaikka'],
								'tuoteno' => $tuotteet[0]['tuoteno']));
		echo "<META HTTP-EQUIV='Refresh'CONTENT='0;URL=inventointi.php?{$url}'>";
		exit();
	}
	# L�ydetyt osumat
	if (isset($tuotteet) and count($tuotteet) > 0) {
		include('views/inventointi/hakutulokset.php');
	}
	# Haku formi
	else {
		include('views/inventointi/haku.php');
	}
}

# Inventointilistat
if ($tee == 'listat') {
	# Reservipaikka
	if (!isset($reservipaikka)) $reservipaikka = 'E';

	# Haetaan inventointilistat
	$query = "	SELECT DISTINCT(inventointilista) as lista,
				count(tuoteno) as tuotteita,
				concat_ws('-', min(tuotepaikat.hyllyalue), max(tuotepaikat.hyllyalue)) as hyllyvali
				FROM tuotepaikat
				JOIN varaston_hyllypaikat on (varaston_hyllypaikat.yhtio=tuotepaikat.yhtio
				                              and varaston_hyllypaikat.hyllyalue=tuotepaikat.hyllyalue
				                              and varaston_hyllypaikat.hyllynro=tuotepaikat.hyllynro
				                              and varaston_hyllypaikat.hyllyvali=tuotepaikat.hyllyvali
				                              and varaston_hyllypaikat.hyllytaso=tuotepaikat.hyllytaso)
				WHERE tuotepaikat.yhtio	= '{$kukarow['yhtio']}'
				and varaston_hyllypaikat.reservipaikka='{$reservipaikka}'
				and inventointilista > 0
				and inventointilista_aika > '0000-00-00 00:00:00'
				GROUP BY inventointilista_aika
				ORDER BY inventointilista";
	$result = pupe_query($query);

	while($row = mysql_fetch_assoc($result)) {
		$row['url'] = "?tee=laske&lista={$row['lista']}&reservipaikka={$reservipaikka}";
		$listat[] = $row;
	}

	if (count($listat) == 1) {
		# Jos l�ytyi vain yksi lista, inventoidaan suoraan sit�
		echo "<META HTTP-EQUIV='Refresh'CONTENT='0;URL=inventointi.php{$listat[0]['url']}'>";
	}
	elseif (count($listat) > 1) {
		$title = t('Useita listoja');
		include('views/inventointi/listat.php');
	}
	else {
		$errors[] =  t("Inventoitavia eri� ei ole");
		$title = t("Inventointi");
		include('views/inventointi/index.php');	}
}

# Lasketaan tuotteet
if ($tee == 'laske' or $tee == 'inventoi') {
	# Mit� parametrej� tarvitaan (tuoteno tai lista)
	assert(!empty($tuoteno) or !empty($lista));

	if (!isset($maara)) $maara = 0;
	if (!is_numeric($maara)) $errors[] = t("M��r�n on oltava numero");

	# Inventoidaan listaa
	if (!empty($lista)) {

		# Hybridilistat, reservipaikka voi olla K tai E
		if (!isset($reservipaikka)) $reservipaikka = 'E';

		# Haetaan listan 'ensimm�inen' tuote
		$query = "	SELECT
					tuote.nimitys,
					tuote.tuoteno,
					tuote.yksikko,
					tuotepaikat.inventointilista,
					tuotepaikat.tyyppi,
			 		concat_ws('-', tuotepaikat.hyllyalue, tuotepaikat.hyllynro,
			 					tuotepaikat.hyllyvali, tuotepaikat.hyllytaso) as tuotepaikka,
			 		concat(	lpad(upper(tuotepaikat.hyllyalue), 5, '0'),
			 				lpad(upper(tuotepaikat.hyllynro), 5, '0'),
			 				lpad(upper(tuotepaikat.hyllyvali), 5, '0'),
			 				lpad(upper(tuotepaikat.hyllytaso), 5, '0')) as sorttauskentta
			 		FROM tuotepaikat
			 		JOIN tuote on (tuote.yhtio=tuotepaikat.yhtio and tuote.tuoteno=tuotepaikat.tuoteno)
			 		JOIN varaston_hyllypaikat on (varaston_hyllypaikat.yhtio=tuotepaikat.yhtio
			 		                              and varaston_hyllypaikat.hyllyalue=tuotepaikat.hyllyalue
			 		                              and varaston_hyllypaikat.hyllynro=tuotepaikat.hyllynro
			 		                              and varaston_hyllypaikat.hyllyvali=tuotepaikat.hyllyvali
			 		                              and varaston_hyllypaikat.hyllytaso=tuotepaikat.hyllytaso)
			 		WHERE tuotepaikat.yhtio='{$kukarow['yhtio']}'
			 		and varaston_hyllypaikat.reservipaikka='{$reservipaikka}'
			 		AND inventointilista='{$lista}'
			 		AND inventointilista_aika > '0000-00-00 00:00:00' # Inventoidut tuotteet on nollattu
			 		ORDER BY sorttauskentta, tuoteno
			 		LIMIT 1";
		$result = pupe_query($query);

		if (mysql_num_rows($result) == 0) {
			include('views/inventointi/erat_loppu.php');
			exit();
		}
	}
	# Inventoidaan haulla, tuote kerrallaan
	else {
		# Haetaan tuotteen ja tuotepaikan tiedot
		$query = "	SELECT
					tuote.nimitys,
					tuote.tuoteno,
					tuote.yksikko,
					tuotepaikat.tyyppi,
					concat_ws('-',tuotepaikat.hyllyalue, tuotepaikat.hyllynro,
								tuotepaikat.hyllyvali, tuotepaikat.hyllytaso) as tuotepaikka
					FROM tuotepaikat
					JOIN tuote on (tuote.yhtio=tuotepaikat.yhtio and tuote.tuoteno=tuotepaikat.tuoteno)
					WHERE tuotepaikat.yhtio = '{$kukarow['yhtio']}'
					AND tuote.tuoteno='{$tuoteno}'
					AND concat_ws('-',tuotepaikat.hyllyalue, tuotepaikat.hyllynro,
								tuotepaikat.hyllyvali, tuotepaikat.hyllytaso) = '{$tuotepaikka}'";
		$result = pupe_query($query);
	}
	$tuote = mysql_fetch_assoc($result);

	# Haetaan sscc jos tyyppi
	if ($tuote['tyyppi'] == 'S') {
		# etsit��n suuntalavan sscc
		$suuntalava_query = "SELECT group_concat(sscc) as sscc
						FROM tilausrivi
						JOIN suuntalavat on (suuntalavat.yhtio=tilausrivi.yhtio AND suuntalavat.tunnus=tilausrivi.suuntalava)
						WHERE tuoteno='{$tuote['tuoteno']}'
							AND tilausrivi.tyyppi='O'
							AND tilausrivi.suuntalava!=''
							AND tilausrivi.yhtio='{$kukarow['yhtio']}'
							AND concat_ws('-', hyllyalue, hyllynro,
										hyllyvali, hyllytaso) = '{$tuote['tuotepaikka']}'";
		$suuntalava_sscc = pupe_query($suuntalava_query);
		$suuntalava_sscc = mysql_fetch_assoc($suuntalava_sscc);
		$sscc = $suuntalava_sscc['sscc'];
	}

	# N�ytet��nk� apulaskuri
	$query = "SELECT
				count(tunnus) as monta
				FROM tuotteen_avainsanat
				WHERE tuoteno='{$tuote['tuoteno']}'
				AND yhtio='{$kukarow['yhtio']}'
				AND (laji='pakkauskoko2' OR laji='pakkauskoko3')";
	$pakkaukset = pupe_query($query);
	$pakkaukset = mysql_fetch_assoc($pakkaukset);

	$apulaskuri_url = '';
	# Jos pakkauksia ei l�ytynyt, ei n�ytet� apulaskuria
	if ($pakkaukset['monta'] > 0) {
		$apulaskuri_url = http_build_query(array('tee' => 'apulaskuri',
									'tuotepaikka' => $tuotepaikka,
									'tuoteno' => $tuote['tuoteno'],
									'lista' => $lista,
									'reservipaikka' => $reservipaikka));
	}

	# Tarkistetaan varmistuskoodi
	if ((isset($varmistuskoodi) or isset($_varmistuskoodi)) and tarkista_varmistuskoodi($tuote['tuotepaikka'], $varmistuskoodi)) {
		$title = t("Laske m��r�");
		include('views/inventointi/laske.php');

	}
	else {
		# Tarkistetaan ett� varaston_hyllypaikka on perustettu
		$hylly = explode('-', $tuotepaikka);
		if (isset($varmistuskoodi) and !tarkista_varaston_hyllypaikka($hylly[0], $hylly[1], $hylly[2], $hylly[3])) {
			$errors[] = "Tuotteen tietoja ei ole m��ritelty varaston hyllypaikoissa!";
		}
		# Tarkistetaan varmistuskoodi
		elseif (isset($varmistuskoodi)) $errors[] = "Virheellinen varmistuskoodi";

		$title = t("Varmistuskoodi");
		include('views/inventointi/varmistuskoodi.php');
	}

	if ($tee == 'inventoi' and count($errors) == 0) {
		$tee = 'inventoidaan';
	}
}

if ($tee == 'apulaskuri') {
	# Pakkaus1
	$query = "SELECT
				if(myynti_era > 0, myynti_era, 1) as myynti_era,
				yksikko
				FROM tuote
				WHERE tuoteno='{$tuoteno}' AND yhtio='{$kukarow['yhtio']}'";
	$result = pupe_query($query);
	$p1 = mysql_fetch_assoc($result);

	# Pakkaus2
	$query ="SELECT
				selite as myynti_era,
				selitetark as yksikko
				FROM tuotteen_avainsanat
				WHERE tuoteno='{$tuoteno}'
				AND yhtio='{$kukarow['yhtio']}'
				AND laji='pakkauskoko2'";
	$result = pupe_query($query);
	$p2 = mysql_fetch_assoc($result);

	# Pakkaus3
	$query ="SELECT
				selite as myynti_era,
				selitetark as yksikko
				FROM tuotteen_avainsanat
				WHERE tuoteno='{$tuoteno}'
				AND yhtio='{$kukarow['yhtio']}'
				AND laji='pakkauskoko3'";
	$result = pupe_query($query);
	$p3 = mysql_fetch_assoc($result);

	$back = http_build_query(array('tee' => 'laske',
									'tuotepaikka' => $tuotepaikka,
									'tuoteno' => $tuoteno,
									'lista' => $lista,
									'reservipaikka' => $reservipaikka));

	$title = t("Apulaskuri");
	include('views/inventointi/apulaskuri.php');
}

if ($tee == 'inventoidaan') {
	assert(!empty($tuoteno) or !empty($maara) or !empty($tuotepaikka));

	# Haetaan tuotteen tiedot
	$query = "	SELECT tunnus,
				concat_ws('-',tuotepaikat.hyllyalue, tuotepaikat.hyllynro, tuotepaikat.hyllyvali, tuotepaikat.hyllytaso) tuotepaikka,
				hyllyalue,
				hyllynro,
				hyllyvali,
				hyllytaso
				FROM tuotepaikat
				WHERE tuoteno='{$tuoteno}'
				AND yhtio='{$kukarow['yhtio']}'
				AND concat_ws('-',tuotepaikat.hyllyalue, tuotepaikat.hyllynro,
							tuotepaikat.hyllyvali, tuotepaikat.hyllytaso) = '{$tuotepaikka}'
				ORDER BY tuotepaikka ";
	$result = pupe_query($query);
	$result = mysql_fetch_assoc($result);

	# Jos tuotteen tiedot OK, voidaan inventoida
	if ($result) {
		$hylly = array($tuoteno, $result['hyllyalue'], $result['hyllynro'], $result['hyllyvali'], $result['hyllytaso']);
		$hash = implode('###', $hylly);

		$tuote = array($result['tunnus'] => $hash);
		$maara = array($result['tunnus'] => $maara);

		# inventointi
		$tee = 'VALMIS';
		$inven_laji = 'Kiert�v�';
		$lisaselite = 'P�ivitt�isinventointi k�sip��tteell�';
		$mobiili = 'YES';
		require('../inventoi.php');

		# Jos inventoidaan listalta, palataan inventoimaan listan seuraava tuote.
		if($lista != 0) {
			$paluu_url = http_build_query(array('tee' => 'laske', 'lista' => $lista, 'reservipaikka' => $reservipaikka));
		}
		# Jos inventoitu tuotepaikalla, palataan takaisin hakuun kyseisell� tuotepaikalla
		elseif($tuotepaikalla=='true') {
			$paluu_url = http_build_query(array('tee' => 'haku', 'viivakoodi' => '', 'tuoteno' => '', 'tuotepaikka' => $tuotepaikka));
		}
		# Palataan alkuun
		else {
			$paluu_url =http_build_query(array('tee' => 'haku'));
		}
		echo "<META HTTP-EQUIV='Refresh'CONTENT='0;URL=inventointi.php?".$paluu_url."'>";
	}
	else {
		$errors[] = "Virhe inventoinnissa.";
	}
	exit();
}

# Virheet
if (isset($errors)) {
	echo "<span class='error'>";
	foreach($errors as $virhe) {
		echo "{$virhe}<br>";
	}
	echo "</span>";
}

require "inc/footer.inc";
