#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <jni.h>

void throwException(JNIEnv *env, const char *message) {
    jclass exceptionClass = env->FindClass("java/lang/Exception");
    if (exceptionClass) {
        env->ThrowNew(exceptionClass, (char *) message);
    }

    env->DeleteLocalRef(exceptionClass);
}


char *pid2Path(int pid, char *string, size_t stringLen) {
    if (pid < 0) {
        return nullptr;
    }

    sprintf(string, "/proc/%d/cmdline", pid);

    int fd;
    if ((fd = open(string, O_RDONLY)) < 0) {
        return nullptr;
    }

    /* Read file contents into string */
    ssize_t len;
    if ((len = read(fd, string, stringLen - 1)) == -1) {
        close(fd);
        return nullptr;
    }

    close(fd);

    string[len] = '\0';

    char *aux = strstr(string, "^@");
    if (aux) {
        *aux = '\0';
    }

    return string;
}

char *fd2Path(int fd, char *string, size_t strLen) {
    if (fd <= 0) {
        return nullptr;
    }

    sprintf(string, "/proc/self/fd/%d", fd);

    ssize_t len;
    if ((len = readlink(string, string, strLen - 1)) < 0) {
        return nullptr;
    }

    string[len] = '\0';
    return string;
}
