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

var ajax_num = 0;
function clock() {
	setTimeout('clock()', 990);
	/* synchronize clock every minute */
	if(cur_date.getSeconds() == 0) {
		var aj = createAjax();
		if(aj) {
			aj.open('GET', 'time.php?x=' + (ajax_num++), true);
			aj.onreadystatechange = function() {
				if(aj.readyState == 4) {
					cur_date = eval('new Date(' + aj.responseText + ')');
				}
			}
			aj.send();
		}
	}
	cur_date.setSeconds(cur_date.getSeconds() + 1);
	el("time").innerHTML = render_time(cur_date);
	el("date").innerHTML = render_date(cur_date);
}

</script>
</head>
<body onload="clock();">
<?php

//////////////////////////////////////////////////////////////////

$pool->render();

echo "<br /><br />Time: ";

?>
<div id="time" style="font-size: 20pt;">
</div>
<div id="date">
</div>
</body>
</html>
