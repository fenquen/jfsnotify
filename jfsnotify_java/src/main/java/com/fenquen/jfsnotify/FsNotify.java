package com.fenquen.jfsnotify;

import java.io.File;
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

    public FsNotify(BlockingQueue<Event> eventQueue, String targetPath) {
        File file =  new File(targetPath);
        if (file.isDirectory()){
            
        }

        this.eventQueue = eventQueue;
        this.targetPath = targetPath;

    }

    public FsNotify(String targetPath) {
        this.targetPath = targetPath;
        this.eventQueue = new LinkedBlockingQueue<>();
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
