CHNG = simple \
       simple_memlink_test \
       simple_refrag \
	   simple_count \
	   simple_tun_interface \
	   simple_bulk \
	   simple_lz4_bulk \
	   simple_lz0_bulk \
	   simple_compression

all: $(CHNG)

.PHONY: $(CHNG)

$(CHNG):
	./run $@/$@.yml simple_lag > $@/$@.sh
	bash $@/$@.sh
