/*
 * http://sofia-sip.sourceforge.net/refdocs/nta/
 * */

#include <stdio.h>  
#include <sofia-sip/nta.h>  
#include <sofia-sip/sip_protos.h>
#include "curl_util.h"
#include "user_data.h"

/* Parked calls user list DB and ulser list pointers */
ul_t * ul_c;         /* ul container/DB */
users_t * ul;        /* new: user and call */
users_t * parked_ul; /* retreived: user and parked call */
/* SIP server binding */
char url_socket[] = "sip:0.0.0.0:6060";
/* Libon platform URL */
char platform_url[] = "server.com";

/***********************************************************/

/*
 * Extract "From:", "To:" and "OnCaleeOS:" and send the PUSH request to the HTTP API
 * */
int extract_and_send(sip_t *sip){
	
	char url[2048];
	/*
 	*  Extract : OnCalleeOS from Sofia unknown header list
	**/
	sip_unknown_t *sip_header = sip->sip_unknown;
	while(sip_header != NULL){
		if(strcmp(sip_header->un_name,"OnCalleeOS") == 0){
                	printf("OnCaleeOS[%s]\n",sip_header->un_value);
                }
		printf("header name[%s]value[%s]\n",sip_header->un_name,sip_header->un_value);
		if(sip_header->un_next == NULL)
			break;
		sip_header = sip_header->un_next;		
	}

	/* TODO : prepare Adroid request */
	

	/* 
 	* Example of request extracted from the Perl version 
 	* my $request = 'https://'.$pf.'/notification/send_incoming_call_push.xml?app_id='.$app_id.'&send_from='.$from.'&send_to='.$to.'&api_sig='.$api_sig
 	*
 	* TODO : The API is now implementing hashed secret authentication
 	* 	  my $hash_input =  'app_id='.$app_id.'send_from='.$from.'send_to='.$to.$secret;
 	*         my $api_sig = md5_base64($hash_input);
 	**/
        if(strcmp(sip->sip_to->a_url->url_user,"27301") == 0){
         sprintf(url,"http://%s/onapi/notification/send_incoming_call_push.xml?app_id=0&send_from=%s&send_to=%s",platform_url,sip->sip_from->a_url->url_user,sip->sip_to->a_url->url_user);
        }
        else{
         sprintf(url,"http://127.0.0.1?send_from=%s&send_to=%s", sip->sip_from->a_url->url_user,sip->sip_to->a_url->url_user);
        }

	printf("HTTP: send[%s]\n",url);
	curl_send(url);
	return 1;
}

/* SIP sofia message Callback function  */
int message_callback(nta_agent_magic_t *context,
                          nta_agent_t *agent,
                          msg_t *msg,
                          sip_t *sip)
{
       	printf("message received call-id[%s] user[%s] message[%s]\n",
					sip->sip_call_id->i_id,
					sip->sip_request->rq_url->url_user,
					sip->sip_request->rq_method_name);
	
	switch(sip->sip_request->rq_method){
		case sip_method_invite:
			extract_and_send(sip);
			ul = ul_store(ul_c,msg,(char*)sip->sip_to->a_url->url_user);
			nta_msg_treply(agent,msg,180,"Ringing",TAG_END());
			printf("SIP[INVITE]: 180 Ringing (parking call)\n");
			break;
		case sip_method_cancel:
			parked_ul = ul_get(ul_c->ul_head,(char *)sip->sip_to->a_url->url_user);
			if(parked_ul != NULL)
				printf("SIP[CANCEL]: parked call [%s][%p]\n",parked_ul->user,parked_ul->msg);
				ul_clear(ul_c,parked_ul);
			nta_msg_treply(agent,msg,200,"OK",TAG_END());
			printf("SIP[CANCEL]: sending 200 OK\n");
			break;
		case sip_method_register:
			parked_ul = ul_get(ul_c->ul_head,(char *)sip->sip_to->a_url->url_user);
			if(parked_ul != NULL){
			    	printf("SIP[REGISTER]: parked call [%s][%p]\n",parked_ul->user,parked_ul->msg);
				// sip_contact_t * c
				char contact[2048];
				// Contact: Transfer <sip:65172@server.com>
				sprintf(contact,"Transfer <sip:%s@%s>",parked_ul->user,platform_url);
				nta_msg_treply(agent,parked_ul->msg,302,"Moved Temporarly",SIPTAG_CONTACT_STR(contact),TAG_END());
			 	// nta_msg_treply(agent,parked_ul->msg,302,"Moved Temporarly",TAG_END());
				printf("SIP[REGISTER]: sending 302 (redirect parked call)\n");
				ul_clear(ul_c,parked_ul);
			}
			nta_msg_treply(agent,msg,200,"OK",TAG_END());
			printf("SIP[REGISTER]: sending 200 OK\n");
			break;
		default:
			nta_msg_discard(agent,msg);
			break;
	}
       	return 1;
}

/*
 *  Main : syncronizing sofia_sip event and curl_multi event and storing user parked calls.
 * */

int  main (int argc, char *argv[])  
{
  su_root_t* root;
  nta_agent_t* agent;

  curl_init();
  ul_c=ul_init();
  ul=ul_c->ul_head;

  root = su_root_create(NULL);

  if (root) {
    /*
    *  Create Sofia stateless SIP agent
    * */
    agent = nta_agent_create(  
		root,  // root pointer to a su_root_t used for synchronization 
		URL_STRING_MAKE(url_socket),  // contact_url URL that agent uses to bind the server sockets 
		message_callback,  // callback pointer to callback function 
		NULL,  // magic pointer to user data                   
		TAG_END()); // tag,value,... tagged arguments

    if (agent) {
	printf("cPUSH-GW[listening]: %s\n",url_socket);
	while(1){
		su_root_sleep(root,100); // Sofia events processing
		curl_perfom();           // Curl events processing
	}
      nta_agent_destroy(agent);
    }

    su_root_destroy(root);
  }
}  

