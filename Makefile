UNAME = $(shell uname)

ifeq ($(UNAME),Linux)
LDFLAGS=-lglut -lGLU -lGL -lm
endif
ifeq ($(UNAME),Darwin)
INCLUDES=-I../../sdks/leap_sdk/LeapSDK/include
LDFLAGS=-framework OpenGL -framework GLUT ../../sdks/leap_sdk/LeapSDK/lib/libLeap.dylib
endif

TARGET=lined_field

OBJS = main.o field_line.o hand_input_listener.o pen_line.o Quaternion.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -v -o ${TARGET} $(LDFLAGS) $(OBJS)
ifeq ($(UNAME), Darwin)
	install_name_tool -change @loader_path/libLeap.dylib ../../sdks/leap_sdk/LeapSDK/lib/libLeap.dylib $@
endif

.cc.o:
	$(CXX) ${INCLUDES} -c $< -o $@

clean:
	rm -f ${TARGET}
	rm -f ${OBJS}


PHONY: all clean
