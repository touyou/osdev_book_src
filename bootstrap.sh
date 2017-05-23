#!/bin/bash

# 多重に実行されないようにする
sudo sh -c 'test -f /etc/bootstrapped && exit'

# aptの処理の高速化用
sudo sed -i'~' -E "s@http://(..\.)?(archive|security)\.ubuntu\.com/ubuntu@http://linux.yz.yamagata-u.ac.jp/pub/linux/ubuntu-archive/@g" /etc/apt/sources.list

sudo DEBIAN_FRONTEND=noninteractive apt-get -qq update
sudo DEBIAN_FRONTEND=noninteractive apt-get -qq install -y gdebi git g++ make parted language-pack-ja-base language-pack-ja kpartx gdb gdisk dosfstools
sudo DEBIAN_FRONTEND=noninteractive apt-get -qq -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install -y grub-efi
sudo update-locale LANG=ja_JP.UTF-8 LANGUAGE="ja_JP:ja"
# この辺りは筆者の趣味
sudo DEBIAN_FRONTEND=noninteractive apt-get -qq install -y emacs silversearcher-ag

# install qemu                                                                                                                                                                                             
wget -q "http://drive.google.com/uc?export=download&id=0BzboiC2yUBwnZkU3QzMzMUc3cW8" -O qemu_2.9.0-1_amd64.deb
sudo gdebi --n -q qemu_2.9.0-1_amd64.deb
# 本来の処理
# sudo DEBIAN_FRONTEND=noninteractive apt-get -qq install -y libglib2.0-dev libfdt-dev libpixman-1-dev zlib1g-dev
# wget http://download.qemu-project.org/qemu-2.9.0.tar.xz
# tar xvJf qemu-2.9.0.tar.xz
# mkdir build-qemu
# cd build-qemu
# ../qemu-2.9.0/configure --target-list=x86_64-softmmu --disable-kvm --enable-debug
# make -j2
# sudo make install
# cd ..

# install OVMF
wget -q "http://drive.google.com/uc?export=download&id=0BzboiC2yUBwnYXhxNjRuNENCUVE" -O edk2-UDK2017.tar.gz
tar zxvf edk2-UDK2017.tar.gz
# 本来の処理
# sudo DEBIAN_FRONTEND=noninteractive apt-get -qq install -y build-essential uuid-dev nasm iasl
# git clone -b UDK2017 http://github.com/tianocore/edk2 --depth=1
# cd edk2
# make -C BaseTools
# . ./edksetup.sh
# build -a X64 -t GCC48 -p OvmfPkg/OvmfPkgX64.dsc
# mkdir ~/edk2-UDK2017
# cp Build/OvmfX64/DEBUG_GCC48/FV/*.fd ~/edk2-UDK2017
# cd ..

sudo mkdir "/mnt/baremetal"
sudo mkdir "/mnt/efi"

sudo sh -c 'date > /etc/bootstrapped'

echo "setup done!"
