# pico_VGA_test

Project for using Pi Pico to set the variable gain register of LMH6401 with SPI.

# compilation / usage

To compile the binary
```
$ mkdir build && cd build
$ cmake ..
$ make
```

To upload the binary, hold down the BOOTSEL button while plugging the Pi Pico in to a USB port.
Then, mount the Pi Pico file system and copy the executable.
It may take a few seconds for the Pico to reboot.
```
# fdisk -l
...
Device     Boot Start    End Sectors  Size Id Type
/dev/sda1           1 262143  262143  128M  e W95 FAT16 (LBA)
# mount /dev/sda1 /mnt/pico
# pwd
~/pico_VGA_test/build
# cp spi_lmh6401.uf2 /mnt/pico
```
