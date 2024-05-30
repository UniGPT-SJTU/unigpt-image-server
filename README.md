# Tiny : 一个简单的静态资源服务器
## api 文档
### GET (Tiny)获取图片

GET /file/{filename}

### POST (Tiny)上传图片

POST /upload

> Body 请求参数

```yaml
file: file
```
> 返回示例

> 200 Response

```json
{
  "status": "string",
  "url": "string"
}
```

## 编译运行
```sh
make run
```

