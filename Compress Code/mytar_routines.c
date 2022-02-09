#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mytar.h"

extern char *use;

/** Copy nBytes bytes from the origin file to the destination file.
 *
 * origin: pointer to the FILE descriptor associated with the origin file
 * destination:  pointer to the FILE descriptor associated with the destination file
 * nBytes: number of bytes to copy
 *
 * Returns the number of bytes actually copied or -1 if an error occured.
 */
int
copynFile(FILE * origin, FILE * destination, int nBytes)
{
	// Complete the function
	int res;
	int contador = 0;
	while((contador < nBytes) && ((res = getc(origin)) != EOF)){
		putc(res, destination);
		++contador;
	}
	
	if(contador != 0) return contador;
	return -1;
}

/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor 
 * 
 * The loadstr() function must allocate memory from the heap to store 
 * the contents of the string read from the FILE. 
 * Once the string has been properly built in memory, the function returns
 * the starting address of the string (pointer returned by malloc()) 
 * 
 * Returns: !=NULL if success, NULL if error
 */
char*
loadstr(FILE * file)
{
	// Complete the function
	char *c = NULL;
	c = (char *) malloc(sizeof(char) * PATH_MAX);
	int cont = 0;
	char res;
	while(((res = getc(file)) != '\0') && (res != EOF)){
		c[cont] = (char)res;
		cont++;
	}

	if(cont > 0){
	 //c[cont] = '\0';
	 return c;
	}
	return NULL;
}

/** Read tarball header and store it in memory.
 *
 * tarFile: pointer to the tarball's FILE descriptor 
 * nFiles: output parameter. Used to return the number 
 * of files stored in the tarball archive (first 4 bytes of the header).
 *
 * On success it returns the starting memory address of an array that stores 
 * the (name,size) pairs read from the tar file. Upon failure, the function returns NULL.
 */
stHeaderEntry*
readHeader(FILE * tarFile, int *nFiles)
{
	// Complete the function
	stHeaderEntry* array = NULL;
	fread(nFiles, sizeof(int), 1, tarFile); //Read one int from tarFile

	array=malloc(sizeof(stHeaderEntry) * (*nFiles));
	int files = *nFiles;
	for(int i = 0; i < files; i++){
		array[i].name = loadstr(tarFile);	//Read the name of file and save it into the pointer
		fread(&array[i].size, sizeof(unsigned int), 1, tarFile);	//Grab the size from the file
	}
	return array;
}

/** Creates a tarball archive 
 *
 * nfiles: number of files to be stored in the tarball
 * filenames: array with the path names of the files to be included in the tarball
 * tarname: name of the tarball archive
 * 
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First reserve room in the file to store the tarball header.
 * Move the file's position indicator to the data section (skip the header)
 * and dump the contents of the source files (one by one) in the tarball archive. 
 * At the same time, build the representation of the tarball header in memory.
 * Finally, rewind the file's position indicator, write the number of files as well as 
 * the (file name,file size) pairs in the tar archive.
 *
 * Important reminder: to calculate the room needed for the header, a simple sizeof 
 * of stHeaderEntry will not work. Bear in mind that, on disk, file names found in (name,size) 
 * pairs occupy strlen(name)+1 bytes.
 *
 */
int
createTar(int nFiles, char *fileNames[], char tarName[])
{
	// Complete the function
	stHeaderEntry *headers = NULL;
	int contador = 0;
	int offData = 0;
	FILE *salida = NULL;
	FILE *auxiliar = NULL;

	headers = malloc(sizeof(stHeaderEntry)*nFiles);
	salida = fopen(tarName, "w");

	for(int i = 0; i < nFiles; i++){
		contador = contador + (strlen(fileNames[i]) + 1);
	}
	offData = contador + sizeof(int) + (nFiles*sizeof(unsigned int));
	fseek(salida, offData, SEEK_SET);

	for(int i = 0; i < nFiles; i++){
		headers[i].name = (char*) malloc(sizeof(fileNames[i]) + 1);
		strcpy(headers[i].name, fileNames[i]);
		auxiliar = fopen(headers[i].name, "r");
		headers[i].size = copynFile(auxiliar, salida, INT_MAX);
		fclose(auxiliar);
	}
	rewind(salida); //fseek abreviado
	fwrite(&nFiles, sizeof(int), 1, salida);

	for(int i = 0; i < nFiles; i++){
		fwrite(headers[i].name, sizeof(char), strlen(headers[i].name), salida);
		putc('\0', salida);
		fwrite(&headers[i].size, sizeof(unsigned int), 1, salida);
		free(headers[i].name);
	}
	free(headers);
	fclose(salida);
	if(nFiles > 0){
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

/** Extract files stored in a tarball archive
 *
 * tarName: tarball's pathname
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First load the tarball's header into memory.
 * After reading the header, the file position indicator will be located at the 
 * tarball's data section. By using information from the 
 * header --number of files and (file name, file size) pairs--, extract files 
 * stored in the data section of the tarball.
 *
 */
int
extractTar(char tarName[])
{
	// Complete the function
	FILE *entrada = NULL;
	FILE *arcNuev = NULL;
	entrada = fopen(tarName, "r");	
	int *nFiles = NULL;
	int files = 0;
	nFiles = malloc(sizeof(int));
	stHeaderEntry *headers;
	headers = readHeader(entrada, nFiles);
	files = *nFiles;
	for(int i = 0; i < files; i++){
		arcNuev = fopen(headers[i].name, "w");
		copynFile(entrada, arcNuev, headers[i].size);
		free(headers[i].name);
		fclose(arcNuev);
	}
	free(nFiles);
	free(headers);
	fclose(entrada);
	if(files != 0){
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}
