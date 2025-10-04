FROM ubuntu:20.04
RUN apt-get update && \
      apt-get -y install sudo

RUN useradd -m docker && echo "docker:docker" | chpasswd && adduser docker sudo && echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

ARG BUILD_USER
USER ${BUILD_USER}
 
RUN sudo sed -i 's@//.*archive.ubuntu.com@//mirrors.ustc.edu.cn@g' /etc/apt/sources.list \
    && sudo sed -i 's/security.ubuntu.com/mirrors.ustc.edu.cn/g' /etc/apt/sources.list \
    && sudo apt update && sudo DEBIAN_FRONTEND=noninteractive apt-get install -y cmake ninja-build zip pkg-config gcc g++ curl git libtool bison
ENV SHELL=/bin/bash
RUN curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.40.3/install.sh | bash \
    && export NVM_DIR="$HOME/.nvm" \
    && [ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh" \
    && [ -s "$NVM_DIR/bash_completion" ] && \. "$NVM_DIR/bash_completion" \
    && nvm install --lts \
    && nvm use --lts \
    && corepack enable \
    && pnpm setup
   
RUN cd /tmp && curl -L -o autoconf.deb https://launchpad.net/ubuntu/+archive/primary/+files/autoconf_2.71-3_all.deb \
    && sudo dpkg -i autoconf.deb && rm autoconf.deb
    
CMD /bin/bash
