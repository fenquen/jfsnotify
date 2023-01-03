# jfsnotify
provide filesystem event notification ablility (such as file creation) under java eco system with the support of jni

due to lack of ability,the project now only support linux fanotify which need root privilege


## development environment

ubuntu 20.04 with kernal version 5.15.0 x86_64

## how to use
```java
public static void main(String[] args) throws Exception {
        // this is a dorectory
        FsNotify fsNotify = new FsNotify("/home/a/fsnotify");

        // monitor the close and open event on the directory's children
        new Thread(() -> {
            try {
                fsNotify.watch(Event.FAN_CLOSE | Event.FAN_OPEN | Event.FAN_EVENT_ON_CHILD);
                System.out.println("watch end");
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }).start();


        Event event = fsNotify.getQueue().take();
        System.out.println(event.type);
        System.out.println(event.fd);
        System.out.println(event.fdPath);
        System.out.println(event.pid);
        System.out.println(event.pidPath);

        System.out.println();

        fsNotify.stopWatch();
    }
```

## problem
by now still don't know the best way to handle the raw fd in java