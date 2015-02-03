#! /bin/bash


echo ""
echo ""
echo "1. Fetching dependencies"
cd ..
git submodule update --init --recursive > /dev/null


echo ""
echo "2. Installing patched version of am335x-pru-package."
mv /usr/lib/libprussdrvd.so /usr/lib/libprussdrvd.so.bkp &> /dev/null
mv /usr/lib/libprussdrvd.a /usr/lib/libprussdrvd.a.bkp &> /dev/null
mv /usr/lib/libprussdrv.so /usr/lib/libprussdrv.so.bkp &> /dev/null
mv /usr/lib/libprussdrv.a /usr/lib/libprussdrv.a.bkp &> /dev/null
mv /usr/include/prussdrv.h /usr/include/prussdrv.h.bkp &> /dev/null
mv /usr/include/pruss_intc_mapping.h /usr/include/pruss_intc_mapping.h.bkp &> /dev/null
mv /usr/bin/pasm /usr/bin/pasm.bkp &> /dev/null

cd vendors/am335x_pru_package/pru_sw/utils/pasm_source 
./linuxbuild
install -m 0755 ../pasm /usr/bin

cd ../../app_loader/interface
PREFIX=/usr make > /dev/null
PREFIX=/usr make install > /dev/null

echo ""
echo "3. Installing beaglebone_pruio library."
cd ../../../../../library
PREFIX=/usr make install > /dev/null

echo ""
echo "4. Installing puredata externals."
cd ../puredata-external
make install > /dev/null

echo ""
echo "5.Disabling HDMI virtual cape."
mount /dev/mmcblk0p1 /mnt/card
cd /mnt/card
sed -i.bak '/^##Disable HDMI$/{N; s/^##Disable HDMI\n#/##Disable HDMI\n/}' uEnv.txt
cd
umount /mnt/card

echo ""
echo "6.Assigning index zero to USB sound card in ALSA."
cd /etc/modprobe.d
sed -i.bak 's/options snd-usb-audio index=-2/options snd-usb-audio index=0/' alsa-base.conf

echo ""
echo "Done. Rebooting now."

reboot