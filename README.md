# eCPM

Inspired by the amazing RunCPM, eCPM is a CP/M 2.2 emulator,
developed  mainly fo the ESP8266 microcontroller.  This is a  far
from perfect emulation, just like RunCPM, it's just enough to run
a lot of old CP/M programs, notably office applications, program-
ming environments and,  obviously,  games.  Mostly text adventure
games.

BIOS and BDOS are completly reimplemented in  C++,  running
on 32 bits, with barely a dozen of "hooks" to interface with
the 8-bit environment.  CCP is not emulated, it is the old i8080
compatible Digital Research version, reassambled.

The  major  goal  of the project was to run on  the  ESP8266
microcontroller,  a cheap and disposable device, and, why not, on
any  other  Arduino device.  The added benefit  is  the  wireless
connectivity  of  the ESP,  allowing us not only to  connect  to,
let's say, a network printer, but to remotely connect to the CP/M
prompt via telnet.  The downside of using an ESP8266 is the small
amount of on-chip RAM,  around 96kb, which might seem enough, and
it  eally  is,  if you don't use the  full  SDK,  providing  WiFi
connectivity. If you need WiFi, do not hope for more than 48kb of
CP/M  memory.  That's why we have experimented with SPI  RAM.  It
works,  but it's slower. We tried buffered RAM and multi-buffered
RAM. It worked faster, but not faster than on-chip RAM.

Another goal was to emulate the filesystem too,  like RunCPM
does,  to  place  your files directly on SD card (organized in  a
directory structure), and not inside disk images. It is easier to
work this way, but you need to provide the translation inside the
emulated BDOS.

This  way,  we don't emulate a machine,  but  the  operating
system, therefore the CP/M 2.2 compatibility level is all you can
get.  No CP/M 3,  no MP/M.  There are a lot of projects out there
doing  this  sucessfully.  They inspired  us:  cpm8266,  Z80pack,
cpmish,  Altair8800, imsai8080esp, TeensyBoardZ80 and, of course,
RunCPM, this project evolved from. I would like to thank you all.
You are the giants on whose shoulders we stand.

Keep  in mind this is work in progress,  a lot might change.
Or not.



















1






