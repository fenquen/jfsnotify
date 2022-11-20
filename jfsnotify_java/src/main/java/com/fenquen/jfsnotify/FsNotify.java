package com.fenquen.jfsnotify;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

public class FsNotify {
    static {
        try {
            System.load(LibraryLoader.copyLibraryToTemp().getAbsolutePath());
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    private BlockingQueue<Event> eventQueue;
    private String targetPath;

    private int closeFd = -1;


    public FsNotify(String targetPath, BlockingQueue<Event> eventQueue) throws FileNotFoundException {
        this.targetPath = targetPath;
        this.eventQueue = eventQueue;

        init();
    }

    public FsNotify(String targetPath) throws FileNotFoundException {
        this.targetPath = targetPath;
        this.eventQueue = new LinkedBlockingQueue<>();

        init();
    }

    private void init() throws FileNotFoundException {
        File file = new File(targetPath);

        // targetPath should be existing ,the fanotify_mark will fail which says:No such file or directory
        if (!file.exists()) {
            throw new FileNotFoundException(targetPath);
        }
    }

    public void watch() throws Exception {
        watch0();
    }

    private native void watch0() throws Exception;

    public void stopWatch() throws Exception {
        stopWatch0();
    }

    private native void stopWatch0() throws Exception;

    public BlockingQueue<Event> getQueue() {
        return eventQueue;
    }

    public static void main(String[] args) throws Exception {
        //System.out.println(System.getProperty("java.library.path"));
        FsNotify fsNotify = new FsNotify("/home/a/fsnotify");

        new Thread(() -> {
            try {
                fsNotify.watch();
                System.out.println("watch end");
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }).start();


        //while (true) {
        Event event = fsNotify.getQueue().take();
        System.out.println(event.type);
        System.out.println(event.fd);
        System.out.println(event.fdPath);
        System.out.println(event.pid);
        System.out.println(event.pidPath);

        System.out.println();

        // }

        fsNotify.stopWatch();
    }
}
