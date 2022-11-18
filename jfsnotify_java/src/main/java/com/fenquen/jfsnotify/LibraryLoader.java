package com.fenquen.jfsnotify;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;

public class LibraryLoader {
    private static final String LIBRARY_FILE_NAME = "libjfsnotify.so";

    public static File copyLibraryToTemp() throws Exception {
        try (InputStream inputStream = LibraryLoader.class.getClassLoader().getResourceAsStream(LIBRARY_FILE_NAME)) {
            if (null == inputStream) {
                throw new FileNotFoundException(LIBRARY_FILE_NAME);
            }

            File temp = File.createTempFile("libjfsnotify", ".so");
            temp.deleteOnExit();

            Files.copy(inputStream, temp.toPath(), StandardCopyOption.REPLACE_EXISTING);

            return temp;
        }
    }
}
