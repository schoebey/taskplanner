#ifndef MANAGER_H
#define MANAGER_H

class Task;
class Group;
class Manager
{
public:
  Manager();

  void addTask();
  void task();
  void taskIds();
  void moveTask();
  void removeTask();

  void addGroup();
  void group();
  void groupIds();
  void removeGroup();
};

#endif // MANAGER_H
