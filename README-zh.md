# nginx-transcode-module
Nginx module for media transcoding

接受http请求，根据配置进行文件名匹配，找到目标文件后，转码为相应格式返回。

```
location ~ (\.mp3)$ {
    nginx-transcode-module;
    transcode_root "/path/audios";
    transcode_output_format "mp3";
}
```

