# Fangtooth

Fangtooth это минимальный веб-сервер с поддержкой GET, HEAD запросов.

## Тестовое окружение

**Запуск**
```bash
git clone https://github.com/17314642/web-server
cd web-server
docker build -t httpd .
docker run -it -p 80:80 httpd
```

**Остановка**
```bash
docker container stop httpd
```

## Бенчмарк между двумя ПК в 1 GBit LAN

**nginx (768 workers, gzip on) (14.968 сек.) (ab -n 1000 -c 4 "http://192.168.50.2:80/wikipedia_russia.html")**
```bash
This is ApacheBench, Version 2.3 <$Revision: 1879490 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 192.168.50.2 (be patient)


Server Software:        nginx/1.18.0
Server Hostname:        192.168.50.2
Server Port:            80

Document Path:          /wikipedia_russia.html
Document Length:        954824 bytes

Concurrency Level:      4
Time taken for tests:   14.968 seconds
Complete requests:      1000
Failed requests:        0
Total transferred:      955071000 bytes
HTML transferred:       954824000 bytes
Requests per second:    66.81 [#/sec] (mean)
Time per request:       59.872 [ms] (mean)
Time per request:       14.968 [ms] (mean, across all concurrent requests)
Transfer rate:          62312.10 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        1    5   2.1      4      15
Processing:    31   55  16.6     51     142
Waiting:        2    5   2.1      5      13
Total:         33   60  17.1     56     149

Percentage of the requests served within a certain time (ms)
  50%     56
  66%     58
  75%     60
  80%     63
  90%     81
  95%     99
  98%    119
  99%    132
 100%    149 (longest request)
```
**Fangtooth 2 потока (15.043 сек.) (ab -n 1000 -c 4 "http://192.168.50.2:8080/wikipedia_russia.html")**
```bash
This is ApacheBench, Version 2.3 <$Revision: 1879490 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 192.168.50.2 (be patient)


Server Software:        Fangtooth/2.0.0
Server Hostname:        192.168.50.2
Server Port:            8080

Document Path:          /wikipedia_russia.html
Document Length:        954824 bytes

Concurrency Level:      4
Time taken for tests:   15.043 seconds
Complete requests:      1000
Failed requests:        0
Total transferred:      954973000 bytes
HTML transferred:       954824000 bytes
Requests per second:    66.48 [#/sec] (mean)
Time per request:       60.171 [ms] (mean)
Time per request:       15.043 [ms] (mean, across all concurrent requests)
Transfer rate:          61996.34 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        2    5   1.7      4      24
Processing:    34   55   9.4     54     103
Waiting:        2   11   6.2      9      48
Total:         38   60   9.3     58     108

Percentage of the requests served within a certain time (ms)
  50%     58
  66%     60
  75%     62
  80%     64
  90%     72
  95%     79
  98%     89
  99%     96
 100%    108 (longest request)
```
