/*
 * Word count application with one thread per input file.
 *
 * You may modify this file in any way you like, and are expected to modify it.
 * Your solution must read each input file from a separate thread. We encourage
 * you to make as few changes as necessary.
 */

/*
 * Copyright © 2021 University of California, Berkeley
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <pthread.h>

#include "word_count.h"
#include "word_helpers.h"

const bool FLAG_PTHREADS = true;

/*
 * main - handle command line, spawning one thread per file.
 */
int main(int argc, char* argv[]) {
  /* Create the empty data structure. */
  word_count_list_t word_counts;
  init_words(&word_counts);

  if (argc <= 1) {
    /* Process stdin in a single thread. */
    count_words(&word_counts, stdin);
  } else if (FLAG_PTHREADS) {
    /* DONE */
    int num_files = argc - 1;
    // // pthread_t threads[num_files]; // ! let's not use VLA
    pthread_t* threads = malloc(sizeof(pthread_t) * num_files);
    struct thread_args {
      int threadid;
      word_count_list_t* wclist;
      char* filename;
    };
    void* threadfun(void* arg);
    struct thread_args* targs = malloc(sizeof(struct thread_args) * num_files);
    for (int i = 0; i < num_files; i++) {
      targs[i] = (struct thread_args){
        .threadid = i + 1, // let main thread be thread 0
        .wclist   = &word_counts,
        .filename = argv[i + 1]
      };
      // fprintf(stderr, "Main: creating thread %d for file %s\n", 
      //         targs[i].threadid, targs[i].filename);
      int rc = pthread_create(&threads[i], NULL, threadfun, &targs[i]);
      if (rc != 0) {
        fprintf(stderr, "Error creating thread for file %s\n", argv[i + 1]);
        free(threads);
        return 1;
      }
    }
    for (int i = 0; i < num_files; i++) {
      pthread_join(threads[i], NULL);
    }
  } else { // for debugging
    /* Process each file in sequence in the main thread. */
    for (int i = 1; i < argc; i++) {
      FILE* infile = fopen(argv[i], "r");
      if (infile == NULL) {
        fprintf(stderr, "Could not open input file %s\n", argv[i]);
        return 1;
      }
      count_words(&word_counts, infile);
      fclose(infile);
    }
  }

  /* Output final result of all threads' work. */
  wordcount_sort(&word_counts, less_count);
  fprint_words(&word_counts, stdout);
  return 0;
}

void* threadfun(void* arg) {
  struct thread_args {
    int threadid;
    word_count_list_t* wclist;
    char* filename;
  };
  struct thread_args* targs = (struct thread_args*)arg;
  // * we need to open the file here
  FILE* infile = fopen(targs->filename, "r");
  if (infile == NULL) {
    fprintf(stderr, "(thread %d) Could not open input file %s\n", 
            targs->threadid, targs->filename);
    pthread_exit(NULL);
  }
  count_words(targs->wclist, infile);
  fclose(infile);
  pthread_exit(NULL);
}
