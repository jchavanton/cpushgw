#include <curl/curl.h>
/*
 * http://curl.haxx.se/libcurl/c/libcurl-multi.html
 * http://curl.haxx.se/libcurl/c/multi-double.html
 * http://curl.piotrkosoft.net/libcurl/c/hiperfifo.html
 *
 * */

/* CURL MULTI START */

CURLM* multi_handle;
int still_running;

/* Initialize Curl Multi */
void curl_init(){
        multi_handle = curl_multi_init();
}

/* Perform  */
void curl_perfom(){
	curl_multi_perform(multi_handle, &still_running);
	
	//if(still_running > 0)
	//	printf("curl_multi running[%d]\n",still_running);

	/* check queue for completed requests */
	CURLMsg *curl_msg;
	int msgs_in_queue = 1;

	while(msgs_in_queue > 0){
		curl_msg = curl_multi_info_read(multi_handle, &msgs_in_queue); 
		if(msgs_in_queue > 0){
		  if(curl_msg->msg == 1){
        		printf("curl_multi msg_pending[%d] [%d]\n",msgs_in_queue,curl_msg->msg);
			curl_multi_remove_handle(multi_handle,curl_msg->easy_handle);
			curl_easy_cleanup(curl_msg->easy_handle);
		  }
		}
	} 
	/* empty queue */
	return;
}

/* function pointer - callback function that receives response in chunck */
static size_t curl_receive_chunk(void *ptr, size_t size, size_t nmemb, void *multi_handle){
	printf("HTTP response received:[%d][%s]\n",still_running,(char *)ptr);
        return nmemb*size;
}

/* send an HTTP request */
void curl_send(const char * uri){
        CURL* curl = NULL;
        curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_URL, uri);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_receive_chunk);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &multi_handle);
        curl_multi_add_handle(multi_handle, curl);
        curl_multi_perform(multi_handle, &still_running);
}
/* CURL END*/
