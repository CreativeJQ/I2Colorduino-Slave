#include "JQueueBuffer.h" // Engage JQueue Buffer Library

unsigned char JQueueBuffer::bufferQueue[JQUEUE_BUFFER_MAX_QUEUE][JQUEUE_BUFFER_SINGLE];
unsigned char JQueueBuffer::currentUsage;
unsigned char JQueueBuffer::nextInputIndex;
unsigned char JQueueBuffer::nextOutputIndex;

JQueueBuffer::JQueueBuffer() {
  currentUsage = 0;
  nextInputIndex = 0;
  nextOutputIndex = 0;
}

unsigned char JQueueBuffer::getLength() {
  return currentUsage;
}

unsigned char JQueueBuffer::getNextInputIndex() {
  return JQueue.getNextIndex(nextInputIndex, 1);
}

unsigned char JQueueBuffer::getNextOutputIndex() {
  return JQueue.getNextIndex(nextOutputIndex, 1);
}

unsigned char JQueueBuffer::peekNextInputIndex() {
  return JQueue.getNextIndex(nextInputIndex);
}

unsigned char JQueueBuffer::peekNextOutputIndex() {
  return JQueue.getNextIndex(nextOutputIndex);
}

unsigned char JQueueBuffer::getNextIndex(unsigned char & i, unsigned char _mode = 0) {
  unsigned char tmpIndex = i;
  if (tmpIndex < JQUEUE_BUFFER_MAX_QUEUE - 1) {
    if (_mode) i = ++tmpIndex;
    return tmpIndex;
  }
  else {
    if (_mode) i = 0;
    return JQUEUE_BUFFER_MAX_QUEUE - 1;
  }
}

void JQueueBuffer::push(unsigned char * input) {
  unsigned char curInputIndex = getNextInputIndex();
  unsigned char curUsage = getLength();
  if (currentUsage < JQUEUE_BUFFER_MAX_QUEUE) {
    currentUsage = curUsage + 1;
    for (unsigned char i = 0; i < JQUEUE_BUFFER_SINGLE; i++)
      bufferQueue[curInputIndex][i] = input[i];
  }
}

void JQueueBuffer::pop(unsigned char * output) {
  unsigned char curOutputIndex = getNextOutputIndex();
  unsigned char curUsage = getLength();
  if (currentUsage > 0) {
    currentUsage = curUsage - 1;
    for (unsigned char i = 0; i < JQUEUE_BUFFER_SINGLE; i++)
      output[i] = bufferQueue[curOutputIndex][i];
  }
}

// global instance
JQueueBuffer JQueue;
