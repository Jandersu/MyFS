# Compilador
CC = gcc

# Arquivos
FILES = myfs.c

myfs: $(FILES) 
	$(CC) $(FILES) -o myfs 'pkg-config fuse --cflags --libs'
	echo 'montando sistema de arquivo: ./myfs -f [mount point]'

clean:
	rm myfs