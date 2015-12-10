#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <sofia-sip/msg.h>
#include "user_data.h"


ul_t * ul_init(void){
	ul_t * ul_c = (struct ul_t *) malloc(sizeof(struct ul_t));
 	ul_c->ul_head = NULL;
	ul_c->count=0;
	return ul_c;
}

/* Store an INVITE message */
users_t * ul_store(ul_t * ul_c, msg_t const *msg, char *user){

	struct users_t * users;
	struct users_t * previous;
	struct users_t * saved_user;
	/* TODO: check if user as already a parked call ...*/


	/* Create an empty struct */
	saved_user = (struct users_t *) malloc(sizeof(struct users_t));
	if(saved_user==NULL)
		return NULL;
	
	/* Save user */
	saved_user->user = (char *) malloc(strlen(user)+1);
	if(saved_user->user==NULL)
                return NULL;
	strncpy(saved_user->user,user,strlen(user)+1);

	/* Save msg */
	saved_user->msg = msg_dup(msg); 
        if(saved_user->msg==NULL)
                return NULL;
	
	/* First element */
	if(ul_c->ul_head==NULL){
		ul_c->ul_head = saved_user;
		ul_c->ul_head->next=NULL;
		ul_c->ul_head->previous=NULL;
		saved_user = ul_c->ul_head;
		printf("User saved first element\n");
	}
	else{ /* Adding new element at the end */
		users = ul_c->ul_head;
		while(users->next != NULL){
			users=users->next;
		}
	
		/* Linking */
		users->next=saved_user;
		saved_user->previous=users;
		saved_user->next=NULL;
	}
	printf("User saved[%s][%p][%p][%p]\n",saved_user->user,saved_user->previous,saved_user,saved_user->next);
        ul_c->count++;
        printf("user stored count[%d]\n",ul_c->count);
	return ul_c->ul_head;
}

void ul_print(users_t * ul_head){
	struct users_t *users=NULL;
	if(ul_head==NULL)
                return;
	users = ul_head;
	int count=0;
        while(users != NULL){
		count++;
		//printf("user[%s]\n",users->user);
		users=users->next;
	}
	printf("user count[%d]\n",count);
	return;
}

/* Retreive the SIP message (parked call) associated with the user */
users_t * ul_get(users_t * ul_head,char *user){
	if(user==NULL)
		return NULL;
	if(ul_head==NULL)
		return NULL;

	struct users_t *users=NULL;
	users = ul_head;
	while(users != NULL){
 		if(strcmp(users->user,user) == 0){
			return users;
                }
		if(users->next==NULL)
			return NULL;
		users=users->next;
	}
	return NULL;
}


void ul_clear(ul_t *ul_c,users_t *ul){
	printf("ul_clear[%s][%p][%p]\n",ul->user,ul->previous,ul);

	/* something wrong? */
        if(ul==NULL)
                return;
        if(ul->user==NULL)
                return;

	/* is it the last element ? */
	if(ul == ul_c->ul_head && ul->next == NULL){
		ul_c->ul_head=NULL;
		printf("User cleared last element[%p][%p][%p]\n",ul,ul_c->ul_head,ul->next);
	} 
	/* check if we are deleting the head of the linked list */ 
	else if(ul == ul_c->ul_head){
		ul_c->ul_head = ul->next;
		ul_c->ul_head->previous = NULL; 
	}
	else{	
	/* deleting an element */
	  if(ul->previous != NULL)
		ul->previous->next=ul->next;
	  if(ul->next != NULL)
		ul->next->previous=ul->previous;
	}

	ul_c->count--;	
	printf("User cleared count[%d]\n",ul_c->count);
	free(ul->user);
	free(ul);
	
	return;
}


