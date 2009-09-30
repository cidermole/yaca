<?php

class Led extends Plugin {
	function Led(&$common, $config) {
		$this->name = "Led";
		Plugin::Plugin($common, $config);
	}
	
	function handleRequest() {
		if($_POST['action_led'.$this->id] == 'send') {
			$msg = new Message($this->common);
			$msg->id = $this->config['canid_write'];
			$msg->data[0] = $_POST['set'];
			$msg->length = 1;
			$msg->send();
		}
	}
	
	function render() {
		$msg = new Message($this->common);
		if(!$msg->request($this->config['canid_read'])) {
			echo "{Led: request " . $this->config['canid_read'] . " failed}";
			return false;
		}
		echo "{Led: status of " . $this->config['canid_read'] . " is " . $msg->data[0] . "} ";
?>
<form action="index.php" method="post">
<input type="submit" value="Switch to <?php echo ($msg->data[0] ? '0' : '1'); ?>" />
<input type="hidden" name="set" value="<?php echo ($msg->data[0] ? '0' : '1'); ?>" />
<input type="hidden" name="action_led<?php echo $this->id; ?>" value="send" />
</form>
<?php
		return true;
	}
}

?>
