
#ifndef _FWGUI_H
#define _FWGUI_H

void event_handler(void);
int choicedlg(char *txt, char *buts, int defb);
void alertdlg(char *alstr);
void errfatldlg(char *errstr);
void waitms(short ms);


#endif
