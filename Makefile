# curl-config --libs
CFLAGS = -g -O0

nta: sip.c  
	gcc -o sip_nta sip_nta.c curl_util.c user_data.c -lcurl `pkg-config sofia-sip-ua --cflags --libs` -DCALLER $(CFLAGS)  

all: sip nta

sip: sip.c  
	gcc -o sip sip.c curl_util.c -lcurl `pkg-config sofia-sip-ua --cflags --libs` -DCALLER $(CFLAGS)  
clean:  
	rm sip 
