TARGETS=yaca-c yaca-flash yaca-program yaca-hexmerge yaca-cached yaca-tr mpmon yaca-timed yaca-logd yaca-ppp yaca-gw

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
