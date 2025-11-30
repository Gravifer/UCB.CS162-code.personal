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

const bool  FLAG_PTHREADS = true;
const u_int MAX_THREADS = -1;

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
    struct idx_lock {
      int pos;
      pthread_mutex_t lock;
    } idx = { .pos = 0, .lock = PTHREAD_MUTEX_INITIALIZER };
    struct thread_args { // ANCHOR[id=pwords-main-thread_args]
      int num_files;
      char** filenames;
      struct idx_lock* idx;
      word_count_list_t* wclist;
      int tid;
    };
    int num_files = argc - 1;
    int num_threads = ((u_int)num_files < MAX_THREADS) ? num_files : MAX_THREADS;
    // // pthread_t threads[num_files]; // ! let's not use VLA
    // pthread_t* threads = malloc(sizeof(pthread_t) * num_files);
    pthread_t* threads = malloc(sizeof(pthread_t) * num_threads); void* worker(void* arg); // capped thread pool
    if (threads == NULL) {
      fprintf(stderr, "Error allocating memory for threads\n");
      return 1;
    }

    struct thread_args* targs = malloc(sizeof(struct thread_args) * num_files);
    if (targs == NULL) {
      fprintf(stderr, "Error allocating memory for thread args\n");
      free(threads);
      return 1;
    }
    for (int i = 0; i < num_threads; i++) {
      targs[i] = (struct thread_args){
        .num_files = num_files,
        .filenames = &argv[1],
        .idx    = &idx,
        .wclist = &word_counts,
        .tid    = i + 1, // let main thread be thread 0
      };
      int rc = pthread_create(&threads[i], NULL, worker, &targs[i]);
      if (rc != 0) {
        fprintf(stderr, "Error creating thread %d (of %d)\n", i, num_threads);
        free(threads);
        return 1;
      }
    }
    for (int i = 0; i < num_threads; i++) {
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

void* worker(void* arg) {
  struct idx_lock {
    int pos;
    pthread_mutex_t lock;
  };
  struct thread_args { // LINK pwords.c#pwords-main-thread_args
    int num_files;
    char** filenames;
    struct idx_lock* idx;
    word_count_list_t* wclist;
    int tid;
  } *t = arg;
  pthread_mutex_t *ilk = &t->idx->lock;

  while (1) {
    char* filename;

    pthread_mutex_lock(ilk);
    if (t->idx->pos >= t->num_files) {
      pthread_mutex_unlock(ilk);
      break;
    }
    filename = t->filenames[t->idx->pos];
    // fprintf(stderr, "(thread %d) Processing file[%d] %s\n", t->tid, t->idx->pos, filename);
    t->idx->pos++;
    pthread_mutex_unlock(ilk);

    FILE* infile = fopen(filename, "r");
    if (!infile) {
      fprintf(stderr, "(thread %d) Could not open input file %s\n",
              t->tid, filename);
      continue;
    }

    count_words(t->wclist, infile);
    fclose(infile);
  }

  pthread_exit(NULL);
}
