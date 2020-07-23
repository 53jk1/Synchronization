# $ python syncqueue.py
#!/usr/bin/python
# -*- coding: utf-8 -*-
from threading import Thread
import Queue
import sys

done = False

class MyProducer(Thread):
    def __init__(self, q):
        super(MyProducer, self).__init__()
        self.q = q
    
    def run(self):
        global done

        # Put 10 jobs in the queue.
        for i in xrange(10):
            self.q.put(i)

        # Wait while they are all selected and processed.
        self.q.join()
        done = True

        # In the following, I use stdout.write instead of print
        # because print is not an atomic operation - the newline
        # is printed in a separate operation, so in a
        # multithreaded environment a race condition occurs
        # where several messages may end up on the same line,
        # reducing readability. If you use sys.stdout.write for
        # a short message, there is a high probability that the
        # entire message will be written to the standard output
        # buffer in one operation.
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
            except Queue.Empty:
                pass # No work in the queue.

        sys.stdout.write("[%i] Exiting.\n" % self.tid)

def main():
    # Start the threads.
    q = Queue.Queue()
    threads = [ MyProducer(q) ]
    threads.extend([ MyConsumer(q, i) for i in xrange(8) ])
    for th in threads: th.start()

    # Wait for the threads to exit.
    for th in threads: th.join()

if __name__ == "__main__":
    main()