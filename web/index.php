<?php

require_once('common.inc.php');
//require_once('led.class.php');
//require_once('main-power.class.php');
require_once('pool-control.class.php');
require_once('time.class.php');

$common = new Common("192.168.1.2", 1111);

//$led = new Led($common, array('canid_read' => 200, 'canid_write' => 1));
//$mp = new MainPower($common, array('canid_powerstatus' => 400, 'canid_charge' => 2));
$time = new Time($common, array('canid_time' => 401));
$pool = new PoolControl($common, array('canid_phstatus' => 402, 'canid_tempstatus' => 403, 'canid_relaystatus' => 201));

$common->handleRequests();
usleep(100 * 1000);

?><html>
<head>
<title>Yaca</title>
<script>

var cur_date = new Date(<?php $time->renderJS(); ?>);

function el(n) {
	if(document.getElementById) {
		// standard browser
		return document.getElementById(n);
	} else {
		// Pocket IE
		return document[n];
	}
}

function clock() {
	cur_date.setSeconds(cur_date.getSeconds() + 1);
	el("time").innerHTML = cur_date.toLocaleString();
	setTimeout('clock()', 1000);
}

</script>
</head>
<body>
<?php

//////////////////////////////////////////////////////////////////

$pool->render();

echo "<br /><br />Time: ";

$time->render();

?>
<div id="time">
</div>
</body>
</html>
