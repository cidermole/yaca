<?php

class Message {
	var $info;
	var $id;
	var $rtr;
	var $length;
	var $data;

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
}

$msg = new Message;

$msg->info = 0;
$msg->id = 3;
$msg->rtr = 1;
$msg->length = 0;
for($i = 0; $i < 8; $i++)
	$msg->data[$i] = 0;

$file = fopen("/home/david/Code/yaca/yaca-cached.pipe", "rw");
fwrite($file, $msg->encode());
$msg->decode(fread($file, 15));
echo "id: " . $msg->id . ", data[0]: " . $msg->data[0];

?>
