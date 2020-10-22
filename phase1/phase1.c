/*
Author: Allison Hurley and Benjamin Kocik
Course: CE-480 Lab project phase 1
Purpose: Demonstrate some basic use of command line arguments passed to a program
Late Modified: 10/22/2020
Use: phase1 string integer
	The first parameter is any string (w/o whitespace), used to show some characters array processing.
	The second parameter is an integer (which is interpretted as an ASCII string), to show how to get the parameter
		back to an integer with atoi()
*/
#include <stdio.h>    // needed to print to the command window screen
#include <stdlib.h>   // ascii to integer conversion
#include <unistd.h>   // used for Getopt

void printbox(int rows, int cols, char value);

int main(int argc, char *argv[])
{
	int opt;
	int nrows;
	int ncols;
	char value;

	nrows = 4;
	ncols = 8;
	value = '#';

	while ((opt = getopt(argc, argv, "r:c:v:")) != -1){
		switch (opt) {
			case 'r': 
				nrows = atoi(optarg);
				break;
			case 'c':
				ncols = atoi(optarg);
				break;
			case 'v':
				value = *optarg;
				break;
			default:
                exit(1);

		}
	}

	printbox(nrows, ncols, value);
}

void printbox(int rows, int cols, char value)
{
	int row, col;
	for (row = 0; row < rows; row++)
	{
		for (col = 0; col < cols; col++)
		{
			printf("%c", value);
		}
		printf("\n");
	}
}