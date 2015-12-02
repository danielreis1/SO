/*
 * list.c - implementation of the integer list functions 
 */


#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "list.h"


list_t* lst_new()
{
   list_t *list;
   list = (list_t*) malloc(sizeof(list_t));
   list->first = NULL;
   return list;
}


void lst_destroy(list_t *list)
{
	struct lst_iitem *item, *nextitem;

	item = list->first;
	while (item!=NULL){
		nextitem = item->next;
		free(item);
		item = nextitem;
	}
	free(list);
}


void insert_new_process(list_t *list, int pid, time_t starttime)
{
	lst_iitem_t *item;

	item = (lst_iitem_t *) malloc (sizeof(lst_iitem_t));
	item->pid = pid;
	item->starttime = starttime;
	item->endtime = 0;
	item->next = list->first;
	list->first = item;
}


int update_terminated_process(list_t *list, int pid, time_t endtime, FILE *fp)
{
   lst_iitem_t *item;
   item = list->first;
   while(item!=NULL){
	   if(item->pid == pid){
		   item->endtime = endtime;
		   int performance = difftime(item->endtime, item->starttime);
		   item->exectime = performance;
		   fprintf(fp,"pid: %d execution time: %d s\n", 
		   item->pid, item->exectime);
		   return performance;
	   }
	   item = item->next; 
   }
}


void lst_print(list_t *list)
{
	lst_iitem_t *item;
	item = list->first;
	printf("--- Beginning of the list ---\n");
	while (item!=NULL){
		printf("pid: %d execution time: %d s\n", 
		item->pid, item->exectime);
		item = item->next;
	}
	printf("--- End of the list ---\n");
}

