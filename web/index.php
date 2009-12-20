<?php

require_once('common.inc.php');
require_once('led.class.php');
require_once('main-power.class.php');

$common = new Common("192.168.1.2", 1111);

$led = new Led($common, array('canid_read' => 200, 'canid_write' => 400));
$mp = new MainPower($common, array('canid_powerstatus' => 22, 'canid_charge' => 23));

$common->handleRequests();
usleep(100 * 1000);

//////////////////////////////////////////////////////////////////

$led->render();

echo "<br /><br />";

$mp->render();

?>
