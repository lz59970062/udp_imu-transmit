#include "data_proc.h"
 
#define measeureF 0
xSemaphoreHandle xSemaphore_data_send = NULL;

Remote_node *node;
typedef struct
{
    uint8_t head, type;
    uint32_t time;
    float ax, ay, az;
    float gx, gy, gz;
    float q0, q1, q2, q3;
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
    // Serial.printf("data_proc_cb:%d,expect:%d\n", packet->length(), robot_data_size);
    if (packet->length() == robot_data_size)
    {
        xSemaphoreTake(xSemaphore_data_send, portMAX_DELAY);
        memcpy(&robot_state_data, packet->data(), robot_data_size);
        xSemaphoreGive(xSemaphore_data_send);
        Serial.printf("Q0:%.2f,Q1:%.2f,Q2:%.2f,Q3:%.2f\n", robot_state_data.q0, robot_state_data.q1, robot_state_data.q2, robot_state_data.q3);
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

            node = udp.get_node("imu_node");
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
                udp.print_node_list();
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
        
        }
    }
}

void tata_proc_init()
{
    xSemaphore_data_send = xSemaphoreCreateMutex();
    xTaskCreatePinnedToCore(task_data_proc,
                            "data_proc", 2048 * 6, NULL, 5, NULL, 0);
}
