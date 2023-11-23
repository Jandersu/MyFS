# Compilador
CC=gcc

# arquivos
FILES=myfs.c

# Flags de compilação
CFLAGS = -D_FILE_OFFSET_BITS=64

myfs: $(FILES) 
	$(CC) $(CFLAGS) $(FILES) -o myfs `pkg-config fuse --cflags --libs`
	echo 'montando sistema de arquivo: ./myfs -f [mount point]'

clean:
	rm myfs