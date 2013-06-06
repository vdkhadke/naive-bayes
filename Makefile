CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS =		NaiveBayes.o 

LIBS =

TARGET =	NaiveBayes 

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
