#ifndef IPEASAN_H
#define IPEASAN_H

#define LOCAL_ID_THRESHOLD          2
#define MAP_SIZE_POW2               16
#define MAP_SIZE                    (1 << MAP_SIZE_POW2)
#define AFL_R(x)                    (random() % (x))

#define WHITE_LIST_OR_BLACK_LIST    0 // 1 for white and 0 for black

#ifndef ENABLE_LOOP_OPTIMIZATION
  #define ENABLE_LOOP_OPTIMIZATION    1
#endif

#ifndef ENABLE_AFL
  #define ENABLE_AFL                  1
#endif

#endif