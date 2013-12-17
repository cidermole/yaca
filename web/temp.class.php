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
		$failspan1 = '<span style="color: #ff0000;">';
		$failspan2 = '</span>';

		$msg_te = new Message($this->common);
		if(!$msg_te->request($this->config['canid_tempstatus'])) {
			echo $failspan1 . "ID" . $this->config['canid_tempstatus'] . $failspan2;
			return false;
		}

		if(!$msg_te->isOK())
			echo $failspan1;
		echo $this->config['label'] . $this->getTemp($msg_te) . " &deg;C";
		if(!$msg_te->isOK())
			echo $failspan2;
		return true;
	}
}

?>
