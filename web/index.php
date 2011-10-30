<?php

require_once('includes.php');

$ppc = (strpos($_SERVER['HTTP_USER_AGENT'], 'NetFront') !== FALSE);
//$ppc = true;

?><html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>Yaca</title>
<script>

var cur_date = new Date(<?php $time->renderJS(); ?>);
var onl, aj_wait = false;

function el(n) {
	// standard browser
	return document.getElementById(n);
}

function createAjax() {
	var x = null;
	try {
		x = new XMLHttpRequest();
	} catch(e) {
		try {
			x = new ActiveXObject("Microsoft.XMLHTTP");
		} catch(e) {
			try {
				x = new ActiveXObject("Msxml2.XMLHTTP");
			} catch(e) {
				x = null;
			}
		}
	}
	return x;
}

function pad(s) {
	if(s.toString().length == 1)
		return '0' + s.toString();
	else
		return s;
}

function render_time(d) {
	return pad(d.getHours()) + ':' + pad(d.getMinutes()) + ':' + pad(d.getSeconds());
}

function render_date(d) {
	return pad(d.getDate()) + '.' + pad(d.getMonth() + 1) + '.' + d.getFullYear();
}

function async_mode(force) {
	if(force || aj_wait) {
		async = true;
		el("time").style.color = '#ff0000';
	}
}

var ajax_num = 0, async = false;
function clock() {
	setTimeout('clock()', 1000);
	<?php /* synchronize clock every minute */ ?>
	if(cur_date.getSeconds() == 0) {
		var aj = createAjax();
		if(aj) {
			aj.open('GET', 'update.php?x=' + (ajax_num++), true);
			aj.onreadystatechange = function() {
				if(aj.readyState == 4) {
					aj_wait = false;
					if(aj.responseText.length < 5) {
						async_mode(true);
						return;
					} else if(async) {
						async = false;
						el("time").style.color = '#000000';
					}
					eval(aj.responseText);
				}
			}
			aj_wait = true;
			aj.send();
			setTimeout('async_mode(false)', 30000);
		}
	}
	cur_date.setSeconds(cur_date.getSeconds() + 1);
	el("time").innerHTML = render_time(cur_date);
	el("date").innerHTML = render_date(cur_date);
}

</script>
<style>

body div {
	position: absolute;
}

#time {
	font-size: 40pt;
	left: 0px;
	top: 0px;
}

#date {
	font-size: 30pt;
	left: 395px;
	top: 0px;
}

body div.temp, body div.temp div {
	font-size: 30pt;
}

#panel_outer {
	left: 0px;
	top: 80px;
}

#outdoor_outer {
	left: 300px;
	top: 80px;
}

body div.temp img {
	vertical-align: middle;
}

#pool {
	left: 0px;
	top: 80px;
	visibility: hidden;
}

#warning {
	background: #ffb050;
}

#error {
	background: #ff0000;
}

</style>
</head>
<body onload="clock();">
<div id="time">
</div>
<div id="date">
</div>
<div id="pool">
<?php $pool->render(); ?>
</div>
<div id="panel_outer" class="temp">
<img src="img/house.png" id="house" alt="Innen:" /> <span id="panel"><?php $controlpanel_temp->render(); ?></span>
</div>
<div id="outdoor_outer" class="temp">
<img src="img/clouds.png" id="clouds" alt="Außen:" /> <span id="outdoor"><?php $outdoor_temp->render(); ?></span>
</div>
</body>
</html>
