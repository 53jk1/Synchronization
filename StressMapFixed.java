// > javac StressMapFixed.java
// > java StressMapFixed

import java.util.*;

public class StressMapFixed {
    public static TreeMap<String, Integer> dict;
    public static final String[] KEYS = {
        "k1", "k2", "k3", "k4", "k5", "k6"
    };

    public static void main(String args[]) {
        dict = new TreeMap<String, Integer>();

        // Taking advantage of the ability to create a class that inherits "in place", I define a thread class Purger and Adder.
        Thread purger = new Thread() {
            public void run() {
                int poor_mans_random = 647;
                for (;;) {
                    synchronized(dict) { // Mutex capture.
                        dict.remove(KEYS[poor_mans_random % 6]);
                    } // Mutex release.
                    poor_mans_random = (poor_mans_random * 4967 + 1777) % 1283;
                }
            }
        };

        Thread adder = new Thread() {
            public void run() {
                int poor_mans_random = 499;
                for(;;) {
                    synchronized(dict) { // Mutex capture.
                        dict.put(KEYS[poor_mans_random % 6], poor_mans_random);
                    } // Mutex release.
                    poor_mans_random = (poor_mans_random * 4967 + 1777) % 1283;
                }
            }
        };

        // Run both threads.
        purger.start();
        adder.start();
        int result = 0;
        for (;;) {
            synchronized(dict) { // Mutex capture.
                Integer value = dict.get("k1");
                if (value != null) {
                    result += value;
                }
            } // Mutex release.
        }
    }
}







                        