/*************************************************************/
/* fwdisc.c  -  Disk IO routines                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* Written and (c) by Thomas Huth                            */
/* See "fwsource.txt" for more information                   */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* This file is distributed under the GNU General Public     */
/* License, version 2 or at your option any later version.   */
/* Please read the file "gpl.txt" for details.               */
/*************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "level.h"
#include "fwgui.h"
#include "fwgraf.h"
#include "fwmain.h"
#include "fwdata.h"
#include "fwdisk.h"

#ifdef SOZOBON
#define USEBACKSLASH
#endif


/* ** Variablen: ** */
#ifdef USEBACKSLASH
char roompath[]=".\\rooms";
#else
char roompath[]="./rooms";
#endif
char roomname[256];

extern char spec_gemz[8];



static unsigned short swap_short(unsigned short in)
{
	unsigned short out;
	out=((char)in)<<8;
	out|=(in>>8);
	return out;
}

static unsigned long swap_long(unsigned long in)
{
	unsigned long out;
	unsigned short o1, o2;
	o1=(unsigned short)(in>>16);
	o2=(unsigned short)(in);
	o1=swap_short(o1);
	o2=swap_short(o2);
	out=((unsigned long)o2)<<16;
	out|=o1;
	return out;
}



/* ***Load a new room*** */
int loadroom(void)
{
	LEVEL_HEADER hd;
	LEVEL_EINTRAG eintrbuf;
	FILE *fhndl;
	long len;
	static long oldlen=0;
	static char *buf=0L;
	char *sfbuf, *ffbuf;
	int dx, dy, i;

	/* Create file name */
	strcpy(roomname, roompath);
#ifdef USEBACKSLASH
	strcat(roomname, "\\room");
#else
	strcat(roomname, "/room");
#endif
	i=strlen(roomname);
	if(roomnr<10)
		roomname[i]=roomnr+'0';
	else
		roomname[i]=roomnr-10+'a';
	if(room_x<10)
		roomname[i+1]=room_x+'0';
	else
		roomname[i+1]=room_x-10+'a';
	if(room_y<10)
		roomname[i+2]=room_y+'0';
	else
		roomname[i+2]=room_y-10+'a';
	roomname[i+3]=0;

	fhndl=fopen(roomname, "rb");
	if (fhndl == NULL)
	{
		errfatldlg("Could not open\nlevel file!");
		return -1;
	}

	if (fread(&hd, sizeof(LEVEL_HEADER), 1, fhndl) != 1)
	{
		errfatldlg("Could not read\nlevel file!");
		return -1;
	}

	if(hd.hmagic!=(long)0x4641574FL)  /*'FAWO'*/
	{
		hd.hmagic=swap_long(hd.hmagic);
		hd.version=swap_short(hd.version);
		hd.anz_obj=swap_short(hd.anz_obj);
		hd.r_wdth=swap_short(hd.r_wdth);
		hd.r_hght=swap_short(hd.r_hght);
	}

	if(hd.hmagic!=(long)0x4641574FL)  /*'FAWO'*/
	{
		char str[200];
		fclose(fhndl);
		sprintf(str, "No Fanwor|level file:|%lx", hd.hmagic);
		errfatldlg(str);
	}

	r_width=hd.r_wdth;
	r_height=hd.r_hght;

	len=2L*sizeof(unsigned char)*r_width*r_height;
	if(len>oldlen)
	{
		if(buf) free(buf);
		buf=malloc(len);
		if(buf==NULL)
		{
			fclose(fhndl);
			errfatldlg("No temporary memory\navailable!");
		}
		oldlen=len;
	}

	if( fread(buf, sizeof(char), len, fhndl) != len )
	{
		free(buf);
		fclose(fhndl);
		errfatldlg("Error while reading\nlevel file!");
	}

	sfbuf=buf;
	for(dy=0; dy<r_height; dy++)
		for(dx=0; dx<r_width; dx++)
		{
			room[dx][dy]=*(sfbuf+dx+dy*r_width);
		}
	ffbuf=sfbuf+r_width*r_height;
	for(dy=0; dy<r_height; dy++)
		for(dx=0; dx<r_width; dx++)
		{
			ffield[dx][dy]=*(ffbuf+dx+dy*r_width);
		}

	spritenr=1;
	doornr=0;
	dx=hd.anz_obj;
	while( dx>0 )
	{
		if( fread(&eintrbuf, sizeof(LEVEL_EINTRAG), 1, fhndl) != 1 )
		{
			fclose(fhndl);
			errfatldlg("Could not load\nlevel file");
			return -1;
		}
		switch(eintrbuf.eintrtyp)
		{
		case 1:
			addsprite(eintrbuf.art, eintrbuf.xpos<<5, eintrbuf.ypos<<5);
			break;
		case 2:
			doors[doornr].x=eintrbuf.xpos;
			doors[doornr].y=eintrbuf.ypos;
			doors[doornr].destnr=eintrbuf.art;
			doors[doornr].exitx=eintrbuf.xl;
			doors[doornr].exity=eintrbuf.yl;
			doors[doornr].destx=eintrbuf.spec1>>4;
			doors[doornr].desty=eintrbuf.spec1 & 0x0F;
			++doornr;
			break;
		case 4:
			if(spritetable[eintrbuf.art].id<8 && spec_gemz[spritetable[eintrbuf.art].id])
				break;  /* SPECIAL: Gems */
			addsprite(eintrbuf.art, eintrbuf.xpos<<5, eintrbuf.ypos<<5);
			break;
		}
		--dx;
	}

	fclose(fhndl);

	for(dx=rwx; dx<rwx+rww; dx++)
		for(dy=rwy; dy<rwy+rwh; dy++)
		{
			drawblock(dx, dy, room[dx][dy]);
		}

	offscr2win(rwx, rwy, rww, rwh);

	return 0;
}
