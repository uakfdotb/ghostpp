# -----------------------------------------------------------
# Mars Arena Bot — GHost++ native Linux build (Fedora 43)
# -----------------------------------------------------------
# Multi-stage build: compile Ghost++ and its dependencies
# from source, then copy to a minimal runtime image.
#
# Ghost++ — Warcraft III game hosting bot.
# Upstream: https://github.com/uakfdotb/ghostpp
# -----------------------------------------------------------

# ======================== Stage 1: Build ========================

FROM fedora:43 AS builder

RUN dnf install -y \
        cmake make gcc gcc-c++ \
        boost-devel \
        gmp-devel \
        zlib-devel \
        bzip2-devel \
        mariadb-connector-c-devel \
    && dnf clean all

WORKDIR /src
COPY . .

# Build bncsutil (Battle.net login utilities)
RUN cd bncsutil \
    && cmake -B build -DCMAKE_BUILD_TYPE=Release \
       -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_INSTALL_LIBDIR=lib \
    && cmake --build build -j$(nproc) \
    && cmake --install build

# Build StormLib (MPQ archive reader)
RUN cd StormLib \
    && cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON \
       -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_INSTALL_LIBDIR=lib \
    && cmake --build build -j$(nproc) \
    && cmake --install build

# Build CascLib (CASC file reader, WC3 1.29+)
RUN cd CascLib \
    && cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON \
       -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_INSTALL_LIBDIR=lib \
    && cmake --build build -j$(nproc) \
    && cmake --install build

# Refresh linker cache so ghost can find the libraries
RUN ldconfig

# Patch Makefile: Fedora uses MariaDB (libmariadb) instead of libmysqlclient;
# add /usr/local/lib to the library search path for bncsutil/storm/casc.
RUN sed -i 's/-lmysqlclient/-lmariadb/; s|-L\.|-L. -L/usr/local/lib|' ghost/Makefile

# Build GHost++
RUN cd ghost && make -j$(nproc)

# ======================== Stage 2: Runtime ========================

FROM fedora:43

RUN dnf install -y --setopt=install_weak_deps=False \
        boost-date-time \
        boost-system \
        boost-filesystem \
        boost-thread \
        gmp \
        zlib \
        bzip2-libs \
        mariadb-connector-c \
    && dnf clean all

# Copy ghost++ binary
COPY --from=builder /src/ghost/ghost++ /usr/local/bin/ghost++

# Copy custom shared libraries (bncsutil, StormLib, CascLib)
COPY --from=builder /usr/local/lib/libbncsutil* /usr/local/lib/
COPY --from=builder /usr/local/lib/libstorm* /usr/local/lib/
COPY --from=builder /usr/local/lib/libcasc* /usr/local/lib/
RUN ldconfig

WORKDIR /ghost

# Copy default configs and data files
COPY default.cfg language.cfg language_*.cfg ./
COPY ip-to-country.csv ipblacklist.txt ./
COPY gameloaded.txt gameover.txt motd.txt ./
COPY mapcfgs/ ./mapcfgs/
COPY maps/ ./maps/

# Runtime directories
RUN mkdir -p replays savegames

# Ghost++ UDP broadcast + game hosting
EXPOSE 6112/udp

ENTRYPOINT ["ghost++"]
