
#ifndef GROUPCONTROLLERINTERFACE_H
#define GROUPCONTROLLERINTERFACE_H

#include "id_types.h"

class QString;

class IGroupController
{
public:
    virtual void renameGroup(group_id id, const QString& sNewName) = 0;
    virtual void createNewTask(group_id groupId) = 0;
    virtual void onTaskMoved(task_id id, group_id groupId, int iPos) = 0;
    virtual void setAutoSortEnabled(group_id) = 0;
    virtual void setAutoSortDisabled(group_id) = 0;

protected:
    IGroupController() = default;
    virtual ~IGroupController() = default;
};

#endif // GROUPCONTROLLERINTERFACE_H
