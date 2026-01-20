#!/bin/bash

KERNEL_VERSION=6.18.3
BUSYBOX_VERSION=1.34.1

MODULE_PATH="../periodic_blink.ko"

mkdir -p initrd
cd initrd

    mkdir -p bin dev proc sys lib/modules

    cd bin
    cp ../../src/busybox-$BUSYBOX_VERSION/busybox ./
    chmod +x busybox
    for prog in $(./busybox --list); do
        ln -s busybox $prog
    done

cd ..

# cp ../../Miscell/test ./


if [ -f "$MODULE_PATH" ]; then
    cp "$MODULE_PATH" .
else
    echo "WARNING: $MODULE_PATH not found! Module missing."
fi



cat <<EOF > init
#!/bin/sh
export PATH=/bin:/sbin

mount -t sysfs sysfs /sys
mount -t proc proc /proc
mount -t devtmpfs devtmpfs /dev 2>/dev/null || {
    mount -t tmpfs tmpfs /dev
    mknod /dev/console c 5 1
}

echo "2 1 4 7" > /proc/sys/kernel/printk

echo "+---------------------------------------+"
echo "|  Welcome to Ansh's Kernel Lab Sandbox |"
echo "+---------------------------------------+"

# if [ -f /periodic_blink.ko ]; then
#     insmod /periodic_blink.ko
#     echo "SUCCESS: periodic_blink.ko loaded."
# else
#     echo "ERROR: periodic_blink.ko missing."
# fi

exec sh
poweroff -f
EOF



chmod +x init

find . | cpio -o -H newc | gzip > ../initrd.img
cd ..
