<?php

require_once('includes.php');

?><script>
parent.cont.onl('<?php echo str_replace("\n", "\\\n", str_replace("'", "\\'", file_get_contents("http://192.168.1.3/yaca/update.php"))); ?>');
</script>
