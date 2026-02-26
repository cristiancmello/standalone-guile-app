FROM mcr.microsoft.com/devcontainers/base:ubuntu-24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake pkg-config git \
    libgtk-4-dev gtk-4-examples \
    dbus-x11 \
    libgl1-mesa-dri \
    libglx-mesa0 \
    highlight \
    guile-3.0 \
    guile-3.0-dev \
    guile-3.0-libs \
    libgc-dev \
    libffi-dev \
    libunistring-dev \
    libgmp-dev \
    libgirepository1.0-dev \
    gir1.2-gtk-4.0 \
    libtool \
    libtool-bin \
    autoconf \
    automake \
    autotools-dev \
    texinfo \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

RUN echo "/usr/lib/wsl/lib" > /etc/ld.so.conf.d/nvidia.conf && ldconfig