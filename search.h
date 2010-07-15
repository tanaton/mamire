#ifndef SEARCH_H_INCLUDE
#define SEARCH_H_INCLUDE

#include <stdio.h>
#include <stdbool.h>
#include <oniguruma.h>

#include "unstring.h"
#include "unarray.h"
#include "unmap.h"

typedef struct path_st {
	unstr_t	*saba;
	unstr_t	*ita;
	unstr_t	*sure;
} path_t;

typedef struct search_st {
	regex_t			*reg;
	unmap_t			*list;
	unstr_t			*name;
} search_t;

typedef struct match_st {
	unstr_t			*match;
	size_t			count;
} match_t;

typedef struct thread_st {
	path_t			path;
	unstr_t			*title;
} thread_t;

search_t *search_new(unstr_t *pattern, unstr_t *name);
bool search_text(search_t *search, unstr_t *data);
bool search_match_text(search_t *search, unstr_t *match);
void search_free(void *p);
thread_t *thread_new(path_t *path, unstr_t *title);
void thread_free(void *p);
match_t *match_new(unstr_t *match);
void match_free(void *p);

#endif

