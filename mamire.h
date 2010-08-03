#ifndef MAMIRE_H_INCLUDE
#define MAMIRE_H_INCLUDE

#include <stdbool.h>
#include <pthread.h>

#include "unstring.h"
#include "unarray.h"
#include "unmap.h"
#include "search.h"

#define MAMIRE_ITA_NAME_PATH				"/2ch/dat/ita.data"
#define MAMIRE_GET_BOARD_PATH				"/2ch/getboard.data"
#define MAMIRE_ROOT_PATH					"/2ch/dat"

#define MAMIRE_WRITE_LINE_MAX				(100)
#define MAMIRE_THREAD_WAIT_USEC				(1000 * 10)
#define MAMIRE_ACTIVE_THREAD_MAX			(4)
#define MAMIRE_BOARD_LIST_NUM				(1)

#define MAMIRE_SEARCH_NAME_YOUTUBE			"youtube"
#define MAMIRE_SEARCH_NAME_NICOVIDEO		"nicovideo"
#define MAMIRE_SEARCH_NAME_NICOVIDEO_LIVE	"nicovideo_live"
#define MAMIRE_SEARCH_NAME_2CH				"2ch"

#define MAMIRE_PATTERN_YOUTUBE				"youtube\\.com/watch\\?v=([\\w\\d\\-_]{11})"
#define MAMIRE_PATTERN_NICOVIDEO			"nicovideo\\.jp/watch/((?!lv)\\w+\\d+)"
#define MAMIRE_PATTERN_NICOVIDEO_LIVE		"nicovideo\\.jp/watch/(lv\\d+)"
#define MAMIRE_PATTERN_2CH					"ttp://(\\w+)\\.(2ch\\.net|bbspink\\.com)/test/read\\.(cgi/|html#)(\\w+)/(\\d{9,10})"

void global_init(void);
void *threads_main(void *p);
bool search_copy(unmap_t *map, search_t *s, pthread_mutex_t *mutex);
void write_file(unmap_t *map, const char *str);
unarray_t *qsort_exec(unmap_t *map);
int compare_match(const void *a, const void *b);
bool thread_concat(unarray_t *a1, unarray_t *a2);
unmap_t *get_board_data( void );
unarray_t *get_board_list( void );
unarray_t *get_thread_list(path_t *board);
void path_free(void *p);

void *mamire_malloc(size_t size);
void *mamire_realloc(void *p, size_t size, size_t len);

#endif /* MAMIRE_H_INCLUDE */

