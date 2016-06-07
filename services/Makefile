

SERV = lz0_compressor lz0_decompressor lz4_compressor lz4_compressor 

SERV_CLEAN = lz0_compressor_clean lz0_decompressor_clean lz4_compressor_clean lz4_compressor_clean 

all: 
	make -C lz0_compressor
	make -C lz0_decompressor
	make -C lz4_decompressor
	make -C lz4_compressor

clean: 
	make clean -C lz0_compressor
	make clean -C lz0_decompressor
	make clean -C lz4_decompressor
	make clean -C lz4_compressor
