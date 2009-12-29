<?php

class Time extends Plugin {
	function Time(&$common, $config) {
		$this->name = "Time";
		Plugin::Plugin($common, $config);
	}
	
	function handleRequest() {
	}
	
	function nf($number) {
		return sprintf("%02d", $number);
	}
	
	function render() {
		$msg = new Message($this->common);
		if(!$msg->request($this->config['canid_time'])) {
			echo "{Time: request " . $this->config['canid_time'] . " failed}";
			return false;
		}
		echo $this->nf($msg->data[0]) . ":" . $this->nf($msg->data[1]) . ":" . $this->nf($msg->data[2]) . " " . $this->nf($msg->data[5]) . "." . $this->nf($msg->data[4]) . "." . ($msg->data[3] + 2000) . " (" . (($msg->data[6] & (1 << 1)) ? 'backup' : 'NTP') . " time source, " . (($msg->data[6] & (1 << 0)) ? 'CEST' : 'CET') . ")";
		return true;
	}
}

?>
