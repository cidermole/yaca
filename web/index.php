<?php

require_once('common.inc.php');
require_once('led.class.php');

$common = new Common("192.168.1.2", 1111);

$led = new Led($common, array('canid_read' => 3));

//////////////////////////////////////////////////////////////////

$led->render();

?>
