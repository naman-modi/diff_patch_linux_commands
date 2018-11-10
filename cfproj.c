#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include"stack.h"
#include"queue.h"
#include"list.h"

#define MAX(a, b) (((b) > (a)) ? (b) : (a))
#define p1start -1
#define p1end -2
#define p2start -3
#define p2end -4

int iflag = 0;
int bflag = 0;
int wflag = 0;
int cflag = 0;
int tflag = 0;
int yflag = 0;

//Reading the first file into a 2-dimensional array
int read_file(int rfd, char f[][1024]);
	
//Starting the longest common subsequence(LCS)  algorithm to make the lcs matrix
void form_lcs_matrix(int f1lines, int f2lines, int (*lcn)[1024], char (*f1)[1024], char (*f2)[1024]);

int i_stringcmp(char (*f1), char (*f2));

int w_stringcmp(char (*f1), char (*f2));

int iw_stringcmp(char (*f1), char (*f2));

int b_stringcmp(char (*f1), char (*f2));

int ib_stringcmp(char (*f1), char (*f2));

/*Finding the line number of files which are same using LCS Matrix
 *returns the total number of lines same using lcs matrix
 */
int get_line_no_from_lcs(int f1lines, int f2lines, int (*lcn)[1024], stack *s1, stack *s2);

int stop(stack *s);

/*Returns the topmost(where l->tail points) element for the list*/
int ltop(list *l);

/*this make list function is used for -c output, so when the consecutive lcs > 6, it introduces p1start, p1end, p2start, and p2end in the list 
 *this function returns the number of total prints required for those files
 */
int make_list_for_c_output(stack *s1, stack *s2, list *l1, list *l2, int lcncount);

/*this function is used to print c output file wise
 *fp represents, the file to be printed
 *fo represents, the other file
 *reverse is 0 when first file of diff has to be printed, and 1 when second file is to be printed
 *print_count is the nth number of print of the file, for which this function is called
 *t_prints is the total number of prints required for diff operation of the two files, depends on the nummber of consecutive lcss in those files
 */
void cprint_once_filewise(list *lp, list *lo, int fplines, int folines, char (*fp)[1024], int lcncount, int reverse, int print_count, int t_prints);

/*this one c_print function is used, when there are no lcs > 6 in the files, and printing needs to be done only once*/
void one_cprint(list *l1, list *l2, int f1lines, int f2lines, char (*f1)[1024], char (*f2)[1024], int lcncount);

/*this coutput function is written to give output for -c attribute*/
void coutputprint(stack *s1, stack *s2, int f1lines, int f2lines, char (*f1)[1024], char (*f2)[1024], int lcncount, char *argv[], int optind); 

/*this function deletes the lcs from list created in c output, once it has been used and no longer required*/
void delete_unwanted_lcs_in_c_output(list *l);

void outputprint(stack *s1, stack *s2, int f1lines, int f2lines, char f1[][1024], char (*f2)[1024], queue *q1, queue *q2, int lcncount); 

int main(int argc, char *argv[]) {
	
	int rfd1, rfd2, f1lines, f2lines, lcn[1024][1024], lcncount, opt;
	char f1[1024][1024], f2[1024][1024];

	stack s1, s2;
	queue q1, q2;
	init(&s1);
	init(&s2);
	qinit(&q1);
	qinit(&q2);
	while ((opt = getopt(argc, argv, "ibwytc")) != -1) {
		switch (opt) {
			case 'i':
				iflag = 1;
				break;
			case 'b':
				bflag = 1;
				break;
			case 'w':
				wflag = 1;
				break;
			case 'c':
				cflag = 1;
				break;
			case 't':
				tflag = 1;
				break;
			case 'y':
				yflag = 1;
				break;
			default: /* '?' */
				fprintf(stderr, "Usage: Wrong options\n");
				exit(EXIT_FAILURE);
		}
	}

	//printf("iflag=%d; bflag=%d; wflag=%d; cflag=%d; optind=%d\n",iflag, bflag, wflag, cflag, optind);

	if(cflag == 1 && yflag == 1) {
		fprintf(stderr, "mydiff: conflicting output style options\n");	
		exit(EXIT_FAILURE);
	}

	if (optind >= argc) {
		fprintf(stderr, "mydiff: missing operand after '%s'\n", argv[optind - 1]);
		exit(EXIT_FAILURE);
	}

	if(argv[optind + 1] == NULL) {
		fprintf(stderr, "mydiff: missing operand after '%s'\n", argv[optind]);
		exit(EXIT_FAILURE);
	}
	//printf("name argument = %s\n %s\n", argv[optind], argv[optind + 1]);
	if(strcmp(argv[optind], argv[optind + 1]) == 0)
		exit(EXIT_FAILURE);

	
	//Opening the two files to be compared
	rfd1 = open(argv[optind], O_RDWR, S_IRUSR | S_IWUSR);
	rfd2 = open(argv[optind + 1], O_RDWR, S_IRUSR | S_IWUSR);
	if(rfd1 == -1)	{
		//printf("mydiff: %s: ", argv[optind]);
		perror("");
		exit(errno);
	}
	if(rfd2 == -1)	{
		printf("mydiff: '%s': ", argv[optind + 1]);
		exit(errno);
	}
	

	//Reading the first file into a 2-dimensional array
	f1lines = read_file(rfd1, f1); //retuens total number of lines in first file
	
	//Reading the second file into a 2-dimensional array
	f2lines = read_file(rfd2, f2); //return total number of lines in second file
	
	//Starting the longest common subsequence(LCS)  algorithm
	//forming lcn matrix;
	form_lcs_matrix(f1lines, f2lines, lcn, f1, f2);
	
	//Finding the line number of files which are same using LCS Matrix
	lcncount = get_line_no_from_lcs(f1lines, f2lines, lcn, &s1, &s2); //return the total number of lines same using lcs matrix

	if(cflag == 1)
		coutputprint(&s1, &s2, f1lines, f2lines, f1, f2, lcncount, argv, optind);
	else
		outputprint(&s1, &s2, f1lines, f2lines, f1, f2, &q1, &q2, lcncount);//prints output of diff

	return 0;
}

int read_file(int rfd, char f[][1024]) {
	int i, j, r;
	i = 0;//Line numbering starts from 0.
	j = 0;
	//Reading the file into a 2-dimensional array
	while(1) {
		r = read(rfd, &(f[i][j]), sizeof(f[i][j]));
		if(r == 0)
			break;
		if(r == -1) {
			perror("Reading error.\n");
			exit(errno);
		}
		if(f[i][j] == '\n')	{
			f[i][(j + 1)] = '\0';
			i++;
			j = -1;
		}
		j++;
	}
	return i;
}

void form_lcs_matrix(int f1lines, int f2lines, int (*lcn)[1024], char (*f1)[1024], char (*f2)[1024]) {
	int m, n, i, j;
	m = f1lines;
	n = f2lines;
	//initializing first column and first row to make LCS Matrix		
	for(i = 0; i <= n; i++)
		lcn[i][0] = 0;
	for(i = 0; i <= m; i++)
		lcn[0][i] = 0;
	//Making the LSC Matrix
	for(i = 1; i <= n; i++) { 
		for(j = 1; j <= m; j++) { 
			if(iflag == 1 && bflag == 0 && wflag == 0) {
				if(i_stringcmp(f1[(j - 1)], f2[i - 1]) == 0)
					lcn[i][j] = lcn[i-1][j-1] + 1;//If same, diagnol + 1
				else
					lcn[i][j] = MAX(lcn[i][j - 1], lcn[i -1][j]);//If not same, Maximum of upper and left
			} else if(iflag == 0 && bflag == 0 && wflag == 1) {
				if(w_stringcmp(f1[(j - 1)], f2[i - 1]) == 0)
					lcn[i][j] = lcn[i-1][j-1] + 1;//If same, diagnol + 1
				else
					lcn[i][j] = MAX(lcn[i][j - 1], lcn[i -1][j]);//If not same, Maximum of upper and left
			} else if(iflag == 0 && bflag == 1 && wflag == 0) {
				if(b_stringcmp(f1[(j - 1)], f2[i - 1]) == 0)
					lcn[i][j] = lcn[i-1][j-1] + 1;//If same, diagnol + 1
				else
					lcn[i][j] = MAX(lcn[i][j - 1], lcn[i -1][j]);//If not same, Maximum of upper and left
			} else if(iflag == 1 && wflag == 1) {
				if(iw_stringcmp(f1[(j - 1)], f2[i - 1]) == 0)
					lcn[i][j] = lcn[i-1][j-1] + 1;//If same, diagnol + 1
				else
					lcn[i][j] = MAX(lcn[i][j - 1], lcn[i -1][j]);//If not same, Maximum of upper and left
			} else if(iflag == 1 && bflag == 1 && wflag == 0) {
				if(ib_stringcmp(f1[(j - 1)], f2[i - 1]) == 0)
					lcn[i][j] = lcn[i-1][j-1] + 1;//If same, diagnol + 1
				else
					lcn[i][j] = MAX(lcn[i][j - 1], lcn[i -1][j]);//If not same, Maximum of upper and left
			} else {
				if(strcmp(f1[(j - 1)], f2[i - 1]) == 0)
					lcn[i][j] = lcn[i-1][j-1] + 1;//If same, diagnol + 1
				else
					lcn[i][j] = MAX(lcn[i][j - 1], lcn[i -1][j]);//If not same, Maximum of upper and left
			}
		} 
	}//LCS Matrix formed. 
}

//this function is used to compare the lines, case insensitively
int i_stringcmp(char (*f1), char (*f2)) {
	char ff1[1024], ff2[1024];
	int i;
	for(i = 0; f1[i] != '\0'; i++) {
		if(f1[i] >= 'A' && f1[i] <= 'Z') //if there is any character which is a capital alphabet, it converts it to lower case
			ff1[i] = 'a' + (f1[i] - 'A');
		else
			ff1[i] = f1[i];
	}
	ff1[i] = '\0'; 
	for(i = 0; f2[i] != '\0'; i++) {
		if(f2[i] >= 'A' && f2[i] <= 'Z')
			ff2[i] = 'a' + (f2[i] - 'A');
		else
			ff2[i] = f2[i];
	}
	ff2[i] = '\0'; 
	if((strcmp(ff1, ff2)) == 0)
		return 0;
	else 
		return 1;
}

//the -w attribute removes all the spaces and tabs from the lines and then compares
int w_stringcmp(char (*f1), char (*f2)) {
	char ff1[1024], ff2[1024];
	int i, j = 0;
	for(i = 0; f1[i] != '\0'; i++) {
		if(f1[i] == ' ' || f1[i] == '\t')
			continue;
		else
			ff1[j] = f1[i];
		j++;
	}
	ff1[j] = '\0';
	j = 0; 
	for(i = 0; f2[i] != '\0'; i++) {
		if(f2[i] == ' ' || f2[i] == '\t')
			continue;
		else
			ff2[j] = f2[i];
		j++;
	}
	ff2[j] = '\0';
	if((strcmp(ff1, ff2)) == 0)
		return 0;
	else 
		return 1;
}

//case insensitive as well as removes spaces
int iw_stringcmp(char (*f1), char (*f2)) {
	char ff1[1024], ff2[1024];
	int i, j = 0;
	for(i = 0; f1[i] != '\0'; i++) {
		if(f1[i] >= 'A' && f1[i] <= 'Z')
			ff1[j] = 'a' + (f1[i] - 'A');
		else if(f1[i] == ' ' || f1[i] == '\t')
			continue;
		else
			ff1[j] = f1[i];
		j++;
	}
	ff1[j] = '\0';
	j = 0; 
	for(i = 0; f2[i] != '\0'; i++) {
		if(f2[i] >= 'A' && f2[i] <= 'Z')
			ff2[j] = 'a' + (f2[i] - 'A');
		else if(f2[i] == ' ' || f2[i] == '\t')
			continue;
		else
			ff2[j] = f2[i];
		j++;
	}
	ff2[j] = '\0';
	if((strcmp(ff1, ff2)) == 0)
		return 0;
	else 
		return 1;
}

//-b attribute if there are many blank spaces it converts it to a single space, and then compares
int b_stringcmp(char (*f1), char (*f2)) {
	char ff1[1024], ff2[1024];
	int i, space = 0, j = 0;
	for(i = 0; f1[i] != '\0'; i++) {
		if(i == 0 && (f1[i] == ' ' || f1[i] == '\t')) {
			ff1[j] = ' ';
			space = 1;
		} else if(f1[i] == ' ' || f1[i] == '\t') {
			if(space == 1)
				continue;
			else {
				space = 1;
				ff1[j] = ' ';
			}
		} else {
			ff1[j] = f1[i];
			space = 0;
		}
		j++;
	}
	ff1[j] = '\0'; 
	j = 0;
	space = 0;
	for(i = 0; f2[i] != '\0'; i++) {
		if(i == 0 && (f2[i] == ' ' || f2[i] == '\t')) {
			ff2[j] = ' ';
			space = 1;
		} else if(f2[i] == ' ' || f2[i] == '\t') {
			if(space == 1)
				continue;
			else if(space == 0) {
				space = 1;
				ff2[j] = ' ';
			}
		} else {
			ff2[j] = f2[i];
			space = 0;
		}
		j++;
	}
	ff2[j] = '\0'; 

	if((strcmp(ff1, ff2)) == 0)
		return 0;
	else 
		return 1;
}

int ib_stringcmp(char (*f1), char (*f2)) {
	char ff1[1024], ff2[1024];
	int i, space = 0, j = 0;
	for(i = 0; f1[i] != '\0'; i++) {
		if(i == 0 && (f1[i] == ' ' || f1[i] == '\t')) {
			ff1[j] = ' ';
			space = 1;
		} else if(f1[i] == ' ' || f1[i] == '\t') {
			if(space == 1)
				continue;
			else {
				space = 1;
				ff1[j] = ' ';
			}
		} else {
			if(f1[i] >= 'A' && f1[i] <= 'Z')
				ff1[j] = 'a' + (f1[i] - 'A');
			else
				ff1[j] = f1[i];
			space = 0;
		}
		j++;
	}
	ff1[j] = '\0'; 
	j = 0;
	space = 0;
	for(i = 0; f2[i] != '\0'; i++) {
		if(i == 0 && (f2[i] == ' ' || f2[i] == '\t')) {
			ff2[j] = ' ';
			space = 1;
		} else if(f2[i] == ' ' || f2[i] == '\t') {
			if(space == 1)
				continue;
			else if(space == 0) {
				space = 1;
				ff2[j] = ' ';
			}
		} else {
			if(f2[i] >= 'A' && f2[i] <= 'Z')
				ff2[j] = 'a' + (f2[i] - 'A');
			else
				ff2[j] = f2[i];
			space = 0;
		}
		j++;
	}
	ff2[j] = '\0'; 

	if((strcmp(ff1, ff2)) == 0)
		return 0;
	else 
		return 1;
}

int get_line_no_from_lcs(int f1lines, int f2lines, int (*lcn)[1024], stack *s1, stack *s2) {
	int i, j;
	int c = 0;
	i = f2lines;
	j = f1lines;	
	//Finding the line number of files which are same using LCS Matrix
	while(lcn[i][j] != 0) {
		if(lcn[i][j] == lcn[i][j-1]) {
			j--;
			continue;
		}
		else if(lcn[i][j] == lcn[i - 1][j]) {
			i--;
			continue;
		}
		else if(lcn[i][j] == (lcn[i - 1][j - 1] + 1)) {
			//pushing the line number of file 1 in the stack
			if(!isfull(s2)) 
				push(s2, (i - 1));
			//pushing the line number of file 2 in the stack
			if(!isfull(s1))
				push(s1, (j - 1));
			//printf("%s", f1[i - 1]);
			i--;
			j--;
			c++;
		}		
	}
	return c;
}

int stop(stack *s) {
	int w;
	w = pop(s);
	push(s, w);
	return w;
}

int ltop(list *l) {
	int n;
	n = l->tail->nn;
	return n;
}

int make_list_for_c_output(stack *s1, stack *s2, list *l1, list *l2, int lcncount) {
	int inex, jnex, diff1, diff2, i, j, count = 1, consecutive_count = 0, p_count = 1;
	if(isempty(s1))
		return p_count;
	while(count < lcncount) {
		i = pop(s1);
		ladd(l1, i);
		j = pop(s2);
		ladd(l2, j);

		count++;	
	
		inex = stop(s1);
		jnex = stop(s2); 

		//calculates the difference between current and next line number of file from stack made by lcs matrix
		diff1 = inex - i;
		diff2 = jnex - j;
		
		//when there are consecutive line numbers in both files from lcs matrix
		if(diff1 == 1 && diff2 == 1) {
			consecutive_count++; 			
			continue;
		} else {
			if(consecutive_count > 5) { //inserting the p1start, p1end, p2start and p2end in the list
				insert(l1, p1start, (consecutive_count + 1));
				insert(l1, p1end, (consecutive_count - 2));
				insert(l1, p2start, 3);
				insert(l1, p2end, 0);
				insert(l2, p1start, (consecutive_count + 1));
				insert(l2, p1end, (consecutive_count - 2));
				insert(l2, p2start, 3);
				insert(l2, p2end, 0);
				p_count++;
			}
			consecutive_count = 0;
		}
	}	
	i = pop(s1);
	ladd(l1, i);
	j = pop(s2);
	ladd(l2, j);
	if(consecutive_count > 5) {
		insert(l1, p1start, (consecutive_count + 1));
		insert(l1, p1end, (consecutive_count - 2));
		insert(l1, p2start, 3);
		insert(l1, p2end, 0);
		insert(l2, p1start, (consecutive_count + 1));
		insert(l2, p1end, (consecutive_count - 2));
		insert(l2, p2start, 3);
		insert(l2, p2end, 0);
		p_count++;
	}
	return p_count;	
}

void coutputprint(stack *s1, stack *s2, int f1lines, int f2lines, char (*f1)[1024], char (*f2)[1024], int lcncount, char *argv[], int optind) {
	list l1, l2;
	char star[17];
	lnode *p;
	int total_prints, count_print = 1, temp_lcn_count, end_print_line_no, i, start_print_line_no, diff;
	linit(&l1);		
	linit(&l2);
	for(i = 0; i < 15; i++)
		star[i] = '*';
	star[15] = '\n';
	star[16] = '\0';
	
	//if in both files completey in lcs, then no need to print anything
	if(lcncount == f1lines && lcncount == f2lines)
		return;
	
	total_prints = make_list_for_c_output(s1, s2, &l1, &l2, lcncount);

	printf("*** %s\n", argv[optind]);
	printf("--- %s\n", argv[optind + 1]);	
	if(total_prints == 1) { //if there are no more than 6 consecutive lcs in the files, then print only once
		one_cprint(&l1, &l2, f1lines, f2lines, f1, f2, lcncount);
		return;
	}
	
	printf("%s", star);
	temp_lcn_count = 0;
	p = l1.tail;
	while(p->nn != p1end) {
		if(p->nn != p1start)
			temp_lcn_count++;
		p = p->prev;
	}
	
	end_print_line_no = (p->next->nn + 1);

	printf("*** 1,%d ****\n", end_print_line_no);
	if(temp_lcn_count < end_print_line_no)
		cprint_once_filewise(&l1, &l2, f1lines, f2lines, f1, temp_lcn_count, 0, count_print, total_prints);
	
	temp_lcn_count = 0;
	p = l2.tail;
	while(p->nn != p1end) {
		if(p->nn != p1start)
			temp_lcn_count++;
		p = p->prev;
	}
	
	end_print_line_no = p->next->nn + 1;

	printf("--- 1,%d ----\n", end_print_line_no);

	if(temp_lcn_count != end_print_line_no)
		cprint_once_filewise(&l2, &l1, f2lines, f1lines, f2, temp_lcn_count, 1, count_print, total_prints);

	delete_unwanted_lcs_in_c_output(&l1);

	delete_unwanted_lcs_in_c_output(&l2);

	while(count_print < (total_prints - 1)) {
		count_print++;
		printf("%s", star);
		
		temp_lcn_count = 0;
		p = l1.tail;
		start_print_line_no = p->nn + 1;
		while(p->nn != p1end) {
			if(p->nn != p2end && p->nn != p1start)
				temp_lcn_count++;
			p = p->prev;
		}
		end_print_line_no = p->next->nn + 1;
		printf("*** %d,%d ****\n", start_print_line_no, end_print_line_no);
		diff = (end_print_line_no - start_print_line_no) + 1;
		if(temp_lcn_count < diff)
			cprint_once_filewise(&l1, &l2, f1lines, f2lines, f1, temp_lcn_count, 0, count_print, total_prints);
		
		temp_lcn_count = 0;
		p = l2.tail;
		start_print_line_no = p->nn + 1;
		while(p->nn != p1end) {
			if(p->nn != p2end && p->nn != p1start)
				temp_lcn_count++;
			p = p->prev;
		}
		end_print_line_no = p->next->nn + 1;
		printf("--- %d,%d ----\n", start_print_line_no, end_print_line_no);
		diff = (end_print_line_no - start_print_line_no) + 1;

		if(temp_lcn_count < diff)
			cprint_once_filewise(&l2, &l1, f2lines, f1lines, f2, temp_lcn_count, 1, count_print, total_prints);
		
		delete_unwanted_lcs_in_c_output(&l1);

		delete_unwanted_lcs_in_c_output(&l2);
	}
	count_print++;
	printf("%s", star);
	
	temp_lcn_count = 0;
	p = l1.tail;
	start_print_line_no = p->nn + 1;
	while(p != NULL) {
		if(p->nn != p2end)
			temp_lcn_count++;
		p = p->prev;
	}
	p = l1.head;
	if(p->nn != p2end)
		end_print_line_no = p->nn + 1;
	else
		end_print_line_no = p->next->nn + 1;
	printf("*** %d,%d ****\n", start_print_line_no, f1lines);
	diff = f1lines - start_print_line_no + 1;
	if(temp_lcn_count < diff)
			cprint_once_filewise(&l1, &l2, f1lines, f2lines, f1, temp_lcn_count, 0, count_print, total_prints);
	
	temp_lcn_count = 0;
	p = l2.tail;
	start_print_line_no = p->nn + 1;
	while(p != NULL) {
		if(p->nn != p2end)
			temp_lcn_count++;
		p = p->prev;
	}
	p = l2.head;
	if(p->nn != p2end)
		end_print_line_no = p->nn + 1;
	else
		end_print_line_no = p->next->nn + 1;
	printf("--- %d,%d ----\n", start_print_line_no, f2lines);
	diff = f2lines - start_print_line_no + 1;
	if(temp_lcn_count < diff)
			cprint_once_filewise(&l2, &l1, f2lines, f1lines, f2, temp_lcn_count, 1, count_print, total_prints);
}

void delete_unwanted_lcs_in_c_output(list *l) {
	lnode *p;
	int i;
	p = l->tail;
	while(p->nn != p2start) {
		p = p->prev;
		i = ldelete(l);
	}
	i = ldelete(l);
	i++;
}

void one_cprint(list *l1, list *l2, int f1lines, int f2lines, char (*f1)[1024], char (*f2)[1024], int lcncount) {
	char star[17];
	int i;
	for(i = 0; i < 15; i++)
		star[i] = '*';
	star[15] = '\n';
	star[16] = '\0';
	printf("%s", star);
	if(listisempty(l2)) {
		if(f1lines <= 1)
			printf("*** %d ****\n", f1lines);
		else 
			printf("*** 1,%d ****\n", f1lines);
		i = 0;
		while(i < f1lines) {
			if(f2lines == 0) 
				printf("- %s", f1[i++]);
			else
				printf("! %s", f1[i++]);
		}
		if(f2lines <= 1)
			printf("--- %d ----\n", f2lines);
		else 
			printf("--- 1,%d ----\n", f2lines);
		i = 0;
		while(i < f2lines) {
			if(f1lines == 0) 
				printf("+ %s", f2[i]);
			else
				printf("! %s", f2[i]);
			i++;
		}
		return;
	}

	if(f1lines == 1) 
		printf("*** 1 ****\n");
	else 
		printf("*** 1,%d ****\n", f1lines);
	if(lcncount < f1lines)
		cprint_once_filewise(l1, l2, f1lines, f2lines, f1, lcncount, 0, 1, 1);

	if(f2lines == 1) 
		printf("--- 1 ----\n");
	else 
		printf("--- 1,%d ----\n", f2lines);
	if(lcncount < f2lines)
		cprint_once_filewise(l2, l1, f2lines, f1lines, f2, lcncount, 1, 1, 1);
	
}
	

void cprint_once_filewise(list *lp, list *lo, int fplines, int folines, char (*fp)[1024], int lcncount, int reverse, int print_count, int t_prints) {
	list l3, l4;
	int i, j, count = 1, temp = 0, inex, jnex, diff1, diff2;
	linit(&l3);
	linit(&l4);
	copylist(lp, &l3);
	copylist(lo, &l4);
	i = ltop(&l3);
	j = ltop(&l4);
	
	if(print_count == 1) {
		if(i == p1start || j == p1start) {
			i = ldelete(&l3);
			j = ldelete(&l4);
			i = ltop(&l3);
			j = ltop(&l4);
		}

		if(i > 0 && j == 0) { //when file to be printed does not have first line in lcs, and other file hass first line in lcs
			if(reverse == 0) {
				while(temp < i) 
					printf("- %s", fp[temp++]); //if file to be printed is firdt file, then delete
			} else {
				while(temp < i)
					printf("+ %s", fp[temp++]);//if file to be printed is the second file, then add
			}
		} else if(i > 0 && j > 0) { //need change(c) when both files don't have first line in lcs
			while(temp < i)
				printf("! %s", fp[temp++]);
		}
	}

	while(count < lcncount) {
		i = ldelete(&l3);
		j = ldelete(&l4);
	
		inex = ltop(&l3);
		jnex = ltop(&l4);
		
		if(inex == p1start || inex == p1end || inex == p2start || inex == p2end) {
			inex = ldelete(&l3);
			inex = ltop(&l3);
			jnex = ldelete(&l4);
			jnex = ltop(&l4);
			if(inex == p1start || inex == p1end || inex == p2start || inex == p2end) {
				inex = ldelete(&l3);
				inex = ltop(&l3);
				jnex = ldelete(&l4);
				jnex = ltop(&l4);
			}
		}

		diff1 = inex - i;
		diff2 = jnex - j;

		printf("  %s", fp[i]);

		temp = i + 1;
		count++;

		if(diff1 > 1 && diff2 == 1) {
			if(reverse == 0) {
				while(temp < inex)
					printf("- %s", fp[temp++]);		
			} else {	
				while(temp < inex)
					printf("+ %s", fp[temp++]);
			}
		} else if(diff1 > 1 && diff2 > 1) {
			while(temp < inex)
				printf("! %s", fp[temp++]);		
		}
	}
	
	i = ldelete(&l3);
	j = ldelete(&l4);
	temp = i + 1;
	
	printf("  %s", fp[i]);
	
	if(print_count == t_prints) {
		if(i < (fplines - 1) && j == (folines - 1)) {
			if(reverse == 0) {
				while(temp < fplines)
					printf("- %s", fp[temp++]);	
			} else {
				while(temp < fplines)
					printf("+ %s", fp[temp++]);
			}
		} else if(i < (fplines - 1) && j < (folines - 1)) {
			while (temp < fplines)
				printf("! %s", fp[temp++]);
		}
	}
}

void outputprint(stack *s1, stack *s2, int f1lines, int f2lines, char f1[][1024], char (*f2)[1024], queue *q1, queue *q2, int lcncount) {
	int inex, jnex, diff1, diff2, i, j, count = 0, temp;
	//if LCS Matrix is a NULL Matrix
	if(isempty(s1)) {
		if(f1lines <= 1 && f2lines <= 1) {
			if(f1lines == 1 && f2lines == 1)
				printf("1c1\n");
			else if(f1lines == 0 && f2lines == 1)
				printf("0a1\n");
			else if(f1lines == 1 && f2lines == 0)
				printf("1d0\n");
		} else {
			if(f1lines == 1)
				printf("1c1,%d\n", f2lines);
			else if(f1lines == 0)
				printf("0a1,%d\n", f2lines);
			else if(f2lines == 1)
				printf("1,%dc1\n", f1lines);
			else if(f2lines == 0)
				printf("1,%dd0\n", f1lines); 
			else
				printf("1,%dc1,%d\n", f1lines, f2lines);
		}
		//printing the lines
		i = 0;
		while(i < f1lines)
			printf("< %s", f1[i++]);
		if(f2lines != 0 && f1lines != 0)
			printf("---\n");
		i = 0;
		while(i < f2lines)
			printf("> %s", f2[i++]);
		return;
	}
	
	//for displaying the output of diff of the first element, here a line, of lcs of lines in both files
	count = 1;//keeping the count of outputs to be displayed
	i = stop(s1);//checks the topmost element of the stack s1
	j = stop(s2);
	if(lcncount == 1) {
		enqueue(q1, i);
		enqueue(q2, j);
	}

	temp = 0;
	//considering different cases for displaying output of first line in stack formed by lcs matrix
	if(i == 0 && j > 0) { //when file 1 has its first line in lcs, need to add(a)
		if(j != 1)
			printf("0a1,%d\n", j);
		else
			printf("0a1\n");
		//printing the lines
		while(temp < j)
			printf("> %s", f2[temp++]);
	} else if(i > 0 && j == 0) { //when file2 has its first line in lcs, need to delete(d)
		if(i != 1)
			printf("1,%dd0\n", i);
		else
			printf("1d0\n");
		//printing the lines
		while(temp < i)
			printf("< %s", f1[temp++]);
	} else if(i > 0 && j > 0) { //need change(c) when both files don't have first line in lcs
		if(i == 1 && j == 1)
			printf("1c1\n");
		else {
			if(i == 1)
				printf("1c1,%d\n", j);
			else if(j == 1)
				printf("1,%dc1\n", i); 
			else
				printf("1,%dc1,%d\n", i, j);
		}
		//printing the lines
		while(temp < i)
			printf("< %s", f1[temp++]);
		printf("---\n");
		temp = 0;
		while(temp < j)
			printf("> %s", f2[temp++]);
	}
	
	//this while loop is for lines in lcs except for output before the first line, and after the last line
	while(count < lcncount) {
		i = pop(s1);
		enqueue(q1, i);
		j = pop(s2);
		enqueue(q2, j);
		//enqueue the line numbers for later use, as need FIFO
		count++;	
	
		inex = stop(s1);
		jnex = stop(s2); 

		//calculates the difference between current and next line number of file from stack made by lcs matrix
		diff1 = inex - i;
		diff2 = jnex - j;
		
	//considering different cases for displaying output
		//when there are consecutive line numbers in both files from lcs matrix
		if(diff1 == 1 && diff2 == 1) 			
			continue;
		//when file1 has consecutive line numbers in lcs matrix	
		else if(diff1 == 1 && diff2 > 1) {
			if(diff2 == 2)
				printf("%da%d\n", inex, jnex);
			else
				printf("%da%d,%d\n", inex, (j + 2), jnex);
			//printing the lines
			temp = j + 1;
			while(temp < jnex)
				printf("> %s", f2[temp++]);   
		} else if(diff1 > 1 && diff2 == 1) {      //when file2 has consecutive line numbers in lcs matrix
			if(diff1 == 2)
				printf("%dd%d\n", inex, jnex);
			else
				printf("%d,%dd%d\n", (i + 2), inex, jnex);
			//printing the lines
			temp = i + 1;
			while(temp < inex)
				printf("< %s", f1[temp++]);   
		} else if(diff1 > 1 && diff2 > 1) {
			if(diff1 == 2 && diff2 == 2)
				printf("%dc%d\n", (i + 2), (j + 2));	
			else if(diff1 == 2 && diff2 > 2)
				printf("%dc%d,%d\n", (i + 2), (j + 2), jnex);
			else if(diff1 > 2 && diff2 == 2)
				printf("%d,%dc%d\n", (i + 2), inex, (j + 2));
			else
				printf("%d,%dc%d,%d\n", (i + 2), inex, (j + 2), jnex);
			//printing the lines
			temp = i + 1;
			while(temp < inex)
				printf("< %s", f1[temp++]);
			printf("---\n");
			temp = j + 1;
			while(temp < jnex)
				printf("> %s", f2[temp++]);	
		}	
	}

	//for displaying output of diff after last line in lcs matrix of both files
	i = pop(s1);
	enqueue(q1, i);
	j = pop(s2);
	enqueue(q2, j);
	
	if(i == (f1lines - 1) && j < (f2lines - 1)) { //when file 1 has its last line in lcs, need to add(a)
		if(j != (f2lines - 2))
			printf("%da%d,%d\n", (i + 1), (j + 2), f2lines);
		else
			printf("%da%d\n", (i + 1), f2lines);
		//printing the lines
		temp = j + 1;
		while(temp < f2lines)
			printf("> %s", f2[temp++]);
	} else if(i < (f1lines - 1) && j == (f2lines - 1)) { //when file2 has its last line in lcs, need to delete(d)
		if(i != (f1lines - 2))
			printf("%d,%dd%d\n", (i + 2), f1lines, (j + 1));
		else
			printf("%dd%d\n", f1lines, (j + 1));
		//printing the lines
		temp = i + 1;
		while(temp < f1lines)
			printf("< %s", f1[temp++]);
	} else if(i < (f1lines - 1)  && j < (f2lines - 1)) { //need change(c) when both files don't have last line in lcs
		if(i == (f1lines - 2) && j == (f2lines - 2))
			printf("%dc%d\n", f1lines, f2lines);
		else {
			if(i == (f1lines - 2))
				printf("%dc%d,%d\n", f1lines, (j + 2), f2lines);
			else if(j == (f2lines - 2))
				printf("%d,%dc%d\n", (i + 2), f1lines, f2lines); 
			else
				printf("%d,%dc%d,%d\n", (i + 2), f1lines, (j + 2), f2lines);
		}
		//printing the lines
		temp = i + 1;
		while(temp < f1lines)
			printf("< %s", f1[temp++]);
		printf("---\n");
		temp = j + 1;
		while(temp < f2lines)
			printf("> %s", f2[temp++]);
	}
} 
