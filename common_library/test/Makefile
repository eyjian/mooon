SUB_DIRS=util sys net
unit_test:
	for subdir in $(SUB_DIRS); \
	do \
		cd $$subdir; \
		make; \
		cd -; \
	done