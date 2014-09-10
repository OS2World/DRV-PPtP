(This file is for developers, I suggest when your an end user you download
the other file from the website, home.wanadoo.nl/~rwklein)

These are the full sources of the PPtP protocol for OS/2.
It has ported from Linux by Christoph Theis (theis@aon.at) from Austria.
He has carried over the source code to me. The Linux sources where released
under GNU GENERAL PUBLIC LICENSE (see the file "copying" in the directory 
pptplinuxos2port).

This zip file contains two directories. I have not modified anything
other then putting the two zip files with sources into one file.

The directory pptplinuxos2port contains the Linux/modified OS/2
stuff. You need EMX run time libraries to use these files!

The directory "patchesxf86sup" contains the sources of the patched
XF86SUP.SYS.

The file "xf86notpatched.zip" contains the sources of the xf86sup.sys,
version 1.511. Not the most recent version it lacks
a small change for console$ which you likely won't need. 1.511 understands
WSEB, FP13-15, MCP/ACP and eCS.


These are the orginal installation instructions for the PPtP protocol:

Installation:
XF86sup.sys is a driver, so you need a line
DEVICE=...\XF82SUP.SYS
in your config.sys

In the %ETC%-directory you need two files
chap.sct:
* * <your password>

and ppp.cfg:
name <your userid>
mru  1400
defaultroute

Unfortunately the XF-Driver cannot deal with packetsize lager than
1400 (or 1500) bytes. It will block, if a packet is larger than that.
You can specify the MRU size in the PPP.CFG file, but not the MTU
size. So, after starting the pptp-Programm, you have to setup the MTU
explicitly.

Programm start:
You need pptp.exe, pppd.exe and callmgr.exe (sp) in the same directory.

# Start pptp
"pptp 10.0.0.138"   # this is the address of my "modem"
# Wait until connection is established, then set the MTU
"ifconfig ppp0 mtu 1400"


At this moment there are three known issues:

The first issue:

Somebody has already tested the protocol with ADSL here in the Netherlands and
found out he could not get NAT going. What the problem is, is unknown.
This was tested on a Warp 4 machine with a NONE Aurora kernel. 


The second issue:

He later tried on a WSeB (Aurora Server), this kernel is now in fixpak 13
and higher, Convience pack and eCS (eComStaion). On his WSeB machine 
the xf86sup.sys, did load, but when he tried to the connect the machine 
trapped. When the first beta of WSeB came out I tried the xf86sup.sys to 
to get BOCHS going, it trapped at startup of the machine.
Based on these two experiences I'm afraid the PPtP protocol will 
not work on fixpak 13 and higher.

Problem three was mentioned in the installation manual:

Unfortunately the XF-Driver cannot deal with packetsize lager than
1400 (or 1500) bytes. It will block, if a packet is larger than that.
You can specify the MRU size in the PPP.CFG file, but not the MTU
size. So, after starting the pptp-Programm, you have to setup the MTU
explicitly.

I'm not a good programmer so I hope somebody is willing to fix these things.

Good luck 

Roderick Klein

P.S. If you improve the protocol, please share this with the OS/2 /eCS community.
Send back a modified ZIP file with compiled binaries to rwklein@wanadoo.nl.
Homepage can be found at home.wanadoo.nl/~rwklein