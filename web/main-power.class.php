<?php

class MainPower extends Plugin {
	function MainPower(&$common, $config) {
		$this->name = "MainPower";
		Plugin::Plugin($common, $config);
	}
	
	function getStatus($msg) {
		$states = array('Idle', 'Boost', 'Charge', 'Charge (topping)');
		return (isset($states[$msg->data[0]]) ? $states[$msg->data[0]] : 'Undefined');
	}
	
	function getVoltage($msg) {
		$raw_data = $msg->data[1] * 0x100 + $msg->data[2];
		return number_format($raw_data * 110.08 / 10240, 2, ',', '.');
	}
	
	function getCurrent($msg) {
		$raw_data = $msg->data[3] * 0x100 + $msg->data[4];
		return number_format($raw_data * 30.72 / 6545.408, 3, ',', '.');
	}
	
	function getTime($msg) {
		$raw_data = $msg->data[5] * 0x10000 + $msg->data[6] * 0x100 + $msg->data[7];
		return ($this->getStatus($msg) == 'Boost' ? 'I(t) = ' : 'T = ') . $raw_data;
	}
	
	function handleRequest() {
		if($_POST['action_mainpower'.$this->id] == 'send') {
			$charge_time = floor($_POST['mah'] * 3600 / 67);
			
			$msg = new Message($this->common);
			$msg->id = $this->config['canid_charge'];
			$msg->data[0] = $charge_time / 0x10000;
			$msg->data[1] = ($charge_time / 0x100) % 0x100;
			$msg->data[2] = $charge_time % 0x100;
			$msg->length = 3;
			$msg->send();
		}
	}
	
	function render() {
		$msg = new Message($this->common);
		if(!$msg->request($this->config['canid_powerstatus'])) {
			echo "{MainPower: request " . $this->config['canid_powerstatus'] . " failed}";
			return false;
		}
		echo "{MainPower: status = " . $this->getStatus($msg) . ", U = " . $this->getVoltage($msg) . ", I = " . $this->getCurrent($msg) . ", " . $this->getTime($msg) . " }";
?>
<form action="index.php" method="post">
mAh: <input name="mah" />
<input type="submit" value="Charge" />
<input type="hidden" name="action_mainpower<?php echo $this->id; ?>" value="send" />
</form>
<?php
		return true;
	}
}

?>
