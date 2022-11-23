package com.fenquen.jfsnotify;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.atomic.AtomicReference;

public class FsNotify {
    static {
        try {
            System.load(LibraryLoader.copyLibraryToTemp().getAbsolutePath());
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    private enum State {
        INITIAL, RUNNING, STOPPING, STOPPED
    }

    private BlockingQueue<Event> eventQueue;
    private String targetPath;

    // socket pair used in naive
    private int closeFd = -1;

    private AtomicReference<State> state = new AtomicReference<>();

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

        state.set(State.INITIAL);
    }

    public void watch(int mask) throws Exception {
        if (State.INITIAL.ordinal() < state.get().ordinal()) {
            return;
        }

        if (state.compareAndSet(State.INITIAL, State.RUNNING)) {
            watch0(mask);
        }
    }

    public List<Event> stopWatch() throws Exception {
        if (state.compareAndSet(State.RUNNING, State.STOPPED)) {
            stopWatch0();
            List<Event> result = new ArrayList<>();
            eventQueue.drainTo(result);
            return result;
        }

        return Collections.emptyList();
    }

    private native void watch0(int mask) throws Exception;

    private native void stopWatch0() throws Exception;

    public BlockingQueue<Event> getQueue() {
        return eventQueue;
    }

    public static void main(String[] args) throws Exception {
        //System.out.println(System.getProperty("java.library.path"));
        FsNotify fsNotify = new FsNotify("/home/a/fsnotify");

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
}
