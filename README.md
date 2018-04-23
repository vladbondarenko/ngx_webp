## ngx_webp

Webp is new (and smaller) image format. This module will convert jpg/png image on fly and send webp response.

## Status

Under development. To be continued.

## Installation

    $ cd nginx-1.x.x
    $ ./configure --add-module=/path/to/ngx_webp
    $ make && make install

## Configuration directives

### `webp`

- **syntax**: `webp on|off`
- **default**: `off`
- **context**: `location`

Enables or disables module.

