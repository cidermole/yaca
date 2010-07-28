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
		echo $this->nf($msg->data[0]) . ":" . $this->nf($msg->data[1]) . ":" . $this->nf($msg->data[2]) . " " . $this->nf($msg->data[6]) . "." . $this->nf($msg->data[5]) . "." . ($msg->data[3] * 0x100 + $msg->data[4]) . " (" . (($msg->data[7] & (1 << 1)) ? 'backup' : 'NTP') . " time source, " . (($msg->data[7] & (1 << 0)) ? 'CEST' : 'CET') . ")";
		return true;
	}

	function renderJS() {
		$msg = new Message($this->common);
		if(!$msg->request($this->config['canid_time'])) {
			return false;
		}

		echo ($msg->data[3] * 0x100 + $msg->data[4]) . ", " . ($msg->data[5] - 1) . ", " . $msg->data[6] . ", " . $msg->data[0] . "," . $msg->data[1] . "," . $msg->data[2];
		return true;
	}
}

?>
