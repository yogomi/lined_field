#ifndef HAND_INPUT_LISTENER_H_
#define HAND_INPUT_LISTENER_H_

#include "Leap.h"

#include "Quaternion.h"
#include "pen_line.h"

#define DEFAULT_CAMERA_X 0
#define DEFAULT_CAMERA_Y 300
#define DEFAULT_CAMERA_Z 600
#define MAX_TRACABLE_POINT_COUNT 10

using namespace Leap;

namespace hand_listener{

class HandInputListener : public Listener {
public:
  virtual void onInit(const Controller& controller);
  virtual void onFrame(const Controller& controller);
  void initialize_world_position();
  Vector convert_to_world_position(const Vector &input_vector);

  int tracing_object_ids[MAX_TRACABLE_POINT_COUNT];
  pen_line::LineList penline_list;
  struct timeval time_buffer;
  Vector previous_position;
  int traceline_counters[MAX_TRACABLE_POINT_COUNT];
  bool rotating;
  Quaternion world_x_quaternion;
  Quaternion world_y_quaternion;

  float camera_x_position;
  float camera_y_position;
  float camera_z_position;

private:
  int open_hand_id_(const Frame& frame);
};

}

#endif // HAND_INPUT_LISTENER_H_
