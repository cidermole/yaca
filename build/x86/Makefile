TARGETS=yaca-c yaca-flash yaca-program yaca-hexmerge yaca-cached yaca-tr mpmon yaca-timed yaca-logd yaca-ppp yaca-gw
DEPLOY_TARGETS=yaca-cached yaca-timed yaca-logd

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
	cd ../bin; scp $(DEPLOY_TARGETS) yaca@nas:/home/yaca/yaca/build/bin
	ssh yaca@nas '/home/yaca/yaca-daemons start'

