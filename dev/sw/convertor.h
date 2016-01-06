#ifndef __CONVERTOR_H__
#define __CONVERTOR_H__

#ifdef __cplusplus
extern "C"
{
#endif

int std2pri(const char *protocol, int protocolLen, uint8_t *io, int ioLen, int *type, void *userData);
int pri2std(const uint8_t *io, int ioLen, char *protocol, int protocolLen, int type, void *userData);

#ifdef __cplusplus
}
#endif

#endif