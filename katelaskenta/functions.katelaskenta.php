<?php
/**
 * Funktio laskee uuden hinnan kateprosentilla.
 *
 * Kateprosentti annetaan prosenteissa, ei desimaaleissa.
 * Voi olla 0-100 v�lilt�.
 */
function lisaa_hintaan_kate($keskihankintahinta, $kateprosentti) {

    $keskihankintahinta = (float)$keskihankintahinta;
    $kateprosentti = (float)$kateprosentti;

    return $keskihankintahinta / ( 1 - ( $kateprosentti / 100 ) );
}

/**
 * Funktio tarkistaa, ett� kateprosentti on sallitulla v�lill�.
 *
 * Kateprosentti ei voi olla yli 100 tai alle 0. Funktio palauttaa
 * true, jos prosentissa ei ole mit��n vikaa.
 */
function tarkista_kateprosentti($kateprosentti) {
    if(!is_numeric($kateprosentti))
        return false;
    
    $kateprosentti = (float)$kateprosentti;
    if($kateprosentti >= 100 || $kateprosentti < 0)
        return false;
    
    return true;
}

/**
 * Funktio siivoaa annetut laskentakomennot.
 *
 * Annetut komennot k�yd��n merkkikerralla l�pi ja niist� siivotaan
 * ylim��r�iset pois. Lis�ksi tarkistetaan, ett� komennot ova olleet
 * sallittujen joukossa.
 *
 * Palautetaan komennot merkkijonoina.
 */
function siivoa_laskentakomennot($komennot) {
    // M��ritet��n ne komennot, jotka ovat sallittuja.
    $sallitut_komennot = array("m", "y", "n", 0);
    // Pilkotaan merkkijono taulukoksi, jossa jokainen
    // merkki on oma solunsa.
    $komennot = str_split($komennot);
    // Poistetaan taulukosta duplicaatit, jotta jokaista komentoa
    // j�� vain yksi kpl.
    $komennot = array_unique($komennot);
    // Poistetaan komennoista merkit, joita ei tunnisteta
    // sallituiksi komennoiksi.
    $komennot = array_intersect($komennot, $sallitut_komennot);
    return join("", $komennot);
}

/**
 * Funktio tarkistaa sy�tteet, joita katelaskentaohjelma l�hett��.
 *
 * Funktio on kooste pienemmist� toimenpiteist�. Jos virheit� ilmenee
 * tarkistusten aikana, lis�t��n ongelmarivit virhe-taulukkoon.
 * Lopuksi palautetaan taulukko, jossa on kaksi sis�kk�ist� taulukkoa.
 * "kunnossa" -taulukko sis�lt�� tarkistuksista l�p�isseet sy�tteet
 * ja "Virheelliset" -taulukko ne rivit, joissa oli ongelmia.
 */
function tarkista_katelaskenta_syotteet($taulukko) {
    // Luodaan uusi virhe-taulukko, johon ker�t��n mahdolliset
    // virheelliset rivit.
    $virherivit = array();
    
    // K�yd��n l�pi valitut rivit k�ytt�j�n sy�tteist�.
    foreach($taulukko["kunnossa"] as &$rivi) {
        
        // Jos kateprosentti on virheellinen, lis�t��n rivi
        // virhe-taulukkoon ja hyp�t��n seuraavaan riviin.
        if(!tarkista_kateprosentti($rivi[1])) {
            $virherivit["'" . $rivi[0] . "'"] = $rivi;
            continue;
        }
        
        // Siivotaan lasketakomennot ylim��r�isist� merkeist�
        $rivi[3] = siivoa_laskentakomennot($rivi[3]);
        // Jos merkkijono siivoamisen j�lkeen on tyhj�, lis�t��n
        // rivi virhe-taulukkoon.
        if(trim($rivi[3]) == "") {
            $virherivit["'" . $rivi[0] . "'"] = $rivi;
            continue;
        }
    }
    
    // Tallennetaan virheelliset rivit omaan alkioonsa.
    $taulukko["virheelliset"] = $virherivit;
    
    // Siivotaan alkuper�isist� riveist� virherivit
    $taulukko["kunnossa"] = array_diff_key($taulukko["kunnossa"], $virherivit);
    
    // Palautetaan taulukko, joka pit�� sis�ll��n kunnossa olevat
    // ja virheelliset rivit.
    return $taulukko;
}

function luo_katelaskenta_update_komennot($taulukko) {
    // Luodaan update-komennoille taulukko, johon kaikki komennot
    // kootaan.
    $update_komennot = array();
    $sql_komento_alku = "UPDATE tuote SET ";
    $sql_komento_loppu = "WHERE tunnus = ";
    
    // K�yd��n l�pi jokainen valittu tuoterivi ja muodostetaan
    // ehtojen mukaan oikeanlainen update-komento.
    foreach($taulukko as $rivi) {
        
        $rivin_tunnus = $rivi[0];
        $rivin_kateprosentti = $rivi[1];
        $rivin_keskihankintahinta = $rivi[2];
        $rivin_komennot = $rivi[3];
        
        $update_kysely = "";
        $update_kysely .= $sql_komento_alku;
        
        // Jos komennossa on 0 merkki jossakin kohti, ei hintamuutoksia
        // tehd�. Tallennetaan vain komento talteen tietokantaan.
        if(mb_strrchr($rivin_komennot, 0)) {
            $update_kysely .= "tuote.katelaskenta = {$rivin_komennot} ";    
        } else {
            // Jos komennossa m, lasketaan myyntihinta.
            if(mb_strrchr($rivin_komennot, "m")) {
                $uusi_hinta = lisaa_hintaan_kate($rivin_keskihankintahinta, $rivin_kateprosentti);
                $update_kysely .= "tuote.myyntihinta = {$uusi_hinta}, ";
            }
            // Jos komennossa y, lasketaan myymalahinta.
            if(mb_strrchr($rivin_komennot, "y")) {
                $uusi_hinta = lisaa_hintaan_kate($rivin_keskihankintahinta, $rivin_kateprosentti);
                $update_kysely .= "tuote.myymalahinta = {$uusi_hinta}, ";
            }
            // Jos komennossa n, lasketaan nettohinta.
            if(mb_strrchr($rivin_komennot, "n")) {
                $uusi_hinta = lisaa_hintaan_kate($rivin_keskihankintahinta, $rivin_kateprosentti);
                $update_kysely .= "tuote.nettohinta = {$uusi_hinta}, ";
            }
            
            // Lis�t��n kyselyyn pakolliset kent�t, jotka tulee jokaiseen
            // komennon lopuksi mukaan.
            $update_kysely .= "tuote.katelaskenta = {$rivin_komennot}, ";
            $update_kysely .= "tuote.myyntikate = {$rivin_kateprosentti}, ";
            $update_kysely .= "tuote.hintamuutospvm = NOW() ";
        }
        // Kyselyn where -ehdon lis��minen.
        $update_kysely .= $sql_komento_loppu . $rivin_tunnus;
        
        // Lis�t��n valmisteltu kysely taulukkoon.
        array_push($update_komennot, $update_kysely);
    }
    
    return $update_komennot;
}

function tallenna_valitut_katemuutokset($data) {
    
    // Luodaan yhdistelm�taulukko, jossa eritell��n virheelliset
    // rivit ja kunnossa olevat. Virheelliset taulukkoon lis�t��n
    // ne rivit, joiden sy�tteiss� prosessin aikana ilmenee ongelmia.
    $yhdistetyt_tuoterivit = array();
    $yhdistetyt_tuoterivit["virheelliset"] = array();
    $yhdistetyt_tuoterivit["kunnossa"] = array();

    // Siivotaan valitut tuoterivit tyhjist� avain => arvo pareista
    $valitut_tuoterivit = array_filter($data["valitutrivit"]);
    
    // Siivotaan valitut kateprosentit valittujen tuoterivien
    // perusteella. J�ljelle j�� vain valitut tuotteet taulukosta.
    $valitut_tuoterivit_kateprosentit = array_intersect_key($data["valitutkateprosentit"], $valitut_tuoterivit);
    
    // Siivotaan valitut keskihankintahinnat valittujen tuoterivien
    // perusteella. J�ljelle j�� vain valitut tuotteet taulukosta.
    $valitut_tuoterivit_keskihankintahinnat = array_intersect_key($data["valitutkeskihankintahinnat"], $valitut_tuoterivit);    
    
    // Siivotaan valitut keskihankintahinnat valittujen tuoterivien
    // perusteella. J�ljelle j�� vain valitut tuotteet taulukosta.
    $valitut_tuoterivit_laskentakomennot = array_intersect_key($data["valituthinnat"], $valitut_tuoterivit);
    
    // Array_merge_recursive -funktiolla taulut yhdistet��n yhdeksi kokonaisuudeksi.
    // Funktio k�ytt�� taulukon avaimia, joilla tiedot koostetaan yksinkertaisemmaksi
    // taulukoksi.
    //
    // Tulevan taulun rakenne on seuraava:
    // [avain] => [tunnus, kateprosentti, keskihankintahinta, komento]
    $yhdistetyt_tuoterivit["kunnossa"] = array_merge_recursive($valitut_tuoterivit,
                                                   $valitut_tuoterivit_kateprosentit,
                                                   $valitut_tuoterivit_keskihankintahinnat,
                                                   $valitut_tuoterivit_laskentakomennot);
    

    // Tarkistetaan sy�tteet ja funktio palauttaa taulukon, jossa
    // on eritelty kunnossa olleet rivit virheellisist�.
    $yhdistetyt_tuoterivit = tarkista_katelaskenta_syotteet($yhdistetyt_tuoterivit);
    
    // Update_komennot -taulukko sis�lt�� valmistellus update -sql komennot
    // Komennot luodaan erillisess� funktiossa, jonne annetaan parametrina
    // jo tarkistetut tiedot. Virheellisille riveille ei tehd� mit��n.
    $update_komennot = array();
    $update_komennot = luo_katelaskenta_update_komennot($yhdistetyt_tuoterivit["kunnossa"]);
    
    // Ajetaan p�ivityskomennot tietokantaan.
    foreach($updatekomennot as $updatesql) {
        pupe_query($updatesql);        
    }
    
    // Palautetaan virheelliset tuotteet.
    // count() == 0 jos virheit� ei ollut.
    return $yhdistetyt_tuoterivit["virheelliset"];
}