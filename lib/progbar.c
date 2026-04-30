#include "progbar.h"

static char* barElements[]={" ","▏","▎","▍","▌","▋","▊","▉","█"};

static void printProgBar(progbar* bar) {
	size_t percent=(100*bar->value)/bar->max;
	printf("%s | %3zu%% | ", bar->title, percent);
	size_t lenVal=bar->length*bar->value;
	size_t fullLen=lenVal/bar->max;
	size_t part=(8*(lenVal%bar->max))/bar->max;
	for(size_t i=0; i<bar->length; i++) {
		if(i<fullLen) {
			printf("%s",barElements[8]);
		}
		else if(i==fullLen) {
			printf("%s",barElements[part]);
		}
		else {
			printf("%s",barElements[0]);
		}
	}
	printf(" | %3zu%%\n", percent);
}

void startProgBar(progbar* bar) {
	bar->value=0;
	printProgBar(bar);
}

void showProgBar(progbar* bar) {
	size_t progVal=(8*bar->length>100 ? 8*bar->length*bar->value : 100*bar->value)/bar->max;
	if(progVal>bar->lastUpdated) {
		bar->lastUpdated=progVal;
		printProgBar(bar);
	}
}

void incProgBar(progbar* bar) {
	bar->value++;
	showProgBar(bar);
}

void completeProgBar(progbar* bar) {
	bar->value=bar->max;
	printf("\033[A\33[2K");
	printf("%s | 100%% | ", bar->title);
	for(size_t i=0; i<bar->length; i++) {
		printf("%s",barElements[8]);
	}
	if(bar->bell)
		printf(" | Done\a\n");
	else
		printf(" | Done\n");
}

void printOverProgbar(progbar* bar, char* str) {
	printf("\033[A\33[2K%s\n",str);
	printProgBar(bar);
}