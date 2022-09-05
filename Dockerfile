FROM ubuntu:20.04
COPY httpd.conf /etc/httpd.conf
COPY Wikipedia.html /var/www/html/index.html
RUN apt update -y && \
    apt upgrade -y && \
    apt install -y g++ make libspdlog-dev
WORKDIR /app
COPY src src/
COPY Makefile .
RUN make
ENTRYPOINT ["compiled/httpd"]
