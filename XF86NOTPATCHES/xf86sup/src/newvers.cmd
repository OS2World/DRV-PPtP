/* REXX */

parse value linein('version.h') with '#define VERSION "\r\nXFREE86 Support Driver V ' major '.' minor '"'
minor = minor + 1
/**/
call lineout 'version.h','#define VERSION "\r\nXFREE86 Support Driver V 'major'.'minor'"',1
call lineout 'version.h','#define VERSIONID 0x'right(major,4,'0')right(minor,4,'0')
/**/
call lineout 'xf86sup.def','LIBRARY XF86SUP',1
call lineout 'xf86sup.def','DESCRIPTION "@#XF86:'major'.'minor'#@ XFREE86 Support Multi Driver"'
call lineout 'xf86sup.def',"PROTMODE"
call lineout 'xf86sup.def',''
call lineout 'xf86sup.def','SEGMENTS'
call lineout 'xf86sup.def',"	_DATA	CLASS	'DATA'		PRELOAD"
call lineout 'xf86sup.def',"	CONST	CLASS	'CONST'		PRELOAD"
call lineout 'xf86sup.def',"	_BSS	CLASS	'BSS'		PRELOAD"
call lineout 'xf86sup.def',"	_TEXT	CLASS	'CODE'		PRELOAD"

exit 0
