<?php

require_once('includes.php');

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
<body onload="clock();">
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
