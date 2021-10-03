FROM ubuntu:latest
COPY httpd.conf /etc/httpd.conf
COPY httptest /var/www/html/httptest/
RUN apt update -y && \
    apt upgrade -y && \
    apt install -y g++ make libspdlog-dev
WORKDIR /app
COPY src src/
COPY Makefile .
RUN make
CMD compiled/httpd
