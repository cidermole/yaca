<?php   
 /*
     Example1 : A simple line chart
 */

 // Standard inclusions      
 include("pChart/pData.class");   
 include("pChart/pChart.class");   
 
 $chart_width = 1000;
 $chart_height = 230;
 $chart_title = "Temperaturverlauf 07.09.2010 - 08.09.2010";
 $divisor = 20;
 
 // Dataset definition    
 $DataSet = new pData;   
 $DataSet->ImportFromCSV("david3.csv",",",array(1,2),FALSE,0);   
// $DataSet->AddAllSeries();   
 $DataSet->AddSerie("Serie1");
 $DataSet->SetAbsciseLabelSerie();   
 $DataSet->SetSerieName("Aussen","Serie1");   
 $DataSet->SetSerieName("Innen","Serie2");   
 $DataSet->SetYAxisName("Temperatur Aussen");
 $DataSet->SetYAxisUnit(" °C");
  
 // Initialise the graph   
 $Test = new pChart($chart_width,$chart_height);
 $Test->setDivisor($divisor);
 $Test->setFontProperties("Fonts/tahoma.ttf",8);   
 $Test->setGraphArea(70,30,$chart_width - 70,$chart_height - 30);   

 $Test->drawFilledRoundedRectangle(7,7,$chart_width-7,$chart_height-7,5,240,240,240);   
 $Test->drawRoundedRectangle(5,5,$chart_width-5,$chart_height-5,5,230,230,230);   
 $Test->drawGraphArea(255,255,255,TRUE);
 $Test->drawScale($DataSet->GetData(),$DataSet->GetDataDescription(),SCALE_NORMAL,150,150,150,TRUE,0,2);   
 $Test->drawGrid(4,TRUE,230,230,230,50);
 
 // Draw the 0 line   
 $Test->setFontProperties("Fonts/tahoma.ttf",6);   
 $Test->drawTreshold(0,143,55,72,TRUE,TRUE);   
  
 // Draw the line graph
 $Test->drawLineGraph($DataSet->GetData(),$DataSet->GetDataDescription());   
// $Test->drawPlotGraph($DataSet->GetData(),$DataSet->GetDataDescription(),3,2,255,255,255);   

  
 $Test->clearScale();
 $Test->setFontProperties("Fonts/tahoma.ttf",8);   
 $DataSet->RemoveSerie("Serie1");
 $DataSet->AddSerie("Serie2");
 $DataSet->SetYAxisName("Temperatur Innen");

 $Test->drawRightScale($DataSet->GetData(),$DataSet->GetDataDescription(),SCALE_NORMAL,150,150,150,TRUE,0,2);   
// $Test->drawGrid(4,TRUE,230,230,230,50);
 $Test->drawLineGraph($DataSet->GetData(),$DataSet->GetDataDescription());   


 // Finish the graph   
 $Test->setFontProperties("Fonts/tahoma.ttf",8);   
 $Test->drawLegend(75,35,$DataSet->GetDataDescription(),255,255,255);   
 $Test->setFontProperties("Fonts/tahoma.ttf",10);   
 $Test->drawTitle(60,22,$chart_title,50,50,50,585);   
// $Test->Render("example1.png");
 $Test->Stroke();
?>
