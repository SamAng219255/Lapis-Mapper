#include <stdio.h>

typedef struct {
	size_t max;
	size_t value;
	char *title;
	size_t length;
	size_t lastUpdated;
	int bell;
} progbar;

#define newProgBar(Max,Length,Title,Bell) ((progbar){.max=Max,.value=0,.length=Length,.title=Title,.lastUpdated=0,.bell=Bell})

extern void printProgBar(progbar* bar);
extern void startProgBar(progbar* bar);
extern void showProgBar(progbar* bar);
extern void incProgBar(progbar* bar);
extern void completeProgBar(progbar* bar);
extern void printOverProgbar(progbar* bar, FILE* stream, char* str);