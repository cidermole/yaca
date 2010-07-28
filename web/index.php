<?php

require_once('includes.php');

?><html>
<head>
<title>Yaca</title>
<script>

var cur_date = new Date(<?php $time->renderJS(); ?> + 1); // XXX year: test, removeme

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

function render_time(d) {
	return d.getHours() + ':' + d.getMinutes() + ':' + d.getSeconds();
}

function render_date(d) {
	return d.getDay() + '.' + d.getMonth() + '.' + d.getYear();
}

var ajax_num = 0;
function clock() {
	/* synchronize clock every minute */
	if(cur_date.getSeconds() == 0) {
		var aj = createAjax();
		if(aj) {
			aj.open('GET', 'time.php?x=' + (ajax_num++), true);
			aj.onreadystatechange = function() {
				if(x.readyState == 4) {
					cur_date = eval('new Date(' + aj.responseText + ')');
				}
			}
			aj.send();
		}
	}
	cur_date.setSeconds(cur_date.getSeconds() + 1);
	el("time").innerHTML = render_time(cur_date);
	el("date").innerHTML = render_date(cur_date);
	setTimeout('clock()', 1000);
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
