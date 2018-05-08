#ifndef USR_TIMER_H
#define USR_TIMER_H

enum {
	USR_TIMER_ONCE,
	USR_TIMER_LOOP,
};

enum {
	ERR_TIMER_OK = 0,
	ERR_TIMER_LINK_LIST_INIT,
	ERR_TIMER_TTID_FULL,
	ERR_TIMER_NO_MEMORY,
};



int USR_TimerInit();
int USR_TimerDeinit();

int USR_TimerHandler();

int USR_TimerNew(int (_Handler)(int, void *), void *, const char *, int) ;
int USR_TimerMod(int, int (_Handler)(int, void *), void *, const char *, int);
int USR_TimerDel(int);

int USR_TimerPrint();
int USR_TimerQuery(int);


#endif
