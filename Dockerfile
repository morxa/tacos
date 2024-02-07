FROM fedora:39

RUN dnf install -y --nodocs 'dnf-command(copr)' \
    && dnf copr enable -y thofmann/eclipse-clp-6 \
    && dnf install -y --nodocs \
        @development-tools \
        boost-devel \
        cmake \
        doxygen \
        eclipse-clp-devel \
        fmt-devel \
        gcc-c++ \
        git \
        graphviz-devel \
        pkgconfig \
        protobuf-compiler \
        protobuf-devel \
        range-v3-devel \
        spdlog-devel \
        tinyxml2-devel \
    && dnf clean all
COPY . /tacos
WORKDIR /tacos
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release -DTACOS_BUILD_BENCHMARKS=ON -DTACOS_GOCOS=ON && cmake --build build -j`nproc`
