# Copyright (C) 2008 Maryland Robotics Club
# Copyright (C) 2008 Joseph Lisee <jlisee@umd.edu>
# All rights reserved.
#
# Author: Joseph Lisee <jlisee@umd.edu>
# File:  packages/python/ram/ai/subsystem.py

# Project Imports
import ext.core as core
import ram.ai as ai
import ram.ai.task
from ram.logloader import resolve

class AI(core.Subsystem):
    """
    The subsystem which forms the core of the AI.  It allows for data storage
    and transfer between states, as well the management of the task system.
    """
    def __init__(self, cfg = None, deps = None):
        if deps is None:
            deps = []
        if cfg is None:
            cfg = {}
            
        core.Subsystem.__init__(self, cfg.get('name', 'Ai'),
                                deps)
        self._connections = []
        
        # Gather all the state machines
        stateMachines = []
        for d in deps:
            if isinstance(d, ai.state.Machine):
                stateMachines.append(d)
        
        # Find the main one
        self._stateMachine = None
        for m in stateMachines:
            if m.getName() == cfg.get('AIMachineName', 'StateMachine'):
                self._stateMachine = m
                break
        #assert (not (self._stateMachine is None)), "Could not find aistate machine"
                
        # Store inter state data
        self._data = {}
        
        
        # Build list of next states
        self._nextTaskMap = {}
        taskOrder = cfg.get('taskOrder', None)
        if taskOrder is None:
            taskOrder = []
        for i, taskName  in enumerate(taskOrder):
            # Determine which task is really next
            nextTask = 'ram.ai.task.End'
            if i != (len(taskOrder) - 1):
                nextTask = taskOrder[i + 1]
            
            # Resolve dotted task names into classes
            taskClass = resolve(taskName)
            nextClass = resolve(nextTask)
            
            # Store the results
            self._nextTaskMap[taskClass] = nextClass
            
        # Build list of failure tasks
        self._failureTaskMap = {}
        failureTasks = cfg.get('failureTasks', None)
        if failureTasks is None:
            failureTasks = {}
        for taskName, failTaskName in failureTasks.iteritems():
            # Resolve dotted task names into classes
            taskClass = resolve(taskName)
            failureTaskClass = resolve(failTaskName)
            
            # Store results
            self._failureTaskMap[taskClass] = failureTaskClass
    
    # IUpdatable methods
    def update(self, timeStep):
        pass

    def backgrounded(self):
        return True
        
    def unbackground(self, join = False):
        core.Subsystem.unbackground(self, join)
        for conn in self._connections:
            conn.disconnect()
    def backgrounded(self):
        return True

    # Properties
    @property
    def mainStateMachine(self):
        return self._stateMachine

    @property
    def data(self):
        return self._data

    # Other methods
    def addConnection(self, conn):
        self._connections.append(conn)

    def getNextTask(self, task):
        """
        Returns the task which will be transitioned to when the given task 
        transitions to the next task.
        """
        return self._nextTaskMap[task]
    
    def getFailureState(self, task):
        """
        Returns the state which will be transitioned to when the given task
        fails in an unrecoverable way.  Recoverable failure are handled 
        internally by that task. 
        """
        return self._failureTaskMap.get(task, None)
    
    
core.registerSubsystem('AI', AI)
