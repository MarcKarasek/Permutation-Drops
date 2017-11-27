// permutation_drops.cpp : main project file.
/*
 * permutation_drops.cpp
 * Copyright (C) Marc Karasek 2011 <marckarasek@gmail.com>
 * 
 * permutation_drops is free software copyrighted by Marc Karasek.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name ``Marc Karasek'' nor the name of any other
 *    contributor may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 
 * permutation_drops IS PROVIDED BY Marc Karasek ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Marc Karasek OR ANY OTHER CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>

using namespace std;

void print_array(unsigned char *array_ptr, unsigned int array_size);
void visit(int k);
void move_array(void);
void drop_counter(unsigned int result_count, bool last);

int rpoly_main(double * coeff, unsigned int poly_degree, int nmbr_passes);

void * array_ptr = NULL; // Input Array Holds the initial data, either user input or sequential values
void * work_ptr = NULL; // Holds the permutation arrays, created on the fly using as input the array_ptr and permutation_array as an index.
void * permutation_array = NULL; // Holds the permutation values used to create the next permutation array from the array_ptr
void * compare_array = NULL; // Holds user input data for how many places to the left to compare against
void * temp = NULL; // General work ptr
int array_size; // User Input for total number of elements in the array
void * results;
unsigned int result_cnt = 0;
unsigned int result_buff_cnt = 1;
void * drop_count;
unsigned int drop_cnt;
unsigned int compare_aggregate = 0;
unsigned long long global_rslt_cnt=0;
#define RESULT_SZ 512
#define ARRAY_SZ 256

using namespace System;

/* Version 3.0 will accept a input text file to generate multiple passes.
Format for the file is as follows:
<total number of passes n>
<array size pass 1>
<drops for pass 1>
...
<array size pass 2>
<drops for pass 2>
...
<array size for pass n>
<drops for pass n>
...
Example of 2 passes 
array 1 = 5
array 2 = 6
2
5
1
2
2
0
6
1
2
2
1
0
*/

int main(int argc, char *argv[])
{
	int x, array_input, nmbr_passes, y;
	unsigned char * head, * tail, *current;
	bool infile = false;
	ifstream ifile;

	printf("Permutations Drop Calulator V3.0\n");

	 if (argc > 1)
	 {
		// open file
		ifile.open(argv[1], ios::in);
		// Check for an error condition
		if (!ifile.good()) 
		{
			printf("Cannot open the input file %s.\n", argv[1]);
			return 0;
		}
		// Use a file as input...
		infile = true;

	 }

	// Allocate a memory region for the results..
	results = malloc(RESULT_SZ);
	if (results == NULL)
	{
		printf("Results Malloc Failed\n");
		return(0);
	}
	memset(results, 0x00, RESULT_SZ);
	
	// Allocate a memory region for the input array
	array_ptr = malloc(ARRAY_SZ);
	if (array_ptr == NULL)
	{
		printf("Array Malloc Failed\n");
		free(results);
		return(0);
	}
	memset(array_ptr, 0x00, ARRAY_SZ);
	
	// Allocate a memory region for the work array
	work_ptr = malloc(ARRAY_SZ);
	if (work_ptr == NULL)
	{
		printf("Work Malloc Failed\n");
		free(array_ptr);
		free(results);
		return(0);
	}
	memset(work_ptr, 0x00, ARRAY_SZ);

	// Allocate a memory region for the permutation array
	permutation_array = malloc(ARRAY_SZ);
	if (permutation_array == NULL)
	{
		printf("Permutation Malloc Failed\n");
		free(array_ptr);
		free(results);
		free(work_ptr);
		return(0);
	}
	memset(permutation_array, 0x00, ARRAY_SZ);

	// Allocate a memory region for the compare array
	compare_array = malloc(ARRAY_SZ);
	if (compare_array == NULL)
	{
		printf("Compare Malloc Failed\n");
		free(array_ptr);
		free(results);
		free(work_ptr);
		free(permutation_array);
		return(0);
	}
	memset(compare_array, 0x00, ARRAY_SZ);
	
	head = (unsigned char *)array_ptr;
	tail = (unsigned char *)array_ptr;
	current = (unsigned char *)array_ptr;

	// If we have an input file, grab the number of passes we want to do.
	if (infile)
	{
		ifile >> nmbr_passes; // Input the total number of passes. 
		if (nmbr_passes < 0)
		{
			printf("Invalid number of passes. Program terminated. \n");
			ifile.close(); //Close the input file before terminating
			return 0;
		}
#ifdef _DEBUG
		printf("Number of Passes = %d\n", nmbr_passes);
#endif
	}
	else
		nmbr_passes = 1; // If we do not have a input file only do one pass

	for (y=0 ; y<nmbr_passes; y++)
	{
		// Clear compare_aggregate for each pass
		compare_aggregate = 0;
		if (infile)
		{
			ifile >> array_size; //Input the size of the array from the the file
			if (array_size < 0) 
			{
				printf("Invalid array size. Program terminated. \n");
				ifile.close(); //Close the input file before terminating
				return 0;
			}
#ifdef _DEBUG
			printf("Array Size = %d\n", array_size);
#endif
		}
		else
		{
			printf("Enter Size of the Array :");
			scanf_s("%d", &array_size);
			printf("\n");
		}
#ifdef DATAINPUT
		// Get the data for the array 
		for (x = 0; x<array_size; x++)
		{
			printf("Enter Array Element [0-255] %d :", x+1);
			scanf_s("%d", &array_input);
	
			*current = (unsigned char)array_input;
			current ++;
			tail++;
		}
#else
		// Initialize the array with seq values..
		for(x=1;x<=array_size;x++)
		{
			*current = x;
			current++;
			tail++;
		}
#endif
		current = (unsigned char *)compare_array;
		if (infile)
		{
			for (x = 0; x<array_size; x++)
			{
				if ((x+1) == array_size)
					array_input = 0;
				else
				{
					ifile >> array_input;
					if (array_input < 0) 
					{
						printf("Invalid Drop Count\n");
						ifile.close();
						return 0;
					}
				}
#ifdef _DEBUG
				printf("array_input %d\n", array_input);
#endif
				*current = (unsigned char)array_input;
#ifdef _DEBUG
				printf("Drop %d = %d\n", x, *current);
#endif
				current ++;
				compare_aggregate += array_input;
				
			}
		}
		else
		{
			// Get the Compare data for the array
			for (x = 0; x<array_size; x++)
			{
				if ((x+1) == array_size)
					array_input = 0;
				else
				{	
					printf("Enter Drop Element [0-255] %d :", x+1);
					scanf_s("%d", &array_input);
				}
				*current = (unsigned char)array_input;
				current ++;
				compare_aggregate += array_input;
			}
		}

		compare_aggregate++;
#ifdef _DEBUG
		printf("Compare Aggregate = %d\n", compare_aggregate);
#endif
		current = head;
#ifdef _DEBUG
		printf("Starting Array \n");
		print_array((unsigned char *)current, array_size);
#endif
		visit(0);

#ifdef _DEBUG
		printf("Results\n");
		print_array((unsigned char *)results, result_cnt);
#endif
		global_rslt_cnt+=result_cnt;
#ifdef _DEBUG
		printf("Result Count = %ld\n", global_rslt_cnt);
#endif
		printf("Drop Counter\n");
		drop_counter(result_cnt, true);
	
		rpoly_main((double *)drop_count, compare_aggregate, y+1);
		// free drop_count, it will be reallocated based on next pass array size 
		free(drop_count);

	}
		free(array_ptr);
		free(results);
		free(work_ptr);
		free(permutation_array);
		free(compare_array);
		ifile.close();
		return (0);
}


void drop_counter( unsigned int result_count, bool last)
{

	unsigned long long * drpcnt;
	unsigned char *rslts;
	unsigned int x;
	static bool init = false;
	rslts = (unsigned char *)results;
	if (init == false)
	{
		drop_count = malloc(compare_aggregate*8);
		memset(drop_count, 0x00, compare_aggregate*8);
		init = true;
	}
	if (drop_count == NULL)
	{
		printf("drop_count malloc failed\n");
		return;
	}
	
	drpcnt = (unsigned long long *) drop_count;
	
	// Count up the drops result_count
	for (x=0;x<result_count;x++)
	{
		drpcnt[rslts[x]]+=1;
	}

	if (last == true)
	{
		// print out the drops
		for (x=0; x<compare_aggregate; x++)
		{
			printf("DropCount[%d] = %ld\n", x, drpcnt[x]);
		}
		// Set up for the next pass if multipass 
		init = false;
	}
	else
		printf(".");

}

void print_array(unsigned char *array_ptr, unsigned int array_size)
{
	unsigned int x;
	
	for (x = 1;x<=array_size; x++)
	{
		printf(" %d", *array_ptr);
		array_ptr ++;
		if (x%32 == 0)
			printf("\n");
	}
	printf("\n");
}

void visit(int k)
{
	int i;
	static int level = -1;
	level = level+1;
	unsigned char * temp;

	temp = (unsigned char *)permutation_array;

	*(unsigned char *)(temp + k) = level;
	
	if (level == array_size)
	{
		move_array();
	}
	else
		for (i = 0; i < array_size; i++)
			if (*(unsigned char *)(temp+i) == 0)
    			visit(i);

	level = level-1; *(unsigned char *)(temp+k) = 0;
}

void do_compare( void)
{
	int x, y;
	unsigned char * compare;
	unsigned char * work;
	unsigned char * result;
	int count = 0;

	compare = (unsigned char *)compare_array;
	work = (unsigned char *)work_ptr;
	result = (unsigned char *)results;
	
	for (x=0; x<array_size; x++)
	{
		if (compare[x] != 0)
		{
			for (y=1; y<=compare[x]; y++)
			{
#ifdef _DEBUG
				printf("work[x] = %d\n", work[x]);
				printf("work[x+y] = %d\n", work[x+y]);
#endif
				if (work[x] > work[x+y])
				{
					count ++;
				}
			}
		}
#ifdef _DEBUG
		printf("Compare %d\n", compare[x]);
		printf("Count = %d\n", count);
#endif
	}
#ifdef _DEBUG
	printf("Count %d\n",count);
	printf("Result Count %d\n", result_cnt);

	if (result_cnt%1024==0)
	{
		printf("Count = %d, Result_Count = %d\n", count, result_cnt);
	}
#endif
	result[result_cnt] = count;
	result_cnt++;	
	// Count up the drops so far and reset the counter.
	if ( result_cnt >= (RESULT_SZ) )
	{
		drop_counter(RESULT_SZ, false);
		global_rslt_cnt+=RESULT_SZ;
		result_cnt = 0;
	}
}

void move_array(void)
{
	int x;
	unsigned char * a, * b, * c;

	a = (unsigned char *)work_ptr;
	b = (unsigned char *)array_ptr;
	c = (unsigned char *)permutation_array;
	
	for (x=0; x < array_size; x++)
	{
		a[x] = b[c[x]-1]; 
	}
#ifdef _DEBUG
	printf("Permutation Array\n");
	print_array((unsigned char *)work_ptr, array_size);
#endif

	do_compare();
}

