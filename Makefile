CC = g++
CFLAGS  = -Wall  
LIBS = -lsndfile
TARGET = idk

all: $(TARGET)
 
$(TARGET): $(TARGET).cpp
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cpp $(LIBS)
 
clean:
	$(RM) $(TARGET)
