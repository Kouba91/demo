/*
*	PUZZLE SOLVE 1.15
*	Structure:	1. Structs			- definitions of objects, which are used for computations
*				2. Input & Output	- reads input from file/buffer, checks for structure, returns 2D array of numbers
*				3. Struct Methods	- methods to create, modify or search struct arrays
*				4. Main Array		- main engine of this program. Search array, find options, backtracking.
*				5. Main Functions	- main functions, that combine all of the above
*				6. Main				- main body of this program
*
*	Author: Jakub Novak
*	Nov-2020
*/


#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* max possible characters of one line of 32x32 field */
#define MAX_NUMBER_OF_CHARS 97
#define MAX_NUMBER_OF_ELEMENTS 200
#define BASIC_NUMBER_OF_OPTIONS 20
#define PREPROCESSOR_ID -1
#define ID_MULTIPLIER 500

/*-----------------------------------------STRUCTS-----------------------------------------*/

/*
*Option strcut. Holds information about one layout of one element and collisions with other options
*/
typedef struct{
	int id;							/* Option.id * ID_MULTIPLIER + position of this id. Is used for implacesArr of other elemets options */
									/* Unique number.	id / ID_MULTIPLIER - 1 represents position of element in array */
									/* 					id % ID_MULTIPLIER - 1 represents position of option in array */
	char * arrayOfIndexes;			/* arrays containing indexes of main array, where it will be filled. {x1,y1,x2,y2,x3,y3...} */
	char inUse;						/* says, if this option is being used right now for determination */
	char implacable;				/* says if this option is not possible to be placed right now */
	int implacedBy;					/* Id of the one, who implaced it, -1 for preprocessor */
	int * implacesArr;				/* Array that holds index of elements and options, that are in collision with this option. {e1,o1,e2,o2,...} */
	short numberOfImplaces;			/* Length of implacesArr */
	short maxNumberOfImplaces;		/* Maximum possible length of implacesArr (for dynamic reallocation) */
}Option;

/*
*Element struct. Holds information about one element, that is, one number, that is in the field and whose rectangle we want to find.
*/
typedef struct{
	int id;							/* id starts from 1, increments by 1. id is assigned to numbers from left top to right bottom */
	char col;						/* number representing column in main array, where element is located */
	char row;						/* number representing row in main array, where element is located */
	char value;						/* value of element. 1-99 */
	char inUse;						/* says, if this element is being used for computations right now */
	short numberOfOptions;			/* number of all possible options, how fields can be filled */
	short maxNumberOfOptions;		/* value representing maximum number of options (for dynamic reallocation) */
	short numberOfImplacableOptions;/* says how many options of one element are unable to be placed */
	short mediumNumberOfImplaces;
	Option * options;				/* Object containing all possible fillings of fields for one element */
}Element;

/*-----------------------------------------INPUT & OUTPUT-----------------------------------------*/

/*
*Read first or last row, which is only represented by - and +, check for errors and if ok, 
*compute number of cols for first line, which will serve as comparator for other rows
*/
int readTopAndBottomLine(int * numberOfCols){
	
	int numberOfColsChars, c;
	/* this is needed to make difference between top and bottom line */
	/* bottom line already starts from char 3, because first two chars have already been read in readInput() */
	numberOfColsChars = *numberOfCols > 0 ? 2 : 0;
	/* reading input */
	while ((c = getc(stdin)) != '\n'){
		if (numberOfColsChars % 3 == 0 && c != '+') return 0;
		else if (numberOfColsChars % 3 != 0 && c != '-') return 0;
		numberOfColsChars++;
		if (numberOfColsChars > MAX_NUMBER_OF_CHARS) return 0;
	}
	/* post processing the input */
	/* If first row, check if counter is valid number and compute number of columns */
	if (*numberOfCols == 0){
		if (numberOfColsChars - 1 >= 3 && (numberOfColsChars - 1) % 3 == 0){
			* numberOfCols = (numberOfColsChars - 1) / 3;
		}else return 0;
	/* If last row, check if it has the same length as other rows */
	} else if (*numberOfCols != (numberOfColsChars - 1) / 3) return 0;
	/* check for whole missing line (for bottom line) */
	if (numberOfColsChars == 2) return 0; 
	
	return 1;
}

/*
*Read rows, that contain fields. Checks for right format.
*Assign values in fields into main array. Empty fields are interpreted into 0
*/
int readRow(unsigned char mainArray[][32], int numberOfRows, int numberOfCols, int * totalNumbers){

	char counter = 1, c;
	while ((c = getc(stdin)) != '|'){
		/* Handle ' ' between fields */
		if (counter > 1 && counter % 3 == 0 && c != ' ') return 0;
		else if (counter > 1 && counter % 3 == 0 && c == ' '){
			counter++;
			continue;
		}
		/* check for numbers */
		int num = 0;
		/* 0-9 are in range 48-57 in ASCII */
		/* 0 is handled separately */
		if ( c >= 49 && c <= 57) num = (c - 48) * 10;
		else if ( c != ' ' ) return 0; 
		/* read next char, because we are still in one field, that may contain numbers */
		c = getc(stdin);
		if ( c >= 49 && c <= 57) num += (c - 48);
		else if ( c == '0' && num == 0) return 0;
		else if ( c != ' ' && c != '0') return 0; 
		/* handle number on bad spot */
		else if (c == ' ' && num != 0) return 0;
		/* check for length */
		if (counter + 2 > MAX_NUMBER_OF_CHARS) return 0;
		/* check, if totalNumbers overflows 200 (test condition: maximum 200 numbers in array) */
		*totalNumbers += num > 0 ? 1 : 0;
		if (*totalNumbers > 200) return 0;
		/* assign number into coressponding spot of array */
		mainArray[numberOfRows][counter / 3] = num;
		counter += 2;
	}
	/* check if length is same as first row */
	if (numberOfCols != (counter) / 3) return 0;
	/* after '|' must always be '\n' */
	if ((c = getc(stdin)) != '\n') return 0;
	return 1;
}

/*
*Read only rows, that are between fields, checks if they are given in right format.
*Returns 1, if all ok, 0 if wrong
*/
int readSemiRow(int numberOfCols){
	/* numberOfColsChars has to start from 2, because first two chars have already been read in readInput() */
	int numberOfColsChars = 2, c;
	/* check for right format */
	while ((c = getc(stdin)) != '\n'){
		if (numberOfColsChars % 3 == 0 && c != '+') return 0;
		else if (numberOfColsChars % 3 != 0 && c != ' ') return 0;
		numberOfColsChars++;
		if (numberOfColsChars > MAX_NUMBER_OF_CHARS) return 0;
	}
	/* check, if length is same as first row */
	if (numberOfCols != (numberOfColsChars - 1) / 3) return 0;
	
	return 1;
}

/*
*Read whole input. Check, if it is in right format.
*Assign to array only numbers and empty fields (represented by 0)
*/
int readInput(unsigned char mainArray[][32], int * numberOfRows, int * numberOfCols, int * totalNumbers){
	
	char c;
	int bottomLineRead = 0;
	int counter = 0;
	int semiCounter = 0;
	/* read top line */
	if (!readTopAndBottomLine(numberOfCols) || * numberOfCols == 0 || * numberOfCols > 32) return 0;
	/* keep reading while line starts with either '|' or '+' */
	while ((c = getc(stdin)) == '|' || c == '+'){
		if(bottomLineRead) return 0;
		counter++;
		/* read rows with fields */
		if (c == '|'){	
			/* For missing semilines */
			if (counter % 2 == 0) return 0;
			/* read one row */
			if (readRow(mainArray, *numberOfRows, *numberOfCols, totalNumbers)) (*numberOfRows)++;
			else return 0;
			if (*numberOfRows > 32) return 0;
		}else{
			/* if c is '+', check for next char */
			c = getc(stdin);
			if (c == ' '){
				/* read semi row */
				semiCounter++;
				if (counter % 2 != 0) return 0;
				if (!readSemiRow(*numberOfCols)) return 0;
			} else if (c == '-'){
				/* read bottom line */
				bottomLineRead = 1;
				if (!readTopAndBottomLine(numberOfCols)) return 0;
			}else return 0;
		}
	}
	/* check if all given conditions are met */
	if (*totalNumbers > 200 || *totalNumbers == 0 || !feof(stdin) || !bottomLineRead
	|| *numberOfRows > 32 || *numberOfCols > 32 || *numberOfRows == 0 || *numberOfCols == 0 || (semiCounter + 1 != *numberOfRows)) return 0;

	return 1;
}

/*
*Print first nad last line, that has given format
*/
void printTopAndBottomLine(int numberOfCols){

	printf("+");
	for (int i = 0; i < numberOfCols; i++) printf("--+");
	printf("\n");
}

/*
*Print semi line, that is between fields
*/
void printSemiLine(unsigned char mainArray[][32], Element * elements, int totalNumbers, int numberOfCols, int currentRow){

	printf("+");
	for (int i = 0; i < numberOfCols; i++){
		if (mainArray[currentRow + 1][i] == mainArray[currentRow][i]) printf("  +");
		else printf("--+");
	}
	printf("\n");

}

/*
*Print line with numbers
*/
void printLine(unsigned char mainArray[][32], Element * elements, int totalNumbers, int numberOfCols, int currentRow, unsigned char elementsArr[][32]){
	
	printf("|");
	for (int i = 0; i < numberOfCols - 1; i++){
		if (mainArray[currentRow][i] == mainArray[currentRow][i + 1]){
			if (elementsArr[currentRow][i] != 0) printf("%2d ", elementsArr[currentRow][i]);
			else printf("   ");
		}else{
			if (elementsArr[currentRow][i] != 0) printf("%2d|", elementsArr[currentRow][i]);
			else printf("  |");
		}
		
	}
	/* last field in row */
	if (elementsArr[currentRow][numberOfCols - 1] != 0) printf("%2d|\n", elementsArr[currentRow][numberOfCols - 1]);
	else printf("  |\n");
}

/*
*Print result table for one and only possible solution
*/
void printResult(unsigned char mainArray[][32], Element * elements, int totalNumbers, int numberOfRows, int numberOfCols){
	
	unsigned char elementsArr[32][32];
	printf("Jedno reseni:\n");
	for (int i = 0; i < numberOfRows; i++)
		for (int j = 0; j < numberOfCols; j++) elementsArr[i][j] = 0;
	
	for (int i = 0; i < totalNumbers; i++) elementsArr[(int)elements[i].row][(int)elements[i].col] = elements[i].value;
	
	printTopAndBottomLine(numberOfCols);
	
	for (int i = 0; i < numberOfRows; i++){
		printLine(mainArray, elements, totalNumbers, numberOfCols, i, elementsArr);
		if (i < numberOfRows - 1) printSemiLine(mainArray, elements, totalNumbers, numberOfCols, i);
	}
	printTopAndBottomLine(numberOfCols);
}

/*-----------------------------------------STRUCT METHODS-----------------------------------------*/

/*
*Creates and returns an array of elements.
*/
Element * createStructuresFromElements(unsigned char mainArray[][32], int totalNumbers, int numberOfRows, int numberOfCols){
	
	Element * elements = (Element*) malloc(totalNumbers * sizeof(*elements));
	int count = 0;
	
	for (int i = 0; i < numberOfRows; i++){
		for (int j = 0; j < numberOfCols; j++){
			if (mainArray[i][j] > 0){
				elements[count].id = count + 1;
				elements[count].value = mainArray[i][j];
				elements[count].row = i;
				elements[count].col = j;
				elements[count].numberOfOptions = 0;
				Option * options = (Option*) malloc(BASIC_NUMBER_OF_OPTIONS * sizeof(*options));
				elements[count].options = options;
				elements[count].maxNumberOfOptions = BASIC_NUMBER_OF_OPTIONS;
				elements[count].numberOfImplacableOptions = 0;
				elements[count].inUse = 0;
				elements[count].mediumNumberOfImplaces = 0;
				count++;
			}
		}
	}
	return elements;
}

/*
*Returns index of element, that has the biggest value with least options
*Or -1. if all elements have been used or -2 if there is an element, that cannot be placed anymore
*/
int findIndexOfBiggestNotInUseElement(Element * elements, int totalNumbers){
	
	int index = -1;
	int topPoints = -1000;
	int points = 0;
	
	for (int i = 0; i < totalNumbers; i++){
		/* Exclude already in use elements */
		if (elements[i].inUse) continue;
		if (elements[i].numberOfOptions - elements[i].numberOfImplacableOptions == 0) return -2;
		points = elements[i].value - (elements[i].numberOfOptions - elements[i].numberOfImplacableOptions) + elements[i].mediumNumberOfImplaces;
		if (points > topPoints){
			topPoints = points;
			index = i;
		}
	}
	
	return index;
}

/*
*Dynamically creates array of indexes for option of element.
*These indexes represent indexes of mainArray in this format:
*{ x1,y1,x2,y2,...,xn,yn } where xn and yn are row and col coordinates of element itself
*Length if this array is value of element times 2.
*/

void createOptionForElement(Element * elements, int index, char coordsArr[], int numberOfCoords){
	
	/* Realloc if needed */
	if (elements[index].numberOfOptions + 1 > elements[index].maxNumberOfOptions){
		elements[index].maxNumberOfOptions *= 2;
		Option * tmp = (Option*) realloc(elements[index].options ,elements[index].maxNumberOfOptions * sizeof(*tmp));
		elements[index].options = tmp;
	}
	
	char * arrayOfIndexes = (char*) malloc((numberOfCoords + 2) * sizeof(*arrayOfIndexes));
	/* fill array, last 2 spots are reserved for element itself */
	for (int i = 0; i < numberOfCoords; i++) arrayOfIndexes[i] = coordsArr[i];
	/* last two indexes of this array are x and y coordinate of this element */
	arrayOfIndexes[numberOfCoords] = elements[index].row;
	arrayOfIndexes[numberOfCoords + 1] = elements[index].col;
	/* Assign all other properties of option */
	elements[index].options[elements[index].numberOfOptions].id = elements[index].id * ID_MULTIPLIER + elements[index].numberOfOptions + 1; //Unique id, that is used for main search
	elements[index].options[elements[index].numberOfOptions].arrayOfIndexes = arrayOfIndexes;
	elements[index].options[elements[index].numberOfOptions].inUse = 0;
	elements[index].options[elements[index].numberOfOptions].implacable = 0;
	elements[index].options[elements[index].numberOfOptions].implacedBy = 0;	
	elements[index].options[elements[index].numberOfOptions].numberOfImplaces = 0;
	elements[index].options[elements[index].numberOfOptions].maxNumberOfImplaces = BASIC_NUMBER_OF_OPTIONS;	
	elements[index].options[elements[index].numberOfOptions].implacesArr = (int*) malloc(BASIC_NUMBER_OF_OPTIONS * sizeof(int));
	elements[index].numberOfOptions += 1;
	
}

/*
*Find options for all elements, that cannot be placed in current layout and set them as implacable
*/
void findImplacableOptions(unsigned char mainArray[][32], Element * elements, int totalNumbers, int callerElementIndex, int callerOptionIndex){
	
	int totalCount = 0;
	int count = 0;
	for (int i = 0; i < totalNumbers; i++){
		if (elements[i].inUse || i == callerElementIndex) {
			continue;
		}
		count = 0;
		for (int j = 0; j < elements[i].numberOfOptions; j++){
			if (!elements[i].options[j].implacable && callerElementIndex == PREPROCESSOR_ID){
				/* -2 because we need to exclude the element itself */
				for (int k = 0; k < (elements[i].value * 2) - 2; k += 2){
					if (mainArray[(int)elements[i].options[j].arrayOfIndexes[k]][(int)elements[i].options[j].arrayOfIndexes[k + 1]] != 0){
						elements[i].options[j].implacable = 1;
						elements[i].options[j].implacedBy = callerElementIndex;
						elements[i].numberOfImplacableOptions += 1;
						break;
					}
				}
			} else if (!elements[i].options[j].implacable){
				/* -2 because we need to exclude the element itself */
				for (int k = 0; k < (elements[i].value * 2) - 2; k += 2){
					/* check for collisions */
					if (mainArray[(int)elements[i].options[j].arrayOfIndexes[k]][(int)elements[i].options[j].arrayOfIndexes[k + 1]] != 0){
						/* realloc if needed */
						if (elements[callerElementIndex].options[callerOptionIndex].numberOfImplaces + 2 >= elements[callerElementIndex].options[callerOptionIndex].maxNumberOfImplaces){
							elements[callerElementIndex].options[callerOptionIndex].maxNumberOfImplaces *= 2;
							int * tmp = (int*) realloc(elements[callerElementIndex].options[callerOptionIndex].implacesArr ,elements[callerElementIndex].options[callerOptionIndex].maxNumberOfImplaces * sizeof(*tmp));
							elements[callerElementIndex].options[callerOptionIndex].implacesArr = tmp;
						}
						/* insert element index and option index of implaced option into array */
						elements[callerElementIndex].options[callerOptionIndex].implacesArr[elements[callerElementIndex].options[callerOptionIndex].numberOfImplaces] = i;
						elements[callerElementIndex].options[callerOptionIndex].implacesArr[elements[callerElementIndex].options[callerOptionIndex].numberOfImplaces + 1] = j;
						elements[callerElementIndex].options[callerOptionIndex].numberOfImplaces += 2;
						count++;
						totalCount++;
						elements[callerElementIndex].mediumNumberOfImplaces++;
						break;
					}
				}
			}
			
		}
		/* In this cas, if this option blocks all options of one element, make this option implacable, cannot be used */
		if (callerElementIndex != -1 && (count + elements[i].numberOfImplacableOptions) ==  elements[i].numberOfOptions){
				elements[callerElementIndex].options[callerOptionIndex].implacable = 1;
				elements[callerElementIndex].options[callerOptionIndex].implacedBy = -1;
				elements[callerElementIndex].mediumNumberOfImplaces -= totalCount;
				elements[callerElementIndex].numberOfImplacableOptions += 1;
				/* break, no need to test with other elements */
				break;
		}
	}
}

/*
*Fill temporary array with each option of each element and try to combine it with
*each option of each element and find collisions
*/
void fillImplacableArrays(unsigned char mainArray[][32], Element * elements, int totalNumbers, int numberOfRows, int numberOfCols){
	
	unsigned char tmpArr[32][32];
		
	for (int i = 0; i < totalNumbers; i++) {
		if (elements[i].inUse) continue;
		for (int j = 0; j < elements[i].numberOfOptions; j++){
			if (elements[i].options[j].implacable == 1) continue;
			for (int k = 0; k < numberOfRows; k++)
				for (int l = 0; l < numberOfCols; l++) tmpArr[k][l] = mainArray[k][l];
					
			for (int k = 0; k < elements[i].value * 2; k+=2) tmpArr[(int)elements[i].options[j].arrayOfIndexes[k]][(int)elements[i].options[j].arrayOfIndexes[k + 1]] = 1;
				
			findImplacableOptions(tmpArr, elements, totalNumbers, i, j);
		}
		elements[i].mediumNumberOfImplaces /= (elements[i].numberOfOptions * 10);
	}
}

/*
*Frees all allocated memory
*/
void freeMemory(Element * elements, int totalNumbers){

	for (int i = 0; i < totalNumbers; i++){
		for (int j = 0; j < elements[i].numberOfOptions; j++){
			free(elements[i].options[j].arrayOfIndexes);
			free(elements[i].options[j].implacesArr);
		}
		free(elements[i].options);
	}
	
	free(elements);
}

/*
*Return 1 if all elements have been placed into the field
*/
int allElementsComplete(Element * elements, int totalNumbers){
	
	for (int i = 0; i < totalNumbers; i++)
		if (elements[i].inUse == 0) return 0;
		
	return 1;
}

/*
*Marks all options of all elements (if they are not marked already)
*that are in collision with provided elements option, as implacable
*/
void markImplacables(Element * elements, int elementIndex, int optionIndex){
	
	int markElIndex;
	int markOpIndex;
	for (int i = 0; i < elements[elementIndex].options[optionIndex].numberOfImplaces; i += 2){
		markElIndex = elements[elementIndex].options[optionIndex].implacesArr[i];
		markOpIndex = elements[elementIndex].options[optionIndex].implacesArr[i + 1];
		if (elements[markElIndex].options[markOpIndex].implacable == 0 && elements[markElIndex].inUse == 0){
			elements[markElIndex].numberOfImplacableOptions += 1;
			elements[markElIndex].options[markOpIndex].implacable = 1;
			elements[markElIndex].options[markOpIndex].implacedBy = elements[elementIndex].options[optionIndex].id;
		}
	}
}

/*
*Unmarks only those options, that were marked by provided elements option
*/
void unmarkImplacables(Element * elements, int elementIndex, int optionIndex){

	int markElIndex;
	int markOpIndex;
	for (int i = 0; i < elements[elementIndex].options[optionIndex].numberOfImplaces; i += 2){
		markElIndex = elements[elementIndex].options[optionIndex].implacesArr[i];
		markOpIndex = elements[elementIndex].options[optionIndex].implacesArr[i + 1];
		if (elements[markElIndex].options[markOpIndex].implacable == 1 && elements[markElIndex].options[markOpIndex].implacedBy == elements[elementIndex].options[optionIndex].id){
			elements[markElIndex].numberOfImplacableOptions -= 1;
			elements[markElIndex].options[markOpIndex].implacable = 0;
		}
	}
}

/*-----------------------------------------MAIN ARRAY-----------------------------------------*/

/*
*Computes all possible options of placement for one element
*Creates arrays of coordinates for each option
*/
void computeOptionsForElement(unsigned char mainArray[][32], Element * elements, int index, int numberOfCols, int numberOfRows, int division, int divider){
	
	/* create temporary array, which will hold coordinates for possible option*/
	/* MAX_NUMBER_OF_ELEMENTS * 2 because it will hold coordinates in format {x1,y1,x2,y2,..}*/
	/* and max number of possible spots in the field is MAX_NUMBER_OF_ELEMENTS */
	char coordsArr[MAX_NUMBER_OF_ELEMENTS * 2];
	int numberOfCoords = 0;
	
	int x = elements[index].row;
	int y = elements[index].col;
	int wrongCounter = 0;
	/* forCycle for cols */
	for (int i = divider; i > 0; i--){
		if ( y - (i - 1) < 0  || y + (divider - i) >= numberOfCols) continue;
		/* forCycle for rows */
		for (int j = division; j > 0; j--){
			/* forCycle for main array */
			if ( x + (j - 1) >= numberOfRows || x - (division - j) < 0) continue;
			for (int k = 0; k < division; k++){
				/* used so i dont have to check other cells, if there is another element */
				/* explained below */
				if (wrongCounter == 2) break;
				for (int l = 0; l < divider; l++){
					if (mainArray[(x + (j - 1)) - k][(y - (i - 1)) + l] != 0) wrongCounter++;
					else{
						/* assign row and col coordinates into temporary array */
						coordsArr[numberOfCoords++] = (x + (j - 1)) - k; //row coordinate
						coordsArr[numberOfCoords++] = (y - (i - 1)) + l; //col coordinate
					}
					/* 2 means, that there is at least one other element, so placement is impossible */
					/* 1 is possible, because that 1 wrong counter is the computed element itself */
					if (wrongCounter == 2) break;
				}
			}
			/* if true, means, that this option is possible */	
			if (wrongCounter < 2){
				createOptionForElement(elements, index, coordsArr, numberOfCoords);
			}
			wrongCounter = 0;
			numberOfCoords = 0;
		}
	}
}

/*
*Finds all options of placement for all elements.
*Uses findOptions method to compute options and assign coordinates
*Return value is void, because computed values are assign into corresponding elements
*/
void findAllOptionsForElements(unsigned char mainArray[][32], Element * elements, int totalNumbers, int numberOfCols, int numberOfRows){
	
	for (int i = 0; i < totalNumbers; i++){
		elements[i].numberOfOptions = 0;
		int value = elements[i].value;
		int division = 0;
		for (int divider = (int)floor(sqrt(value)); divider > 0; divider--){
			/* exclude non rectangle objects */
			if (value % divider != 0) continue;
			division = value / divider;
			/* method to compute number of options for element */
			computeOptionsForElement(mainArray, elements, i, numberOfCols, numberOfRows, division, divider);
			/* if value is perfect power, we cannot continue with next method, because some results would be doubled */
			if (divider * divider == value) continue;
			/* invert rows and cols */
			/* compute more options for inverted rows and cols */
			computeOptionsForElement(mainArray, elements, i, numberOfCols, numberOfRows, divider, division);	
			/* return inverted rows and cols back to their original values, so we dont end up in infinite loop */

		}
	}
}

/*
*Fills main array with id numbers of elements, that have only one possible option of placement
*Return 1, if some element was found, 0 if not
*Also checks, if some inly option overlays another only option. If so, noPossibleSolution is assigned true
*/
int fillArrayWithOnlyOnePossibleOptions(unsigned char mainArray[][32], Element * elements, int totalNumbers, int * noPossibleSolution, int * alreadyComplete){
	
	int foundAndFilled = 0;
	for (int i = 0; i < totalNumbers; i++){
	//printf("id: %d, value: %d, numberOFOptions: %d, numberOfImplacableOptions: %d\n",elements[i].id, elements[i].value, elements[i].numberOfOptions, elements[i].numberOfImplacableOptions);
		if (elements[i].numberOfOptions - elements[i].numberOfImplacableOptions == 1 && elements[i].inUse == 0) {
			int index = -1;
			for (int k = 0; k < elements[i].numberOfOptions; k++) if (elements[i].options[k].implacable == 0) { index = k; elements[i].inUse = 1; break;}
			foundAndFilled = 1;
			(*alreadyComplete)++;
			for (int j = 0; j < elements[i].value * 2; j += 2){
				/* Check for overlay, assign noPossibleSolution if true and return 0*/
				/* -2 because we need to exclude last two indexes of array, which are reserved for element itself */
				if ( (j < (elements[i].value * 2) - 2) && mainArray[(int)elements[i].options[index].arrayOfIndexes[j]][(int)elements[i].options[index].arrayOfIndexes[j+1]] != 0){
					*noPossibleSolution = 1;
					return 0;
				}
				mainArray[(int)elements[i].options[index].arrayOfIndexes[j]][(int)elements[i].options[index].arrayOfIndexes[j+1]] = elements[i].id;
			}
		} else if (elements[i].numberOfOptions - elements[i].numberOfImplacableOptions == 0 && elements[i].inUse == 0) { *noPossibleSolution = 1; return 0; }
	}
	return foundAndFilled;
}

/*
*Check if whole field is filled. If so, return 1, if not, return 0
*/
int wholeFieldFilled(unsigned char mainArray[][32], int numberOfRows, int numberOfCols){
	
	for (int i = 0; i < numberOfRows; i++)
		for (int j = 0; j < numberOfCols; j++) if (mainArray[i][j] == 0) return 0;
		
	return 1;
}

/*
*Combine all options of all elements into one array. Than check for empty spaces
*Return 1, if there is a free spot, else return 0
*DEPRACATED
*/
int emptyAfterAllOptionsOfAllElements(unsigned char mainArray[][32], Element * elements, int totalNumbers, int numberOfRows, int numberOfCols){
	
	unsigned char tmpArr[32][32];
	
	for (int i = 0; i < numberOfRows; i++)
		for (int j = 0; j < numberOfCols; j++) tmpArr[i][j] = mainArray[i][j];
		
	for (int i = 0; i < totalNumbers; i++)
		for (int j = 0; j < elements[i].numberOfOptions; j++)
			for (int k = 0; k < elements[i].value * 2; k += 2)	tmpArr[(int)elements[i].options[j].arrayOfIndexes[k]][(int)elements[i].options[j].arrayOfIndexes[k + 1]] = 1;

	return !wholeFieldFilled(tmpArr, numberOfRows, numberOfCols);
}

/*
*Support function for compute results function. Copies main array into result array
*and adds correct options of all elements into it
*/
void fillResultArray(unsigned char mainArray[][32], int totalNumbers, int numberOfRows, int numberOfCols, int result[], int alreadyComplete, Element * elements, unsigned char resultArray[][32]){
	/* Copy array */
	for (int k = 0; k < numberOfRows; k++)
		for (int l = 0; l < numberOfCols; l++) resultArray[k][l] = mainArray[k][l];
	/* Assign correct options into result array */
	for (int i = 0; i < totalNumbers - alreadyComplete; i++){
		int elIndex = result[i] / ID_MULTIPLIER - 1;
		int opIndex = result[i] % ID_MULTIPLIER - 1;
		for (int j = 0; j < elements[elIndex].value * 2; j += 2) resultArray[(int)elements[elIndex].options[opIndex].arrayOfIndexes[j]][(int)elements[elIndex].options[opIndex].arrayOfIndexes[j+1]] = elements[elIndex].id;
	}

}

/*-----------------------------------------MAIN FUNCTIONS-----------------------------------------*/

/*
*First find all possible options of placement for all elements. Than fill array with elements, that have only one way of being placed.
*Check the rest of elements for number of possible placements and repeat filling array, if possible.
*Set all options of elements, that are marked as implacable to complete, because they wont be used in next operations
*Check for some cases of no possible solution - empty fields after placement of all options; all elements are marked as complete and some fields are empty
*For impossible solutions, print result and return 0
*For already found one and only solution print result and return 1
*If no result was found yet, return 2
*/
int preProcessing(unsigned char mainArray[][32], Element * elements, int numberOfRows, int numberOfCols, int totalNumbers, int * noPossibleSolution, int * alreadyComplete){
	
	/* Find all possible placements for all elements */
	findAllOptionsForElements(mainArray, elements, totalNumbers, numberOfCols, numberOfRows);
	/* If there is an element with only one way of placement, fill these indexes of the main array with id of that element */
	/* Than search options of all possible elements and remove options, that are no longer possible */
	/* Repeat, until there are no elements with only one possible placement */
	while (fillArrayWithOnlyOnePossibleOptions(mainArray, elements, totalNumbers, noPossibleSolution, alreadyComplete)){
		findImplacableOptions(mainArray, elements, totalNumbers, PREPROCESSOR_ID, PREPROCESSOR_ID);
	}
	/* Check, if some element is now unable to be placed. If so, puzzle doesnt have a solution */
	for (int i = 0; i < totalNumbers; i++) if (elements[i].numberOfOptions - elements[i].numberOfImplacableOptions == 0) *noPossibleSolution = 1;
	/* End program, if there is no possible solution */
	if (*noPossibleSolution || ( allElementsComplete(elements, totalNumbers) && !wholeFieldFilled(mainArray, numberOfRows, numberOfCols)) ){
		printf("Reseni neexistuje.\n");
		return 0;
	}
	/* Check if whole field is filled. If so, there is only one solution */
	/* Print that solution and quit */
	if (wholeFieldFilled(mainArray, numberOfRows, numberOfCols)){
		printResult(mainArray, elements, totalNumbers, numberOfRows, numberOfCols);
		return 1;
	}	
	
	return 2;
}

/*
*Main engine of this program. Use backtracking to find all possible solutions
*Variable result will hold number of results. Variable resultArr holds array with last result
*Uses recursive backtracking algorithm
*Returns 0, if no solution is possible (to save time) or 1, signaling, we can continue computing
*/
int computeResults(unsigned char mainArray[][32], Element * elements, int totalNumbers, int elementIndex, int * results, int options[], short spot, unsigned char resultArray[][32], int numberOfRows, int numberOfCols, int alreadyComplete){
	/* MArk this element as in use, so it cant be used in next recursion */
	elements[elementIndex].inUse = 1;
	
	int optionIndex = -1;
	int nextIndex = -1;
	/* For cycle will as many times, as there are options of the element, that are not implacable */
	for (int cycle = 0; cycle < elements[elementIndex].numberOfOptions - elements[elementIndex].numberOfImplacableOptions; cycle++){
		for (int i = 0; i < elements[elementIndex].numberOfOptions; i++){
			/* Choose only options, that are not in use */
			if (!elements[elementIndex].options[i].inUse && !elements[elementIndex].options[i].implacable){
				optionIndex = i;
				elements[elementIndex].options[i].inUse = 1;
				break;
			}
		}
		/* Mark all options of all elements, that are implacable by this option */
		markImplacables(elements, elementIndex, optionIndex);
		
		/* Check for next available index. if -2, impossible to place. If -1, all elements have been used and this element is the last one */
		nextIndex = findIndexOfBiggestNotInUseElement(elements, totalNumbers);
			//printf("Caller --- id: %d, value: %d ++++++ NextIndex --- id: %d, value: %d\n", elements[elementIndex].id, elements[elementIndex].value, elements[nextIndex].id, elements[nextIndex].value);
		if (nextIndex != -2){
			/* Put id of this option into result array */
			options[spot] = elements[elementIndex].options[optionIndex].id;
			if (nextIndex != -1) {		
				/* Call next recursion */		
				if (computeResults(mainArray, elements, totalNumbers, nextIndex, results, options, spot + 1, resultArray, numberOfRows, numberOfCols, alreadyComplete) == 0) return 0;
			}else {
				/* In case of last element, fill result array and check, if whole array is filled. If not, result doesnt exist, return 0 */
				if (*results == 0){
					fillResultArray(mainArray, totalNumbers, numberOfRows, numberOfCols, options, alreadyComplete, elements, resultArray);
					if (!wholeFieldFilled(resultArray, numberOfRows, numberOfCols)) return 0;
				}
				(*results)++;
			}
		}
		/* Unmark ONLY options of all elements, that were implaced by this option */
		unmarkImplacables(elements, elementIndex, optionIndex);
		
	}
	/* free inUse options of this element, so they can all be used (non implacable ones) in next recursion */
	for (int i = 0; i < elements[elementIndex].numberOfOptions; i++){
		if (elements[elementIndex].options[i].inUse){
			elements[elementIndex].options[i].inUse = 0;
		}
	}
	/* Unmark this element, so it can be used in next recursion */
	elements[elementIndex].inUse = 0;
	
	return 1;
}

/*-----------------------------------------MAIN-----------------------------------------*/



int main(void){
	
	unsigned char mainArray[32][32];
	int numberOfRows = 0, numberOfCols = 0, totalNumbers = 0;
	int noPossibleSolution = 0;
	int results = 0;
	int alreadyComplete = 0;
	printf("Zadejte puzzle:\n");
	/* Assign input into main array, end program, if error occured while reading input */
	if (!readInput(mainArray, &numberOfRows, &numberOfCols, &totalNumbers)){
		printf("Nespravny vstup.\n");
		return 1;
	}
	/* Create array of elements (numbers > 0) */
	Element * elements = createStructuresFromElements(mainArray, totalNumbers, numberOfRows, numberOfCols);
	/* Call preprocessor to find case of no possible solution or try to quick find simple solution */
	/* If one and only solution was found, prints it. If it found, that it is impossible to fill the field, prints result */	
	if (preProcessing(mainArray, elements, numberOfRows, numberOfCols, totalNumbers, &noPossibleSolution, &alreadyComplete) == 2){
		/* Fill array of option of each element with indexes of other options, that are implaced with calling option */
		fillImplacableArrays(mainArray, elements, totalNumbers, numberOfRows, numberOfCols);
		/* Find best fitting index of element, that will enter recursion as first */
		int index = findIndexOfBiggestNotInUseElement(elements, totalNumbers);
		
		int spot = 0;
		/* This array will hold ids of correct result */
		int options[201];
		/* This array is the one that will be printed, if there is only one correct result */
		unsigned char resultArray[32][32];
		/* Recursive backtracking of all correct results */
		computeResults(mainArray, elements, totalNumbers, index, &results, options, spot, resultArray, numberOfRows, numberOfCols, alreadyComplete);
		/* Print number of correct results, if there were more than 1 */
		if (results > 1) printf("Celkem reseni: %d\n", results);
		/* Print correct result, if there was only one correct result */
		else if (results == 1) printResult(resultArray, elements, totalNumbers, numberOfRows, numberOfCols);
		/* Print, if no results are possible */
		else printf("Reseni neexistuje.\n");
	}

	/* free all allocated memory */
	freeMemory(elements, totalNumbers);

	return 0;
}
