#include <stdio.h>

typedef struct {
	size_t max; // Maximum bar value
	size_t value; // Current bar value
	char *title; // Bar title
	size_t length; // Length of actual bar (does not include title or displayed percentages, 17 characters plus title)
	size_t lastUpdated; // The display value last time the bar was printed
	int bell; // Boolean | Whether a bell should be printed/played when bar completes.
} progbar;

#define newProgBar(Max,Length,Title,Bell) ((progbar){.max=Max,.value=0,.length=Length,.title=Title,.lastUpdated=0,.bell=Bell})

extern void startProgBar(progbar* bar); // Initializes an empty progress bar and shows it.
extern void showProgBar(progbar* bar); // Displays the progress bar (overwrites previous bar)
extern void incProgBar(progbar* bar); // Increments the bar's value and shows it.
extern void completeProgBar(progbar* bar); // Sets the value to max and shows it with special formatting indicating completion.
extern void printOverProgbar(progbar* bar, char* str); // Prints a message above the progress bar.