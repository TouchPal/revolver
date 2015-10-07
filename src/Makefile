ifndef CXX
CXX=g++
endif

ifndef CC
CC=gcc
endif

ifndef LD
LD=g++
endif

CXXFLAGS=-std=gnu++0x -g -fPIC -L../lib/lib
LDFLAGS=-g -fPIC -L../lib/lib
TESTLDFLAGS=-g -std=gnu++0x -fPIC -L../lib/lib

ifdef DEBUG
CXXFLAGS += -O0
LDFLAGS += -O0
TESTLDFLAGS += -O0
else
CXXFLAGS += -O3
LDFLAGS += -O3
TESTLDFLAGS += -O3
endif

LIBRARIES= -lboost_thread -lboost_chrono -lboost_system -lboost_log -lboost_log_setup -lpthread -lev -lstdc++ 
TEST_LIBRARIES= -lboost_unit_test_framework -lboost_thread -lboost_chrono -lboost_system -lboost_log -lboost_filesystem -lboost_log_setup -lpthread -lev -lstdc++ 

INCLUDES= -I../lib/include  

PROTO_PATH=../proto

CXXFLAGS += $(INCLUDES)

TARGET=../bin/postman

ALL_TEST = revolver_test



.PHONY:all
all:$(TARGET)

.PHONY:test
test:$(ALL_TEST)

.PHONY:tools
tools:$(ALL_TOOL)

OBJS= \
	  thread_class.o \



PBOBJS=\

OBJS += $(PBOBJS)

TEST_OBJS = \
			test.o \
			test_thread_class.o \

TOOL_OBJS = \
          
$(TARGET):main.o $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBRARIES)

$(ALL_TEST):$(TEST_OBJS) $(OBJS)
	$(CC) $(TESTLDFLAGS) -o $@ $^ $(TEST_LIBRARIES) 

%.pb.cc %.pb.h:$(PROTO_PATH)/%.proto
	../lib/bin/protoc -I$(PROTO_PATH) --cpp_out=. $^

%.pb.d:%.pb.cc
	@$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.d:%.cpp
	@$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

sinclude $(TEST_OBJS:.o=.d)
sinclude $(OBJS:.o=.d)
sinclude $(TOOL_OBJS:.o=.d)

.PHONY:clean
clean:
	rm -rf $(OBJS) $(OBJS:.o=.d) $(TEST_OBJS) $(TEST_OBJS:.o=.d) $(TARGET) $(ALL_TEST) $(PBOBJS:.pb.o=.pb.h) $(PBOBJS:.pb.o=.pb.cc) main.o main.d


