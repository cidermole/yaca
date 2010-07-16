baud=38400
# setserial /dev/tts/0 spd_shi

/etc/rc.d/S61yaca -> /etc/init.d/yaca

# cat /etc/init.d/yaca
#!/bin/sh /etc/rc.common

START=61

start() {
        setserial /dev/tts/1 spd_shi
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
