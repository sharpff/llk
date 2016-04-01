#ifndef HAL_EZCONN_H
#define HAL_EZCONN_H



int ezconn_init();

/* for device */
int ezconn_sniffer_start();
int ezconn_sniffer_stop();

/* for app */
int ezconn_prov_start();
int ezconn_prov_stop();

void ezconn_exit();

#endif