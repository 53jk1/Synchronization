#!/usr/bin/python
# -*- coding: utf-8 -*-

from threading import Condition, Lock, Thread
from time import sleep

class WorkContainer:
    def __init__(self):
        self.work = []
        self.mutex = Lock()

        # Creation of a conditional variable (with internal messages enabled, which has educational value and is offered by the class).
        self.cond = Condition(lock = self.mutex, verbose = True)

        self.work_complete = False

        class ProducerThread(Thread):
            def __init__(self, work):
                super(ProducerThread, self).__init__()
                self.work = work

            def run(self):

                # Generate new data.
                for i in range(16):
                    sleep(0.15) # Data preparation takes a long time.
                    new_data = i

                    # Add a new data packet to the list and start one thread.
                    self.work.mutex.acquire() # Same with self.cond.acquire().
                    self.work.work.append(new_data)
                    self.work.cond.notify(1)
                    self.work.mutex.release()

                #Koniec pracy
                with self.work.mutex: # RAII style mutex seizure.
                    self.work.work_complete = True
                    self.work.cond.notifyAll()

                class ConsumerThread(Thread):
                    def __init__(self, work):
                        super(ConsumerThread, self).__init__()
                        self.work = work

                    def run(self):
                        self.work.mutex.acquire()
                        while True:

                            # Check if there is any work to be done or if we should terminate the activity.
                            data = None
                            if self.work.work:
                                data = self.work.work.pop(0)
                            elif self.work.work_complete:
                                self.work.mutex.release()
                                print("Work complete!")
                                break

                            # In the absence of data go to waiting for data.
                            if data is None:
                                self.work.cond.wait()
                                continue

                            # Data processing - no need for mutex. If in the meantime new data will be left
                            # added, this thread will not receive a notification
                            # (because it is not in a state of waiting), so it is important to in the next iteration
                            # loops again to check if data is available.
                            self.work.mutex.release()

                            print("Working on data: %i" %data)
                            sleep(0.1) # Hard work. A lot of hard work.
                            print("Done working on: %i" %data)

                            # Take the mutex again.
                            self.work.mutex.acquire()


        def main():
            # Start threads.
            work = WorkContainer()
            pro_th = ProducerThread(work)
            con_th = ConsumerThread(work)
            pro_th.start()
            con_th.start()
            print("Threads started")

            # Wait for them to finish.
            pro_th.join()
            con_th.join()

        if __name__ == "__main__":
            main()

