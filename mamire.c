#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <unmap.h>

#include "mamire.h"
#include "search.h"

static unmap_t *g_map_youtube;
static unmap_t *g_map_nicovideo;
static unmap_t *g_map_nicovideo_live;
static unmap_t *g_map_2ch;
static pthread_mutex_t g_mutex_youtube = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_mutex_nicovideo = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_mutex_nicovideo_live = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_mutex_2ch = PTHREAD_MUTEX_INITIALIZER;

static int g_thread_count;

int main()
{
	size_t size = 0;
	size_t i = 0;
	pthread_t pt;
	path_t *path = 0;
	unarray_t *ita_list = get_board_list();
	global_init();
	g_thread_count = 0;
	size = ita_list->length;
	for(i = 0; i < size; i++){
		while(g_thread_count >= MAMIRE_ACTIVE_THREAD_MAX){
			usleep(MAMIRE_THREAD_WAIT_USEC);
		}
		path = unarray_at(ita_list, i);
		pthread_create(&pt, NULL, threads_main, path);
		g_thread_count++;
	}
	while(g_thread_count){
		usleep(MAMIRE_THREAD_WAIT_USEC);
	}

	unarray_free(ita_list, path_free);

	write_file(g_map_youtube, MAMIRE_SEARCH_NAME_YOUTUBE);
	write_file(g_map_nicovideo, MAMIRE_SEARCH_NAME_NICOVIDEO);
	write_file(g_map_nicovideo_live, MAMIRE_SEARCH_NAME_NICOVIDEO_LIVE);
	write_file(g_map_2ch, MAMIRE_SEARCH_NAME_2CH);

	unmap_free(g_map_youtube, match_free);
	unmap_free(g_map_nicovideo, match_free);
	unmap_free(g_map_nicovideo_live, match_free);
	unmap_free(g_map_2ch, match_free);
	return 0;
}

void global_init(void)
{
	g_map_youtube = unmap_init(32);
	g_map_nicovideo = unmap_init(32);
	g_map_nicovideo_live = unmap_init(32);
	g_map_2ch = unmap_init(32);
}

void *threads_main(void *p)
{
	size_t i = 0;
	path_t *board_path = p;
	unarray_t *thread_list;
	unstr_t *filename;
	unstr_t *data;
	unstr_t *sure_index;
	thread_t *thread;
	search_t *map_youtube;
	search_t *map_nicovideo;
	search_t *map_nicovideo_live;
	search_t *map_2ch;

	pthread_detach(pthread_self());

	thread_list = get_thread_list(board_path);
	if(!thread_list){
		g_thread_count--;
		return NULL;
	}
	filename = unstr_init_memory(64);
	sure_index = unstr_init_memory(16);
	map_youtube = search_new(MAMIRE_PATTERN_YOUTUBE, MAMIRE_SEARCH_NAME_YOUTUBE);
	map_nicovideo = search_new(MAMIRE_PATTERN_NICOVIDEO, MAMIRE_SEARCH_NAME_NICOVIDEO);
	map_nicovideo_live = search_new(MAMIRE_PATTERN_NICOVIDEO_LIVE, MAMIRE_SEARCH_NAME_NICOVIDEO_LIVE);
	map_2ch = search_new(MAMIRE_PATTERN_2CH, MAMIRE_SEARCH_NAME_2CH);
	for(i = 0; i < thread_list->length; i++){
		thread = unarray_at(thread_list, i);
		unstr_substr(sure_index, thread->path.sure, 4);
		unstr_sprintf(
			filename,
			MAMIRE_ROOT_PATH "/%$/%$/%$.dat",
			thread->path.ita,
			sure_index,
			thread->path.sure
		);
		data = unstr_file_get_contents(filename);
		if(unstr_isset(data)){
			search_text(map_youtube, data, 1);
			search_text(map_nicovideo, data, 1);
			search_text(map_nicovideo_live, data, 1);
			search_text_2ch(map_2ch, data);
		}
		unstr_free(data);
	}

	search_copy(g_map_youtube, map_youtube, &g_mutex_youtube);
	search_copy(g_map_nicovideo, map_nicovideo, &g_mutex_nicovideo);
	search_copy(g_map_nicovideo_live, map_nicovideo_live, &g_mutex_nicovideo_live);
	search_copy(g_map_2ch, map_2ch, &g_mutex_2ch);

	search_free(map_youtube);
	search_free(map_nicovideo);
	search_free(map_nicovideo_live);
	search_free(map_2ch);

	unarray_free(thread_list, thread_free);

	unstr_delete(3, filename, data, sure_index);
	g_thread_count--;
	return NULL;
}

bool search_copy(unmap_t *map, search_t *s, pthread_mutex_t *mutex)
{
	size_t i = 0;
	size_t size = 0;
	match_t *m = 0;
	match_t *match = 0;

	pthread_mutex_lock(mutex);

	if(!map || !s){
		pthread_mutex_unlock(mutex);
		return false;
	}
	size = unmap_size(s->list);
	for(i = 0; i < size; i++){
		match = unmap_at(s->list, i);
		if(match != NULL){
			m = unmap_get(map, match->match->data, match->match->length);
			if(m == NULL){
				m = match_new(match->match);
				m->count = match->count;
				unmap_set(map, match->match->data, match->match->length, m);
			} else {
				m->count += match->count;
			}
		}
	}

	pthread_mutex_unlock(mutex);
	return true;
}

void write_file(unmap_t *map, const char *str)
{
	unstr_t *tmp = unstr_init_memory(128);
	unstr_t *txt = unstr_init_memory(4096 * 10);
	unstr_t *filename = unstr_sprintf(NULL, "/2ch/dat/%s.tsv", str);
	unarray_t *list = qsort_exec(map);
	match_t *match = 0;
	size_t size = unarray_size(list);
	size_t i = 0;
	if(size > MAMIRE_WRITE_LINE_MAX){
		size = MAMIRE_WRITE_LINE_MAX;
	}
	for(i = 0; i < size; i++){
		match = unarray_at(list, i);
		unstr_sprintf(tmp, "%$\t%d\n", match->match, match->count);
		unstr_strcat(txt, tmp);
	}
	unstr_file_put_contents(filename, txt, "w");
	unstr_delete(3, tmp, txt, filename);
}

unarray_t *qsort_exec(unmap_t *map)
{
	size_t i = 0;
	size_t mapsize = unmap_size(map);
	unarray_t *array = unarray_init(mapsize + 1);
	for(i = 0; i < mapsize; i++){
		unarray_push(array, unmap_at(map, i));
	}
	qsort(array->data, unarray_size(array), sizeof(void *), compare_match);
	return array;
}

int compare_match(const void *a, const void *b)
{
	match_t **aa = (match_t **)a;
	match_t **bb = (match_t **)b;
	return (*bb)->count - (*aa)->count;
}

bool thread_concat(unarray_t *a1, unarray_t *a2)
{
	size_t i = 0;
	thread_t *t = 0;
	if(!a1 || !a2){
		return false;
	}
	for(i = 0; i < a2->length; i++){
		t = unarray_at(a2, i);
		if(t != NULL){
			unarray_push(a1, thread_new(&(t->path), t->title));
		}
	}
	return true;
}

unmap_t *get_board_data()
{
	size_t index = 0;
	size_t length = 0;
	unmap_t *map = unmap_init(16);
	unstr_t *line = 0;
	unstr_t *filename = unstr_init(MAMIRE_GET_BOARD_PATH);
	unstr_t *data = unstr_file_get_contents(filename);
	line = unstr_strtok(data, "\n", &index);
	while(line != NULL){
		length = unstr_strlen(line);
		if(length != 0){
			unstr_free_func(unmap_get(map, line->data, length));
			unmap_set(map, line->data, length, unstr_copy(line));
		}
		unstr_free(line);
		line = unstr_strtok(data, "\n", &index);
	}
	unstr_delete(3, line, data, filename);
	return map;
}

unarray_t *get_board_list()
{
	size_t index = 0;
	path_t *path;
	unarray_t *arr = unarray_init(1024);
	unmap_t *map = get_board_data();
	unstr_t *line = 0;
	unstr_t *p1 = unstr_init_memory(16);
	unstr_t *p2 = unstr_init_memory(16);
	unstr_t *p3 = unstr_init_memory(16);
	unstr_t *filename = unstr_init(MAMIRE_ITA_NAME_PATH);
	unstr_t *data = unstr_file_get_contents(filename);
	line = unstr_strtok(data, "\n", &index);
	while(line != NULL){
		if(unstr_sscanf(line, "$/$<>$", p1, p2, p3) == 3){
//			if(unmap_find(map, p2->data, unstr_strlen(p2)) != NULL){
				path = mamire_malloc(sizeof(path_t));
				path->ita = unstr_copy(p2);
				unarray_push(arr, path);
				printf("%s\n", p2->data);
//			}
		}
		unstr_free(line);
		line = unstr_strtok(data, "\n", &index);
	}
	unstr_delete(6, line, data, filename, p1, p2, p3);
	unmap_free(map, (void (*)(void *))unstr_free_func);
	return arr;
}

unarray_t *get_thread_list(path_t *board)
{
	size_t index = 0;
	path_t path;
	thread_t *thread;
	unarray_t *list = 0;
	unstr_t *filename = 0;
	unstr_t *data = 0;
	unstr_t *line = 0;
	unstr_t *p1;
	unstr_t *p2;
	if(	board == NULL || unstr_empty(board->ita)){
		return NULL;
	}
	filename = unstr_sprintf(NULL, MAMIRE_ROOT_PATH "/%$/subject.txt", board->ita);
	printf("%s\n", filename->data);
	data = unstr_file_get_contents(filename);
	if(unstr_empty(data)){
		return NULL;
	}
	p1 = unstr_init_memory(16);
	p2 = unstr_init_memory(16);
	path.ita = unstr_init_memory(16);
	path.sure = unstr_init_memory(16);
	list = unarray_init(512);
	line = unstr_strtok(data, "\n", &index);
	while(line != NULL){
		if(unstr_sscanf(line, "$.dat<>$", p1, p2) == 2){
			unstr_strcpy(path.ita, board->ita);
			unstr_strcpy(path.sure, p1);
			thread = thread_new(&path, p2);
			unarray_push(list, thread);
		}
		unstr_free(line);
		line = unstr_strtok(data, "\n", &index);
	}
	unstr_delete(7, filename, data, line, p1, p2, path.ita, path.sure);
	return list;
}

void path_free(void *p)
{
	path_t *path = p;
	unstr_delete(2, path->ita, path->sure);
	free(path);
}


void *mamire_malloc(size_t size)
{
	void *p = malloc(size);
	if(p == NULL){
		perror("mamire_malloc:");
	} else {
		memset(p, 0, size);
	}
	return p;
}

void *mamire_realloc(void *p, size_t size, size_t len)
{
	p = realloc(p, size);
	if(p == NULL){
		/* 領域の確保に失敗した場合、perrorを呼び出す。 */
		perror("mamire_realloc:");
	} else {
		len++;
		if(size > len){
			memset(((char *)p) + len, 0, size - len);
		}
	}
	return p;
}

