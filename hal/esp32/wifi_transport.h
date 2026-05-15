#ifndef WIFI_TRANSPORT_H
#define WIFI_TRANSPORT_H

#ifdef ENABLE_WIFI_TRANSPORT

#ifdef __cplusplus
extern "C" {
#endif

void wifi_transport_early_init(void);
void wifi_transport_init(void);
void wifi_transport_loop(void);
bool wifi_transport_connected(void);

#ifdef __cplusplus
}
#endif

#endif /* ENABLE_WIFI_TRANSPORT */
#endif /* WIFI_TRANSPORT_H */
