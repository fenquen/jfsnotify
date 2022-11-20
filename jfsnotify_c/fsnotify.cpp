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

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     com_fenquen_jfsnotify_FsNotifier
 * Method:    watch0
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_fenquen_jfsnotify_FsNotify_watch0(JNIEnv *env, jobject thiz) {
    static jclass fsnotifyClass = env->GetObjectClass(thiz);

    // eventQueue
    static jfieldID eventQueueField = env->GetFieldID(fsnotifyClass, "eventQueue",
                                                      "Ljava/util/concurrent/BlockingQueue;");
    if (eventQueueField == nullptr) {
        THROW_PLAIN(env, "eventQueueField == nullptr");
        return;
    }
    jobject eventQueue = env->GetObjectField(thiz, eventQueueField);

    // targetPath
    static jfieldID targetPathField = env->GetFieldID(fsnotifyClass, "targetPath", "Ljava/lang/String;");
    if (targetPathField == nullptr) {
        THROW_PLAIN(env, "targetPathField == nullptr");
        return;
    }
    jobject targetPath = env->GetObjectField(thiz, targetPathField);

    // queue
    static jclass queueClass = env->FindClass(QUEUE_CLASS);
    if (queueClass == nullptr) {
        THROW_PLAIN(env, "queueClass == nullptr");
        return;
    }
    static jmethodID addMethod = env->GetMethodID(queueClass, "add", "(Ljava/lang/Object;)Z");
    if (addMethod == nullptr) {
        THROW_PLAIN(env, "addMethod == nullptr");
        return;
    }

    // event
    static jclass eventClass = env->FindClass(EVENT_CLASS);
    if (eventClass == nullptr) {
        THROW_PLAIN(env, "eventClass == nullptr");
        return;
    }
    static jmethodID envConstructMethod = env->GetMethodID(eventClass, INIT, "()V");
    if (envConstructMethod == nullptr) {
        THROW_PLAIN(env, "envConstructMethod == nullptr");
        return;
    }

    static jfieldID typeField = env->GetFieldID(eventClass, "type", "I");
    static jfieldID fdField = env->GetFieldID(eventClass, "fd", "I");
    static jfieldID fdPathField = env->GetFieldID(eventClass, "fdPath", "Ljava/lang/String;");
    static jfieldID pidField = env->GetFieldID(eventClass, "pid", "I");
    static jfieldID pidPathField = env->GetFieldID(eventClass, "pidPath", "Ljava/lang/String;");

    int fanotifyFd = fanotify_init(FAN_CLASS_NOTIF, O_RDWR);
    if (fanotifyFd == FAIL_CODE) {
        THROW(env, "%s%s", "fanotify_init fail,reason:", strerror(errno));
        return;
    }
    const char *cstr = env->GetStringUTFChars((jstring) targetPath, nullptr);
    // the file must exist
    int ret = fanotify_mark(fanotifyFd,
                            FAN_MARK_ADD,
                            FAN_CLOSE | FAN_OPEN | FAN_EVENT_ON_CHILD,
                            0,
                            cstr);
    env->ReleaseStringUTFChars((jstring) targetPath, cstr);
    if (ret == -1) {
        THROW(env, "%s%s", "fanotify_mark fail,reason:", strerror(errno));
        return;
    }

    int socketPairs[2];
    ret = socketpair(AF_UNIX, SOCK_STREAM, 0, socketPairs);
    if (ret == FAIL_CODE) {
        THROW(env, "%s%s", "socketpair fail ,reason:", strerror(errno));
        return;
    }
    static jfieldID closeFdField = env->GetFieldID(fsnotifyClass, "closeFd", "I");
    env->SetIntField(thiz, closeFdField, socketPairs[0]);

    struct pollfd fds[2];
    fds[0].fd = fanotifyFd;
    fds[0].events = POLLIN;
    fds[1].fd = socketPairs[1];
    fds[1].events = POLLIN;

    char buffer[8192];
    char fdPath[PATH_MAX];
    char pidPath[PATH_MAX];

    bool needstop = false;
    for (;;) {
        if (needstop) {
            break;
        }

        if (poll(fds, 2, -1) == FAIL_CODE) {
            THROW(env, "%s%s", "poll() return -1,reason:", strerror(errno));
            break;
        }

        // get the close signal
        if (fds[1].revents & POLLIN) {
            needstop = true;
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

                    env->SetIntField(event, typeField, jint(eventMetadata->mask));
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

/*
 * Class:     com_fenquen_jfsnotify_FsNotify
 * Method:    stopWatch0
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_fenquen_jfsnotify_FsNotify_stopWatch0(JNIEnv *env, jobject thiz) {
    static jclass fsnotifyClass = env->GetObjectClass(thiz);
    static jfieldID closeFdField = env->GetFieldID(fsnotifyClass, "closeFd", "I");

    jint closeFd = env->GetIntField(thiz, closeFdField);
    if (closeFd == INVALID_FD) {
        return;
    }

    if (write(closeFd, "close", 6) == FAIL_CODE) {
        THROW(env, "%s%s", "write fail,reason:", strerror(errno));
        return;
    }
}

#ifdef __cplusplus
}
#endif
