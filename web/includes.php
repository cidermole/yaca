<?php

require_once('common.inc.php');
//require_once('led.class.php');
//require_once('main-power.class.php');
require_once('pool-control.class.php');
require_once('time.class.php');
require_once('temp.class.php');

$common = new Common("192.168.1.3", 1111);

//$led = new Led($common, array('canid_read' => 200, 'canid_write' => 1));
//$mp = new MainPower($common, array('canid_powerstatus' => 400, 'canid_charge' => 2));
$time = new Time($common, array('canid_time' => 401));
$pool = new PoolControl($common, array('canid_phstatus' => 402, 'canid_tempstatus' => 403, 'canid_relaystatus' => 201));
$controlpanel_temp = new Temp($common, array('canid_tempstatus' => 404, 'label' => ''));
$outdoor_temp = new Temp($common, array('canid_tempstatus' => 405, 'label' => ''));

$common->handleRequests();
usleep(100 * 1000);

?>
