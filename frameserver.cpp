#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libwebsockets.h>

static int callback_http(struct libwebsocket_context * ssl_cert_filepath, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
{
	return 0;
}

static int callback_frame(struct libwebsocket_context * self, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
{



    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED: // just log message that someone is connecting
            printf("connection established\n");
       //     var streamHeader = new Buffer(8);
		//streamHeader.write(STREAM_MAGIC_BYTES);
		//streamHeader.writeUInt16BE(width, 4);
		//streamHeader.writeUInt16BE(height, 6);
		//socket.send(streamHeader, {binary:true});
            break;
        case LWS_CALLBACK_SERVER_WRITEABLE: { // the funny part
            // create a buffer to hold our response
            // it has to have some pre and post padding. You don't need to care
            // what comes there, libwebsockets will do everything for you. For more info see
            // http://git.warmcat.com/cgi-bin/cgit/libwebsockets/tree/lib/libwebsockets.h#n597


  			FILE * pFile;
  			long lSize;
  			unsigned char * img;
  			size_t len;
			
  			pFile = fopen ( "/Users/picturae/Desktop/LIDO_VID_0115.mpg" , "r" );
  			if (pFile==NULL) {fputs ("File error",stderr); exit (1);}
			
  			// obtain file size:
  			fseek (pFile , 0 , SEEK_END);
  			lSize = ftell (pFile);
  			rewind (pFile);
			
  			// allocate memory to contain the whole file:
  			img = (unsigned char*) malloc (sizeof(unsigned char)*lSize);
			
			
  			if (img == NULL) {fputs ("Memory error",stderr); exit (2);}
			
  			// copy the file into the buffer:
  			len = fread (img,1,lSize,pFile);
  			if (len != lSize) {fputs ("Reading error",stderr); exit (3);}
			
  			/* the whole file is now loaded in the memory buffer. */
			
  			// terminate
  			fclose (pFile);
			
  			printf("img len: %lu\n", len );
			
  			unsigned char * buffer = (unsigned char*) malloc(LWS_SEND_BUFFER_PRE_PADDING + len + LWS_SEND_BUFFER_POST_PADDING );
  			size_t bufferLen = LWS_SEND_BUFFER_PRE_PADDING + len + LWS_SEND_BUFFER_POST_PADDING;
  			   
            printf("buffer len: %lu\n", bufferLen );
  			memcpy (buffer + LWS_SEND_BUFFER_PRE_PADDING, img, len );


            // log what we recieved and what we're going to send as a response.
            // that disco syntax `%.*s` is used to print just a part of our buffer
            // http://stackoverflow.com/questions/5189071/print-part-of-char-array
            printf("received data: %s\n", (char *) in );
            //printf("response data: %s\n", buffer );
            // send response
            // just notice that we have to tell where exactly our response starts. That's
            // why there's `buf[LWS_SEND_BUFFER_PRE_PADDING]` and how long it is.
            // we know that our response has the same length as request because
            // it's the same message in reverse order.
            len = strlen((char *)buffer);
            int ret = libwebsocket_write(wsi, &buffer[LWS_SEND_BUFFER_PRE_PADDING], bufferLen, LWS_WRITE_BINARY);
 			printf("libwebsocket_write: %d\n", ret );
 			ret = libwebsocket_callback_on_writable(self, wsi);
            //libwebsocket_write(wsi, &buf[LWS_SEND_BUFFER_PRE_PADDING], len, LWS_WRITE_TEXT);
            // release memory back into the wild
            free(img);
            free (buffer);
            usleep(40000);
            break;
        }
        default:
            break;
    }
    
    return 0;
}


static struct libwebsocket_protocols protocols[] = {
    /* first protocol must always be HTTP handler */
    {
        "http-only",   // name
        callback_http, // callback
        0              // per_session_data_size
    },
    {
        "frame-protocol", // protocol name - very important!
        callback_frame,   // callback
        0,                // we don't use any per session data
        1000000 		  //rx_buffer_size;
    },
    {
        NULL, NULL, 0   /* End of list */
    }
};

int main(int argc, char **argv) 
{
    // server url will be http://localhost:9000
    int port = 9000;
    struct libwebsocket_context *context;
    struct lws_context_creation_info context_info =
    {
        .port = port, .iface = NULL, .protocols = protocols, .extensions = NULL,
        .ssl_cert_filepath = NULL, .ssl_private_key_filepath = NULL, .ssl_ca_filepath = NULL,
        .gid = -1, .uid = -1, .options = 0, NULL, .ka_time = 0, .ka_probes = 0, .ka_interval = 0
    };
    // create libwebsocket context representing this server
    context = libwebsocket_create_context(&context_info);

    if (context == NULL) {
        fprintf(stderr, "libwebsocket init failed\n");
        return -1;
    }
    
    printf("starting server...\n");
                printf("magic %hu\n", static_cast<unsigned short>(720) );

    // infinite loop, to end this server send SIGTERM. (CTRL+C)
    while (1) {
        libwebsocket_service(context, 50);

        printf("ping...\n");
        libwebsocket_callback_on_writable_all_protocol(&protocols[1]);
        
        // libwebsocket_service will process all waiting events with their
        // callback functions and then wait 50 ms.
        // (this is a single threaded webserver and this will keep our server
        // from generating load while there are not requests to process)
    }
    
    libwebsocket_context_destroy(context);
    
    return 0;
}
