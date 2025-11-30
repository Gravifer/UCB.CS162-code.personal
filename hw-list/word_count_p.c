/*
 * Implementation of the word_count interface using Pintos lists and pthreads.
 *
 * You may modify this file, and are expected to modify it.
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

#ifndef PINTOS_LIST
#error "PINTOS_LIST must be #define'd when compiling word_count_lp.c"
#endif

#ifndef PTHREADS
#error "PTHREADS must be #define'd when compiling word_count_lp.c"
#endif

#include "word_count.h"

#ifndef SPECULATE
#undef SPECULATE
#endif

void init_words(word_count_list_t* wclist) { /* TODO */
  list_init(&wclist->lst);
  pthread_mutex_init(&wclist->lock, NULL);
}

size_t len_words(word_count_list_t* wclist) {
  /* DONE */
  pthread_mutex_lock(&wclist->lock);
  size_t cnt = 0;
  for (struct list_elem* e = list_begin(&wclist->lst);
       e != list_end(&wclist->lst); e = list_next(e)) {
    cnt++;
  }
  pthread_mutex_unlock(&wclist->lock);
  return cnt;
}

word_count_t* find_word(word_count_list_t* wclist, char* word) {
  for (struct list_elem* e = list_begin(&wclist->lst);
       e != list_end(&wclist->lst); e = list_next(e)) {
    word_count_t* wc = list_entry(e, word_count_t, elem);
    if (strcmp(wc->word, word) == 0)
      return wc;
  }
  return NULL;
}

word_count_t* add_word(word_count_list_t* wclist, char* word) {
  /* DONE */

  #ifdef SPECULATE
  // Speculate - Allocate new object before locking
  word_count_t* new_wc = malloc(sizeof(word_count_t));
  if (!new_wc) return NULL;
  new_wc->word = strdup(word);
  if (!new_wc->word) { free(new_wc); return NULL; }
  new_wc->count = 1;
  #endif // SPECULATE

  pthread_mutex_lock(&wclist->lock);
  word_count_t* wc = find_word(wclist, word);
  if (wc != NULL) {
    wc->count++;
    pthread_mutex_unlock(&wclist->lock);
    #ifdef SPECULATE
    // remeber to free the allocated new_wc since we didn't use it
    free(new_wc->word);
    free(new_wc);
    #endif // SPECULATE
    return wc;
  }
  // if not found, create a new word_count_t
  #ifndef SPECULATE
  word_count_t* new_wc = malloc(sizeof(word_count_t));
  if (new_wc == NULL) {
    pthread_mutex_unlock(&wclist->lock);
    return NULL;
  }
  new_wc->word = malloc(strlen(word) + 1);
  if (new_wc->word == NULL) {
    free(new_wc);
    pthread_mutex_unlock(&wclist->lock);
    return NULL;
  }
  strcpy(new_wc->word, word);
  new_wc->count = 1;
  #endif // NO SPECULATE
  list_push_back(&wclist->lst, &new_wc->elem);
  pthread_mutex_unlock(&wclist->lock);
  return new_wc;
}

void fprint_words(word_count_list_t* wclist, FILE* outfile) {
  /* DONE */
  /* Please follow this format: fprintf(<file>, "%i\t%s\n", <count>, <word>); */
  pthread_mutex_lock(&wclist->lock);
  for (struct list_elem* e = list_begin(&wclist->lst);
       e != list_end(&wclist->lst); e = list_next(e)) {
    word_count_t* wc = list_entry(e, word_count_t, elem);
    fprintf(outfile, "%i\t%s\n", wc->count, wc->word);
  }
  pthread_mutex_unlock(&wclist->lock);
}

static bool less_list(const struct list_elem* ewc1, const struct list_elem* ewc2, void* aux) {
  word_count_t* wc1 = list_entry(ewc1, word_count_t, elem);
  word_count_t* wc2 = list_entry(ewc2, word_count_t, elem);
  bool (*less)(const word_count_t*, const word_count_t*) = aux;
  return less(wc1, wc2);
}

void wordcount_sort(word_count_list_t* wclist,
                    bool less(const word_count_t*, const word_count_t*)) {
  /* DONE */
  pthread_mutex_lock(&wclist->lock);
  list_sort(&wclist->lst, less_list, less);
  pthread_mutex_unlock(&wclist->lock);
}
