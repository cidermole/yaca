TARGETS=yaca-c yaca-flash yaca-program yaca-hexmerge yaca-cached yaca-tr mpmon yaca-timed yaca-logd yaca-ppp yaca-gw
NAS_DEPLOY_TARGETS=yaca-cached yaca-timed yaca-logd
WRT_DEPLOY_TARGETS=yaca-gw yaca-ppp

all:
	for target in $(TARGETS); do \
		make -C $$target; \
	done

clean:
	for target in $(TARGETS); do \
		make -C $$target clean; \
	done

mrproper:
	for target in $(TARGETS); do \
		make -C $$target mrproper; \
	done

deploy-nas:
	ssh yaca@nas '/home/yaca/yaca-daemons stop'
	cd ../bin; scp $(NAS_DEPLOY_TARGETS) yaca@nas:/home/yaca/yaca/build/bin
	ssh yaca@nas '/home/yaca/yaca-daemons start'

deploy-wrt:
	ssh yaca@nas '/home/yaca/yaca-daemons stop'
	ssh root@wrt 'killall yaca-ppp; killall yaca-gw'
	for target in $(WRT_DEPLOY_TARGETS); do \
		make -C $$target deploy; \
	done
	ssh root@wrt '/yaca/bin/yaca-gw /yaca/src/x86/yaca-gw/conf/yaca-gw.conf'
	# TODO: start yaca-ppp
	ssh yaca@nas '/home/yaca/yaca-daemons start'
