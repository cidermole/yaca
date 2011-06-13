<?php

class PoolControl extends Plugin {
	function PoolControl(&$common, $config) {
		$this->name = "PoolControl";
		Plugin::Plugin($common, $config);
	}
	
	function getPh($msg) {
		$raw_data = $msg->data[0] * 0x100 + $msg->data[1];
		return $raw_data / 100.0;
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
		$ph_display = number_format($ph, 2, ',', '.');
		$relay = ($msg_st->data[0] == 1);
		echo "Pool: pH " . $ph_display . ", " . $this->getTemp($msg_te) . " &deg;C (" . ($relay ? "Pool" : "Garage") . "), Pumpe " . ($relay ? "ein" : "aus");

		/**
		 * pH correction instructions
		 *
		 * If the pH value is not between the limits (7.0 - 7.4), print an orange/red box containing
		 * instructions on how to correct the pH value.
		 **/
		$problem = "";
		$diff = 0; // difference from maximum / minimum value
		$corr_diff = 0; // difference from correction target
		if($ph < 7) {
			$corr_diff = 7.0 - $ph; // from low pH, only correct up to pH 7.0 (pH usually rises over time)
			$diff = 7.0 - $ph;
			$problem = "niedrig";
			$what = "Plus";
		} else if($ph > 7.4) {
			$corr_diff = $ph - 7.2; // from high pH, correct down to pH 7.2 (so there is some time for it rising again)
			$diff = $ph - 7.4;
			$problem = "hoch";
			$what = "Minus";
		}

		// Pool: 13.7 m^3 (~ 14)
		// pH-Plus as well as pH-Minus say: "100 g per 10 m^3 leads to 0.1 pH change" --> 1400 g per pH
		$amount = round(1400 * $corr_diff, -1); // round amount to 10 g
		$class = $diff > 0.15 ? "error" : "warning";

		if($problem != "") {
			echo "<br/><div id=\"" . $class . "\">pH-Wert zu " . $problem . ", " . $amount . " g pH-" . $what . " zugeben.</div>";
		}

		return true;
	}
}

?>
