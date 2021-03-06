#ifndef HAND_DRIVER_H
#define HAND_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hand/hand.h"

typedef struct {
    bool object_present[4];
    bool object_color[4];
} hand_sensors_t;

void hand_driver_set_fingers(const char *hand_id, finger_state_t* status);

void hand_driver_set_right_fingers(finger_state_t* status);
void hand_driver_set_left_fingers(finger_state_t* status);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <uavcan/uavcan.hpp>
int hand_driver_init(uavcan::INode &node);
#endif

#endif /* HAND_DRIVER_H */
