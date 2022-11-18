#ifndef JFSNOTIFY_C_COMMON_H
#define JFSNOTIFY_C_COMMON_H

#define INIT "<init>"
#define QUEUE_CLASS "java/util/concurrent/BlockingQueue"
#define EVENT_CLASS "com/fenquen/jfsnotify/Event"
#define UNKNOWN "unknown"

#define THROW_PLAIN(e, ...) THROW(e,"%s",##__VA_ARGS__)

#define THROW(e, format, ...) do { \
    char buffera[256]; \
    sprintf(buffera, format, ##__VA_ARGS__); \
    throwException(e, buffera); \
} while (0)

extern void throwException(JNIEnv *env, const char *message);

extern char *pid2Path(int pid, char *string, size_t stringLen);

extern char *fd2Path(int fd, char *string, size_t strLen);

#define FAIL_CODE (-1)

#endif //JFSNOTIFY_C_COMMON_H
