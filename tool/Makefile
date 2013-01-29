SUBDIRS = dev-config el-discover el-prop
##########################################################

all:
	@for dir in ${SUBDIRS}; do (cd $$dir && ${MAKE} all); done

clean:
	@for dir in ${SUBDIRS}; do (cd $$dir && ${MAKE} clean); done
