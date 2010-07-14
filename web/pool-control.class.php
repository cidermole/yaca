<?php

class PoolControl extends Plugin {
	function PoolControl(&$common, $config) {
		$this->name = "PoolControl";
		Plugin::Plugin($common, $config);
	}
	
	function getPh($msg) {
		$raw_data = $msg->data[0] * 0x100 + $msg->data[1];
		return number_format($raw_data / 100.0, 2, ',', '.');
	}
	
	function getTemp($msg) {
		$raw_data = $msg->data[0] * 0x100 + $msg->data[1];
		return number_format($raw_data / 10.0, 1, ',', '.');
	}
	
	function render() {
		$msg_ph = new Message($this->common);
		if(!$msg_ph->request($this->config['canid_phstatus'])) {
			echo "{PoolControl: request " . $this->config['canid_phstatus'] . " failed}";
			return false;
		}
		$msg_te = new Message($this->common);
		if(!$msg_te->request($this->config['canid_tempstatus'])) {
			echo "{PoolControl: request " . $this->config['canid_tempstatus'] . " failed}";
			return false;
		}

		echo "{PoolControl: pH = " . $this->getPh($msg_ph) . ", T = " . $this->getTemp($msg_te) . " &deg;C }";
		return true;
	}
}

?>
