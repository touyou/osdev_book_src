#!/bin/sh

# エラー回避用
export DEBIAN_FRONTEND=noninteractive

# 多重に実行されないようにする
sudo sh -c 'test -f /etc/bootstrapped && exit'

# aptで入れられるものは基本的にaptで
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
sudo apt update
sudo apt install -y git g++-6 make parted language-pack-ja-base language-pack-ja kpartx gdb
sudo update-locale LANG=ja_JP.UTF-8 LANGUAGE="ja_JP:ja"
# この辺りは筆者の趣味
sudo apt install -y emacs silversearcher-ag

# install qemu
sudo apt-get install -y libglib2.0-dev libfdt-dev libpixman-1-dev zlib1g-dev
wget http://download.qemu-project.org/qemu-2.9.0.tar.xz
tar xvJf qemu-2.9.0.tar.xz
mkdir build-qemu
cd build-qemu
../qemu-2.9.0/configure --target-list=x86_64-softmmu --disable-kvm --enable-debug
make -j2
sudo make install
cd ..

# install grub
sudo apt-get install -y autoconf bison flex libdevmapper-dev
wget http://alpha.gnu.org/gnu/grub/grub-2.02~beta3.tar.gz
tar zxvf grub-2.02~beta3.tar.gz
cd grub-2.02~beta3
./autogen.sh
./configure
make
sudo make install
cd ..

# ホストとの時刻同期のためにNTPのインストール
# 仮想環境はホストとよく時刻がずれる。ホストと仮想環境の時刻が
# ずれていると、makeの時にエラーが出て鬱陶しいので、NTPで補正する
# TODO: ホストが適切に時刻同期されていない場合、どうする?
sudo apt-get install -y ntp
sudo service ntp stop
sudo ntpdate ntp.nict.jp
sudo sed -i -e 's/^server/#server/g' /etc/ntp.conf
sudo sh -c 'echo "server ntp.nict.jp" >> /etc/ntp.conf'
sudo service ntp start

sudo sh -c 'date > /etc/bootstrapped'

echo "setup done!"
