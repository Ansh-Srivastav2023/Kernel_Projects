#!/bin/bash

KERNEL_VERSION=6.18.3
BUSYBOX_VERSION=1.34.1


mkdir -p src
cd src

    # kernel
    KERNEL_MAJOR=$(echo $KERNEL_VERSION | sed 's/\([0-9]*\)[^0-9].*/\1/')
    wget https://www.kernel.org/pub/linux/kernel/v$KERNEL_MAJOR.x/linux-$KERNEL_VERSION.tar.xz
    tar -xf linux-$KERNEL_VERSION.tar.xz
    cd linux-$KERNEL_VERSION
        make defconfig 
        make -j8 || exit
    cd ..

    #BUSYBOX
    wget https://busybox.net/downloads/busybox-$BUSYBOX_VERSION.tar.bz2
    tar -xf busybox-$BUSYBOX_VERSION.tar.bz2
    cd busybox-$BUSYBOX_VERSION
        make defconfig
        make menuconfig     # explicitly enable static
        # OR
        scripts/config --enable CONFIG_STATIC
        make oldconfig

        make -j$(nproc)
        make install
    cd ..
cd ..


cp /home/anx/linux-$KERNEL_VERSION/arch/x86_64/boot/bzImage ./


#initrd
mkdir initrd
cd initrd

    mkdir -p bin dev proc sys
    cd bin
        cp ../../src/busybox-$BUSYBOX_VERSION/busybox ./

        for prog in $(./busybox --list); do
            ln -s busybox $prog
        done
    cd ..

    echo '#!/bin/sh' > init
    echo 'mount -t sysfs sysfs /sys' >> init
    echo 'mount -t proc proc /proc' >> init
    echo 'mount -t devtmpfs devtmpfs /dev' >> init
    echo 'echo "2 1 4 7" > /proc/sys/kernel/printk' >> init
    echo '/bin/sh' >> init
    echo 'poweroff -f' >> init

    chmod +x init
    find . | cpio -o -H newc | gzip > ../initrd.img 
cd ..