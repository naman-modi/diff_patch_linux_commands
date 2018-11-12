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

#define A 1
#define C 2
#define D 3

int patch_hunk_normal(int pline, char (*f1)[1024], char (*f2)[1024], int fd, stack *s1, stack *s2, int f2lines, int f1lines, int rev);

int patch_command_normal(char (*f1)[1024], char (*f2)[1024], int fd, int f2lines, int f1lines);

int line_numbers(char (*f), stack *s1, stack *s2);
	
int patch_command_by_c(char (*f1)[1024], char (*f2)[1024], int rfd3, stack *s1, stack *s2, int f2lines, int f1lines);

int read_file(int rfd, char (*f)[1024]);

int patch_hunk_c(int pline, char (*f1)[1024], char (*f2)[1024], int rfd3, stack *s1, stack *s2, int f2lines, int f1lines);

int stop(stack *s);

int main(int argc, char *argv[]) {
	
	int i, rfd1, rfd2, f1lines, f2lines, success;
	char f1[1024][1024], f2[1024][1024], threestar[4], *star;

	stack s1, s2;
	init(&s1);
	init(&s2);
	
	for(i = 0; i < 3; i++) 
		threestar[i] = '*';
	threestar[3] = '\0';

	rfd1 = open(argv[1], O_RDWR, S_IRUSR | S_IWUSR);
	rfd2 = open(argv[2], O_RDWR, S_IRUSR | S_IWUSR);
	
	if(rfd1 == -1)	{
		//printf("mydiff: %s: ", argv[optind]);
		perror("");
		exit(errno);
	}
	if(rfd2 == -1)	{
		printf("mydiff: '%s': ", argv[2]);
		exit(errno);
	}
	
	//Reading the second file into a 2-dimensional array
	f1lines = read_file(rfd1, f1); //retuens total number of lines in first file
	
	//Reading the second file into a 2-dimensional array
	f2lines = read_file(rfd2, f2); //return total number of lines in second file
	
	if(f2lines == 0) {
		close(rfd1);
		close(rfd2);
		return 0;
	}	
	
	close(rfd1);
	
	rfd1 = open(argv[1], O_RDONLY | O_WRONLY | O_TRUNC);

	printf("patching file %s\n", argv[1]);
	
	star = strtok(f2[0], " ");
	if(strcmp(star, threestar) == 0) 
		success = patch_command_by_c(f1, f2, rfd1, &s1, &s2, f2lines, f1lines);
	else
		success = patch_command_normal(f1, f2, rfd1, f2lines, f1lines);
	
	success++;
	f1lines++;
	close(rfd1);
	close(rfd2);
	return 0;
}

int read_file(int rfd, char (*f)[1024]) {
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
	
int patch_command_by_c(char (*f1)[1024], char (*f2)[1024], int rfd3, stack *s1, stack *s2, int f2lines, int f1lines) {
	char fifteenstarn[17];
	int i, k, pline = 0, value, hunk_count = 0, count = 0, fail = 0;
	queue p;
	qinit(&p);
	for(i = 0; i < 15; i++)
		fifteenstarn[i] = '*';
	fifteenstarn[15] = '\n';
	fifteenstarn[16] = '\0';
	while(pline < f2lines) {
		if(strcmp(f2[pline], fifteenstarn) == 0) {
			enqueue(&p, (pline + 1));
			hunk_count++;
		}
		pline++;
	}
	while(!qisempty(&p)) {
		pline = dequeue(&p);
		value = patch_hunk_c(pline, f1, f2, rfd3, s1, s2, f2lines, f1lines);

		if(value == -1024) {
			printf("Hunk #%d FAILED.\n", count + 1);
			fail++;
		} else if(value == -2048)
			break;
		else {
			if(fail >= 1) 
				printf("Hunk #%d succedeed. (offset %d lines)\n", count + 1, value);	
		}
		count++;
	}
	if(fail > 0 && value != 2048) {
		printf("%d out of %d Hunks FAILED.\n", fail, hunk_count);
	}
	if(fail == hunk_count || value == -2048) {
		i = 0;
		while(i < f1lines) {
			k = strlen(f1[i]);
			write(rfd3, f1[i], k);
			i++;
		}
	}
	return 0;
}

int patch_hunk_c(int pline, char (*f1)[1024], char (*f2)[1024], int rfd3, stack *s1, stack *s2, int f2lines, int f1lines) {
	char *l1, *l2, *final, fourstar[5], temp[1024], temp2[1024], *dash, threedash[4], fourdash[5], fifteenstar[17], check[1024][1024];
	int i, j, initialpline, ch, diff = 0, blankflag = 0, fileoneblank, linestart1, lineend1, linestart2, lineend2, lineno, k, count, finalline;
	initialpline = pline;
	for(i = 0; i < 15; i++)
		fifteenstar[i] = '*';
	fifteenstar[15] = '\n';
	fifteenstar[16] = '\0';
	for(i = 0; i < 4; i++)
		fourstar[i] = '*';
	fourstar[4] = '\0';
	for(i = 0; i < 3; i++)
		threedash[i] = '-';
	threedash[3] = '\0';
	for(i = 0; i < 4; i++)
		fourdash[i] = '-';
	fourdash[4] = '\0';
//checking for line number using strtok
	l1 = strtok(f2[pline], " ");
	l1 = strtok(NULL, ", ");
	l2 = strtok(NULL, " \n");
	linestart1 = atoi(l1);
	if(strcmp(l2, fourstar) != 0)
		lineend1 = atoi(l2);
	else
		lineend1 = linestart1;
	temp[0] = '\0';

	strcpy(temp, f2[pline + 1]);
	dash = strtok(temp, " \n");
	if(strcmp(dash, threedash) == 0) {
		fileoneblank = 1;
		pline++;
		dash = strtok(f2[pline], " ");
		l1 = strtok(NULL, ", ");
		l2 = strtok(NULL, " \n");
		linestart2 = atoi(l1);
		if(strcmp(l2, fourdash) != 0)
			lineend2 = atoi(l2);
		else
			lineend2 = linestart2;
	} else {
		fileoneblank = 0;
		pline = pline + (lineend1 - linestart1) + 2; 
		dash = strtok(f2[pline], " ");
		l1 = strtok(NULL, ", ");
		l2 = strtok(NULL, " \n");
		linestart2 = atoi(l1);
		if(strcmp(l2, fourdash) != 0)
			lineend2 = atoi(l2);
		else
			lineend2 = linestart2;

	}
//	printf("one blank %d\n", fileoneblank);
	if(fileoneblank == 0) {//when
		count = initialpline + 1;
		finalline = count + (lineend1 - linestart1);
		k = linestart1 - 1;
		ch = 0;
		while(count <= finalline) {
			strcpy(temp2, f2[count]);
			if(temp2[0] == ' ') {
				j = 0;
				for(i = 2; temp2[i] != '\0'; i++) {
					temp[j++] = temp2[i];
				}
				temp[j] = '\0';
				strcpy(check[ch], temp);
			} else if(temp2[0] == '+' || temp2[0] == '-' || temp2[0] == '!') {
				final = strtok(temp2, " ");
				final = strtok(NULL, "\0");
				strcpy(check[ch], final);
			} else {
				printf("mypatch:  **** malperformed at line %d: %s\n", count + 1, temp2);
				return -2048;
			}
			temp[0] = '\0';
			temp2[0] = '\0';
			count++;
			ch++;
			k++;
		}
		i = 0;
		diff = lineend1 - linestart1;
		while(i <= (f1lines - diff)) {
			if(strcmp(check[0], f1[i]) == 0) {
				k = 0;
				while(k < ch) {
					if(strcmp(check[k], f1[i + k]) == 0) {
						k++;
						continue;
					} else 
						break;
				}
				if(k == ch) {
					diff = i - (linestart1 - 1);
					break;
				}
			}
			i++;
		}
		if(i > (f1lines - diff))
			return -1024;
	} else {
		count = pline + 1;
		finalline = count + (lineend2 - linestart2);
		ch = 0;
		while(count <= finalline) {
			strcpy(temp2, f2[count]);
			if(temp2[0] == ' ') {
				j = 0;
				for(i = 2; temp2[i] != '\0'; i++) {
					temp[j++] = temp2[i];
				}
				temp[j] = '\0';
				strcpy(check[ch], temp);
				ch++;
			} else if((f2[count][0] != '+') && (f2[count][0] != '-') && (f2[count][0] != '!')) {
				printf("mypatch:  **** malperformed at line %d: %s", count + 1, f2[count]);
				return -2048;
			}
			temp[0] = '\0';
			temp2[0] = '\0';
			count++;
		}
		i = 0;
		diff = ch - 1;
		while(i <= (f1lines - diff)) {
			if(strcmp(check[0], f1[i]) == 0) {
				k = 0;
				while(k < ch) {
					if(strcmp(check[k], f1[i + k]) == 0) {
						k++;
						continue;
					} else 
						break;
				}
				if(k == ch) {
					diff = i - (linestart1 - 1);
					break;
				}
			}
			i++;
		}
		if(i > (f1lines - diff))
			return -1024;
	}

	if(!isempty(s1)) {
		lineno = stop(s1);
		for(k = lineno; k < (linestart1 - 1); k++) {
			if(k >= f1lines) 
				return -1024;
			strcpy(temp, f1[k]);
			i = strlen(temp);
			write(rfd3, temp, i);
			temp[0] = '\0';
		}
	}
	
	if(pline == (f2lines - 1)) 
		blankflag = 1;
	else {
		strcpy(temp, f2[pline + 1]);
		if(strcmp(temp, fifteenstar) == 0)
			blankflag = 1;
		temp[0] = '\0';
	}
//	printf("%d-blankflag\n", blankflag);
	if(blankflag == 0) {
		count = pline + 1;
		finalline = count + (lineend2 - linestart2);
		while(count <= finalline) {
			if(f2[count][0] == ' ') {
				j = 0;
				for(i = 2; f2[count][i] != '\0'; i++) {
					temp[j++] = f2[count][i];
				}
				temp[j] = '\0';		
				i = strlen(temp);

				write(rfd3, temp, i);
			} else if(f2[count][0] == '+' || f2[count][0] == '-' || f2[count][0] == '!') {
				final = strtok(f2[count], " ");
				final = strtok(NULL, "\0");
				i = strlen(final);

				write(rfd3, final, i);
			} else {
				printf("mypatch:  **** malperformed at line %d: %s\n", count + 1, f2[count]);
				return -2048;
			}
			temp[0] = '\0';
			count++;
		} 
	} else {
		count = pline - (lineend1 - linestart1 + 1);
		finalline = pline;
		while(count < finalline) {
			if(f2[count][0] == ' ') {
				j = 0;
				for(i = 2; f2[count][i] != '\0'; i++) {
					temp[j++] = f2[count][i];
				}
				temp[j] = '\0';		
				i = strlen(temp);
				write(rfd3, temp, i);
			} else if((f2[count][0] != '+') && (f2[count][0] != '-') && (f2[count][0] != '!')) {
				printf("mypatch:  **** malperformed at line %d: %s", count + 1, f2[count]);
				return -2048;
			}

			count++;
			temp[0] = '\0';
		}
	}

	push(s1, linestart1);
	if(lineend1 != linestart1)
		push(s1, lineend1);
	push(s2, linestart2);
	if(lineend2 != linestart2)
			push(s2, lineend2);
	return diff;
}

int patch_command_normal(char (*f1)[1024], char (*f2)[1024], int fd, int f2lines, int f1lines) {
	int i, value, pline;
	char *final, temp[1024];
	queue p;
	stack s1, s2;
	init(&s1);
	init(&s2);
	qinit(&p);
	
	i = 0;
	while(i < f2lines) {
		if(f2[i][0] >= '0' && f2[i][0] <= '9') {
			strcpy(temp, f2[i]);
			final = strtok(temp, "acd");
			if(strcmp(f2[i], final) != 0)
				enqueue(&p, i);
		}	
		i++;
	}
	
	while(!qisempty(&p)) {
		pline = dequeue(&p);
		if(!qisempty(&p))
			value = patch_hunk_normal(pline, f1, f2, fd, &s1, &s2, f2lines, f1lines, 0);
		else
			value = patch_hunk_normal(pline, f1, f2, fd, &s1, &s2, f2lines, f1lines, 1);
		
		if(value == -1024)
			printf("Hunk  FAILED.\n");
	}

	return 0;
}

int patch_hunk_normal(int pline, char (*f1)[1024], char (*f2)[1024], int fd, stack *s1, stack *s2, int f2lines, int f1lines, int rev) {
	int alpha, ls1, ls2, le1, le2, k, count, finalline, i, prevline;
	char *fe;
	alpha = line_numbers(f2[pline], s1, s2);
	printf("%d\n", alpha);
	le1 = pop(s1);
	ls1 = pop(s1);
	le2 = pop(s2);
	ls2 = pop(s2);
	
	if(!isempty(s1))
		prevline = stop(s1);
	else
		prevline = 0;

	for(k = prevline; k < (ls1 - 1); k++) {
		if(k > f1lines)
			return -1024;
		i = strlen(f1[k]);
		write(fd, f1[k], i);
	}
	if(alpha == A) {
		if(k > f1lines)
			return -1024;
		i = strlen(f1[k]);
		write(fd, f1[k], i);
		count = pline + 1;
		finalline = count + (le2 - ls2);
		printf("%s%s", f2[count], f2[finalline]);
		while(count <= finalline) {
			fe = strtok(f2[count], " ");
			fe = strtok(NULL, "\0");
			i = strlen(fe);
			write(fd, fe, i);
			count++;
		}
	} else if(alpha == C) {
		count = pline + 3 + le1 - ls1;
		finalline = count + (le2 - ls2);
		printf("%s%s", f2[count], f2[finalline]);
		while(count <= finalline) {
			fe = strtok(f2[count], " ");
			fe = strtok(NULL, "\0");
			i = strlen(fe);
			write(fd, fe, i);
			count++;
		}	
	}

	if(rev == 0) {
		if(le2 != f2lines - 1) {
			count = le1;
			finalline = f1lines;
			while(count < finalline) {
				if(k > f1lines)
					return -1024;
				i = strlen(f1[count]);
				write(fd, f1[count], i);
				count++;
			}
		}
	}

	push(s1, ls1);
	if(le1 != ls1)
		push(s1, le1);
	push(s2, ls2);
	if(le2 != ls2)
		push(s2, le2);
	
	return 0;
}


int line_numbers(char (*f), stack *s1, stack *s2) {
	char t[1024], t1[1024], t2[1024], t3[1024], *fn;
	int linestart1, val, lineend1, linestart2, lineend2, i, j, k;
	strcpy(t, f);
	for(i = 0; f[i] != '\0'; i++) {
		if(f[i] == 'a') {
			t[i] = '\0';
			val = A;
			break;
		} else if(f[i] == 'c') {
			t[i] = '\0';
			val = C;
			break;
		} else if(f[i] == 'd') {
			t[i] = '\0';
			val = D;
			break;
		}
	}

	strcpy(t2, t);
	fn = strtok(t2, ",");
	linestart1 = atoi(fn);
	if(strcmp(t, fn) != 0) {
		fn = strtok(NULL, "\0");
		lineend1 = atoi(fn);
	}
	else
		lineend1 = linestart1; 

	k = 0;
	for(j = i + 1; f[j] != '\n'; j++) {
		t3[k] = f[j];
		k++;
	}
	t3[k] = '\0';
	
	strcpy(t1, t3);
	fn = strtok(t1, ",");
	linestart2 = atoi(fn);
	if(strcmp(t3, fn) != 0) {
		fn = strtok(NULL, "\0");
		lineend2 = atoi(fn);
	}
	else
		lineend2 = linestart2;

	//printf("%d %d %d %d\n", linestart1, lineend1, linestart2, lineend2); 
	
	push(s1, linestart1);
	push(s1, lineend1);
	push(s2, linestart2);
	push(s2, lineend2);
	
	return val;
}


int stop(stack *s) {
	int w;
	w = pop(s);
	push(s, w);
	return w;
}
