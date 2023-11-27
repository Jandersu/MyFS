# Compilador
CC=gcc

# arquivos
FILES=myfs_v2.c

# Flags de compilação
CFLAGS = -D_FILE_OFFSET_BITS=64

myfs: $(FILES) 
	$(CC) $(CFLAGS) $(FILES) -o myfs `pkg-config fuse --cflags --libs`
	echo 'Montando sistema de arquivo: ./myfs -f [mount point]'

clean:
	rm myfs