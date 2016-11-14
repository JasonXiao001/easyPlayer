////
//// Created by Administrator on 2016/10/28.
////
//
//#ifndef EASYPLAYER_CLOCK_H
//#define EASYPLAYER_CLOCK_H
//
//#include <time.h>
//#include <mutex>
//
//class Clock {
//public:
//    void start() {
//        if (playing) return;
//        struct timespec now;
//        clock_gettime(CLOCK_MONOTONIC, &now);
//        startTime = (long long) now.tv_sec*NANO_SECOND + now.tv_nsec;;
//        playing = true;
//    }
//    void stop() {
//        if (!playing) return;
//        struct timespec now;
//        clock_gettime(CLOCK_MONOTONIC, &now);
//        long long endTime = (long long) now.tv_sec*NANO_SECOND + now.tv_nsec;
//        timePassed += endTime - startTime;
//        playing = false;
//    }
//    double getTimePassed() {
//        if (!playing) {
//            return (double)timePassed/NANO_SECOND;
//        }else {
//            struct timespec now;
//            clock_gettime(CLOCK_MONOTONIC, &now);
//            long long endTime = (long long) now.tv_sec*NANO_SECOND + now.tv_nsec;
//            long long currPassed = endTime - startTime;
//            return (double)(timePassed + currPassed)/NANO_SECOND;
//        }
//    }
//private:
//    long long timePassed = 0;
//    long long startTime = 0;
//    bool playing = false;
//    const long long NANO_SECOND = 1000000000LL;
//};
//
//
//#endif //EASYPLAYER_CLOCK_H
