#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>


#include "matrix.h"


#define MAX_CMD_COUNT 50

/*protected functions*/
void load_matrix (Matrix_t* m, unsigned int* data);

/* 
 * PURPOSE: instantiates a new matrix with the passed name, rows, cols 
 * INPUTS: 
 *	name the name of the matrix limited to 50 characters 
 *  rows the number of rows the matrix
 *  cols the number of cols the matrix
 * RETURN:
 *  If no errors occurred during instantiation then true
 *  else false for an error in the process.
 *
 **/

bool create_matrix (Matrix_t** new_matrix, const char* name, const unsigned int rows,
						const unsigned int cols) {
	// Check parameters
	if(!new_matrix || !name) {
		return false;
	} else if((rows == 0) || (cols == 0)) {
		return false;
	}

	// Allocate Matrix_t structure
	*new_matrix = calloc(1,sizeof(Matrix_t));
	if (!(*new_matrix)) {
		return false;
	}
	
	// Allocate the data for the matrix
	(*new_matrix)->data = calloc(rows * cols,sizeof(unsigned int));
	if (!(*new_matrix)->data) {
		return false;
	}
	
	// Set values
	(*new_matrix)->rows = rows;
	(*new_matrix)->cols = cols;
	unsigned int len = strlen(name) + 1; 
	if (len > MATRIX_NAME_LEN) {
		return false;
	}
	strncpy((*new_matrix)->name,name,len);
	return true;

}

/* 
 * PURPOSE: Frees memory associated with Matrix supplied
 * INPUTS: 
 *	m : Pointer to the Matrix_t pointer desired to have memory freed
 * RETURN: NONE
 **/
void destroy_matrix (Matrix_t** m) {
	//Check parameter
	if(!m || !(*m) || !((*m)->data)) {
		return;
	}

	free((*m)->data);
	free(*m);
	*m = NULL;
}

/* 
 * PURPOSE: Check if matrices are equivalent
 * INPUTS: 
 *	a : Pointer to Matrix_t of first matrix to compare
 *	b : Pointer to Matrix_t of second matrix to compare
 * RETURN: True if the supplied matrices are equivalent, else false
 **/
bool equal_matrices (Matrix_t* a, Matrix_t* b) {
	// Check parameters
	if (!a || !b || !a->data || !b->data) {
		return false;	
	}

	int result = memcmp(a->data,b->data, sizeof(unsigned int) * a->rows * a->cols);
	if (result == 0) {
		return true;
	}
	return false;
}

/* 
 * PURPOSE: Copy contents of one matrix into another
 * INPUTS: 
 *	src : Pointer to Matrix_t to used be as the source for the copying
 *	dest : Pointer to Matrix_t to used be as the destination for the copying
 * RETURN: NONE
 **/
bool duplicate_matrix (Matrix_t* src, Matrix_t* dest) {
	// Check parameters
	if (!src || !dest || !src->data || !dest->data) {
		return false;
	}
	/*
	 * copy over data
	 */
	unsigned int bytesToCopy = sizeof(unsigned int) * src->rows * src->cols;
	memcpy(dest->data,src->data, bytesToCopy);	
	return equal_matrices (src,dest);
}

/* 
 * PURPOSE: Bitwise shift matrix based on arguments supplied
 * INPUTS: 
 *	a : Pointer to Matrix_t to shift
 *	direction : Character to specify left or right shift
 *  shift : Unsigned int to shift matrix by
 * RETURN: True if successful shift occurred, else false
 **/
bool bitwise_shift_matrix (Matrix_t* a, char direction, unsigned int shift) {
	
	// Check parameters
	if (!a || !a->data) {
		return false;
	} else if((direction != 'l') && (direction != 'r')) {
		return false;
	}

	// Left shift
	if (direction == 'l') {
		unsigned int i = 0;
		for (; i < a->rows; ++i) {
			unsigned int j = 0;
			for (; j < a->cols; ++j) {
				a->data[i * a->cols + j] = a->data[i * a->cols + j] << shift;
			}
		}

	}
	else { // Right shift
		unsigned int i = 0;
		for (; i < a->rows; ++i) {
			unsigned int j = 0;
			for (; j < a->cols; ++j) {
				a->data[i * a->cols + j] = a->data[i * a->cols + j] >> shift;
			}
		}
	}
	
	return true;
}

/* 
 * PURPOSE: Add two matrices together
 * INPUTS: 
 *	a : Pointer to first Matrix_t to add
 *	b : Pointer to second Matrix_t to add
 *  c : Pointer to Matrix_t to store result of addition between a and b
 * RETURN: True if successful addition occurred, else false
 **/
bool add_matrices (Matrix_t* a, Matrix_t* b, Matrix_t* c) {

	// Check parameters
	if(!a || !b || !c || !a->data || !b->data || !c->data) {
		return false;
	}
	if (a->rows != b->rows && a->cols != b->cols) {
		return false;
	}

	for (int i = 0; i < a->rows; ++i) {
		for (int j = 0; j < b->cols; ++j) {
			c->data[i * a->cols +j] = a->data[i * a->cols + j] + b->data[i * a->cols + j];
		}
	}
	return true;
}

/* 
 * PURPOSE: Print contents of matrix
 * INPUTS: 
 *	m : Pointer to Matrix_t to display
 * RETURN: NONE
 **/
void display_matrix (Matrix_t* m) {
	
	// Check parameter
	if(!m || !m->data) {
		return;
	}

	printf("\nMatrix Contents (%s):\n", m->name);
	printf("DIM = (%u,%u)\n", m->rows, m->cols);
	for (int i = 0; i < m->rows; ++i) {
		for (int j = 0; j < m->cols; ++j) {
			printf("%u ", m->data[i * m->cols + j]);
		}
		printf("\n");
	}
	printf("\n");

}

/* 
 * PURPOSE: Read a matrix from a file into a Matrix_t structure
 * INPUTS: 
 *	martix_input_filename : filename to read matrix from
 *	m : Pointer to Matrix_t pointer load matrix from file into
 * RETURN: True if successful read and load, else false
 **/
bool read_matrix (const char* matrix_input_filename, Matrix_t** m) {
	// Check parameter
	if(!matrix_input_filename || !m) {
		return false;
	}


	int fd = open(matrix_input_filename,O_RDONLY);
	if (fd < 0) {
		printf("FAILED TO OPEN FOR READING\n");
		if (errno == EACCES ) {
			perror("DO NOT HAVE ACCESS TO FILE\n");
		}
		else if (errno == EADDRINUSE ){
			perror("FILE ALREADY IN USE\n");
		}
		else if (errno == EBADF) {
			perror("BAD FILE DESCRIPTOR\n");	
		}
		else if (errno == EEXIST) {
			perror("FILE EXIST\n");
		}
		return false;
	}

	/*read the wrote dimensions and name length*/
	unsigned int name_len = 0;
	unsigned int rows = 0;
	unsigned int cols = 0;
	
	if (read(fd,&name_len,sizeof(unsigned int)) != sizeof(unsigned int)) {
		printf("FAILED TO READING FILE\n");
		if (errno == EACCES ) {
			perror("DO NOT HAVE ACCESS TO FILE\n");
		}
		else if (errno == EADDRINUSE ){
			perror("FILE ALREADY IN USE\n");
		}
		else if (errno == EBADF) {
			perror("BAD FILE DESCRIPTOR\n");	
		}
		else if (errno == EEXIST) {
			perror("FILE EXIST\n");
		}
		return false;
	}
	char name_buffer[50];
	if (read (fd,name_buffer,sizeof(char) * name_len) != sizeof(char) * name_len) {
		printf("FAILED TO READ MATRIX NAME\n");
		if (errno == EACCES ) {
			perror("DO NOT HAVE ACCESS TO FILE\n");
		}
		else if (errno == EADDRINUSE ){
			perror("FILE ALREADY IN USE\n");
		}
		else if (errno == EBADF) {
			perror("BAD FILE DESCRIPTOR\n");	
		}
		else if (errno == EEXIST) {
			perror("FILE EXIST\n");
		}

		return false;	
	}

	if (read (fd,&rows, sizeof(unsigned int)) != sizeof(unsigned int)) {
		printf("FAILED TO READ MATRIX ROW SIZE\n");
		if (errno == EACCES ) {
			perror("DO NOT HAVE ACCESS TO FILE\n");
		}
		else if (errno == EADDRINUSE ){
			perror("FILE ALREADY IN USE\n");
		}
		else if (errno == EBADF) {
			perror("BAD FILE DESCRIPTOR\n");	
		}
		else if (errno == EEXIST) {
			perror("FILE EXIST\n");
		}

		return false;
	}

	if (read(fd,&cols,sizeof(unsigned int)) != sizeof(unsigned int)) {
		printf("FAILED TO READ MATRIX COLUMN SIZE\n");
		if (errno == EACCES ) {
			perror("DO NOT HAVE ACCESS TO FILE\n");
		}
		else if (errno == EADDRINUSE ){
			perror("FILE ALREADY IN USE\n");
		}
		else if (errno == EBADF) {
			perror("BAD FILE DESCRIPTOR\n");	
		}
		else if (errno == EEXIST) {
			perror("FILE EXIST\n");
		}

		return false;
	}

	unsigned int numberOfDataBytes = rows * cols * sizeof(unsigned int);
	unsigned int *data = calloc(rows * cols, sizeof(unsigned int));
	if (read(fd,data,numberOfDataBytes) != numberOfDataBytes) {
		printf("FAILED TO READ MATRIX DATA\n");
		if (errno == EACCES ) {
			perror("DO NOT HAVE ACCESS TO FILE\n");
		}
		else if (errno == EADDRINUSE ){
			perror("FILE ALREADY IN USE\n");
		}
		else if (errno == EBADF) {
			perror("BAD FILE DESCRIPTOR\n");	
		}
		else if (errno == EEXIST) {
			perror("FILE EXIST\n");
		}

		return false;	
	}

	if (!create_matrix(m,name_buffer,rows,cols)) {
		return false;
	}

	load_matrix(*m,data);
	free(data);
	if (close(fd)) {
		return false;

	}
	return true;
}

/* 
 * PURPOSE: Write a Matrix_t to a file
 * INPUTS: 
 *	martix_output_filename : filename to write matrix to
 *	m : Pointer to Matrix_t to write to file
 * RETURN: True if successful write, else false
 **/
bool write_matrix (const char* matrix_output_filename, Matrix_t* m) {
	
	//Check parameter
	if(!matrix_output_filename || !m || !m->data) {
		return false;
	}

	int fd = open (matrix_output_filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
	/* ERROR HANDLING USING errorno*/
	if (fd < 0) {
		printf("FAILED TO CREATE/OPEN FILE FOR WRITING\n");
		if (errno == EACCES ) {
			perror("DO NOT HAVE ACCESS TO FILE\n");
		}
		else if (errno == EADDRINUSE ){
			perror("FILE ALREADY IN USE\n");
		}
		else if (errno == EBADF) {
			perror("BAD FILE DESCRIPTOR\n");	
		}
		else if (errno == EEXIST) {
			perror("FILE EXISTS\n");
		}
		return false;
	}
	/* Calculate the needed buffer for our matrix */
	unsigned int name_len = strlen(m->name) + 1;
	unsigned int numberOfBytes = sizeof(unsigned int) + (sizeof(unsigned int)  * 2) + name_len + sizeof(unsigned int) * m->rows * m->cols + 1;
	/* Allocate the output_buffer in bytes
	 * IMPORTANT TO UNDERSTAND THIS WAY OF MOVING MEMORY
	 */
	unsigned char* output_buffer = calloc(numberOfBytes,sizeof(unsigned char));
	unsigned int offset = 0;
	memcpy(&output_buffer[offset], &name_len, sizeof(unsigned int)); // IMPORTANT C FUNCTION TO KNOW
	offset += sizeof(unsigned int);	
	memcpy(&output_buffer[offset], m->name,name_len);
	offset += name_len;
	memcpy(&output_buffer[offset],&m->rows,sizeof(unsigned int));
	offset += sizeof(unsigned int);
	memcpy(&output_buffer[offset],&m->cols,sizeof(unsigned int));
	offset += sizeof(unsigned int);
	memcpy (&output_buffer[offset],m->data,m->rows * m->cols * sizeof(unsigned int));
	offset += (m->rows * m->cols * sizeof(unsigned int));
	output_buffer[numberOfBytes - 1] = EOF;

	if (write(fd,output_buffer,numberOfBytes) != numberOfBytes) {
		printf("FAILED TO WRITE MATRIX TO FILE\n");
		if (errno == EACCES ) {
			perror("DO NOT HAVE ACCESS TO FILE\n");
		}
		else if (errno == EADDRINUSE ){
			perror("FILE ALREADY IN USE\n");
		}
		else if (errno == EBADF) {
			perror("BAD FILE DESCRIPTOR\n");	
		}
		else if (errno == EEXIST) {
			perror("FILE EXIST\n");
		}
		return false;
	}
	
	if (close(fd)) {
		return false;
	}
	free(output_buffer);

	return true;
}

/* 
 * PURPOSE: Load random numbers into matrix supplied
 * INPUTS: 
 *	m : Pointer to Matrix_t to load random numbers into
 *	start_range : Starting number for the range of random numbers to use
 *  end_range : Ending number for the range of random numbers to use
 * RETURN: True if successful initialization of random values, else false
 **/
bool random_matrix(Matrix_t* m, unsigned int start_range, unsigned int end_range) {
	
	//Check parameter
	if(!m || !m->data) {
		return false;
	}
	for (unsigned int i = 0; i < m->rows; ++i) {
		for (unsigned int j = 0; j < m->cols; ++j) {
			m->data[i * m->cols + j] = rand() % (end_range + 1 - start_range) + start_range;
		}
	}
	return true;
}

/*Protected Functions in C*/

/* 
 * PURPOSE: Load data into Matrix_t
 * INPUTS: 
 *	m : Pointer to Matrix_t to load data into numbers into
 *	data : Pointer to data to load into the matrix
 * RETURN: NONE
 **/
void load_matrix (Matrix_t* m, unsigned int* data) {
	// Check parameters
	if(!m || !m->data || !data) {
		return;
	}
	memcpy(m->data,data,m->rows * m->cols * sizeof(unsigned int));
}

/* 
 * PURPOSE: Add matrix to the array of matrices
 * INPUTS: 
 *	mats : Array of Martix_t pointers
 *  new_matrix : Pointer to new matrix to add
 *  num_mats : Number of matrices in array
 * RETURN: The array index one after the new matrix position, else returns -1
 **/
unsigned int add_matrix_to_array (Matrix_t** mats, Matrix_t* new_matrix, unsigned int num_mats) {
	static long int current_position = 0;
	const long int pos = current_position % num_mats;

	// Check parameters
	if(!mats || !new_matrix) {
		return -1;
	}

	if ( mats[pos] ) {
		destroy_matrix(&mats[pos]);
	} 
	mats[pos] = new_matrix;
	current_position++;
	return pos;
}
