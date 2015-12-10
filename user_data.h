/*
 * Storage for active requests
 * */
typedef struct users_t{
  msg_t *msg;
  char *user;
  struct users_t *previous;
  struct users_t *next;
} users_t;


typedef struct ul_t{
  int count;
  struct users_t *ul_head;
} ul_t;



/* Initialize */
ul_t * ul_init(void);

/* Store an INVITE message */
users_t * ul_store(ul_t * ul_c,const msg_t *msg,char *user);

/* Retreive an INVITE message */
users_t * ul_get(users_t * ul_head,char *user);

/* Clear the ul entry */
void ul_clear(ul_t * ul_c,users_t * ul);
