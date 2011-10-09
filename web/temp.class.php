<?php

class Temp extends Plugin {
	function Temp(&$common, $config) {
		$this->name = "Temp";
		Plugin::Plugin($common, $config);
	}
	
	function getTemp($msg) {
		$raw_data = $msg->data[0] * 0x100 + $msg->data[1];
		if($raw_data > 32767)
			$raw_data = -(65536 - $raw_data);
		return number_format($raw_data / 10.0, 1, ',', '.');
	}
	
	function render() {
		$msg_te = new Message($this->common);
		if(!$msg_te->request($this->config['canid_tempstatus'])) {
			echo "ID" . $this->config['canid_tempstatus'];
			return false;
		}

		echo $this->config['label'] . $this->getTemp($msg_te) . " &deg;C";
		return true;
	}
}

?>
