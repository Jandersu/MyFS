#define FUSE_USE_VERSION 31
#include <stdio.h>
#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define FILE_PATH "/Documentos/TesteMakefile"

/** Function to add an entry in a readdir() operation
 *
 * The *off* parameter can be any non-zero value that enables the
 * filesystem to identify the current point in the directory
 * stream. It does not need to be the actual physical position. A
 * value of zero is reserved to indicate that seeking in directories
 * is not supported.
 * 
 * @param buf the buffer passed to the readdir() operation
 * @param name the file name of the directory entry
 * @param stbuf file attributes, can be NULL
 * @param off offset of the next entry or zero
 * @param flags fill flags
 * @return 1 if buffer is full, zero otherwise
 */
 
 typedef int (*fuse_fill_dir_t) (void *buf, const char *name,
				const struct stat *stbuf, off_t off);

static struct stat regular_file = {.st_mode = S_IFREG | 00400};

int my_getattr (const char *path, struct stat *s){

	/*if(!strcmp(path, "/")){
		s->st_mode = S_IFDIR | 00400;
	}
	if(!strcmp(path, "/my_file")){
		s->st_mode = regular_file.st_mode;
		s->st_size = strlen("Arquivo\n");
	}*/
	
	printf( "[getattr] Called\n");
	printf("Attributes of %s requested\n", path);
	
	s->st_uid = getuid();
	s->st_gid = getgid();
	s->st_atime = time(NULL);
	s->st_mtime = time (NULL);
	
	if (strcmp(path, "/") == 0)
	{
		s->st_mode = S_IFDIR | 0755;
		s->st_nlink = 2;
	}
	else
	{
		s->st_mode = S_IFREG | 0644;
		s->st_nlink = 1;
		s->st_size = 1024;
	}
	
	return 0;
}

int my_opendir (const char *, struct fuse_file_info *fInfo){

	return 0;
}

int my_readdir (const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset,
			struct fuse_file_info *fInfo){
	
	/*static struct stat regular_file = {.st_mode = S_IFREG | 00400};
    	if(!strcmp(path, "/")){
		filler(buf, "my_file", &regular_file, 0);
	}*/
	
	printf( "--> Getting The List of Files of %s\n", path );
	
	filler( buffer, ".", NULL, 0 ); // Current Directory
	filler( buffer, "..", NULL, 0 ); // Parent Directory
	
	if ( strcmp( path, "/" ) == 0 ) // If the user is trying to show the files/directories of the 			root directory show the following
	{
		filler( buffer, "file54", NULL, 0 );
		filler( buffer, "file349", NULL, 0 );
	}
	return 0;			
}

int my_mkdir (const char *path, mode_t mode){
	
	int res;

	res = mkdir(path, mode);
	if (res == -1)
		return -errno;
	
	return 0;
}

int my_rmdir (const char *){
	return 0;
}

int my_openFile (const char *path, struct fuse_file_info *fInfo){
	int res;

	res = open(path, fInfo->flags);
	if (res == -1)
		return -errno;

	fInfo->fh = res;
	return 0;
}

int my_readFile (const char *path, char *buffer, size_t size, off_t offset,
		     struct fuse_file_info *fInfo){
		     
	(void)fInfo;

    FILE *file = fopen(FILE_PATH, "rb");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        return -errno;
    }
    
    if (fseek(file, offset, SEEK_SET) != 0) {
        perror("Erro ao posicionar o cursor no arquivo");
        fclose(file);
        return -errno;
    }

    size_t bytesRead = fread(buffer, 1, size, file);
    if (bytesRead < 0) {
        perror("Erro ao ler dados do arquivo");
        fclose(file);
        return -errno;
    }

    fclose(file);

    return bytesRead;
}

int my_writeFile (const char *path, const char *buffer, size_t size, off_t offset,
		      struct fuse_file_info *fInfo){
		(void) fInfo;
		
		char filePath[255];
		snprintf(filePath, sizeof(filePath), "%s%s", "~/Documentos/TesteMakefile/", path);
		
		int fd = open(filePath, O_WRONLY);
		if (fd == -1){
			perror("ERRO ao abrir o arquivo para escrita\n");
			return -errno;
		}
		// Posiciona o cursor no offset correto no arquivo
    	if (lseek(fd, offset, SEEK_SET) == -1) {
        	perror("Erro ao posicionar o cursor no arquivo");
        	close(fd);
        	return -errno;
    	}

    	// Escreve os dados no arquivo
    	ssize_t bytesWritten = write(fd, buffer, size);
    	if (bytesWritten == -1) {
        	perror("Erro ao escrever dados no arquivo");
        	close(fd);
        	return -errno;
    	}

    	// Fecha o arquivo
    	close(fd);

    	return bytesWritten;
}

static struct fuse_operations operations = {
	.getattr = my_getattr,
	.opendir = my_opendir, // to do
	.readdir = my_readdir,
	.mkdir = my_mkdir, // to do
	.rmdir = my_rmdir, // to do
	.open = my_openFile,
	.read = my_readFile,
	.write = my_writeFile // to do
};

int main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &operations, NULL);
}