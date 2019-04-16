#ifndef __MQTT_H__
#define __MQTT_H__

#define MQTT_RETRY_TIMEOUT          10   // second
#define MQTT_CONNECT_TIMEOUT        30   // second
#define MQTT_RECONNECT_TIMEOUT      5    // second
#define MQTT_SEND_TASK_PRIO         2
#define MQTT_SEND_TASK_SIZE         10   // events
#define MQTT_QUEUE_FIFO_SIZE        2048
#define MQTT_WORK_BUFFER_SIZE       2048
//#define MQTT_PROTOCOL_NAMEv31   /*MQTT version 3.1 compatible with Mosquitto v0.15*/
#define MQTT_PROTOCOL_NAMEv311    /*MQTT version 3.11 compatible with https://eclipse.org/paho/clients/testing/*/


#include <c_types.h>
#include "ets_func.h"
#include "user_interface.h"
#include "ip_addr.h"
#include "espconn.h"
#include "blob_fifo.h"

#define MQTT_SSL_OFF 0
#define MQTT_SSL_ON  1

#if defined(MQTT_PROTOCOL_NAMEv31)
#define MQTT_CONNECT_PROTOCOL_NAME      "MQIsdp"
#define MQTT_CONNECT_PROTOCOL_VERSION   3
#elif defined(MQTT_PROTOCOL_NAMEv311)
#define MQTT_CONNECT_PROTOCOL_NAME      "MQTT"
#define MQTT_CONNECT_PROTOCOL_VERSION   4
#else
#error "Please define protocol version"
#endif

#define MQTT_FLAG_DUP               BIT(3)
#define MQTT_FLAG_QOS_2             BIT(2)
#define MQTT_FLAG_QOS_1             BIT(1)
#define MQTT_FLAG_QOS_M             0x03
#define MQTT_FLAG_QOS_S             1
#define MQTT_FLAG_RETAIN            BIT(0)

#define MQTT_CONNECT_FLAG_USERNAME          BIT(7)
#define MQTT_CONNECT_FLAG_PASSWORD          BIT(6)
#define MQTT_CONNECT_FLAG_WILL_RETAIN       BIT(5)
#define MQTT_CONNECT_FLAG_WILL_QOS_2        BIT(4)
#define MQTT_CONNECT_FLAG_WILL_QOS_1        BIT(3)
#define MQTT_CONNECT_FLAG_WILL_QOS_M        0x03
#define MQTT_CONNECT_FLAG_WILL_QOS_S        3
#define MQTT_CONNECT_FLAG_WILL              BIT(2)
#define MQTT_CONNECT_FLAG_CLEAN_SESSION     BIT(1)

#define MQTT_CONNACK_STATUS_OK                      0
#define MQTT_CONNACK_STATUS_INVALID_VERSION         1
#define MQTT_CONNACK_STATUS_INVALID_CLIENT_ID       2
#define MQTT_CONNACK_STATUS_SERVER_NOT_AVAILABLE    3
#define MQTT_CONNACK_STATUS_BAD_USER_PASS           4
#define MQTT_CONNACK_STATUS_NOT_AUTHORIZED          5
#define MQTT_CONNACK_STATUS_DNS_ERROR               255

#define MQTT_SUBACK_STATUS_QOS0         0
#define MQTT_SUBACK_STATUS_QOS1         1
#define MQTT_SUBACK_STATUS_QOS2         2
#define MQTT_SUBACK_STATUS_FAILED       128

#define MQTT_PACKET_TYPE_CONNECT        1
#define MQTT_PACKET_TYPE_CONNACK        2
#define MQTT_PACKET_TYPE_PUBLISH        3
#define MQTT_PACKET_TYPE_PUBACK         4
#define MQTT_PACKET_TYPE_PUBREC         5
#define MQTT_PACKET_TYPE_PUBREL         6
#define MQTT_PACKET_TYPE_PUBCOMP        7
#define MQTT_PACKET_TYPE_SUBSCRIBE      8
#define MQTT_PACKET_TYPE_SUBACK         9
#define MQTT_PACKET_TYPE_UNSUBSCRIBE    10
#define MQTT_PACKET_TYPE_UNSUBACK       11
#define MQTT_PACKET_TYPE_PINGREQ        12
#define MQTT_PACKET_TYPE_PINGRESP       13
#define MQTT_PACKET_TYPE_DISCONNECT     14


typedef struct mqtt_pending_message_t mqtt_pending_message_t;
typedef struct mqtt_client_t mqtt_client_t;
typedef void (* mqtt_timeout_callback_fn_t)(mqtt_client_t *);
typedef void (* mqtt_conn_callback_fn_t)(mqtt_client_t *, uint8_t);
typedef void (* mqtt_disconn_callback_fn_t)(mqtt_client_t *, uint8_t);
typedef void (* mqtt_sub_callback_fn_t)(mqtt_client_t *, uint16_t, uint8_t);
typedef void (* mqtt_msg_callback_fn_t)(mqtt_client_t *, uint16_t);
typedef void (* mqtt_data_callback_fn_t)(mqtt_client_t *, const char *, uint16_t, const void *, uint32_t);

struct mqtt_pending_message_t
{
    uint16_t id;
    uint16_t retry_tick;
    uint8_t *data;
    uint32_t data_len;
    mqtt_pending_message_t *prev;
    mqtt_pending_message_t *next;
};

struct mqtt_client_t
{
    struct espconn *conn;
    char *client_id;
    char *username;
    char *password;
    char *will_topic;
    char *will_message;
    char *host;
    uint16_t port;
    uint16_t keep_alive;
    uint8_t connect_flags;
    uint8_t ssl : 1;
    uint8_t connecting : 1;
    uint8_t reconnecting : 1;
    uint8_t disconnecting : 1;
    uint8_t tcp_connected : 1;
    uint8_t connected : 1;
    uint8_t sending : 1;
    uint8_t *work_buffer;
    uint32_t work_buffer_len;
    uint16_t message_id;
    mqtt_pending_message_t *waiting_suback;
    mqtt_pending_message_t *waiting_unsuback;
    mqtt_pending_message_t *waiting_puback;
    mqtt_pending_message_t *waiting_pubrec;
    mqtt_pending_message_t *waiting_pubrel;
    mqtt_pending_message_t *waiting_pubcomp;
    mqtt_conn_callback_fn_t connect_cb;
    mqtt_disconn_callback_fn_t disconnect_cb;
    mqtt_timeout_callback_fn_t timeout_cb;
    mqtt_sub_callback_fn_t subscribe_cb;
    mqtt_msg_callback_fn_t unsubscribe_cb;
    mqtt_msg_callback_fn_t publish_cb;
    mqtt_data_callback_fn_t data_cb;
    ets_timer_t timer;
    uint16_t keep_alive_tick;
    uint16_t reconnect_tick;
    uint16_t connect_tick;
    blob_fifo_t *msg_fifo;
};

mqtt_client_t* ICACHE_FLASH_ATTR mqtt_init(const char *client_id, const char *host, const uint16_t port, const uint8_t ssl, const uint16_t keep_alive, uint8_t clean_session);
uint8_t ICACHE_FLASH_ATTR mqtt_delete(mqtt_client_t *client);
uint8_t ICACHE_FLASH_ATTR mqtt_set_auth(mqtt_client_t *client, const char *client_user, const char *client_pass);
uint8_t ICACHE_FLASH_ATTR mqtt_set_lwt(mqtt_client_t *client, const char *topic, const void *data, const uint8_t qos, const uint8_t retain);
uint8_t ICACHE_FLASH_ATTR mqtt_set_connect_callback(mqtt_client_t *client, mqtt_conn_callback_fn_t fn);
uint8_t ICACHE_FLASH_ATTR mqtt_set_disconnect_callback(mqtt_client_t *client, mqtt_disconn_callback_fn_t fn);
uint8_t ICACHE_FLASH_ATTR mqtt_set_subscribe_callback(mqtt_client_t *client, mqtt_sub_callback_fn_t fn);
uint8_t ICACHE_FLASH_ATTR mqtt_set_unsubscribe_callback(mqtt_client_t *client, mqtt_msg_callback_fn_t fn);
uint8_t ICACHE_FLASH_ATTR mqtt_set_publish_callback(mqtt_client_t *client, mqtt_msg_callback_fn_t fn);
uint8_t ICACHE_FLASH_ATTR mqtt_set_timeout_callback(mqtt_client_t *client, mqtt_timeout_callback_fn_t fn);
uint8_t ICACHE_FLASH_ATTR mqtt_set_data_callback(mqtt_client_t *client, mqtt_data_callback_fn_t fn);
uint8_t ICACHE_FLASH_ATTR mqtt_connect(mqtt_client_t *client);
uint8_t ICACHE_FLASH_ATTR mqtt_disconnect(mqtt_client_t *client);
uint8_t ICACHE_FLASH_ATTR mqtt_subscribe(mqtt_client_t *client, const char *topic, const uint8_t qos);
uint8_t ICACHE_FLASH_ATTR mqtt_unsubscribe(mqtt_client_t *client, const char *topic);
uint8_t ICACHE_FLASH_ATTR mqtt_publish(mqtt_client_t *client, const char *topic, const void *data, const uint32_t data_length, const uint8_t qos, const uint8_t retain);
uint8_t ICACHE_FLASH_ATTR mqtt_ping(mqtt_client_t *client);

#endif
