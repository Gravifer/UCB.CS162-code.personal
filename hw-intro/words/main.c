/*

  Word Count using dedicated lists

*/

/*
Copyright © 2019 University of California, Berkeley

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <assert.h>
#include <getopt.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>

#include "word_count.h"

/* Global data structure tracking the words encountered */
WordCount *word_counts = NULL;

/* The maximum length of each word in a file */
#define MAX_WORD_LEN 64
bool FLAG_IGNORE_MONOGRAM = true;

/*
 * 3.1.1 Total Word Count
 *
 * Returns the total amount of words found in infile.
 * Useful functions: fgetc(), isalpha().
 */
int num_words(FILE* infile) {
  // when FLAG_IGNORE_MONOGRAM, ignore words of length 1
  int num_words = 0;
  int in_word = -1; // -1 not in a word, encountered length when in a word
  char c = (char)fgetc(infile);
  while (c != EOF) {
    if (isalpha(c)) {
      if (in_word == -1) {
        in_word = 1; // start of a new word
      } else {
        in_word++; // continue the word
      }
    } else { // non-alphabetic character
      if (in_word != -1) {
        // we were in a word, now it ends
        if (!(FLAG_IGNORE_MONOGRAM && in_word == 1)) {
          num_words++;
        }
        in_word = -1; // reset for next word
      }
    }
    c = (char)fgetc(infile);
  }

  return num_words;
}

/*
 * 3.1.2 Word Frequency Count
 *
 * Given infile, extracts and adds each word in the FILE to `wclist`.
 * Useful functions: fgetc(), isalpha(), tolower(), add_word().
 * 
 * As mentioned in the spec, your code should not panic or
 * segfault on errors. Thus, this function should return
 * 1 in the event of any errors (e.g. wclist or infile is NULL)
 * and 0 otherwise.
 */
int count_words(WordCount **wclist, FILE *infile) {
  if (wclist == NULL || infile == NULL) {
    dbg("Error: wclist or infile is NULL");
    return 1;
  }
  dbg("Counting words in file (fd at %p)", infile);
  char buffer[MAX_WORD_LEN + 1];
  int fget_word(FILE *infile, char* buffer, size_t bufsize);
  while (fget_word(infile, buffer, sizeof(buffer)) == 0) {
    if (FLAG_IGNORE_MONOGRAM && strlen(buffer) == 1) {
      dbg("Ignoring monogram word \"%s\"", buffer);
      continue;
    }
    if (add_word(wclist, buffer) != 0) {
      dbg("Error adding word \"%s\" to word count list", buffer);
      return 1;
    }
  }
  return 0;
}
int fget_word(FILE *infile, char *buffer, size_t bufsize) {
  if (infile == NULL || buffer == NULL || bufsize == 0) {
    dbg("Error: infile or buffer is NULL, or bufsize is 0");
    return 1;
  }
  dbg("Getting next word from file (fd at %p)", infile);
  size_t idx = 0;
  char c = (char)fgetc(infile);
  // Skip non-alphabetic characters
  while (c != EOF && !isalpha(c)) {
    c = (char)fgetc(infile);
  }
  // Read alphabetic characters into buffer
  while (c != EOF && isalpha(c) && idx < bufsize - 1) {
    buffer[idx++] = (char)tolower(c);
    c = (char)fgetc(infile);
  }
  buffer[idx] = '\0'; // Null-terminate the string
  // If we read at least one character, return success
  if (idx > 0) {
    dbg("Found word: \"%s\"", buffer);
    return 0;
  } else {
    dbg("No more words found in file (fd at %p)", infile);
    return 1; // No word found
  }
}

/*
 * Comparator to sort list by frequency.
 * Useful function: strcmp().
 */
static bool wordcount_less(const WordCount *wc1, const WordCount *wc2) {
  if (wc1->count == wc2->count) {
    return strcmp(wc1->word, wc2->word) < 0;
  }
  return wc1->count < wc2->count;
}

// In trying times, displays a helpful message.
static int display_help(void) {
	printf("Flags:\n"
	    "--count (-c): Count the total amount of words in the file, or STDIN if a file is not specified. This is default behavior if no flag is specified.\n"
	    "--frequency (-f): Count the frequency of each word in the file, or STDIN if a file is not specified.\n"
	    "--help (-h): Displays this help message.\n");
	return 0;
}

/*
 * Handle command line flags and arguments.
 */
int main (int argc, char *argv[]) {
  dbg("Starting word count program");
  // Count Mode (default): outputs the total amount of words counted
  bool count_mode = true;
  int total_words = 0;

  // Freq Mode: outputs the frequency of each word
  bool freq_mode = false;

  FILE *infile = NULL; // LINK words/main.c#do-files

  // Variables for command line argument parsing
  int i;
  static struct option long_options[] =
  {
      {"count", no_argument, 0, 'c'},
      {"frequency", no_argument, 0, 'f'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
  };

  // Sets flags
  while ((i = getopt_long(argc, argv, "cfh", long_options, NULL)) != -1) {
      switch (i) {
          case 'c':
              count_mode = true;
              freq_mode = false;
              break;
          case 'f':
              count_mode = false;
              freq_mode = true;
              break;
          case 'h':
              return display_help();
      }
  }

  if (!count_mode && !freq_mode) {
    printf("Please specify a mode.\n");
    return display_help();
  }

  /* Create the empty data structure */
  i = init_words(&word_counts);
  if (i != 0) { // || word_counts == NULL) {
    fprintf(stderr, "Error initializing word count list\n");
    return 1;
  }

  if ((argc - optind) < 1) {
    // No input file specified, instead, read from STDIN instead.
      infile = stdin;
      if (count_mode) {
        dbg("Counting words from %s", "STDIN");
        total_words += num_words(infile);
      } else {
        dbg("Collecting word freqs from STDIN");
        if (count_words(&word_counts, infile)) {
          fprintf(stderr, "Error counting words in file %s\n", "STDIN");
          fclose(infile);
          return 1;
        }
      }
  } else { // ANCHOR do-files
    // At least one file specified. Useful functions: fopen(), fclose().
    // The first file can be found at argv[optind]. The last file can be
    // found at argv[argc-1].
    for (int argi = optind; argi < argc; argi++) {
      infile = fopen(argv[argi], "r");
      if (infile == NULL) {
        fprintf(stderr, "Could not open file %s\n", argv[argi]);
        return 1;
      }

      if (count_mode) {
        dbg("Counting words from %s", argv[argi]);
        total_words += num_words(infile);
      } else {
        dbg("Collecting word freqs from %s", argv[argi]);
        if (count_words(&word_counts, infile)) {
          fprintf(stderr, "Error counting words in file %s\n", argv[argi]);
          fclose(infile);
          return 1;
        }
      }

      fclose(infile);
    }
  }

  if (count_mode) {
    printf("The total number of words is: %i\n", total_words);
  } else {
    wordcount_sort(&word_counts, wordcount_less);

    printf("The frequencies of each word are: \n");
    fprint_words(word_counts, stdout);
}
  return 0;
}
