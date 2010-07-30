<?php

require_once('includes.php');

$ppc = (strpos($_SERVER['HTTP_USER_AGENT'], 'NetFront') !== FALSE);
//$ppc = true;

?><html>
<head>
<title>Yaca</title>
<script>

var cur_date = new Date(<?php $time->renderJS(); ?>);
var onl, netfront = false, aj_wait = false;

function el(n) {
	if(document.getElementById) {
		// standard browser
		return document.getElementById(n);
	} else {
		// Pocket IE
		return document[n];
	}
}

function FramedRequest() {
	this.readyState = 0;
	this.responseText = '';
	onl = function() {}
}

FramedRequest.prototype.open = function(method, url, async) {
	this.url = url;
}

FramedRequest.prototype.send = function() {
	var This = this;
	onl = function(t) {
		This.readyState = 4;
		This.responseText = t;
		This.onreadystatechange();
	}
	parent.exec.location.href = this.url;
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
	if(!x) {
		netfront = true;
		x = new FramedRequest();
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
			aj.open('GET', (netfront ? 'update_f.php?x=' : 'update.php?x=') + (ajax_num++), true);
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

<?php if($ppc) { ?>
#time {
	font-size: 20pt;
	position: absolute;
	left: 0px;
	top: 0px;
}

#date {
	position: absolute;
	left: 230px;
	top: 0px;
}

#pool {
	position: absolute;
	left: 0px;
	top: 40px;
}
<?php } ?>

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
</body>
</html>
