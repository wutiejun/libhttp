/* 
 * Copyright (c) 2016 Lammert Bies
 * Copyright (c) 2013-2016 the Civetweb developers
 * Copyright (c) 2004-2013 Sergey Lyubka
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * ============
 * Release: 1.8
 */

#include "httplib_main.h"
#include "httplib_memory.h"
#include "httplib_string.h"

/*
 * struct httplib_connection *httplib_connect_websocket_client();
 *
 * The function httplib_connect_websocket_client() connects as a client to a
 * websocket on another server. If this succeeds a connection pointer is
 * returned, otherwise NULL.
 */

struct httplib_connection *httplib_connect_websocket_client(const char *host,
                            int port,
                            int use_ssl,
                            char *error_buffer,
                            size_t error_buffer_size,
                            const char *path,
                            const char *origin,
                            httplib_websocket_data_handler data_func,
                            httplib_websocket_close_handler close_func,
                            void *user_data)
{
	struct httplib_connection *conn = NULL;

#if defined(USE_WEBSOCKET)
	struct httplib_context *newctx = NULL;
	struct websocket_client_thread_data *thread_data;
	static const char *magic = "x3JJHMbDL1EzLkh9GBhXDw==";
	static const char *handshake_req;

	if (origin != NULL) {
		handshake_req = "GET %s HTTP/1.1\r\n"
		                "Host: %s\r\n"
		                "Upgrade: websocket\r\n"
		                "Connection: Upgrade\r\n"
		                "Sec-WebSocket-Key: %s\r\n"
		                "Sec-WebSocket-Version: 13\r\n"
		                "Origin: %s\r\n"
		                "\r\n";
	} else {
		handshake_req = "GET %s HTTP/1.1\r\n"
		                "Host: %s\r\n"
		                "Upgrade: websocket\r\n"
		                "Connection: Upgrade\r\n"
		                "Sec-WebSocket-Key: %s\r\n"
		                "Sec-WebSocket-Version: 13\r\n"
		                "\r\n";
	}

	/* Establish the client connection and request upgrade */
	conn = httplib_download(host, port, use_ssl, error_buffer, error_buffer_size, handshake_req, path, host, magic, origin);

	/* Connection object will be null if something goes wrong */
	if (conn == NULL || (strcmp(conn->request_info.request_uri, "101") != 0)) {
		if (!*error_buffer) {
			/* if there is a connection, but it did not return 101,
			 * error_buffer is not yet set */
			XX_httplib_snprintf(conn, NULL, error_buffer, error_buffer_size, "Unexpected server reply");
		}
		if (conn != NULL) {
			XX_httplib_free(conn);
			conn = NULL;
		}
		return conn;
	}

	/* For client connections, httplib_context is fake. Since we need to set a
	 * callback function, we need to create a copy and modify it. */
	newctx = (struct httplib_context *)XX_httplib_malloc(sizeof(struct httplib_context));
	memcpy(newctx, conn->ctx, sizeof(struct httplib_context));
	newctx->user_data = user_data;
	newctx->context_type = 2;       /* client context type */
	newctx->cfg_worker_threads = 1; /* one worker thread will be created */
	newctx->workerthreadids = (pthread_t *)XX_httplib_calloc(newctx->cfg_worker_threads, sizeof(pthread_t));
	conn->ctx = newctx;
	thread_data = (struct websocket_client_thread_data *) XX_httplib_calloc(sizeof(struct websocket_client_thread_data), 1);
	thread_data->conn = conn;
	thread_data->data_handler = data_func;
	thread_data->close_handler = close_func;
	thread_data->callback_data = NULL;

	/* Start a thread to read the websocket client connection
	 * This thread will automatically stop when httplib_disconnect is
	 * called on the client connection */
	if (XX_httplib_start_thread_with_id( XX_httplib_websocket_client_thread, (void *)thread_data, newctx->workerthreadids) != 0) {

		XX_httplib_free((void *)thread_data);
		XX_httplib_free((void *)newctx->workerthreadids);
		XX_httplib_free((void *)newctx);
		XX_httplib_free((void *)conn);
		conn = NULL;
	}
#else
	/* Appease "unused parameter" warnings */
	(void)host;
	(void)port;
	(void)use_ssl;
	(void)error_buffer;
	(void)error_buffer_size;
	(void)path;
	(void)origin;
	(void)user_data;
	(void)data_func;
	(void)close_func;
#endif

	return conn;

}  /* httplib_connect_websocket_client */
