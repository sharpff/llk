#ifndef __STATE_H__
#define __STATE_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include "leconfig.h"

typedef enum {
    E_STATE_NONE = -1,
    E_STATE_START,
    E_STATE_CONFIGURING,
    E_STATE_SNIFFER_GOT,
    E_STATE_AP_CONNECTING,
    E_STATE_AP_CONNECTED, // 4
    E_STATE_CLOUD_LINKED, // 5
    E_STATE_CLOUD_AUTHED, // 6
    E_STATE_CLOUD_ONLINE, // 7
    E_STATE_MAX
}StateId;


int isApConnected(void);
int isCloudAuthed(void);
int isCloudOnlined(void);
StateId changeStateId(StateId state);
StateId getStateId(void);
int lelinkPollingState(uint32_t msDelay, void *r2r, void *q2a);
int lelinkNwPostCmdExt(const void *node);
int resetConfigData(int bussinessOnly);

void reboot(int isAsync);

#ifdef __cplusplus
}
#endif

#endif
