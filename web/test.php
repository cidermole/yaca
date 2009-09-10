<?php

class Common {
	var $socket;
	
	function Common() {
		$this->socket = fsockopen("192.168.1.2", 1111, $errno, $errstr, 2);
		stream_set_timeout($this->socket, 2);
	}
}
$common = new Common;

class Message {
	var $info;
	var $id;
	var $rtr;
	var $length;
	var $data;
	var $common;

	function Message(&$common) {
		$this->common = &$common;
		$this->info = 0;
		$this->id = 0;
		$this->rtr = 0;
		$this->length = 0;
		for($i = 0; $i < 8; $i++)
			$this->data[$i] = 0;
	}

	function decode($d) {
		$bytes = array();

		for($i = 0; $i < strlen($d); $i++)
			$bytes[$i] = ord($d[$i]);

		$this->info = $bytes[0];
		$this->id =	$bytes[1] * 0x00000001 +
					$bytes[2] * 0x00000100 +
					$bytes[3] * 0x00010000 +
					$bytes[4] * 0x01000000;
		$this->rtr = $bytes[5];
		$this->length = $bytes[6];
		$this->data = array();
		for($i = 0; $i < 8; $i++)
			$this->data[$i] = $bytes[7 + $i];
	}

	function encode() {
		$bytes = "";

		$bytes .= chr($this->info);
		$bytes .= chr(($this->id >> (8 * 0)) & 0xff);
		$bytes .= chr(($this->id >> (8 * 1)) & 0xff);
		$bytes .= chr(($this->id >> (8 * 2)) & 0xff);
		$bytes .= chr(($this->id >> (8 * 3)) & 0xff);
		$bytes .= chr($this->rtr);
		$bytes .= chr($this->length);
		for($i = 0; $i < 8; $i++)
			$bytes .= chr($this->data[$i]);

		return $bytes;
	}
	
	function request($id) {
		$this->id = $id;
		$this->rtr = 1;
		$bytes = $this->encode();
		fwrite($this->common->socket, $bytes);
		$ret = fread($this->common->socket, 15);
		$info = stream_get_meta_data($this->common->socket);
		if($info['timed_out'])
			return false;
		$this->decode($ret);
		return true;
	}
}

$msg = new Message($common);
if($msg->request(3))
	echo "id: " . $msg->id . ", data[0]: " . $msg->data[0];
else
	echo "Timeout error.";

if($msg->request(3))
	echo "id: " . $msg->id . ", data[0]: " . $msg->data[0];
else
	echo "Timeout error.";

fclose($common->socket);

?>
