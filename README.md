## ngx_webp

Webp is new (and smaller) image format. This module will convert jpg/png image on fly and send webp response.

## Status

Under development. To be continued.

## Installation

    First you need to install cwebp tool as per https://developers.google.com/speed/webp/docs/cwebp

    $ cd nginx-1.x.x
    $ ./configure --add-module=/path/to/ngx_webp
    $ make && make install

    Then you need to copy "webp" script to /etc/nginx/ dir. So /etc/nginx/webp will be triggered for each appropriate Accept header and Content-Type.

## Configuration directives

### `webp`

- **syntax**: `webp`
- **context**: `location`

Enables or disables module.

### Example

location ~ "\.jpg" {
webp;
}

$ curl -SLIXGET -H "accept:image/webp" http://127.0.0.1/1.jpg

HTTP/1.1 200 OK

Server: nginx/1.13.12

Date: Wed, 25 Apr 2018 10:16:45 GMT

Content-Length: 223980

Last-Modified: Wed, 25 Apr 2018 10:16:45 GMT

Connection: keep-alive

Content-Type: image/webp



$ curl -SLIXGET -H "accept:image/*" http://127.0.0.1/1.jpg

HTTP/1.1 200 OK

Server: nginx/1.13.12

Date: Wed, 25 Apr 2018 10:17:53 GMT

Content-Length: 325991

Last-Modified: Wed, 18 Apr 2018 19:55:14 GMT

Connection: keep-alive

Content-Type: image/jpeg

### Notice
As webp convertion takes some CPU usage I recommend to use some kind of caching of nginx responses, like Varnish.
