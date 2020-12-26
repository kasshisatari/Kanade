# Copyright 2020 oscillo
# 
# Permission is hereby granted, free of charge,
# to any person obtaining a copy of this software
# and associated documentation files(the "Software"),
# to deal in the Software without restriction,
# including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense,
# and / or sell copies of the Software, and to permit
# persons to whom the Software is furnished to do so,
# subject to the following conditions :
# 
# The above copyright notice and this permission
# notice shall be included in all copies or substantial
# portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY
# OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
# LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
# OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#sudo chromium-browser --noerrdialogs --kiosk --incognito --no-default-browser-check --no-sandbox --test-type
sudo apt-get update
sudo apt-get install -y libgtk-3-dev exfat-fuse exfat-utils libavutil-dev libavcodec-dev libavformat-dev libvlc-dev qrencode hostapd dnsmasq unclutter libsystemd-dev

if test -e "/home/pi/Desktop/Kanade/wlan0"; then
  sudo cp /home/pi/Desktop/Kanade/wlan0 /etc/network/interfaces.d/wlan0
fi
sudo cp /etc/network/interfaces.d/wlan0 /home/pi/Desktop/Kanade/wlan0
sudo ed /etc/network/interfaces.d/wlan0 << EOF
a
iface lo inet loopback
iface eth0 inet dhcp
allow-hotplug wlan0
iface wlan0 inet static
  address 192.168.9.1
  netmask 255.255.255.0
.
wq
EOF

if test -e "/home/pi/Desktop/Kanade/dhcpcd.conf"; then
  sudo cp /home/pi/Desktop/Kanade/dhcpcd.conf /etc/dhcpcd.conf
fi
sudo cp /etc/dhcpcd.conf /home/pi/Desktop/Kanade/dhcpcd.conf
sudo ed /etc/dhcpcd.conf << EOF
$
a
interface wlan0
  static ip_address=192.168.9.1/24
  nohook wpa_supplicant
.
wq
EOF

cd /etc/hostapd/
sudo cp /usr/share/doc/hostapd/examples/hostapd.conf .
cpuinfo=$(sum /proc/cpuinfo | cut -d ' ' -f1)
sudo ed /etc/hostapd/hostapd.conf << EOF
%s/ssid=test/ssid=Kanade$cpuinfo/
wq
EOF

if test -e "/home/pi/Desktop/Kanade/init.d_hostapd"; then
  sudo cp /home/pi/Desktop/Kanade/init.d_hostapd /etc/init.d/hostapd
fi
sudo cp /etc/init.d/hostapd /home/pi/Desktop/Kanade/init.d_hostapd
sudo ed /etc/init.d/hostapd << EOF
%s/DAEMON_CONF=/DAEMON_CONF=\/etc\/hostapd\/hostapd.conf/
wq
EOF

sudo ed /etc/sysctl.conf << EOF
%s/#net.ipv4.ip_forward=1/net.ipv4.ip_forward=1/
wq
EOF

sudo iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
sudo iptables -t nat -A POSTROUTING -o wlan1 -j MASQUERADE
sudo sh -c "iptables-save > /etc/iptables.ipv4.nat"

if test -e "/home/pi/Desktop/Kanade/rc.local"; then
  sudo cp /home/pi/Desktop/Kanade/rc.local /etc/rc.local
fi
sudo cp /etc/rc.local /home/pi/Desktop/Kanade/rc.local
sudo ed /etc/rc.local << EOF
n
i
iptables-restore < /etc/iptables.ipv4.nat
.
wq
EOF

if test -e "/home/pi/Desktop/Kanade/dnsmasq.conf"; then
  sudo cp /home/pi/Desktop/Kanade/dnsmasq.conf /etc/dnsmasq.conf
fi
sudo cp /etc/dnsmasq.conf /home/pi/Desktop/Kanade/dnsmasq.conf
sudo ed /etc/dnsmasq.conf << EOF
$
a
interface=wlan0
domain-needed
bogus-priv
dhcp-range=192.168.9.50,192.168.9.150,12h
.
wq
EOF

if test -e "/home/pi/Desktop/Kanade/autostart"; then
  sudo cp /home/pi/Desktop/Kanade/autostart /etc/xdg/lxession/LXDE-pi/autostart
fi
sudo cp /etc/xdg/lxsession/LXDE-pi/autostart /home/pi/Desktop/Kanade/autostart
sudo ed /etc/xdg/lxsession/LXDE-pi/autostart << EOF
$
a
@xset s off
@xset -dpms
@xset s noblank
@unclutter
@sh /home/pi/Desktop/Kanade/start.sh
.
wq
EOF

sudo ed /boot/config.txt << EOF
%s/#hdmi_force/hdmi_force/
%s/#hdmi_group/hdmi_group/
%s/#hdmi_mode=1/hdmi_mode=16/
%s/#dtoverlay=vc4-fkms-v3d/dtoverlay=hifiberry-dac/
wq
EOF

if [ $# = 1 ] && [ $1 = "tv" ]; then
sudo ed /boot/config.txt << EOF
%s/#config_hdmi_boost=4/sdtv_mode=1/
%s/#hdmi_drive=2/sdtv_aspect=3/
%s/#arm_freq=800/enable_tvout=1/
.
wq
EOF
fi

sudo ed /etc/xdg/pcmanfm/default/pcmanfm.conf << EOF
%s/autorun=1/autorun=0/
wq
EOF

sudo ed /etc/xdg/pcmanfm/LXDE-pi/pcmanfm.conf << EOF
%s/autorun=1/autorun=0/
wq
EOF

sudo ed /home/pi/.config/pcmanfm/LXDE-pi/pcmanfm.conf << EOF
%s/autorun=1/autorun=0/
wq
EOF

sudo systemctl unmask hostapd
sudo systemctl enable hostapd

sudo swapoff --all
sudo apt-get purge -y dphys-swapfile
sudo rm -fr /var/swap

cd /home/pi/Desktop/Kanade
make

sudo cp /home/pi/Desktop/Kanade/Kanade.desktop /usr/share/applications

sudo reboot
