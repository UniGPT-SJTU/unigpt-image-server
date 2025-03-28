# Tiny : A Simple Image Server
## Brief Introduction
This is a really simple **image server**, written in C, only supported running on **GNU/Linux**. It's single-threaded and has bugs. :(

The code is really simple, only including some basic Linux network api. Some may find it helpful. :)
## API Docs
### get image

GET /file/{filename}

### POST upload image

POST /upload

> Body args

```yaml
file: file_path
```
> Return example

> 200 Response

```json
{
  "status": "string",
  "url": "string"
}
```

## Compile and Run
You can customize some config in `config.h`, especially macro `SERVER_IP`. 
Compile and run program: 
```sh
make run
```

In default case, the image server is running on port `10339`. 