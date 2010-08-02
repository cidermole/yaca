baud=38400

# opkg install setserial
##### old ### setserial /dev/tts/1 spd_shi: instead of 38400, run 230 kbaud (which is 250 in practice on wrt54gl)

#### new:
# setserial /dev/tts/1 spd_cust
# setserial /dev/tts/1 divisor 10  # -> 125 kbaud
# cat /proc/tty/driver/serial

/etc/rc.d/S61yaca -> /etc/init.d/yaca

# cat /etc/init.d/yaca
#!/bin/sh /etc/rc.common

START=61

start() {
        setserial /dev/tts/1 spd_cust
        setserial /dev/tts/1 divisor 10
        /yaca-gw /test.conf
}

stop() {
        killall yaca-gw
}

reload() {
        if [ ! `pidof yaca-gw` ]; then
                start
        fi
}
