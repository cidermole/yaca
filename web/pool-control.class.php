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
		$msg_st = new Message($this->common);
		if(!$msg_st->request($this->config['canid_relaystatus'])) {
			echo "{PoolControl: request " . $this->config['canid_relaystatus'] . " failed}";
			return false;
		}

		$ph = $this->getPh($msg_ph);
		$relay = ($msg_st->data[0] == 1);
		echo "Pool: pH = " . $ph . ", " . $this->getTemp($msg_te) . " &deg;C (" . ($relay ? "Pool" : "Garage") . "), Pumpe " . ($relay ? "ein" : "aus");

		$problem = "";
		$diff = 0;
		if($ph < 7) {
			$diff = 7.2 - $ph;
			$problem = "niedrig";
			$what = "Plus";
		} else if($ph > 7.4) {
			$diff = $ph - 7.2;
			$problem = "hoch";
			$what = "Minus";
		}

		$amount = round(1600 * $diff, -1);
		$class = $diff > 0.15 ? "error" : "warning";

		if($problem != "") {
			echo "<br/><div id=\"" . $class . "\">pH-Wert zu " . $problem . ", " . $amount . " g pH-" . $what . " zugeben.</div>";
		}

		return true;
	}
}

?>
