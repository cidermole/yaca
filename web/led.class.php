<?php

class Led extends Plugin {
	function Led(&$common, $config) {
		$this->name = "Led";
		Plugin::Plugin($common, $config);
	}
	
	function handleRequest() {}
	
	function render() {
		$msg = new Message($this->common);
		if(!$msg->request($this->config['canid_read'])) {
			echo "{Led: request " . $this->config['canid_read'] . " failed}";
			return false;
		}
		echo "{Led: status of " . $this->config['canid_read'] . " is " . $msg->data[0] . "}";
		return true;
	}
}

?>
