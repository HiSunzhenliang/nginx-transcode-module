# nginx-transcode-module

Nginx module for media transcoding

## Description

This project is a nginx third-party module.

Accept HTTP requests, find the files according to the configuration, transcode it and return it in the required format.

## Install

1. Clone project

`git clone https://github.com/HiSunzhenliang/nginx-transcode-module.git`

2. Requirements
    - Nginx
    - libsox

3. Compile Nginx with nginx-transcode-module

`./configure --add-dynamic-module=/.../nginx-transcode-module`

`make && make install`

## Usage

In nginx config file:

```
location ~ / {
    nginx-transcode-module;
    transcode_root "/path/audios";
    transcode_output_format "mp3";
}
```
```
$ ls /path/audios
a.wav b.wav c.wav ...
```

Send request like:
```
curl -v http://127.0.0.1:8000/a.wav -o a.mp3`
```
Get `a.mp3` transcoded from `a.wav`.

If `transcode_output_format` is not specified, output format will be set to the extension of the uri.
```
location ~ / {
    nginx-transcode-module;
    transcode_root "/path/audios";
    # transcode_output_format "mp3";
}
```
Send request like:
```
curl -v http://127.0.0.1:8000/b.mp3 -o b.mp3`
```
Get `b.mp3` transcoded from `b.wav`.

## Copyright & License
```
MIT License

Copyright (c) 2021 sunzhenliang

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
