SUB_DIRS=utils sys net
unit_test:
	for subdir in $(SUB_DIRS); \
	do \
		cd $$subdir; \
		make; \
		cd -; \
	done
