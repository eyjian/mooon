EXTRA_DIST = log.conf.in jhttpd.conf.in jian.conf.in
 
confdir=$(prefix)/conf
conf_DATA=log.conf jhttpd.conf jian.conf
 
log.conf: log.conf.in
	cp $< $@
 
jhttpd.conf: jhttpd.conf.in
	cp $< $@

jian.conf: jian.conf.in
	cp $< $@
	
clean:
	rm -f log.conf
	rm -f jhttpd.conf
	rm -f jian.conf
