#include <iostream>
#include <math.h>
#include <stdlib.h>

#include <time.h>
#include <sys/time.h>

#include "Leap.h"

#include "Quaternion.h"
#include "pen_line.h"
#include "hand_input_controller.h"

namespace hand_controller{

HandInputController::HandInputController()
: tracing_object_id(0)
, previous_position(0,0,0)
, traceline_counter(0)
, rotating(false)
, transfarring(false)
, world_x_quaternion(1.0f, 0.0f, 0.0f, 0.0f)
, world_y_quaternion(1.0f, 0.0f, 0.0f, 0.0f)
, camera_x_position(DEFAULT_CAMERA_X)
, camera_y_position(DEFAULT_CAMERA_Y)
, camera_z_position(DEFAULT_CAMERA_Z)
{
}

HandInputController::~HandInputController() {
}

void HandInputController::process_input() {
  const Leap::Frame frame = controller.frame();
  struct timeval now;
  gettimeofday(&now, NULL);
  if (frame.hands().isEmpty()) {
    rotating = false;
    transfarring = false;
    traceline_counter = 0;
  } else if (frame.hands().count() == 1
      || (frame.hands().count() > 1
        && frame.hands()[0].pointables().count() < 3
        && frame.hands()[1].pointables().count() < 3)) {
    const Leap::Hand hand = frame.hands()[0];
    const Leap::PointableList pointables = hand.pointables();
    if (pointables.count() <= 2) {
      const Leap::Pointable tracing_object = hand.pointable(tracing_object_id);
      if (tracing_object.isValid()) {
          rotating = false;
          transfarring = false;
          Leap::Vector tip_position = tracing_object.tipPosition();
          tip_position = convert_to_world_position(tip_position);
          if (traceline_counter > 5){
            if (tip_position.distanceTo(previous_position) > 1) {
              previous_position = tip_position;
              penline_list.begin()->push_back(tip_position);
            }
          } else {
            if (timercmp(&now, &time_buffer, >)) {
              if (tip_position.distanceTo(previous_position) < 10) {
                previous_position = tip_position;
                ++traceline_counter;
                if(traceline_counter == 6){
                  pen_line::Line start_point;
                  std::cout << random() % 11 << std::endl;
                  start_point.push_back(Leap::Vector((random() % 11) / 10.0f
                        , (random() % 11) / 10.0f
                        , (random() % 11) / 10.0f));
                  start_point.push_back(tip_position);
                  penline_list.push_front(start_point);
                }
              } else {
                previous_position = tip_position;
                traceline_counter = 0;
              }
              gettimeofday(&time_buffer, NULL);
              time_buffer.tv_usec += 30 * 1000;
            }
          }
      } else {
        traceline_counter = 0;
        gettimeofday(&time_buffer, NULL);
        time_buffer.tv_usec += 100 * 1000;
        tracing_object_id = pointables[0].id();
        if(pointables[0].isValid()){
          previous_position = convert_to_world_position(tracing_object.tipPosition());
        }
      }
    } else if (pointables.count() > 2) {
      traceline_counter = 0;
      const Leap::Vector parm_position = hand.palmPosition();
      if (!rotating) {
        rotating = true;
        previous_position = parm_position;
      } else if (parm_position.distanceTo(previous_position) > 0.3) {
        Leap::Vector move_vector(parm_position - previous_position);
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
  } else {
    rotating = false;
    traceline_counter = 0;
    const Leap::Hand hand1 = frame.hands()[0];
    const Leap::Hand hand2 = frame.hands()[1];
    const Leap::Vector hand_position1 = hand1.palmPosition();
    const Leap::Vector hand_position2 = hand2.palmPosition();
    Leap::Vector center_position = (hand_position1 + hand_position2) / 2;
    if (hand_position1.isValid() && hand_position2.isValid()
        && hand1.fingers().count() > 3 && hand2.fingers().count() > 3) {
      if (!transfarring) {
        transfarring = true;
        previous_position = center_position;
      } else if (center_position.distanceTo(previous_position) > 0.2) {
        Leap::Vector move_vector(center_position - previous_position);
        move_vector = move_vector * 6;
        camera_x_position += move_vector.x;
        camera_y_position += move_vector.y;
        camera_z_position += move_vector.z;
        previous_position = center_position;
      }
    }
  }
  gettimeofday(&time_buffer, NULL);
}

void HandInputController::initialize_world_position() {
  camera_x_position = DEFAULT_CAMERA_X;
  camera_y_position = DEFAULT_CAMERA_Y;
  camera_z_position = DEFAULT_CAMERA_Z;
  world_x_quaternion = Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
  world_y_quaternion = Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
}

Leap::Vector HandInputController::convert_to_world_position(const Leap::Vector &input_vector) {
  Quaternion q(0.0f
      , input_vector.x - camera_x_position + DEFAULT_CAMERA_X
      , input_vector.y + camera_y_position - DEFAULT_CAMERA_Y
      , input_vector.z + camera_z_position - DEFAULT_CAMERA_Z);
  q = conj(world_x_quaternion) * q * world_x_quaternion;
  q = conj(world_y_quaternion) * q * world_y_quaternion;
  Leap::Vector v(q[1], q[2], q[3]);
  return v;
}


}
