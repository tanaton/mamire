#ifndef MAMIRE_H_INCLUDE
#define MAMIRE_H_INCLUDE

#include <stdbool.h>
#include <pthread.h>

#include "unstring.h"
#include "unarray.h"
#include "unmap.h"
#include "search.h"

#define MAMIRE_ITA_NAME_PATH			"/2ch/dat/ita.data"
#define MAMIRE_ROOT_PATH				"/2ch/dat"

#define MAMIRE_SEARCH_NAME_YOUTUBE		"youtube"
#define MAMIRE_SEARCH_NAME_NICOVIDEO	"nicovideo"
#define MAMIRE_SEARCH_NAME_2CH			"2ch"

#define MAMIRE_PATTERN_YOUTUBE			"youtube\\.(com|jp)\\/watch\\?v=[\\w\\d\\-\\_]{11}"
#define MAMIRE_PATTERN_NICOVIDEO		"nicovideo.jp\\/watch/sm[\\d]+"
#define MAMIRE_PATTERN_2CH				"(\\w+\\.2ch\\.net|\\w+\\.bbspink\\.com)\\/test\\/read\\.\\w+[\\/|#]\\w+\\/\\d{9,10}"

void global_init(void);
void *threads_main(void *p);
bool search_copy(search_t *s1, search_t *s2, pthread_mutex_t *mutex);
bool thread_concat(unarray_t *a1, unarray_t *a2);
unarray_t *getBoardList( void );
unarray_t *getThreadList(path_t *board);
void path_free(void *p);

void *mamire_malloc(size_t size);
void *mamire_realloc(void *p, size_t size, size_t len);

#endif /* MAMIRE_H_INCLUDE */

