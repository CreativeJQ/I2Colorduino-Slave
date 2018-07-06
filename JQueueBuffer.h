#ifndef _JQUEUE_BUFFER_
#define _JQUEUE_BUFFER_

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#include "pins_arduino.h"
#endif

/*
  Dispatcher
*/
#define JQUEUE_BUFFER_SINGLE 32
#define JQUEUE_BUFFER_MAX_QUEUE 20

class JQueueBuffer {
  private:
    static unsigned char bufferQueue[JQUEUE_BUFFER_MAX_QUEUE][JQUEUE_BUFFER_SINGLE];
    static unsigned char currentUsage;
    static unsigned char nextInputIndex;
    static unsigned char nextOutputIndex;

  public:
    JQueueBuffer();
    static unsigned char getNextInputIndex();
    static unsigned char getNextOutputIndex();
    static unsigned char peekNextInputIndex();
    static unsigned char peekNextOutputIndex();
    static unsigned char getLength();
    static unsigned char getNextIndex(unsigned char &, unsigned char _mode = 0);
    static void push(unsigned char *);
    static void pop(unsigned char *);
};

extern JQueueBuffer JQueue;

#endif // _JQUEUE_BUFFER_
