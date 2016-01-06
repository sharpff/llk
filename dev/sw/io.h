#ifndef __IO_H__
#define __IO_H__

#ifdef __cplusplus
extern "C"
{
#endif

int ioWrite(int type, const uint8_t *data, int dataLen);
int ioRead(int type, uint8_t *data, int dataLen);

#ifdef __cplusplus
}
#endif

#endif