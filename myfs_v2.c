#define FUSE_USE_VERSION 30
#include <stdio.h>
#include <stdlib.h>
#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>

/* Diretorio raiz  do ponto de montagem */
static const char *dir = "/";

/* Descrição da função filler usada em fuse_readdir 
*    -> Descrição retirada de fuse.h
*/
/** Function to add an entry in a readdir() operation
 *
 * The *off* parameter can be any non-zero value that enables the
 * filesystem to identify the current point in the directory
 * stream. It does not need to be the actual physical position. A
 * value of zero is reserved to indicate that seeking in directories
 * is not supported.
 * 
 * @param buffer the buffer passed to the readdir() operation
 * @param name the file name of the directory entry
 * @param stbuf file attributes, can be NULL
 * @param offet offset of the next entry or zero
 * @param flags fill flags
 * @return 1 if buffer is full, zero otherwise
 */
 
 typedef int (*fuse_fill_dir_t) (void *buffer, const char *name,
				const struct stat *stbuf, off_t offset);

/*Essa função é responsável por buscar as informações de arquivos
* e diretorios. Quando executado o comando ls -l
*  é informado  as permissões, o proprietário, 
* tamanho e ultimo acesso/ modifcação e nome do arquivo/diretorio
* exemplo:
* drwxr-xr-x 2 root root 4096 nov 26 10:54 dir0
*/
int my_getattr (const char *path, struct stat *s){
	
	// Verificação no debug para saber se foi chamada
	printf("Função [getattr] chamada\n");

	int res = 0;
	char full_path[256];

	snprintf(full_path, sizeof(full_path), "%s%s", dir, path);

	res = lstat(full_path, s);
	if (res == -1)
		return -errno;
	
	return 0;
}

/* Função responsavel por ler diretorios .*/
int my_readdir (const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset,
			struct fuse_file_info *fInfo){
	// Verificação no debug para saber se foi chamada
	printf("Função [readdir] chamada\n");

	
	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void)	fInfo;
	
	char full_path[256];
	snprintf(full_path, sizeof(full_path), "%s%s", dir, path);
	
	dp = opendir(full_path);
	if (dp == NULL)
		return -errno;
	
	while ((de = readdir(dp)) != NULL) {
		if (filler(buffer, de->d_name, NULL, 0) != 0){
			closedir(dp);
			return -ENOMEM;
		}
	}

	closedir(dp);

	return 0;
}

/*Função responsavel por abrir arquivos */
int my_open(const char *path, struct fuse_file_info *fInfo){

	// Verificação no debug para saber se foi chamada
	printf("Função [open] chamada\n");
	
	int res = 0;

	char full_path[256];

	snprintf(full_path, sizeof(full_path), "%s%s", dir, path);
	
	res = open(full_path, fInfo->flags);
	if (res == -1)
		return -errno;
	
	fInfo->fh = res;

	return 0;
}

/* Função respnsavel por ler o conteudo de arquivos abertos pela função fuse_open() */
int my_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fInfo){
	
	// Verificação no debug para saber se foi chamada
	printf("Função [read] chamada\n");

	int res = 0;
	(void) path;

	res = pread(fInfo->fh, buffer, size, offset);
	if (res == -1)
		res = -errno;
	
	
	return res;
}

/*função responsavel por escrever em arquivos abertos chamado pela função open() */
int my_write(const char *path,  const char *buffer, size_t size, off_t offset, struct fuse_file_info *fInfo){

	// Verificação no debug para saber se foi chamada
	printf("Função [write] chamada\n");

	int res = 0;

	(void) path;

	res = pwrite(fInfo->fh, buffer, size, offset);
	if (res == -1)
		res = -errno;
	
	return res;
}

/* Função para criar diretorios */
int my_mkdir (const char *path, mode_t mode){

	// Verificação no debug para saber se foi chamada
	printf("Função [mkdir] chamada\n");

	int res = 0;
	char full_path[256];

	snprintf(full_path, sizeof(full_path), "%s%s", dir, path);

	res = mkdir(full_path, mode| __S_IFDIR);
	if (res == -1)
		return -errno;
	return 0;
}

/* Função criada  para criação de arquivos, principalmente arquivos regulares*/
int my_mknod(const char *path, mode_t mode, dev_t dev){

	// Verificação no debug para saber se foi chamada
	printf("Função [mknod] chamada\n");

	int res = 0;
	char full_path[256];
	snprintf(full_path, sizeof(full_path), "%s%s", dir, path);

	if (S_ISREG(mode)){
		res = open(full_path, (O_CREAT | O_EXCL | O_WRONLY), mode);
		if (res >= 0)
			res = close(res);
	} 
	else if (S_ISFIFO(mode)){
		res = mkfifo(full_path, mode);
	}
	else{
		res = mknod(full_path, mode, dev);
	}

	if (res ==  -1)
		return -errno;

	return 0;
}

/*Função para remoção de diretorios */
int my_rmdir(const char *path){
	
	// Verificação no debug para saber se foi chamada
	printf("Função [rmdir] chamada\n");

	int res = 0;
	char full_path[256];

	snprintf(full_path, sizeof(full_path), "%s%s", dir, path);

	res = rmdir(full_path);
	if (res == -1)
		return -errno;
	
	return 0;
}

/*Função para a remoção de arquivos */
int my_unlink(const char *path){

	// Verificação no debug para saber se foi chamada
	printf("Função [unlink] chamada\n");

	int res = 0;
	char full_path[256];
	snprintf(full_path, sizeof(full_path), "%s%s", dir, path);

	res = unlink(full_path);
	if (res == -1)
		return -errno;
	
	return 0;
}

/* Operações fuse implementadas 
* Para adição de novas operações, fazer da seguinte forma:
* .nome_operacao = escolher_nome_que_quiser 
* .nome_operacao pode ser encontrada em fuse.h, junto
* das suas definições.*/
static struct fuse_operations operations = {
		.getattr = my_getattr,
		.readdir = my_readdir,
		.open = my_open,
		.read = my_read,
		.write = my_write,
		.mkdir = my_mkdir,
		.mknod = my_mknod,
		.rmdir = my_rmdir,
		.unlink = my_unlink
};

int main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &operations, NULL);
}