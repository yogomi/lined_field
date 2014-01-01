#include <iostream>
#include <math.h>
#include <stdlib.h>

#include <time.h>
#include <sys/time.h>

#include "Leap.h"

#include "Quaternion.h"
#include "pen_line.h"
#include "hand_input_listener.h"

using namespace Leap;

namespace hand_listener{

void HandInputListener::onInit(const Controller& controller)
{
  memset(tracing_object_ids, 0, sizeof(int) * MAX_TRACABLE_POINT_COUNT);
  memset(traceline_counters, 0, sizeof(int) * MAX_TRACABLE_POINT_COUNT);
  previous_position = Vector(0,0,0);
  rotating = false;
  world_x_quaternion = Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
  world_y_quaternion = Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
  camera_x_position = DEFAULT_CAMERA_X;
  camera_y_position = DEFAULT_CAMERA_Y;
  camera_z_position = DEFAULT_CAMERA_Z;
}

void HandInputListener::onFrame(const Controller& controller) {
  const Frame frame = controller.frame();
  int open_hand_id = open_hand_id_(frame);
  if (frame.hands().isEmpty()) {
    memset(traceline_counters, 0, sizeof(int) * MAX_TRACABLE_POINT_COUNT);
  } else if (open_hand_id < 0) {
    for (int i=0; i<frame.hands().count(); i++) {
      trace_finger_(frame.hands()[i]);
    }
  } else {
    rotate_camera_(frame.hand(open_hand_id));
  }
  gettimeofday(&time_buffer, NULL);
}

void HandInputListener::initialize_world_position() {
  camera_x_position = DEFAULT_CAMERA_X;
  camera_y_position = DEFAULT_CAMERA_Y;
  camera_z_position = DEFAULT_CAMERA_Z;
  world_x_quaternion = Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
  world_y_quaternion = Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
}

Vector HandInputListener::convert_to_world_position(const Vector &input_vector) {
  Quaternion q(0.0f
      , input_vector.x - camera_x_position + DEFAULT_CAMERA_X
      , input_vector.y + camera_y_position - DEFAULT_CAMERA_Y
      , input_vector.z + camera_z_position - DEFAULT_CAMERA_Z);
  q = conj(world_x_quaternion) * q * world_x_quaternion;
  q = conj(world_y_quaternion) * q * world_y_quaternion;
  Vector v(q[1], q[2], q[3]);
  return v;
}

int HandInputListener::open_hand_id_(const Frame& frame) {
  for (int i=0; i<frame.hands().count(); i++) {
    if (frame.hands()[i].pointables().count() > 3) {
      return frame.hands()[i].id();
    }
  }
  return -1;
}

void HandInputListener::trace_finger_(const Hand& hand) {
  struct timeval now;
  gettimeofday(&now, NULL);
  const PointableList pointables = hand.pointables();
  const Pointable tracing_object = hand.pointable(tracing_object_ids[0]);
  rotating = false;
  if (tracing_object.isValid()) {
    Vector tip_position = tracing_object.tipPosition();
    tip_position = convert_to_world_position(tip_position);
    if (traceline_counters[0] > 5){
      if (tip_position.distanceTo(previous_position) > 1) {
        previous_position = tip_position;
        penline_list.begin()->push_back(tip_position);
      }
    } else {
      if (timercmp(&now, &time_buffer, >)) {
        if (tip_position.distanceTo(previous_position) < 10) {
          previous_position = tip_position;
          ++traceline_counters[0];
          if(traceline_counters[0] == 6){
            pen_line::Line start_point;
            std::cout << random() % 11 << std::endl;
            start_point.push_back(Vector((random() % 11) / 10.0f
                  , (random() % 11) / 10.0f
                  , (random() % 11) / 10.0f));
            start_point.push_back(tip_position);
            penline_list.push_front(start_point);
          }
        } else {
          previous_position = tip_position;
          traceline_counters[0] = 0;
        }
        gettimeofday(&time_buffer, NULL);
        time_buffer.tv_usec += 30 * 1000;
      }
    }
  } else {
    traceline_counters[0] = 0;
    gettimeofday(&time_buffer, NULL);
    time_buffer.tv_usec += 100 * 1000;
    tracing_object_ids[0] = pointables[0].id();
    if(pointables[0].isValid()){
      previous_position = convert_to_world_position(tracing_object.tipPosition());
    }
  }
}

void HandInputListener::rotate_camera_(const Hand& hand) {
  traceline_counters[0] = 0;
  const Vector parm_position = hand.palmPosition();
  if (!rotating) {
    rotating = true;
    previous_position = parm_position;
  } else if (parm_position.distanceTo(previous_position) > 0.3) {
    Vector move_vector(parm_position - previous_position);
    float hard = move_vector.x / 200;
    float s = sin(hard);
    Quaternion rotate_quaternion(cos(hard)
        , 0*s
        , 1*s
        , 0*s);
    world_y_quaternion = world_y_quaternion * rotate_quaternion;
    hard = move_vector.y / 200;
    s = sin(hard);
    rotate_quaternion = Quaternion(cos(hard)
        , 1*s
        , 0*s
        , 0*s);
    world_x_quaternion = world_x_quaternion * rotate_quaternion;
    camera_z_position += move_vector.z * 6;
    previous_position = parm_position;
  }
}

} // namespace hand_listener
