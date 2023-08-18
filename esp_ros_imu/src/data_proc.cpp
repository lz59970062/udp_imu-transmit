#include "data_proc.h"
#include "JY901.h"
#define measeureF 0
xSemaphoreHandle xSemaphore_data_send = NULL;

Remote_node *node;
typedef struct
{
    uint8_t head, type;
    uint32_t time;
    float ax,ay,az;
    float gx,gy,gz;
    float q0,q1,q2,q3;
    uint32_t state_flags;
    uint32_t check;
} robot_state_data_t;
const size_t robot_data_size = sizeof(robot_state_data_t);
robot_state_data_t robot_state_data;

uint32_t check_sum(uint8_t *data, uint8_t len)
{
    uint32_t sum = 0;
    for (int i = 0; i < len; i++)
    {
        sum += data[i];
    }
    return sum;
}
void data_proc_cb(AsyncUDPPacket *packet)
{
    if (packet->length() > 0)
    {
         
    }
}

void task_data_proc(void *pvParameter)
{

    TickType_t lasttick = xTaskGetTickCount();
#if measeureF
    int tick = 0, last_time, freq;
#endif
    while (1)
    {
        vTaskDelayUntil(&lasttick, 5);

        if (node == NULL)
        {

            node = udp.get_node("process_node");
            if (node != NULL)
            {
                Serial.println("[udpnode] node found");
                node->inited = 1;
                // udp.adversting = 0;
                // udp.node_finding = 0;
                node->data_cb = data_proc_cb;
            }
            else
            {
                Serial.println("[udpnode]node not found");
            }
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }

        if (node != NULL && node->inited == 0)
        {
            node->inited = 1;
            node->data_cb = data_proc_cb;
            vTaskDelay(2 / portTICK_PERIOD_MS);
        }
        else if (node != NULL && node->inited == 1)
        {
            xSemaphoreTake(xSemaphore_data_send, portMAX_DELAY);
            robot_state_data.head = 0x55;
            robot_state_data.type = 0x01;
            robot_state_data.ax=imu.accx;
            robot_state_data.ay=imu.accy;
            robot_state_data.az=imu.accz;
            robot_state_data.gx=imu.gyrox;
            robot_state_data.gy=imu.gyroy;
            robot_state_data.gz=imu.gyroz;
            imu.getQ(&robot_state_data.q0,&robot_state_data.q1,&robot_state_data.q2,&robot_state_data.q3);
            robot_state_data.state_flags = 0;
            robot_state_data.check = check_sum((uint8_t *)&robot_state_data, robot_data_size - 4);
            node->send_data((uint8_t *)&robot_state_data, robot_data_size);
#if measeureF
            if (millis() - last_time > 1000)
            {
                freq = tick;

                Serial.printf("freq:%d\n", freq);
                tick = 0;
                last_time = millis();
            }
            tick++;
#endif
            xSemaphoreGive(xSemaphore_data_send);
            // node->send_data(data_str);
            // vTaskDelay(5 / portTICK_PERIOD_MS);
        }
    }
}

void tata_proc_init()
{
    xSemaphore_data_send = xSemaphoreCreateMutex();
    xTaskCreatePinnedToCore(task_data_proc,
                            "data_proc", 2048 * 6, NULL, 5, NULL, 0);
}
