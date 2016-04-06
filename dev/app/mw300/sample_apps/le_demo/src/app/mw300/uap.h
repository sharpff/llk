#ifndef _UAP_H_
#define _UAP_H_

int wlanUAPInit(const char *uuid);
void eventUAPStarted(void *data);
void eventUAPStopped(void *data);

#endif /* end of include guard: _UAP_H_ */
