#include "mqtt.h"

ets_event_t mqtt_task_queue[MQTT_SEND_TASK_SIZE];

static uint8_t ICACHE_FLASH_ATTR mqtt_proto_append_blob(uint8_t **buf, uint32_t *buf_len, const uint8_t *data, const uint32_t data_len)
{
    if(!buf || !*buf || !buf_len)
        return 0;

    if(!data)
        return 0;

    if(data_len > *buf_len)
        return 0;

    ets_memcpy(*buf, data, data_len);

    *buf += data_len;
    *buf_len -= data_len;

    return 1;
}
static uint8_t ICACHE_FLASH_ATTR mqtt_proto_append_uint8(uint8_t **buf, uint32_t *buf_len, const uint8_t num)
{
    if(!buf || !*buf || !buf_len)
        return 0;

    if(1 > *buf_len)
        return 0;

    *(*buf)++ = num;

    *buf_len -= 1;

    return 1;
}
static uint8_t ICACHE_FLASH_ATTR mqtt_proto_parse_uint8(uint8_t **buf, uint32_t *buf_len, uint8_t *num)
{
    if(!buf || !*buf || !buf_len)
        return 0;

    if(1 > *buf_len)
        return 0;

    uint8_t rnum;

    rnum = *(*buf)++;

    if(num)
        *num = rnum;

    *buf_len -= 1;

    return 1;
}
static uint8_t ICACHE_FLASH_ATTR mqtt_proto_append_uint16(uint8_t **buf, uint32_t *buf_len, const uint16_t num)
{
    if(!buf || !*buf || !buf_len)
        return 0;

    if(2 > *buf_len)
        return 0;

    *(*buf)++ = num >> 8;
    *(*buf)++ = num & 0xFF;

    *buf_len -= 2;

    return 1;
}
static uint8_t ICACHE_FLASH_ATTR mqtt_proto_parse_uint16(uint8_t **buf, uint32_t *buf_len, uint16_t *num)
{
    if(!buf || !*buf || !buf_len)
        return 0;

    if(2 > *buf_len)
        return 0;

    uint16_t rnum;

    rnum = ((uint16_t)(*(*buf)++) << 8);
    rnum |= *(*buf)++;

    if(num)
        *num = rnum;

    *buf_len -= 2;

    return 1;
}
static uint8_t ICACHE_FLASH_ATTR mqtt_proto_append_string(uint8_t **buf, uint32_t *buf_len, const char *str)
{
    if(!buf || !*buf || !buf_len)
        return 0;

    uint16_t str_len = 0;

    if(str)
        str_len = ets_strlen(str);

    if(str_len + 2 > *buf_len)
        return 0;

    *(*buf)++ = str_len >> 8;
    *(*buf)++ = str_len & 0xFF;

    ets_memcpy(*buf, str, str_len);

    *buf += str_len;
    *buf_len -= str_len + 2;

    return 1;
}
static uint8_t ICACHE_FLASH_ATTR mqtt_proto_parse_string(uint8_t **buf, uint32_t *buf_len, const char **str, uint16_t *str_len)
{
    if(!buf || !*buf || !buf_len)
        return 0;

    if(2 > *buf_len)
        return 0;

    uint16_t len;

    len = ((uint16_t)(*(*buf)++) << 8);
    len |= *(*buf)++;

    if(len + 2 > *buf_len)
        return 0;

    if(str_len)
        *str_len = len;

    if(str)
        *str = *buf;

    *buf += len;
    *buf_len -= len + 2;

    return 1;
}
static uint8_t ICACHE_FLASH_ATTR mqtt_proto_append_fixed_header(uint8_t **buf, uint32_t *buf_len, const uint8_t packet_type, const uint8_t flags, const uint32_t len)
{
    if(!buf || !*buf || !buf_len)
        return 0;

    if(len > 268435455) // Max MQTT length
    {
        return 0;
    }
    else if(len > 2097151) // TODO: support 32-bit size packets
    {
        /*
        if(5 > *buf_len)
            return 0;

        uint8_t *buf_copy = *buf;

        *buf_copy++ = ((packet_type & 0x0F) << 4) | (flags & 0x0F);

        *buf_copy++ = 0x80 | (len % 128);
        len /= 128;
        *buf_copy++ = 0x80 | (len % 128);
        len /= 128;
        *buf_copy++ = 0x80 | (len % 128);
        *buf_copy++ = len / 128;

        *buf_len -= 5;
        */
        return 0;
    }
    else if(len > 16383) // TODO: support 32-bit size packets
    {
        /*
        if(4 > *buf_len)
            return 0;

        uint8_t *buf_copy = *buf += 1;

        *buf_copy++ = ((packet_type & 0x0F) << 4) | (flags & 0x0F);

        *buf_copy++ = 0x80 | (len % 128);
        len /= 128;
        *buf_copy++ = 0x80 | (len % 128);
        *buf_copy++ = len / 128;

        *buf_len -= 4;
        */
        return 0;
    }
    else if(len > 127)
    {
        if(3 > *buf_len)
            return 0;

        uint8_t *buf_copy = *buf += 2;

        *buf_copy++ = ((packet_type & 0x0F) << 4) | (flags & 0x0F);

        *buf_copy++ = 0x80 | (len % 128);
        *buf_copy++ = len / 128;

        *buf_len -= 3;
    }
    else
    {
        if(2 > *buf_len)
            return 0;

        uint8_t *buf_copy = *buf += 3;

        *buf_copy++ = ((packet_type & 0x0F) << 4) | (flags & 0x0F);

        *buf_copy++ = len;

        *buf_len -= 2;
    }

    return 1;
}
static uint8_t ICACHE_FLASH_ATTR mqtt_proto_parse_fixed_header(uint8_t **buf, uint32_t *buf_len, uint8_t *packet_type, uint8_t *flags, uint32_t *len)
{
    if(!buf || !*buf || !buf_len)
        return 0;

    if(1 > *buf_len)
        return 0;

    if(flags)
        *flags = **buf & 0x0F;

    if(packet_type)
        *packet_type = **buf >> 4;

    *buf += 1;
    *buf_len -= 1;

    for(uint8_t i = 0; i < 4; i++)
    {
        if(1 > *buf_len)
            return 0;

        uint8_t byte = *(*buf)++;

        *buf_len -= 1;

        if(len)
            *len += (byte & 0x7F) << (7 * i);

        if(!(byte & 0x80))
            break;
    }

    return 1;
}

static uint8_t ICACHE_FLASH_ATTR mqtt_add_pending_message(mqtt_pending_message_t **list, uint16_t message_id, const uint8_t *data, const uint32_t data_len)
{
    if(!list)
        return 0;

    if(!message_id)
        return 0;

    if(!data || !data_len)
        return 0;

    mqtt_pending_message_t *new_pending = (mqtt_pending_message_t *)ets_zalloc(sizeof(mqtt_pending_message_t));

    if(!new_pending)
        return 0;

    new_pending->data = (uint8_t *)ets_zalloc(data_len);

    if(!new_pending->data)
    {
        ets_free(new_pending);

        return 0;
    }

    ets_memcpy(new_pending->data, data, data_len);

    new_pending->data_len = data_len;
    new_pending->id = message_id;
    new_pending->retry_tick = 0;

    mqtt_pending_message_t *old_pending = *list;

    new_pending->next = old_pending;

    if(old_pending)
        old_pending->prev = new_pending;

    *list = new_pending;

    return 1;
}
static uint8_t ICACHE_FLASH_ATTR mqtt_remove_pending_message(mqtt_pending_message_t **list, mqtt_pending_message_t *message)
{
    if(!list)
        return 0;

    if(!message)
        return 0;

    if(*list == message)
        *list = message->next;
    else if(message->prev)
        message->prev->next = message->next;

    if(message->next)
        message->next->prev = message->prev;

    ets_free(message->data);
    ets_free(message);

    return 1;
}
static mqtt_pending_message_t* ICACHE_FLASH_ATTR mqtt_find_pending_message(mqtt_pending_message_t *list, const uint16_t message_id)
{
    if(!message_id)
        return NULL;

    for(mqtt_pending_message_t *message = list; message; message = message->next)
    {
        if(message->id == message_id)
            return message;
    }

    return NULL;
}
static uint8_t ICACHE_FLASH_ATTR mqtt_clear_pending_messages(mqtt_pending_message_t **list)
{
    if(!list)
        return 0;

    while(*list)
    {
        mqtt_pending_message_t *message = *list;
        *list = (*list)->next;

        ets_free(message->data);
        ets_free(message);
    }

    return 1;
}

static inline uint16_t ICACHE_FLASH_ATTR mqtt_get_next_message_id(mqtt_client_t *client)
{
    while(!++client->message_id);

    return client->message_id;
}

static void ICACHE_FLASH_ATTR mqtt_timer_callback(void *arg)
{
    if(!arg)
        return;

    mqtt_client_t *client = (mqtt_client_t *)arg;

    if(client->connecting)
        client->connect_tick++;
    else if(!client->tcp_connected && client->reconnecting)
        client->reconnect_tick++;
    else if(client->connected)
        client->keep_alive_tick++;

    if(client->connecting && client->connect_tick >= MQTT_CONNECT_TIMEOUT)
    {
        client->connecting = 0;

        if(client->reconnecting)
        {
            client->reconnect_tick = 0;
        }
        else
        {
            if(client->timeout_cb)
                client->timeout_cb(client);
        }
    }
    else if(client->reconnecting && client->reconnect_tick >= MQTT_RECONNECT_TIMEOUT)
    {
        client->reconnect_tick = 0;

        mqtt_connect(client);
    }
    else if(client->keep_alive_tick >= client->keep_alive)
    {
        mqtt_ping(client);
    }

    if(client->connected)
    {
        for(mqtt_pending_message_t *message = client->waiting_suback; message; message = message->next)
        {
            message->retry_tick++;

            if(message->retry_tick >= MQTT_RETRY_TIMEOUT)
            {
                if(blob_fifo_write(client->msg_fifo, message->data, message->data_len))
                    message->retry_tick = 0;
            }
        }

        for(mqtt_pending_message_t *message = client->waiting_unsuback; message; message = message->next)
        {
            message->retry_tick++;

            if(message->retry_tick >= MQTT_RETRY_TIMEOUT)
            {
                if(blob_fifo_write(client->msg_fifo, message->data, message->data_len))
                    message->retry_tick = 0;
            }
        }

        for(mqtt_pending_message_t *message = client->waiting_puback; message; message = message->next)
        {
            message->retry_tick++;

            if(message->retry_tick >= MQTT_RETRY_TIMEOUT)
            {
                message->data[0] |= MQTT_FLAG_DUP;

                if(blob_fifo_write(client->msg_fifo, message->data, message->data_len))
                    message->retry_tick = 0;
            }
        }

        for(mqtt_pending_message_t *message = client->waiting_pubrec; message; message = message->next)
        {
            message->retry_tick++;

            if(message->retry_tick >= MQTT_RETRY_TIMEOUT)
            {
                message->data[0] |= MQTT_FLAG_DUP;

                if(blob_fifo_write(client->msg_fifo, message->data, message->data_len))
                    message->retry_tick = 0;
            }
        }

        for(mqtt_pending_message_t *message = client->waiting_pubrel; message; message = message->next)
        {
            message->retry_tick++;

            if(message->retry_tick >= MQTT_RETRY_TIMEOUT)
            {
                if(blob_fifo_write(client->msg_fifo, message->data, message->data_len))
                    message->retry_tick = 0;
            }
        }

        for(mqtt_pending_message_t *message = client->waiting_pubcomp; message; message = message->next)
        {
            message->retry_tick++;

            if(message->retry_tick >= MQTT_RETRY_TIMEOUT)
            {
                if(blob_fifo_write(client->msg_fifo, message->data, message->data_len))
                    message->retry_tick = 0;
            }
        }
    }

    if(client->tcp_connected)
        ets_post(MQTT_SEND_TASK_PRIO, 0, client);
}
static void ICACHE_FLASH_ATTR mqtt_task_handler(ets_event_t *event)
{
    if(!event || !event->par)
        return;

    mqtt_client_t *client = (mqtt_client_t *)event->par;

    if(blob_fifo_is_empty(client->msg_fifo))
    {
        if(client->disconnecting)
            if(client->ssl)
                espconn_secure_disconnect(client->conn);
            else
                espconn_disconnect(client->conn);

        return;
    }

    if(client->sending)
        return;

    uint32_t packet_len = 0;

    if(!blob_fifo_read(client->msg_fifo, client->work_buffer, &packet_len, client->work_buffer_len))
        return;

    if(client->ssl)
        if(!espconn_secure_send(client->conn, client->work_buffer, packet_len))
            client->sending = 1;
    else
        if(!espconn_send(client->conn, client->work_buffer, packet_len))
            client->sending = 1;
}
static void ICACHE_FLASH_ATTR mqtt_callback_receive(void *arg, char *data, uint16_t data_size)
{
    if(!arg)
        return;

    struct espconn *conn = (struct espconn *)arg;

    if(!conn->reverse)
        return;

    mqtt_client_t *client = (mqtt_client_t *)conn->reverse;

    uint8_t *rx_data = data;
    uint32_t rx_data_len = data_size;

    while(rx_data_len)
    {
        uint8_t packet_type = 0;
        uint8_t flags = 0;
        uint32_t remaining_len = 0;

        if(!mqtt_proto_parse_fixed_header(&rx_data, &rx_data_len, &packet_type, &flags, &remaining_len))
            return;

        switch(packet_type)
        {
            case MQTT_PACKET_TYPE_CONNACK:
            {
                if(client->connected || !client->connecting) // Invalid CONNACK packet
                {
                    if(client->ssl)
                        espconn_secure_disconnect(client->conn);
                    else
                        espconn_disconnect(client->conn);

                    return;
                }

                uint8_t session_present = 0;

                if(!mqtt_proto_parse_uint8(&rx_data, &rx_data_len, &session_present))
                    return;

                uint8_t return_code = 0;

                if(!mqtt_proto_parse_uint8(&rx_data, &rx_data_len, &return_code))
                    return;

                client->connecting = 0;
                client->keep_alive_tick = 0;
                client->connected = (return_code == MQTT_CONNACK_STATUS_OK);

                if(client->connect_cb)
                    client->connect_cb(client, return_code);
            }
            break;
            case MQTT_PACKET_TYPE_PINGRESP:
            {
                if(!client->connected)
                    return;

                client->keep_alive_tick = 0;
            }
            break;
            case MQTT_PACKET_TYPE_SUBACK:
            {
                if(!client->connected)
                    return;

                uint16_t message_id = 0;

                if(!mqtt_proto_parse_uint16(&rx_data, &rx_data_len, &message_id))
                    return;

                uint8_t return_code = 0;

                if(!mqtt_proto_parse_uint8(&rx_data, &rx_data_len, &return_code))
                    return;

                mqtt_pending_message_t *pending_msg = mqtt_find_pending_message(client->waiting_suback, message_id);

                if(!pending_msg)
                    break;

                mqtt_remove_pending_message(&client->waiting_suback, pending_msg);

                if(client->subscribe_cb)
                    client->subscribe_cb(client, message_id, return_code);
            }
            break;
            case MQTT_PACKET_TYPE_UNSUBACK:
            {
                if(!client->connected)
                    return;

                uint16_t message_id = 0;

                if(!mqtt_proto_parse_uint16(&rx_data, &rx_data_len, &message_id))
                    return;

                mqtt_pending_message_t *pending_msg = mqtt_find_pending_message(client->waiting_unsuback, message_id);

                if(!pending_msg)
                    break;

                mqtt_remove_pending_message(&client->waiting_unsuback, pending_msg);

                if(client->unsubscribe_cb)
                    client->unsubscribe_cb(client, message_id);
            }
            break;
            case MQTT_PACKET_TYPE_PUBLISH:
            {
                if(!client->connected)
                    return;

                const char *topic = NULL;
                uint16_t topic_len = 0;

                if(!mqtt_proto_parse_string(&rx_data, &rx_data_len, &topic, &topic_len))
                    return;

                uint16_t message_id = 0;

                if(!mqtt_proto_parse_uint16(&rx_data, &rx_data_len, &message_id))
                    return;

                remaining_len -= topic_len + 4;
                rx_data_len -= remaining_len;

                if(client->data_cb && !mqtt_find_pending_message(client->waiting_pubrel, message_id))
                    client->data_cb(client, topic, topic_len, rx_data, remaining_len);

                switch(flags & (MQTT_FLAG_QOS_M << MQTT_FLAG_QOS_S))
                {
                    case MQTT_FLAG_QOS_1:
                    {
                        uint8_t *buffer = client->work_buffer;
                        uint8_t *data_buffer = buffer + 5;
                        uint32_t buffer_len = client->work_buffer_len;
                        uint32_t data_buffer_len = buffer_len - 5;

                        if(!mqtt_proto_append_uint16(&data_buffer, &data_buffer_len, message_id))
                            return;

                        if(!mqtt_proto_append_fixed_header(&buffer, &buffer_len, MQTT_PACKET_TYPE_PUBACK, 0, data_buffer - buffer - 5))
                            return;

                        if(!blob_fifo_write(client->msg_fifo, buffer, data_buffer - buffer))
                            return;

                        ets_post(MQTT_SEND_TASK_PRIO, 0, client);
                    }
                    break;
                    case MQTT_FLAG_QOS_2:
                    {
                        uint8_t *buffer = client->work_buffer;
                        uint8_t *data_buffer = buffer + 5;
                        uint32_t buffer_len = client->work_buffer_len;
                        uint32_t data_buffer_len = buffer_len - 5;

                        if(!mqtt_proto_append_uint16(&data_buffer, &data_buffer_len, message_id))
                            return;

                        if(!mqtt_proto_append_fixed_header(&buffer, &buffer_len, MQTT_PACKET_TYPE_PUBREC, 0, data_buffer - buffer - 5))
                            return;

                        if(!blob_fifo_write(client->msg_fifo, buffer, data_buffer - buffer))
                            return;

                        ets_post(MQTT_SEND_TASK_PRIO, 0, client);

                        if(!mqtt_find_pending_message(client->waiting_pubrel, message_id) && !mqtt_add_pending_message(&client->waiting_pubrel, message_id, buffer, data_buffer - buffer))
                            return;
                    }
                    break;
                }
            }
            break;
            case MQTT_PACKET_TYPE_PUBACK:
            {
                if(!client->connected)
                    return;

                uint16_t message_id = 0;

                if(!mqtt_proto_parse_uint16(&rx_data, &rx_data_len, &message_id))
                    return;

                mqtt_pending_message_t *pending_msg = mqtt_find_pending_message(client->waiting_puback, message_id);

                if(!pending_msg)
                    break;

                mqtt_remove_pending_message(&client->waiting_puback, pending_msg);

                if(client->publish_cb)
                    client->publish_cb(client, message_id);
            }
            break;
            case MQTT_PACKET_TYPE_PUBREC:
            {
                if(!client->connected)
                    return;

                uint16_t message_id = 0;

                if(!mqtt_proto_parse_uint16(&rx_data, &rx_data_len, &message_id))
                    return;

                mqtt_pending_message_t *pending_msg = mqtt_find_pending_message(client->waiting_pubrec, message_id);

                if(!pending_msg)
                    break;

                mqtt_remove_pending_message(&client->waiting_pubrec, pending_msg);

                uint8_t *buffer = client->work_buffer;
                uint8_t *data_buffer = buffer + 5;
                uint32_t buffer_len = client->work_buffer_len;
                uint32_t data_buffer_len = buffer_len - 5;

                if(!mqtt_proto_append_uint16(&data_buffer, &data_buffer_len, message_id))
                    return;

                if(!mqtt_proto_append_fixed_header(&buffer, &buffer_len, MQTT_PACKET_TYPE_PUBREL, MQTT_FLAG_QOS_1, data_buffer - buffer - 5))
                    return;

                if(!blob_fifo_write(client->msg_fifo, buffer, data_buffer - buffer))
                    return;

                ets_post(MQTT_SEND_TASK_PRIO, 0, client);

                if(!mqtt_add_pending_message(&client->waiting_pubcomp, message_id, buffer, data_buffer - buffer))
                    return;
            }
            break;
            case MQTT_PACKET_TYPE_PUBREL:
            {
                if(!client->connected)
                    return;

                uint16_t message_id = 0;

                if(!mqtt_proto_parse_uint16(&rx_data, &rx_data_len, &message_id))
                    return;

                mqtt_pending_message_t *pending_msg = mqtt_find_pending_message(client->waiting_pubrel, message_id);

                if(!pending_msg)
                    break;

                mqtt_remove_pending_message(&client->waiting_pubrel, pending_msg);

                uint8_t *buffer = client->work_buffer;
                uint8_t *data_buffer = buffer + 5;
                uint32_t buffer_len = client->work_buffer_len;
                uint32_t data_buffer_len = buffer_len - 5;

                if(!mqtt_proto_append_uint16(&data_buffer, &data_buffer_len, message_id))
                    return;

                if(!mqtt_proto_append_fixed_header(&buffer, &buffer_len, MQTT_PACKET_TYPE_PUBCOMP, 0, data_buffer - buffer - 5))
                    return;

                if(!blob_fifo_write(client->msg_fifo, buffer, data_buffer - buffer))
                    return;

                ets_post(MQTT_SEND_TASK_PRIO, 0, client);
            }
            break;
            case MQTT_PACKET_TYPE_PUBCOMP:
            {
                if(!client->connected)
                    return;

                uint16_t message_id = 0;

                if(!mqtt_proto_parse_uint16(&rx_data, &rx_data_len, &message_id))
                    return;

                mqtt_pending_message_t *pending_msg = mqtt_find_pending_message(client->waiting_pubcomp, message_id);

                if(!pending_msg)
                    break;

                mqtt_remove_pending_message(&client->waiting_pubcomp, pending_msg);

                if(client->publish_cb)
                    client->publish_cb(client, message_id);
            }
            break;
            default:
                return;
        }
    }
}
static void ICACHE_FLASH_ATTR mqtt_callback_sent(void *arg)
{
    if(!arg)
        return;

    struct espconn *conn = (struct espconn *)arg;

    if(!conn->reverse)
        return;

    mqtt_client_t *client = (mqtt_client_t *)conn->reverse;

    client->sending = 0;
    client->keep_alive_tick = 0;

    if(client->disconnecting && blob_fifo_is_empty(client->msg_fifo))
        if(client->ssl)
            espconn_secure_disconnect(client->conn);
        else
            espconn_disconnect(client->conn);

    ets_post(MQTT_SEND_TASK_PRIO, 0, client);
}
static void ICACHE_FLASH_ATTR mqtt_callback_disconnect(void *arg)
{
    if(!arg)
        return;

    struct espconn *conn = (struct espconn *)arg;

    if(!conn->reverse)
        return;

    mqtt_client_t *client = (mqtt_client_t *)conn->reverse;

    client->reconnecting = 0;
    client->connecting = 0;
    client->connected = 0;
    client->tcp_connected = 0;
    client->sending = 0;

    ets_timer_disarm(&client->timer);

    if(client->disconnect_cb)
        client->disconnect_cb(client, 0);
}
static void ICACHE_FLASH_ATTR mqtt_callback_reconnect(void *arg, int8_t err)
{
    if(!arg)
        return;

    struct espconn *conn = (struct espconn *)arg;

    if(!conn->reverse)
        return;

    mqtt_client_t *client = (mqtt_client_t *)conn->reverse;

    if(client->disconnecting)
    {
        mqtt_callback_disconnect(arg);

        return;
    }

    if(client->disconnect_cb)
        client->disconnect_cb(client, 1);

    client->reconnecting = 1;
    client->reconnect_tick = 0;

    client->connecting = 0;
    client->connected = 0;
    client->tcp_connected = 0;
    client->sending = 0;
}
static void ICACHE_FLASH_ATTR mqtt_callback_connect(void *arg)
{
    if(!arg)
        return;

    struct espconn *conn = (struct espconn *)arg;

    if(!conn->reverse)
        return;

    mqtt_client_t *client = (mqtt_client_t *)conn->reverse;

    client->reconnecting = 0;
    client->tcp_connected = 1;
    client->sending = 0;

	espconn_regist_disconcb(client->conn, mqtt_callback_disconnect);
	espconn_regist_recvcb(client->conn, mqtt_callback_receive);
    espconn_regist_sentcb(client->conn, mqtt_callback_sent);

    if(client->connect_flags & MQTT_CONNECT_FLAG_CLEAN_SESSION)
    {
        mqtt_clear_pending_messages(&client->waiting_suback);
        mqtt_clear_pending_messages(&client->waiting_unsuback);
        mqtt_clear_pending_messages(&client->waiting_puback);
        mqtt_clear_pending_messages(&client->waiting_pubrec);
        mqtt_clear_pending_messages(&client->waiting_pubrel);
        mqtt_clear_pending_messages(&client->waiting_pubcomp);
    }

    ets_post(MQTT_SEND_TASK_PRIO, 0, client);
}
static void ICACHE_FLASH_ATTR mqtt_callback_dns_resolve(const char *name, ip_addr_t *ip, void *arg)
{
    if(!arg)
        return;

    struct espconn *conn = (struct espconn *)arg;

    if(!conn->reverse)
        return;

    mqtt_client_t *client = (mqtt_client_t *)conn->reverse;

    if (!ip)
	{
        if(client->connect_cb)
            client->connect_cb(client, MQTT_CONNACK_STATUS_DNS_ERROR);

		return;
	}

	if(name)
		ets_memcpy(client->conn->proto.tcp->remote_ip, ip, sizeof(ip_addr_t));

	espconn_regist_connectcb(client->conn, mqtt_callback_connect);
	espconn_regist_reconcb(client->conn, mqtt_callback_reconnect);

    client->connecting = 1;
    client->connect_tick = 0;

    if(client->ssl)
        espconn_secure_connect(client->conn);
    else
        espconn_connect(client->conn);
}

mqtt_client_t* mqtt_init(const char *client_id, const char *host, const uint16_t port, const uint8_t ssl, const uint16_t keep_alive, uint8_t clean_session)
{
    if(!host || !port)
        return NULL;

    mqtt_client_t *client = (mqtt_client_t *)ets_zalloc(sizeof(mqtt_client_t));

    if(!client)
    	return NULL;

    if(client_id)
    {
        uint32_t client_id_len = ets_strlen(client_id);

        if(client_id_len)
        {
            client->client_id = (char *)ets_zalloc(client_id_len + 1);

            if(!client->client_id)
            {
                ets_free(client);

                return NULL;
            }

            ets_memcpy(client->client_id, client_id, client_id_len);

            client->client_id[client_id_len] = 0;
        }
        else
        {
            clean_session = 1;
        }
    }
    else
    {
        clean_session = 1;
    }

    uint32_t host_len = ets_strlen(host);

    if(!host_len)
    {
        ets_free(client->client_id);
        ets_free(client);

        return NULL;
    }

    client->host = (char *)ets_zalloc(host_len + 1);

    if(!client->host)
    {
        ets_free(client->client_id);
        ets_free(client);

        return NULL;
    }

    ets_memcpy(client->host, host, host_len);

    client->host[host_len] = 0;

    client->port = port;
    client->ssl = !!ssl;
    client->keep_alive = keep_alive;

    if(clean_session)
        client->connect_flags |= MQTT_CONNECT_FLAG_CLEAN_SESSION;

    client->work_buffer = (uint8_t *)ets_zalloc(MQTT_WORK_BUFFER_SIZE);

    if(!client->work_buffer)
    {
        ets_free(client->client_id);
        ets_free(client->host);
        ets_free(client);

        return NULL;
    }

    client->work_buffer_len = MQTT_WORK_BUFFER_SIZE;

    ets_task(mqtt_task_handler, MQTT_SEND_TASK_PRIO, mqtt_task_queue, MQTT_SEND_TASK_SIZE);

    return client;
}
uint8_t mqtt_delete(mqtt_client_t *client)
{
    if(!client)
        return 0;

    ets_timer_disarm(&client->timer);

    mqtt_clear_pending_messages(&client->waiting_suback);
    mqtt_clear_pending_messages(&client->waiting_unsuback);
    mqtt_clear_pending_messages(&client->waiting_puback);
    mqtt_clear_pending_messages(&client->waiting_pubrec);
    mqtt_clear_pending_messages(&client->waiting_pubrel);
    mqtt_clear_pending_messages(&client->waiting_pubcomp);

    blob_fifo_delete(client->msg_fifo);

    if(client->conn)
        ets_free(client->conn->proto.tcp);

    ets_free(client->conn);
    ets_free(client->work_buffer);
    ets_free(client->username);
    ets_free(client->password);
    ets_free(client->will_topic);
    ets_free(client->will_message);
    ets_free(client->client_id);
    ets_free(client->host);
    ets_free(client);

    return 1;
}
uint8_t mqtt_set_auth(mqtt_client_t *client, const char *username, const char *password)
{
    if(!client)
        return 0;

    ets_free(client->username);
    ets_free(client->password);

    client->username = NULL;
    client->password = NULL;
    client->connect_flags &= ~(MQTT_CONNECT_FLAG_USERNAME | MQTT_CONNECT_FLAG_PASSWORD);

    if(!username)
        return 1;

    uint32_t username_len = ets_strlen(username);

    if(!username_len)
        return 0;

    client->username = (char *)ets_zalloc(username_len + 1);

    if(!client->username)
        return 0;

    ets_memcpy(client->username, username, username_len);

    client->connect_flags |= MQTT_CONNECT_FLAG_USERNAME;

    if(!password)
        return 1;

    uint32_t password_len = ets_strlen(password);

    if(!password_len)
        return 0;

    client->password = (char *)ets_zalloc(password_len + 1);

    if(!client->password)
        return 0;

    ets_memcpy(client->password, password, password_len);

    client->connect_flags |= MQTT_CONNECT_FLAG_PASSWORD;

    return 1;
}
uint8_t mqtt_set_lwt(mqtt_client_t *client, const char *topic, const void *data, const uint8_t qos, const uint8_t retain)
{
    if(!client)
        return 0;

    ets_free(client->will_topic);
    ets_free(client->will_message);

    client->will_topic = NULL;
    client->will_message = NULL;
    client->connect_flags &= ~(MQTT_CONNECT_FLAG_WILL | (MQTT_CONNECT_FLAG_WILL_QOS_M << MQTT_CONNECT_FLAG_WILL_QOS_S) | MQTT_CONNECT_FLAG_WILL_RETAIN);

    if(!topic)
        return 1;

    if(!data)
        return 0;

    uint32_t topic_len = ets_strlen(topic);

    if(!topic_len)
        return 0;

    client->will_topic = (char *)ets_zalloc(topic_len + 1);

    if(!client->will_topic)
        return 0;

    ets_memcpy(client->will_topic, topic, topic_len);

    uint32_t data_len = ets_strlen(data);

    if(!data_len)
    {
        ets_free(client->will_topic);

        return 0;
    }

    client->will_message = (char *)ets_zalloc(data_len + 1);

    if(!client->will_message)
    {
        ets_free(client->will_topic);

        return 0;
    }

    ets_memcpy(client->will_message, data, data_len);

    client->connect_flags |= MQTT_CONNECT_FLAG_WILL | ((qos & MQTT_CONNECT_FLAG_WILL_QOS_M) << MQTT_CONNECT_FLAG_WILL_QOS_S) | (retain ? MQTT_CONNECT_FLAG_WILL_RETAIN : 0);

    return 1;
}
uint8_t mqtt_set_connect_callback(mqtt_client_t *client, mqtt_conn_callback_fn_t fn)
{
    if(!client)
        return 0;

    client->connect_cb = fn;

    return 1;
}
uint8_t mqtt_set_disconnect_callback(mqtt_client_t *client, mqtt_disconn_callback_fn_t fn)
{
    if(!client)
        return 0;

    client->disconnect_cb = fn;

    return 1;
}
uint8_t mqtt_set_subscribe_callback(mqtt_client_t *client, mqtt_sub_callback_fn_t fn)
{
    if(!client)
        return 0;

    client->subscribe_cb = fn;

    return 1;
}
uint8_t mqtt_set_unsubscribe_callback(mqtt_client_t *client, mqtt_msg_callback_fn_t fn)
{
    if(!client)
        return 0;

    client->unsubscribe_cb = fn;

    return 1;
}
uint8_t mqtt_set_publish_callback(mqtt_client_t *client, mqtt_msg_callback_fn_t fn)
{
    if(!client)
        return 0;

    client->publish_cb = fn;

    return 1;
}
uint8_t mqtt_set_timeout_callback(mqtt_client_t *client, mqtt_timeout_callback_fn_t fn)
{
    if(!client)
        return 0;

    client->timeout_cb = fn;

    return 1;
}
uint8_t mqtt_set_data_callback(mqtt_client_t *client, mqtt_data_callback_fn_t fn)
{
    if(!client)
        return 0;

    client->data_cb = fn;

    return 1;
}
uint8_t mqtt_connect(mqtt_client_t *client)
{
    if(!client)
        return 0;

    if(!client->host || !client->port)
        return 0;

    if(!client->work_buffer || !client->work_buffer_len)
        return 0;

    if(client->conn)
        ets_free(client->conn->proto.tcp);

    ets_free(client->conn);

    blob_fifo_delete(client->msg_fifo);

    client->msg_fifo = blob_fifo_init(NULL, MQTT_QUEUE_FIFO_SIZE);

    if(!client->msg_fifo)
        return 0;

    client->conn = (struct espconn *)ets_zalloc(sizeof(struct espconn));

    if(!client->conn)
        return 0;

    client->conn->proto.tcp = (esp_tcp *)ets_zalloc(sizeof(esp_tcp));

    if (!client->conn->proto.tcp)
        return 0;

	client->conn->type = ESPCONN_TCP;
	client->conn->state = ESPCONN_NONE;
    client->conn->reverse = client;
	client->conn->proto.tcp->local_port = espconn_port();
	client->conn->proto.tcp->remote_port = client->port;
	*(uint32_t*)client->conn->proto.tcp->remote_ip = 0;

    uint8_t *buffer = client->work_buffer;
    uint8_t *data_buffer = buffer + 5;
    uint32_t buffer_len = client->work_buffer_len;
    uint32_t data_buffer_len = buffer_len - 5;

    if(!mqtt_proto_append_string(&data_buffer, &data_buffer_len, MQTT_CONNECT_PROTOCOL_NAME))
        return 0;

    if(!mqtt_proto_append_uint8(&data_buffer, &data_buffer_len, MQTT_CONNECT_PROTOCOL_VERSION))
        return 0;

    if(!mqtt_proto_append_uint8(&data_buffer, &data_buffer_len, client->connect_flags))
        return 0;

    if(!mqtt_proto_append_uint16(&data_buffer, &data_buffer_len, client->keep_alive))
        return 0;

    if(!mqtt_proto_append_string(&data_buffer, &data_buffer_len, client->client_id))
        return 0;

    if(client->connect_flags & MQTT_CONNECT_FLAG_WILL)
    {
        if(!mqtt_proto_append_string(&data_buffer, &data_buffer_len, client->will_topic))
            return 0;

        if(!mqtt_proto_append_string(&data_buffer, &data_buffer_len, client->will_message))
            return 0;
    }

    if(client->connect_flags & MQTT_CONNECT_FLAG_USERNAME)
    {
        if(!mqtt_proto_append_string(&data_buffer, &data_buffer_len, client->username))
            return 0;
    }

    if(client->connect_flags & MQTT_CONNECT_FLAG_PASSWORD)
    {
        if(!mqtt_proto_append_string(&data_buffer, &data_buffer_len, client->password))
            return 0;
    }

    if(!mqtt_proto_append_fixed_header(&buffer, &buffer_len, MQTT_PACKET_TYPE_CONNECT, 0, data_buffer - buffer - 5))
        return 0;

    if(!blob_fifo_write(client->msg_fifo, buffer, data_buffer - buffer))
        return 0;

    client->disconnecting = 0;

    err_t dns_result = espconn_gethostbyname(client->conn, client->host, (ip_addr_t*)client->conn->proto.tcp->remote_ip, mqtt_callback_dns_resolve);

    if (dns_result == ESPCONN_OK)
	{
		mqtt_callback_dns_resolve(NULL, (ip_addr_t*)client->conn->proto.tcp->remote_ip, client->conn);
	}
	else if (dns_result != ESPCONN_INPROGRESS)
    {
		return 0;
	}

	ets_timer_disarm(&client->timer);
	ets_timer_setfn(&client->timer, mqtt_timer_callback, client);
	ets_timer_arm_new(&client->timer, 1000, 1, 1);

    return 1;
}
uint8_t mqtt_disconnect(mqtt_client_t *client)
{
    if(!client)
        return 0;

    if(!client->work_buffer || !client->work_buffer_len)
        return 0;

    if(client->connected)
    {
        uint8_t *buffer = client->work_buffer;
        uint8_t *data_buffer = buffer + 5;
        uint32_t buffer_len = client->work_buffer_len;
        uint32_t data_buffer_len = buffer_len - 5;

        if(!mqtt_proto_append_fixed_header(&buffer, &buffer_len, MQTT_PACKET_TYPE_DISCONNECT, 0, data_buffer - buffer - 5))
            return 0;

        if(!blob_fifo_write(client->msg_fifo, buffer, data_buffer - buffer))
            return 0;

        ets_post(MQTT_SEND_TASK_PRIO, 0, client);
    }
    else
    {
        if(client->ssl)
            espconn_secure_disconnect(client->conn);
        else
            espconn_disconnect(client->conn);
    }

    client->disconnecting = 1;

    return 1;
}
uint8_t mqtt_subscribe(mqtt_client_t *client, const char *topic, const uint8_t qos)
{
    if(!client)
        return 0;

    if(!client->work_buffer || !client->work_buffer_len)
        return 0;

    if(!client->tcp_connected)
        return 0;

    uint8_t *buffer = client->work_buffer;
    uint8_t *data_buffer = buffer + 5;
    uint32_t buffer_len = client->work_buffer_len;
    uint32_t data_buffer_len = buffer_len - 5;

    uint16_t message_id = mqtt_get_next_message_id(client);

    if(!mqtt_proto_append_uint16(&data_buffer, &data_buffer_len, message_id))
        return 0;

    if(!mqtt_proto_append_string(&data_buffer, &data_buffer_len, topic))
        return 0;

    if(!mqtt_proto_append_uint8(&data_buffer, &data_buffer_len, qos & MQTT_FLAG_QOS_M))
        return 0;

    if(!mqtt_proto_append_fixed_header(&buffer, &buffer_len, MQTT_PACKET_TYPE_SUBSCRIBE, MQTT_FLAG_QOS_1, data_buffer - buffer - 5))
        return 0;

    if(!blob_fifo_write(client->msg_fifo, buffer, data_buffer - buffer))
        return 0;

    ets_post(MQTT_SEND_TASK_PRIO, 0, client);

    if(!mqtt_add_pending_message(&client->waiting_suback, message_id, buffer, data_buffer - buffer))
        return 0;

    return 1;
}
uint8_t mqtt_unsubscribe(mqtt_client_t *client, const char *topic)
{
    if(!client)
        return 0;

    if(!client->work_buffer || !client->work_buffer_len)
        return 0;

    if(!client->tcp_connected)
        return 0;

    uint8_t *buffer = client->work_buffer;
    uint8_t *data_buffer = buffer + 5;
    uint32_t buffer_len = client->work_buffer_len;
    uint32_t data_buffer_len = buffer_len - 5;

    uint16_t message_id = mqtt_get_next_message_id(client);

    if(!mqtt_proto_append_uint16(&data_buffer, &data_buffer_len, message_id))
        return 0;

    if(!mqtt_proto_append_string(&data_buffer, &data_buffer_len, topic))
        return 0;

    if(!mqtt_proto_append_fixed_header(&buffer, &buffer_len, MQTT_PACKET_TYPE_UNSUBSCRIBE, MQTT_FLAG_QOS_1, data_buffer - buffer - 5))
        return 0;

    if(!blob_fifo_write(client->msg_fifo, buffer, data_buffer - buffer))
        return 0;

    ets_post(MQTT_SEND_TASK_PRIO, 0, client);

    if(!mqtt_add_pending_message(&client->waiting_unsuback, message_id, buffer, data_buffer - buffer))
        return 0;

    return 1;
}
uint8_t mqtt_publish(mqtt_client_t *client, const char *topic, const void *data, const uint32_t data_length, const uint8_t qos, const uint8_t retain)
{
    if(!client)
        return 0;

    if(!client->work_buffer || !client->work_buffer_len)
        return 0;

    if(!client->tcp_connected)
        return 0;

    uint8_t *buffer = client->work_buffer;
    uint8_t *data_buffer = buffer + 5;
    uint32_t buffer_len = client->work_buffer_len;
    uint32_t data_buffer_len = buffer_len - 5;

    uint16_t message_id = mqtt_get_next_message_id(client);

    if(!mqtt_proto_append_string(&data_buffer, &data_buffer_len, topic))
        return 0;

    if(!mqtt_proto_append_uint16(&data_buffer, &data_buffer_len, message_id))
        return 0;

    if(!mqtt_proto_append_blob(&data_buffer, &data_buffer_len, (const uint8_t *)data, data_length))
        return 0;

    if(!mqtt_proto_append_fixed_header(&buffer, &buffer_len, MQTT_PACKET_TYPE_PUBLISH, ((qos & MQTT_FLAG_QOS_M) << MQTT_FLAG_QOS_S) | (!!retain ? MQTT_FLAG_RETAIN : 0), data_buffer - buffer - 5))
        return 0;

    if(!blob_fifo_write(client->msg_fifo, buffer, data_buffer - buffer))
        return 0;

    ets_post(MQTT_SEND_TASK_PRIO, 0, client);

    switch(qos & MQTT_FLAG_QOS_M)
    {
        case 0:
            if(client->publish_cb)
                client->publish_cb(client, 0);
        break;
        case 1:
            if(!mqtt_add_pending_message(&client->waiting_puback, message_id, buffer, data_buffer - buffer))
                return 0;
        break;
        case 2:
            if(!mqtt_add_pending_message(&client->waiting_pubrec, message_id, buffer, data_buffer - buffer))
                return 0;
        break;
    }

    return 1;
}
uint8_t mqtt_ping(mqtt_client_t *client)
{
    if(!client)
        return 0;

    if(!client->work_buffer || !client->work_buffer_len)
        return 0;

    if(!client->tcp_connected)
        return 0;

    uint8_t *buffer = client->work_buffer;
    uint8_t *data_buffer = buffer + 5;
    uint32_t buffer_len = client->work_buffer_len;
    uint32_t data_buffer_len = buffer_len - 5;

    if(!mqtt_proto_append_fixed_header(&buffer, &buffer_len, MQTT_PACKET_TYPE_PINGREQ, 0, data_buffer - buffer - 5))
        return 0;

    if(!blob_fifo_write(client->msg_fifo, buffer, data_buffer - buffer))
        return 0;

    ets_post(MQTT_SEND_TASK_PRIO, 0, client);

    return 1;
}
