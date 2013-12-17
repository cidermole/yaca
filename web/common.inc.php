<?php

// message.info bits (flags)
define("INFO_AUTOINFO", 0x01); // auto-info of state change (not implemented)
define("INFO_FAIL",     0x02); // fail reply (multiple timeouts -> MAX_FAILS reached)


class Common {
	var $socket;
	var $counters;
	var $plugins;
	
	function __construct($host, $port) {
		$this->counters = array();
		$this->plugins = array();
		$timeout = 2;
		$this->socket = fsockopen($host, $port, $errno, $errstr, $timeout);
		stream_set_timeout($this->socket, $timeout);
	}
	
	function addPlugin(&$plugin) {
		if(!isset($this->counters[$plugin->name]))
			$this->counters[$plugin->name] = 0;
		$plugin->setId($this->counters[$plugin->name]++);
		$this->plugins[count($this->plugins)] = &$plugin;
	}
	
	function handleRequests() {
		for($i = 0; $i < count($this->plugins); $i++) {
			$this->plugins[$i]->handleRequest();
		}
	}
	
	function __destruct() {
		fclose($this->socket);
	}
}

///////////////////////////////////////////////////////////////////////////////

class Plugin {
	var $name;
	var $id;
	var $common;
	var $config;
	
	function Plugin(&$common, $config) {
		$this->name = "Plugin";
		$this->config = $config;
		$common->addPlugin($this);
		$this->common = &$common;
	}
	function setId($id) {
		$this->id = $id;
	}
	function handleRequest() {}
	function render() {}
}

///////////////////////////////////////////////////////////////////////////////

class Message {
	var $info;
	var $id;
	var $rtr;
	var $length;
	var $data;
	var $common;
	var $timestamp;

	function Message(&$common) {
		$this->common = &$common;
		$this->info = 0;
		$this->id = 0;
		$this->rtr = 0;
		$this->length = 0;
		for($i = 0; $i < 8; $i++)
			$this->data[$i] = 0;
		$this->timestamp = 0;
	}

	function getTimestamp() {
		return new DateTime('@' . $this->timestamp);
	}

	function isFail() {
		return ($this->info & INFO_FAIL);
	}

	function isCurrent() {
		// current if reply received less than 5 minutes ago
		return ($this->timestamp->diff(new DateTime()) < new DateInterval('PT5M'));
	}

	function isOK() {
		return ($this->isCurrent() && !$this->isFail());
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
		$this->timestamp =	$bytes[15] * 0x00000001 +
							$bytes[16] * 0x00000100 +
							$bytes[17] * 0x00010000 +
							$bytes[18] * 0x01000000;
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
		if($this->isFail())
			return false;
		return true;
	}
	
	function send() {
		fwrite($this->common->socket, $this->encode());
	}
}

?>
