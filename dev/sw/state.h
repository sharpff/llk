#ifndef __STATE_H__
#define __STATE_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum {
    E_STATE_NONE = -1,
    E_STATE_START,
    E_STATE_CONFIGURING,
    E_STATE_SNIFFER_GOT,
    E_STATE_AP_CONNECTING,
    E_STATE_AP_CONNECTED,
    E_STATE_CLOUD_LINKED,
    E_STATE_CLOUD_AUTHED,
    E_STATE_MAX
}StateId;


int isCloudAuthed(void);
StateId changeStateId(StateId next);
int lelinkPollingState(uint32_t msDelay, void *r2r, void *q2a);

#ifdef __cplusplus
}
#endif

#endif
