package com.fenquen.jfsnotify;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

public class FsNotify implements AutoCloseable {
    static {
        System.load("/home/a/github/jfsnotify/jfsnotify_c/cmake-build-debug/libjfsnotify.so");
    }

    private BlockingQueue<Event> eventQueue;
    private String targetPath;

    public FsNotify(BlockingQueue<Event> eventQueue, String targetPath) {
        this.eventQueue = eventQueue;
        this.targetPath = targetPath;
    }

    public FsNotify(String targetPath) {
        this.targetPath = targetPath;
        this.eventQueue = new LinkedBlockingQueue<>();
    }

    public void watch() throws Exception {
        watch0(eventQueue, targetPath);
    }

    private native void watch0(BlockingQueue<Event> eventQueue, String targetPath) throws Exception;

    @Override
    public void close() {

    }

    public BlockingQueue<Event> getQueue() {
        return eventQueue;
    }

    public static void main(String[] args) throws Exception {
        System.out.println(System.getProperty("java.library.path"));
        FsNotify fsNotify = new FsNotify("/home/a/fsnotify");

        new Thread(() -> {
            try {
                fsNotify.watch();
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }).start();


        while (true) {
            Event event = fsNotify.getQueue().take();
            System.out.println(event.type);
            System.out.println(event.fd);
            System.out.println(event.fdPath);
            System.out.println(event.pid);
            System.out.println(event.pidPath);

            System.out.println();

        }
    }
}