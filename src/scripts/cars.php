<?php


include("class/pData.class.php");
include("class/pDraw.class.php");
include("class/pImage.class.php");



$lines = file("/bulk/temp/yaca-log/cars");

// V 2011-05-14 20:46:07.068 CarCounter::Count 1

function lineDate($line) {
	return substr($line, 2, 19);
}

// V 2011-05-14 21:02:11.058

$i = new DateTime("20110514T210000");
$sum = 0;
$abscissa = array();
$data = array();

foreach($lines as $line) {
	if(substr($line, 0, 1) != "V")
		continue;
	$d = DateTime::createFromFormat("Y-m-d H:i:s", lineDate($line));

	if($d > $i) {
		//$abscissa[] = $i->format("m-d H");
		$abscissa[] = $i->format("H");
		$data[] = $sum;
		//echo $i->format("Y-m-d H:i:s") . " " . $sum . "<br/>";
		while($d > $i)
			$i->add(new DateInterval("PT3600S"));
		$sum = 0;
	}
	$sum++;
}


// ==========================================================================
// ==========================================================================


$myData = new pData();
/*$myData->addPoints(array(-37,-9,10,44,26,32,0,-27),"Serie1");
$myData->setSerieDescription("Serie1","Serie 1");
$myData->setSerieOnAxis("Serie1",0);

$myData->addPoints(array(-29,29,48,0,29,19,35,22),"Serie2");
$myData->setSerieDescription("Serie2","Serie 2");
$myData->setSerieOnAxis("Serie2",0);

$myData->addPoints(array(8,40,-5,-10,39,-27,18,22),"Serie3");
$myData->setSerieDescription("Serie3","Serie 3");
$myData->setSerieOnAxis("Serie3",0);

$myData->addPoints(array("January","February","March","April","May","June","July","August"),"Absissa");
$myData->setAbscissa("Absissa");*/

$myData->addPoints($data, "Autos");
$myData->setSerieDescription("Autos", "Autos / h");
$myData->setSerieOnAxis("Autos", 0);

$myData->addPoints($abscissa, "Abscissa");
$myData->setAbscissa("Abscissa");

$myData->setAxisPosition(0,AXIS_POSITION_LEFT);
$myData->setAxisName(0,"Autos / h");
$myData->setAxisUnit(0,"");
$autoSettings = array("R"=>229,"G"=>11,"B"=>11); // set series 0 to red
$myData->setPalette("Autos", $autoSettings);

$myPicture = new pImage(700,230,$myData);
$Settings = array("R"=>170, "G"=>183, "B"=>87);
$myPicture->drawFilledRectangle(0,0,700,230,$Settings);

$Settings = array("StartR"=>219, "StartG"=>231, "StartB"=>139, "EndR"=>1, "EndG"=>138, "EndB"=>68, "Alpha"=>20);
$myPicture->drawGradientArea(0,0,700,230,DIRECTION_VERTICAL,$Settings);

$myPicture->drawRectangle(0,0,699,229,array("R"=>0,"G"=>0,"B"=>0));

$myPicture->setShadow(TRUE,array("X"=>1,"Y"=>1,"R"=>50,"G"=>50,"B"=>50,"Alpha"=>20));

$myPicture->setFontProperties(array("FontName"=>"fonts/Forgotte.ttf","FontSize"=>14));
$TextSettings = array("Align"=>TEXT_ALIGN_MIDDLEMIDDLE
, "R"=>255, "G"=>255, "B"=>255);
$myPicture->drawText(350,25,"Autos",$TextSettings);

$myPicture->setShadow(FALSE);
$myPicture->setGraphArea(50,50,675,190);
$myPicture->setFontProperties(array("R"=>0,"G"=>0,"B"=>0,"FontName"=>"fonts/pf_arma_five.ttf","FontSize"=>6));

$Settings = array("Pos"=>SCALE_POS_LEFTRIGHT
, "Mode"=>SCALE_MODE_START0
, "LabelingMethod"=>LABELING_ALL
, "GridR"=>255, "GridG"=>255, "GridB"=>255, "GridAlpha"=>50, "TickR"=>0, "TickG"=>0, "TickB"=>0, "TickAlpha"=>50, "LabelRotation"=>0, "LabelSkip"=>3, "CycleBackground"=>1, "DrawXLines"=>1, "DrawSubTicks"=>1, "SubTickR"=>255, "SubTickG"=>0, "SubTickB"=>0, "SubTickAlpha"=>50, "DrawYLines"=>ALL);
$myPicture->drawScale($Settings);

$myPicture->setShadow(TRUE,array("X"=>1,"Y"=>1,"R"=>50,"G"=>50,"B"=>50,"Alpha"=>10));

$Config = "";
$myPicture->drawLineChart($Config);

$Config = array("FontR"=>0, "FontG"=>0, "FontB"=>0, "FontName"=>"fonts/pf_arma_five.ttf", "FontSize"=>6, "Margin"=>6, "Alpha"=>30, "BoxSize"=>5, "Style"=>LEGEND_NOBORDER
, "Mode"=>LEGEND_HORIZONTAL
);
$myPicture->drawLegend(563,16,$Config);

$myPicture->stroke();


?>
