#include <poll.h>
#include <sys/fanotify.h>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <climits>
#include <cerrno>
#include <jni.h>
#include <jni_md.h>
#include <sys/socket.h>

#include "com_fenquen_jfsnotify_FsNotify.h"
#include "common.h"

static int fd = -1;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     com_fenquen_jfsnotify_FsNotifier
 * Method:    watch0
 * Signature: (Ljava/util/concurrent/LinkedBlockingQueue;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_fenquen_jfsnotify_FsNotify_watch0(JNIEnv *env,
                                                                  jobject thiz,
                                                                  jobject eventQueue,
                                                                  jstring targetPath) {
    // queue
    jclass queueClass = env->FindClass(QUEUE_CLASS);
    if (queueClass == nullptr) {
        THROW_PLAIN(env, "queueClass == nullptr");
    }
    jmethodID addMethod = env->GetMethodID(queueClass, "add", "(Ljava/lang/Object;)Z");
    if (addMethod == nullptr) {
        THROW_PLAIN(env, "addMethod == nullptr");
    }

    // event
    jclass eventClass = env->FindClass(EVENT_CLASS);
    if (eventClass == nullptr) {
        THROW_PLAIN(env, "eventClass == nullptr");
    }
    jmethodID envConstructMethod = env->GetMethodID(eventClass, INIT, "()V");
    if (envConstructMethod == nullptr) {
        THROW_PLAIN(env, "envConstructMethod == nullptr");
    }

    jfieldID typeFiled = env->GetFieldID(eventClass, "type", "I");
    jfieldID fdField = env->GetFieldID(eventClass, "fd", "I");
    jfieldID fdPathField = env->GetFieldID(eventClass, "fdPath", "Ljava/lang/String;");
    jfieldID pidField = env->GetFieldID(eventClass, "pid", "I");
    jfieldID pidPathField = env->GetFieldID(eventClass, "pidPath", "Ljava/lang/String;");

    int fanotifyFd = fanotify_init(FAN_CLASS_NOTIF, O_RDWR);
    if (fanotifyFd == FAIL) {
        THROW(env, "%s%s", "fanotify_init fail,reason:", strerror(errno));
    }
    int ret = fanotify_mark(fanotifyFd,
                            FAN_MARK_ADD,
                            FAN_CLOSE | FAN_OPEN | FAN_EVENT_ON_CHILD,
                            0,
                            env->GetStringUTFChars(targetPath, nullptr));
    if (ret == -1) {
        THROW(env, "%s%s", "fanotify_mark fail,reason:", strerror(errno));
    }

    int socketPairs[2];
    ret = socketpair(AF_UNIX, SOCK_STREAM, 0, socketPairs);
    if (ret == -1) {
        THROW(env, "%s%s", "socketpair fail ,reason:", strerror(errno));
    }
    fd = socketPairs[0];

    struct pollfd fds[2];
    fds[0].fd = fanotifyFd;
    fds[0].events = POLLIN;
    fds[1].fd = socketPairs[1];
    fds[1].events = POLLIN;

    char buffer[8192];
    char fdPath[PATH_MAX];
    char pidPath[PATH_MAX];

    for (;;) {
        if (poll(fds, 2, -1) < 0) {
            THROW(env, "%s%s", "poll() return -1,reason:", strerror(errno));
            break;
        }

        if (fds[0].revents & POLLIN) {
            ssize_t length;
            if ((length = read(fds[0].fd, buffer, 8192)) > 0) {
                auto *eventMetadata = (struct fanotify_event_metadata *) buffer;

                while (FAN_EVENT_OK (eventMetadata, length)) {
                    const char *pidPath0 = pid2Path(eventMetadata->pid, pidPath, PATH_MAX) ? pidPath : UNKNOWN;
                    const char *fdPath0 = fd2Path(eventMetadata->fd, fdPath, PATH_MAX) ? fdPath : UNKNOWN;

                    if (eventMetadata->mask & FAN_OPEN) {
                        printf("FAN_OPEN\n");
                    }
                    if (eventMetadata->mask & FAN_ACCESS) {
                        printf("FAN_ACCESS\n");
                    }
                    if (eventMetadata->mask & FAN_MODIFY) {
                        printf("FAN_MODIFY\n");
                    }
                    if (eventMetadata->mask & FAN_CLOSE_WRITE) {
                        printf("FAN_CLOSE_WRITE\n");
                    }
                    if (eventMetadata->mask & FAN_CLOSE_NOWRITE) {
                        printf("FAN_CLOSE_NOWRITE\n");
                    }
                    fflush(stdout);

                    // Event event = new Event();
                    jobject event = env->NewObject(eventClass, envConstructMethod);

                    env->SetIntField(event, typeFiled, jint(eventMetadata->mask));
                    env->SetIntField(event, fdField, eventMetadata->fd);
                    env->SetObjectField(event, fdPathField, env->NewStringUTF(fdPath0));
                    env->SetIntField(event, pidField, jint(eventMetadata->pid));
                    env->SetObjectField(event, pidPathField, env->NewStringUTF(pidPath0));

                    // queue.add(event);
                    env->CallVoidMethod(eventQueue, addMethod, event);

                    if (eventMetadata->fd > 0) {
                        close(eventMetadata->fd);
                    }

                    eventMetadata = FAN_EVENT_NEXT (eventMetadata, length);
                }
            }
        }
    }

    close(fanotifyFd);
}
#ifdef __cplusplus
}
#endif
