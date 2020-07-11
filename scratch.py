#!/usr/bin/python
# -*- coding: utf-8 -*-
import _thread
import queue
import sys
import threading

done = False

class MyProducer(Thread):
    def __init__(self,q):
        super(MyProducer, self).__init__()
        self.q = q

    def run(self):
        global done

        #Put 10 tasks in the queue
        for i in range(10):
            self.q.put(i)

        #Wait for all of them to be selected and processed.
        self.q.join()
        done = True

        # Below, I use stdout.write instead of print, because print is not an atomic operation - the newline character
        # is printed in a separate operation, which causes a race situation in a multi-threaded environment, where
        # several messages can go to the same line, which reduces readability. If you use sys.stdout.write for a short
        # message, it is likely that the entire message will be rewritten to the standard output buffer in one
        # operation.

        sys.stdout.write("[P] Done.\n")

    class MyConsumer(Thread):
        def __init__(self, q, tid):
            super(MyConsumer, self).__init__()
            self.q = q
            self.tid = tid

        def run(self):
            global done
            while not done:
                try:
                    work = self.q.get(timeout = 0.01)
                    sys.stdout.write("[%i] %u\n" % (self.tid, work))
                    self.q.task_done()
                except queue.Empty:
                    pass #No job in queue.

            sys.stdout.write("[%i] Exiting.\n" % self.tid)

    def main():
        #Start threads
        q = queue.queue()
        threads = [ MyProducer(q) ]
        threads.extend([ MyConsumer(q, i) for i in range(8)])
        for th in threads: th.start()

        #Wait for the threads to finish
        for th in threads: th.join()

    if __name__ == "__main__":
        main()