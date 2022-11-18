package com.fenquen.jfsnotify;

/**
 * see {@literal <}linux/types.h{@literal >}
 */
public class Event {
    /**
     * File was accessed
     */
    public static final int FAN_ACCESS = 0x00000001;

    /**
     * File was modified
     */
    public static final int FAN_MODIFY = 0x00000002;

    /**
     * Metadata changed
     */
    public static final int FAN_ATTRIB = 0x00000004;

    /**
     * Writtable file closed
     */
    public static final int FAN_CLOSE_WRITE = 0x00000008;

    /**
     * Unwrittable file closed <br>
     * when you cat a.txt then you will receive FAN_OPEN , FAN_CLOSE_NOWRITE
     */
    public static final int FAN_CLOSE_NOWRITE = 0x00000010;

    /**
     * File was opened
     */
    public static final int FAN_OPEN = 0x00000020;

    /**
     * File was moved from X
     */
    public static final int FAN_MOVED_FROM = 0x00000040;

    /**
     * File was moved to Y
     */
    public static final int FAN_MOVED_TO = 0x00000080;

    /**
     * Subfile was created
     */
    public static final int FAN_CREATE = 0x00000100;

    /**
     * Subfile was deleted
     */
    public static final int FAN_DELETE = 0x00000200;

    /**
     * Self was deleted
     */
    public static final int FAN_DELETE_SELF = 0x00000400;

    /**
     * Self was moved
     */
    public static final int FAN_MOVE_SELF = 0x00000800;

    /**
     * File was opened for exec
     */
    public static final int FAN_OPEN_EXEC = 0x00001000;

    /**
     * Event queued overflowed
     */
    public static final int FAN_Q_OVERFLOW = 0x00004000;

    /**
     * File open in perm check
     */
    public static final int FAN_OPEN_PERM = 0x00010000;

    /**
     * File accessed in perm check
     */
    public static final int FAN_ACCESS_PERM = 0x00020000;

    /**
     * File open/exec in perm check
     */
    public static final int FAN_OPEN_EXEC_PERM = 0x00040000;

    public int type;

    public int fd;
    public String fdPath;

    public int pid;
    public String pidPath;

    public Event() {

    }

    // "(ILjava/lang/String;)V"
    public Event(int type, String fdPath) {
        this.type = type;
        this.fdPath = fdPath;
    }
}
