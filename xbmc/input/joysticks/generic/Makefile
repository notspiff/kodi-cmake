SRCS=GenericJoystickActionHandler.cpp \
     GenericJoystickButtonMapper.cpp \
     GenericJoystickInputHandler.cpp \
     GenericJoystickMultiPressDetector.cpp \
     GenericRawAxisInputHandler.cpp \
     GenericRawButtonInputHandler.cpp \
     GenericRawHatInputHandler.cpp \

LIB=input_joysticks_generic.a

include ../../../../Makefile.include
-include $(patsubst %.cpp,%.P,$(patsubst %.c,%.P,$(SRCS)))
