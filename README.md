# Fangtooth

Fangtooth это минимальный веб-сервер в котором каждую строчку кода страшно исполнять, а при каждом запуске надо надеяться на запуск без проблем.

## Тестовое окружение

**Запуск**
```bash
git clone https://github.com/17314642/web-server
cd web-server
docker build -t vvaria-httpd .
docker run -p 80:80 vvaria-httpd
```
