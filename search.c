#include <stdio.h>
#include <pthread.h>
#include <oniguruma.h>

#include "search.h"
#include "mamire.h"
#include "unstring.h"
#include "unmap.h"
#include "unarray.h"

static pthread_mutex_t g_onig_mutex = PTHREAD_MUTEX_INITIALIZER;

search_t *search_new(unstr_t *pattern, unstr_t *name)
{
	regex_t *reg;
	OnigErrorInfo einfo;
	search_t *search = 0;
	int r;
	
	pthread_mutex_lock(&g_onig_mutex);
	r = onig_new(&reg, (UChar *)pattern->data, (UChar *)(pattern->data + unstr_strlen(pattern)),
		ONIG_OPTION_NONE, ONIG_ENCODING_SJIS, ONIG_SYNTAX_PERL, &einfo);
	pthread_mutex_unlock(&g_onig_mutex);

	if(r != ONIG_NORMAL){
		UChar s[ONIG_MAX_ERROR_MESSAGE_LEN];
		onig_error_code_to_str(s, r, &einfo);
		printf("ERROR: %s\n", s);
		return NULL;
	}
	search = mamire_malloc(sizeof(search_t));
	search->reg = reg;
	search->list = unmap_init(16, 1024, 512);
	search->name = unstr_copy(name);
	return search;
}

bool search_text(search_t *search, unstr_t *data)
{
	OnigRegion *region = onig_region_new();
	unstr_t *str_match;
	UChar *start;
	UChar *end;
	UChar *range;
	int ret;

	end = (UChar *)(data->data + unstr_strlen(data));
	start = (UChar *)(data->data);
	range = end;

	while(true){
		ret = onig_search(search->reg, (UChar *)data->data, end, start, range, region, ONIG_OPTION_NONE);

		if(ret >= 0){
			str_match = unstr_substr_char(data->data + region->beg[0], region->end[0] - region->beg[0]);
			search_match_text(search, str_match);
			//printf("%s\n", str_match->data);
			unstr_free(str_match);
			start = (UChar *)(data->data + region->end[0]);
		} else if(ret == ONIG_MISMATCH){
			break;
		} else { /* error */
			UChar s[ONIG_MAX_ERROR_MESSAGE_LEN];
			onig_error_code_to_str(s, ret);
			printf("ERROR: %s\n", s);
			return false;
		}
		//onig_region_free(region, 1);
		onig_region_clear(region);
	}
	onig_region_free(region, 1);
	return true;
}

bool search_match_text(search_t *search, unstr_t *match)
{
	match_t *m = unmap_get(search->list, match->data, unstr_strlen(match));
	if(m == NULL){
		m = match_new(match);
		unmap_set(search->list, match->data, unstr_strlen(match), m, NULL);
	} else {
		m->count++;
	}
	return true;
}

void search_free(void *p)
{
	search_t *s = p;
	pthread_mutex_lock(&g_onig_mutex);
	onig_free(s->reg);
	pthread_mutex_unlock(&g_onig_mutex);
	unmap_free(s->list, match_free);
	unstr_free(s->name);
	free(s);
}

thread_t *thread_new(path_t *path, unstr_t *title)
{
	thread_t *p = mamire_malloc(sizeof(thread_t));
	p->path.saba = unstr_copy(path->saba);
	p->path.ita = unstr_copy(path->ita);
	p->path.sure = unstr_copy(path->sure);
	p->title = unstr_copy(title);
	return p;
}

void thread_free(void *p)
{
	thread_t *t = p;
	unstr_delete(4, t->path.saba, t->path.ita, t->path.sure, t->title);
	free(t);
}

match_t *match_new(unstr_t *match)
{
	match_t *p = mamire_malloc(sizeof(match_t));
	p->match = unstr_copy(match);
	p->count = 1;
	return p;
}

void match_free(void *p)
{
	match_t *m = p;
	unstr_free(m->match);
	free(m);
}

