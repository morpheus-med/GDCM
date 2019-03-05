FROM alpine:3.8

RUN set -x && \
    apk add --no-cache --virtual \
    build-dependencies \
    cmake \
    make \
    g++ \
    zlib-dev \
    openssl-dev \
    patch \
    file \
    bash \
    vim \
    nodejs \
    gdb

WORKDIR /build/GDCM/

CMD [ "/bin/bash" ]
