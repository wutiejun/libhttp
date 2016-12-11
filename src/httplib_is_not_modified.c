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
 */



#include "libhttp-private.h"



/*
 * int XX_httplib_is_not_modified( const struct mg_connection *conn, const struct file *filep );
 *
 * The function XX_httplib_is_not_modified() returns true, if a resource has
 * not been modified sinze a given datetime and a 304 response should therefore
 * be sufficient.
 */

#if !defined(NO_CACHING)

int XX_httplib_is_not_modified( const struct mg_connection *conn, const struct file *filep ) {

	char etag[64];
	const char *ims = mg_get_header( conn, "If-Modified-Since" );
	const char *inm = mg_get_header( conn, "If-None-Match"     );

	XX_httplib_construct_etag( etag, sizeof(etag), filep );
	if ( filep == NULL ) return 0;
	return (inm != NULL && !mg_strcasecmp(etag, inm)) || (ims != NULL && (filep->last_modified <= XX_httplib_parse_date_string(ims)));

}  /* XX_httplib_is_not_modified */

#endif /* !NO_CACHING */