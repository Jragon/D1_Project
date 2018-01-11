#include "ring_buff.h"

// holds head and tail, compaired to rbAttributes
struct ring_buffer {
  uint8_t numberOfElements;
  uint8_t *buffer;

  // these hold the index of the head and tail pos
  // means maximum index of the buff is 0xFF -> 256 elements long
  volatile uint8_t head;
  volatile uint8_t tail;
};

static struct ring_buffer _rb[MAX_BUFFS];

static int _rb_full(struct ring_buffer *rb);
static int _rb_empty(struct ring_buffer *rb);

int ring_buffer_init(rbIndex_t *rbi, uint8_t numberOfElements,
                     uint8_t *buffer) {
  // number of initialized ringbuffs
  static uint8_t rbCount = 0;

  if ((rbCount > MAX_BUFFS)) return 0;

  // check number of elements is a power of two
  // this allows us to avoid the modulus operator
  // http://mziccard.me/2015/05/08/modulo-and-division-vs-bitwise-operations/
  if (((numberOfElements - 1) & numberOfElements) == 0) {
    _rb[rbCount].head = 0;
    _rb[rbCount].tail = 0;
    _rb[rbCount].buffer = buffer;
    _rb[rbCount].numberOfElements = numberOfElements;

    *rbi = rbCount;
    rbCount++;
  }

  return 1;
}

int ring_buffer_put(rbIndex_t rbi, const uint8_t *data) {
  // if buff isn't full
  // return error and discard data if full
  if ((rbi < MAX_BUFFS) && (_rb_full(&_rb[rbi]) == 0)) {
    // insert at the head
    // see above link on modulus
    uint8_t insertOffset = (_rb[rbi].head & (_rb[rbi].numberOfElements - 1));
    // copy the data to buffer
    _rb[rbi].buffer[insertOffset] = *data;

    _rb[rbi].head++;
  } else {
    return 0;
  }

  return 1;
}

int ring_buffer_get(rbIndex_t rbi, uint8_t *data) {
  if ((rbi < MAX_BUFFS) && (_rb_empty(&_rb[rbi]) == 0)) {
    // see above link on modulus
    uint8_t readOffset = (_rb[rbi].tail & (_rb[rbi].numberOfElements - 1));
    *data = _rb[rbi].buffer[readOffset];

    // move tail up
    _rb[rbi].tail++;
  } else {
    return 0;
  }

  return 1;
}

static int _rb_full(struct ring_buffer *rb) {
  return ((rb->head - rb->tail) == rb->numberOfElements) ? 1 : 0;
}

static int _rb_empty(struct ring_buffer *rb) {
  // 0, unsigned to take advantage of the fact that the subratiction will wrap.
  // could use the mod op here but it's significantly slower.
  return ((rb->head - rb->tail) == 0U) ? 1 : 0;
}