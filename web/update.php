<?php

require_once('includes.php');

?>
cur_date = new Date(<?php $time->renderJS(); ?>);
el("pool").innerHTML = '<?php $pool->render(); ?>';
el("panel").innerHTML = '<?php $controlpanel_temp->render(); ?>';
