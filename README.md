# Fangtooth

Fangtooth это минимальный веб-сервер в котором каждую строчку кода страшно исполнять, а при каждом запуске надо надеяться на запуск без проблем.

## Тестовое окружение

**Запуск**
```bash
git clone https://github.com/17314642/web-server
cd web-server
docker build -t vvaria-httpd .
docker run -d -p 80:80 vvaria-httpd
./httptest.py
docker container stop {CONTAINER_NAME}
```

## Бенчмарк (i5-6400 3.3GHz)

**nginx (5.289 сек.) (ab -n 100000 -c 8 "http://127.0.0.1:8081/httptest/wikipedia_russia.html")**
```bash
This is ApacheBench, Version 2.3 <$Revision: 1843412 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 127.0.0.1 (be patient)
Completed 10000 requests
Completed 20000 requests
Completed 30000 requests
Completed 40000 requests
Completed 50000 requests
Completed 60000 requests
Completed 70000 requests
Completed 80000 requests
Completed 90000 requests
Completed 100000 requests
Finished 100000 requests


Server Software:        nginx/1.18.0
Server Hostname:        127.0.0.1
Server Port:            8081

Document Path:          /httptest/wikipedia_russia.html
Document Length:        162 bytes

Concurrency Level:      8
Time taken for tests:   5.289 seconds
Complete requests:      100000
Failed requests:        0
Non-2xx responses:      100000
Total transferred:      32100000 bytes
HTML transferred:       16200000 bytes
Requests per second:    18908.73 [#/sec] (mean)
Time per request:       0.423 [ms] (mean)
Time per request:       0.053 [ms] (mean, across all concurrent requests)
Transfer rate:          5927.44 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.0      0       1
Processing:     0    0   0.1      0      16
Waiting:        0    0   0.1      0      15
Total:          0    0   0.1      0      16

Percentage of the requests served within a certain time (ms)
  50%      0
  66%      0
  75%      0
  80%      0
  90%      0
  95%      0
  98%      1
  99%      1
 100%     16 (longest request)
```
**Fangtooth 2 потока (5.188 сек.) (ab -n 100000 -c 8 "http://127.0.0.1:8080/httptest/wikipedia_russia.html")**
```bash
This is ApacheBench, Version 2.3 <$Revision: 1843412 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 127.0.0.1 (be patient)
Completed 10000 requests
Completed 20000 requests
Completed 30000 requests
Completed 40000 requests
Completed 50000 requests
Completed 60000 requests
Completed 70000 requests
Completed 80000 requests
Completed 90000 requests
Completed 100000 requests
Finished 100000 requests


Server Software:        Fangtooth/1.0.0
Server Hostname:        127.0.0.1
Server Port:            8080

Document Path:          /httptest/wikipedia_russia.html
Document Length:        0 bytes

Concurrency Level:      8
Time taken for tests:   5.188 seconds
Complete requests:      100000
Failed requests:        0
Non-2xx responses:      100000
Total transferred:      10700000 bytes
HTML transferred:       0 bytes
Requests per second:    19276.82 [#/sec] (mean)
Time per request:       0.415 [ms] (mean)
Time per request:       0.052 [ms] (mean, across all concurrent requests)
Transfer rate:          2014.28 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.0      0       4
Processing:     0    0   0.2      0      11
Waiting:        0    0   0.2      0      11
Total:          0    0   0.2      0      11

Percentage of the requests served within a certain time (ms)
  50%      0
  66%      0
  75%      0
  80%      0
  90%      0
  95%      0
  98%      1
  99%      1
 100%     11 (longest request)
```

**Fangtooth 4 потока (5.403 сек.) (ab -n 100000 -c 8 "http://127.0.0.1:8080/httptest/wikipedia_russia.html")**
```bash
This is ApacheBench, Version 2.3 <$Revision: 1843412 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 127.0.0.1 (be patient)
Completed 10000 requests
Completed 20000 requests
Completed 30000 requests
Completed 40000 requests
Completed 50000 requests
Completed 60000 requests
Completed 70000 requests
Completed 80000 requests
Completed 90000 requests
Completed 100000 requests
Finished 100000 requests


Server Software:        Fangtooth/1.0.0
Server Hostname:        127.0.0.1
Server Port:            8080

Document Path:          /httptest/wikipedia_russia.html
Document Length:        0 bytes

Concurrency Level:      8
Time taken for tests:   5.403 seconds
Complete requests:      100000
Failed requests:        0
Non-2xx responses:      100000
Total transferred:      10700000 bytes
HTML transferred:       0 bytes
Requests per second:    18506.91 [#/sec] (mean)
Time per request:       0.432 [ms] (mean)
Time per request:       0.054 [ms] (mean, across all concurrent requests)
Transfer rate:          1933.83 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.1      0       7
Processing:     0    0   0.3      0      20
Waiting:        0    0   0.3      0      20
Total:          0    0   0.3      0      20

Percentage of the requests served within a certain time (ms)
  50%      0
  66%      0
  75%      0
  80%      0
  90%      0
  95%      1
  98%      1
  99%      1
 100%     20 (longest request)
```
