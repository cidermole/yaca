<?php

$lines = file("/bulk/temp/yaca-log/cars");

// V 2011-05-14 20:46:07.068 CarCounter::Count 1

function lineDate($line) {
	return substr($line, 2, 19);
}

// V 2011-05-14 21:02:11.058

$i = new DateTime("20110514T210000");
$sum = 0;

foreach($lines as $line) {
	if(substr($line, 0, 1) != "V")
		continue;
	$d = DateTime::createFromFormat("Y-m-d H:i:s", lineDate($line));

	if($d > $i) {
		echo $i->format("Y-m-d H:i:s") . " " . $sum . "<br/>";
		while($d > $i)
			$i->add(new DateInterval("PT3600S"));
		$sum = 0;
	}
	$sum++;
}

?>
